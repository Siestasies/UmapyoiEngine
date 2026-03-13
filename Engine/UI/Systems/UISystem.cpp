/*!
\file   UISystem.cpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Jedrek Lee Jing Wei (everything else)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\author Leong Wai Men (isActive, mouse input & window res bug fixing)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Implementation of the main UI system with three-pass architecture.

This file provides the concrete implementation for UI layout computation,
input handling, and draw list generation. It coordinates all UI components
through an explicit three-pass system: Layout, Input, and BuildDrawList.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "UISystem.h"
#include "../Helpers/Layout.h"
#include "../Helpers/Input.h"
#include "../Events/WindowEvents.h"
#include "../Events/AudioEvents.h"
#include "../Events/IMGUIEvents.h"
#include "../Events/LuaScriptingEvents.h"

#include "../ECS/Systems/LuaScriptingSystem.hpp"
#include "Debugging/Debugger.hpp"

#include "Systems/ResourcesTypes.hpp"
#include "HybridInputSystem.h"
#include "Components/Transform.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <map>
#include <unordered_set>

namespace Uma_UI
{
    /*!
     * \brief Initializes the UI system and subscribes to window events.
     */
    void UISystem::Init()
    {
        if (!pCoordinator) { std::cerr << "UISystem::Init - Warning: Coordinator not set!" << std::endl; }
        if (!pEventSystem) { std::cerr << "UISystem::Init - Warning: EventSystem not set!" << std::endl; }
        if (!pGraphics) { std::cerr << "UISystem::Init - Warning: Graphics not set!" << std::endl; }
        if (!pResourcesManager) { std::cerr << "UISystem::Init - Warning: ResourcesManager not set!" << std::endl; }

        mHitTestCache.clear();

        //mFeedbackSystem.Init(pCoordinator, pGraphics, pEventSystem, this);

        // Push serialized colours/values into Image components immediately
        if (pCoordinator)
        {
            auto& sliderArray = pCoordinator->GetComponentArray<Slider>();
            for (size_t i = 0; i < sliderArray.Size(); ++i)
                UpdateSliderVisual(sliderArray.GetEntity(i));

            auto& checkboxArray = pCoordinator->GetComponentArray<Checkbox>();
            for (size_t i = 0; i < checkboxArray.Size(); ++i)
                UpdateCheckboxVisual(checkboxArray.GetEntity(i));
        }
    }

    /*!
     * \brief Updates the UI system through explicit passes: Layout, Effects and Input.
     * \param dt Delta time in seconds.
     */
    void UISystem::Update(float dt)
    {
        (void)dt;

        if (!pCoordinator || !pEventSystem || !pGraphics || !pResourcesManager) return;

        //mFeedbackSystem.Update(dt);

        Vec2 screenSize = pGraphics->GetSceneViewport();
        if (screenSize != mScreenSize) mScreenSize = screenSize, MarkAllDirty();

        LayoutPass();
        EffectsPass(dt);
    }

    /*!
     * \brief Shuts down the UI system and clears all cached data.
     */
    void UISystem::Shutdown()
    {
        //mFeedbackSystem.Shutdown();

        mHitTestCache.clear();
        mMouseButtonDown = false;
        mMouseButtonDownLastFrame = false;
        mMouseConsumedThisFrame = false;
        pCoordinator = nullptr;
        pEventSystem = nullptr;
        pGraphics = nullptr;
        pResourcesManager = nullptr;
    }

    /*!
     * \brief First pass: Computes NDC rectangles for all UI elements based on layout settings.
     *
     * Uses Transform hierarchy to traverse UI tree, but computes UI-specific layout.
     */
    void UISystem::LayoutPass()
    {
        auto& canvasArray = pCoordinator->GetComponentArray<Canvas>();
        std::vector<std::pair<Uma_ECS::Entity, int>> canvases;

        // Gather all canvases
        for (size_t i = 0; i < canvasArray.Size(); ++i)
        {
            Uma_ECS::Entity entity = canvasArray.GetEntity(i);
            auto& canvas = canvasArray.GetComponentAt(i);
            canvases.push_back({ entity, canvas.sortingOrder });
        }

        // Sort canvases by sorting order
        std::sort(canvases.begin(), canvases.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });

        // Process each canvas and its hierarchy
        for (const auto& [canvasEntity, sortOrder] : canvases)
        {
            auto& canvas = pCoordinator->GetComponent<Canvas>(canvasEntity);
            canvas.scaleFactor = ComputeCanvasScale(canvas, mScreenSize.x, mScreenSize.y); // Need to fix this.

            //Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
            //    "LayoutPass | Screen: " + std::to_string((int)mScreenSize.x) + "x" + std::to_string((int)mScreenSize.y));

            //Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
            //    "LayoutPass | ScaleMode: " + std::to_string((int)canvas.scaleMode) +
            //    " | RefRes: " + std::to_string((int)canvas.referenceResolution.x) + "x" + std::to_string((int)canvas.referenceResolution.y) +
            //    " | MatchWidthOrHeight: " + std::to_string(canvas.matchWidthOrHeight) +
            //    " | CanvasScale: " + std::to_string(ComputeCanvasScale(canvas, mScreenSize.x, mScreenSize.y)));

            // Recursively compute layout for this canvas's children
            ComputeLayoutRecursive(canvasEntity, GetScreenRect(), canvas.scaleFactor);
        }
    }

    /*!
     * \brief Recursively computes layout for an entity and its children.
     * \param entity Current entity to process.
     * \param parentRect Parent's computed NDC rect.
     * \param canvasScale Canvas scale factor.
     */
    void UISystem::ComputeLayoutRecursive(Uma_ECS::Entity entity, const Rect& parentRect, float canvasScale)
    {
        std::unordered_set<Uma_ECS::Entity> visited;

        // Stack stores {entity, parentRect} so each node knows its parent's computed rect
        std::vector<std::pair<Uma_ECS::Entity, Rect>> stack;
        stack.push_back({ entity, parentRect });

        while (!stack.empty())
        {
            auto [current, curParentRect] = stack.back();
            stack.pop_back();

            if (visited.count(current))
            {
                std::cerr << "UISystem: Circular parent-child relationship detected for entity: "
                    << current << std::endl;
                continue;
            }
            visited.insert(current);

            // Compute this entity's rect if it has a RectTransform
            if (pCoordinator->GetComponentArray<RectTransform>().Has(current))
            {
                auto& rectTransform = pCoordinator->GetComponent<RectTransform>(current);

                if (rectTransform.isDirty || rectTransform.computedRect.width <= 0.0f)
                {
                    rectTransform.computedRect = ComputeRectInNDC(
                        rectTransform, curParentRect, canvasScale, mScreenSize.x, mScreenSize.y);
                    rectTransform.isDirty = false;

                    //Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
                    //    "Layout entity: " + std::to_string(current) +
                    //    " | rect center=(" + std::to_string(rectTransform.computedRect.x) + ", " + std::to_string(rectTransform.computedRect.y) + ")" +
                    //    " size=(" + std::to_string(rectTransform.computedRect.width) + ", " + std::to_string(rectTransform.computedRect.height) + ")" +
                    //    " | parentRect center=(" + std::to_string(curParentRect.x) + ", " + std::to_string(curParentRect.y) + ")" +
                    //    " size=(" + std::to_string(curParentRect.width) + ", " + std::to_string(curParentRect.height) + ")");

                }
            }

            // Get this entity's computed rect to pass to children
            Rect currentRect = curParentRect;
            if (pCoordinator->GetComponentArray<RectTransform>().Has(current))
            {
                currentRect = pCoordinator->GetComponent<RectTransform>(current).computedRect;
            }

            // Push children in reverse order to maintain original traversal order
            if (pCoordinator->GetComponentArray<Uma_ECS::Transform>().Has(current))
            {
                auto& transform = pCoordinator->GetComponent<Uma_ECS::Transform>(current);
                for (auto it = transform.children.rbegin(); it != transform.children.rend(); ++it)
                {
                    Uma_ECS::Entity child = *it;
                    if (pCoordinator->GetComponentArray<RectTransform>().Has(child))
                    {
                        auto& rectTransform = pCoordinator->GetComponent<RectTransform>(child);
                        rectTransform.isDirty = true;
                    }
                    stack.push_back({ child, currentRect });
                }
            }
        }
    }

    /*!
     * \brief Second pass: Handles mouse input, updates button states, and emits UI events.
     */
    void UISystem::InputPass()
    {
        mMouseConsumedThisFrame = false;
        mMousePositionScreen = Uma_Engine::HybridInputSystem::GetSceneMousePosition();

        mMousePositionNDC = Uma_UI::ScreenToNDC(
            mMousePositionScreen.x, mMousePositionScreen.y, mScreenSize.x, mScreenSize.y);

        bool wasMouseButtonDownLastFrame = mMouseButtonDownLastFrame;
        mMouseButtonDownLastFrame = mMouseButtonDown;

        if (pGraphics)
        {
            mMouseButtonDown = glfwGetMouseButton(pGraphics->GetWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        }
        else
        {
            mMouseButtonDown = false;
        }

        bool mouseJustPressed = mMouseButtonDown && !wasMouseButtonDownLastFrame;
        bool mouseJustReleased = !mMouseButtonDown && wasMouseButtonDownLastFrame;

        mHitTestCache.clear();
        auto sortedEntities = GetSortedUIEntities();

        for (Uma_ECS::Entity entity : sortedEntities)
        {
            if (!pCoordinator->IsActiveInHierarchy(entity))
                continue;

            if (!pCoordinator->GetComponentArray<RectTransform>().Has(entity))
                continue;

            auto& buttonArray = pCoordinator->GetComponentArray<Button>();
            auto& sliderArray = pCoordinator->GetComponentArray<Slider>();
            auto& checkboxArray = pCoordinator->GetComponentArray<Checkbox>();

            if (!(buttonArray.Has(entity) || sliderArray.Has(entity) || checkboxArray.Has(entity))) continue;

            auto& rectTransform = pCoordinator->GetComponent<RectTransform>(entity);
            mHitTestCache.push_back({ entity, rectTransform.computedRect });

            if (sliderArray.Has(entity))
            {
                auto& slider = sliderArray.GetData(entity);
                if (slider.handle != static_cast<Uma_ECS::Entity>(-1))
                {
                    auto& handleRTArray = pCoordinator->GetComponentArray<RectTransform>();
                    if (handleRTArray.Has(slider.handle))
                    {
                        auto& handleRT = handleRTArray.GetData(slider.handle);
                        mHitTestCache.push_back({ entity, handleRT.computedRect });
                    }
                }
            }
        }

        Uma_ECS::Entity hitEntity = Uma_UI::RaycastUI(mMousePositionNDC, mHitTestCache);

        if (hitEntity != static_cast<Uma_ECS::Entity>(-1))
        {
            mMouseConsumedThisFrame = true;
        }

        auto system = pCoordinator->GetSystem<Uma_ECS::LuaScriptingSystem>();

        auto& buttonArray = pCoordinator->GetComponentArray<Button>();
        for (size_t i = 0; i < buttonArray.Size(); ++i)
        {
            Uma_ECS::Entity entity = buttonArray.GetEntity(i);
            auto& button = buttonArray.GetComponentAt(i);

            if (!button.interactable)
            {
                button.currentState = Uma_UI::ButtonState::Disabled;
                UpdateButtonVisual(entity);
                continue;
            }

            bool isHovered = (entity == hitEntity);

            if (isHovered)
            {
                if (!button.wasHoveredLastFrame)
                {
                    button.currentState = Uma_UI::ButtonState::Hovered;
                    if (!button.scriptName.empty())
                    {
                        system->CallScriptFunction(entity, button.scriptName, "OnPointerEnter");
                    }
                }
                else if (button.currentState == Uma_UI::ButtonState::Hovered)
                {
                    button.currentState = Uma_UI::ButtonState::Hovered;
                }

                if (mouseJustPressed)
                {
                    button.currentState = Uma_UI::ButtonState::Pressed;
                    button.wasPressedWhileHovered = true;
                    if (!button.scriptName.empty())
                    {
                        system->CallScriptFunction(entity, button.scriptName, "OnPointerDown");
                    }
                }
                else if (mouseJustReleased && button.wasPressedWhileHovered)
                {
                    button.currentState = Uma_UI::ButtonState::Hovered;
                    if (!button.scriptName.empty())
                    {
                        system->CallScriptFunction(entity, button.scriptName, "OnPointerClick");
                        system->CallScriptFunction(entity, button.scriptName, "OnPointerUp");
                    }
                    button.wasPressedWhileHovered = false;
                }
                else if (mMouseButtonDown && button.wasPressedWhileHovered)
                {
                    button.currentState = Uma_UI::ButtonState::Pressed;
                }
            }
            else
            {
                if (button.wasHoveredLastFrame)
                {
                    if (!button.scriptName.empty())
                    {
                        system->CallScriptFunction(entity, button.scriptName, "OnPointerExit");
                    }

                    if (button.wasPressedWhileHovered && mouseJustReleased)
                    {
                        if (!button.scriptName.empty())
                        {
                            system->CallScriptFunction(entity, button.scriptName, "OnPointerUp");
                        }
                        button.wasPressedWhileHovered = false;
                    }
                }

                if (!mMouseButtonDown)
                {
                    button.wasPressedWhileHovered = false;
                }

                button.currentState = Uma_UI::ButtonState::Normal;
            }

            button.wasHoveredLastFrame = isHovered;
            UpdateButtonVisual(entity);
        }

        if (mouseJustReleased)
        {
            for (size_t i = 0; i < buttonArray.Size(); ++i)
            {
                auto& button = buttonArray.GetComponentAt(i);
                button.wasPressedWhileHovered = false;
            }
        }

        auto& sliderArray = pCoordinator->GetComponentArray<Slider>();
        auto& rectTransformArray = pCoordinator->GetComponentArray<RectTransform>();

        if (mDraggingSlider != static_cast<Uma_ECS::Entity>(-1))
        {
            if (mMouseButtonDown && sliderArray.Has(mDraggingSlider))
            {
                auto& slider = sliderArray.GetData(mDraggingSlider);
                auto& rectTransform = rectTransformArray.GetData(mDraggingSlider);

                float normalizedValue = 0.f;

                if (slider.direction == SliderDirection::LeftToRight || slider.direction == SliderDirection::RightToLeft)
                {
                    float sliderLeft = rectTransform.computedRect.Left();
                    float sliderRight = rectTransform.computedRect.Right();
                    float sliderWidth = sliderRight - sliderLeft;

                    if (sliderWidth > 0.f)
                    {
                        normalizedValue = (mMousePositionNDC.x - sliderLeft) / sliderWidth;
                        if (slider.direction == SliderDirection::RightToLeft)
                        {
                            normalizedValue = 1.f - normalizedValue;
                        }
                    }
                }
                else
                {
                    float sliderBottom = rectTransform.computedRect.Bottom();
                    float sliderTop = rectTransform.computedRect.Top();
                    float sliderHeight = sliderTop - sliderBottom;

                    if (sliderHeight > 0.f)
                    {
                        normalizedValue = (mMousePositionNDC.y - sliderBottom) / sliderHeight;
                        if (slider.direction == SliderDirection::TopToBottom) normalizedValue = 1.f - normalizedValue;
                    }
                }

                float oldValue = slider.value;
                slider.value = Uma_Engine::Clamp(normalizedValue, slider.minValue, slider.maxValue);

                if (oldValue != slider.value)
                {
                    UpdateSliderVisual(mDraggingSlider);

                    if (!slider.scriptName.empty())
                    {
                        system->CallScriptFunction(mDraggingSlider, slider.scriptName, "OnValueChanged");
                    }
                }
            }
            else
            {
                if (sliderArray.Has(mDraggingSlider))
                {
                    auto& slider = sliderArray.GetData(mDraggingSlider);
                    slider.isDragging = false;

                    if (!slider.scriptName.empty())
                    {
                        system->CallScriptFunction(mDraggingSlider, slider.scriptName, "OnPointerUp");
                    }
                }

                mDraggingSlider = static_cast<Uma_ECS::Entity>(-1);
            }
        }

        for (size_t i = 0; i < sliderArray.Size(); ++i)
        {
            Uma_ECS::Entity entity = sliderArray.GetEntity(i);
            auto& slider = sliderArray.GetComponentAt(i);

            if (!slider.interactable)
            {
                slider.isHovered = false;
                slider.isDragging = false;
                UpdateSliderVisual(entity);
                continue;
            }

            bool isHovered = (entity == hitEntity);
            bool wasHovered = slider.isHovered;
            slider.isHovered = isHovered;

            if (isHovered && !wasHovered)
            {
                if (!slider.scriptName.empty())
                {
                    system->CallScriptFunction(entity, slider.scriptName, "OnPointerEnter");
                }
            }
            else if (!isHovered && wasHovered && !slider.isDragging)
            {
                if (!slider.scriptName.empty())
                {
                    system->CallScriptFunction(entity, slider.scriptName, "OnPointerExit");
                }
            }

            if (isHovered && mouseJustPressed)
            {
                slider.isDragging = true;
                mDraggingSlider = entity;

                if (!slider.scriptName.empty())
                {
                    system->CallScriptFunction(entity, slider.scriptName, "OnPointerDown");
                }

                if (rectTransformArray.Has(entity))
                {
                    auto& rectTransform = rectTransformArray.GetData(entity);
                    float normalizedValue = 0.f;

                    if (slider.direction == SliderDirection::LeftToRight || slider.direction == SliderDirection::RightToLeft)
                    {
                        float sliderLeft = rectTransform.computedRect.Left();
                        float sliderRight = rectTransform.computedRect.Right();
                        float sliderWidth = sliderRight - sliderLeft;

                        if (sliderWidth > 0.f)
                        {
                            normalizedValue = (mMousePositionNDC.x - sliderLeft) / sliderWidth;
                            if (slider.direction == SliderDirection::RightToLeft) normalizedValue = 1.f - normalizedValue;
                        }
                    }
                    else
                    {
                        float sliderBottom = rectTransform.computedRect.Bottom();
                        float sliderTop = rectTransform.computedRect.Top();
                        float sliderHeight = sliderTop - sliderBottom;

                        if (sliderHeight > 0.f)
                        {
                            normalizedValue = (mMousePositionNDC.y - sliderBottom) / sliderHeight;
                            if (slider.direction == SliderDirection::TopToBottom) normalizedValue = 1.f - normalizedValue;
                        }
                    }

                    float oldValue = slider.value;
                    slider.value = Uma_Engine::Clamp(normalizedValue, slider.minValue, slider.maxValue);

                    if (oldValue != slider.value)
                    {
                        UpdateSliderVisual(entity);

                        if (!slider.scriptName.empty())
                        {
                            system->CallScriptFunction(mDraggingSlider, slider.scriptName, "OnValueChanged");
                        }
                    }
                }
            }
        }

        auto& checkboxArray = pCoordinator->GetComponentArray<Checkbox>();

        for (size_t i = 0; i < checkboxArray.Size(); ++i)
        {
            Uma_ECS::Entity entity = checkboxArray.GetEntity(i);
            auto& checkbox = checkboxArray.GetComponentAt(i);

            if (!checkbox.interactable)
            {
                checkbox.currentState = CheckboxState::Disabled;
                UpdateCheckboxVisual(entity);
                continue;
            }

            bool isHovered = (entity == hitEntity);

            if (isHovered && !checkbox.wasHoveredLastFrame)
            {
                checkbox.currentState = CheckboxState::Hovered;
                if (!checkbox.scriptName.empty())
                {
                    system->CallScriptFunction(entity, checkbox.scriptName, "OnPointerEnter");
                }
            }
            else if (!isHovered && checkbox.wasHoveredLastFrame)
            {
                checkbox.currentState = CheckboxState::Normal;
                if (!checkbox.scriptName.empty())
                {
                    system->CallScriptFunction(entity, checkbox.scriptName, "OnPointerExit");
                }

                if (checkbox.wasPressedWhileHovered && !mMouseButtonDown)
                {
                    checkbox.wasPressedWhileHovered = false;
                }
            }
            else if (isHovered && checkbox.wasHoveredLastFrame && checkbox.currentState != CheckboxState::Pressed)
            {
                checkbox.currentState = CheckboxState::Hovered;
            }

            if (isHovered && mouseJustPressed)
            {
                checkbox.currentState = CheckboxState::Pressed;
                checkbox.wasPressedWhileHovered = true;
                if (!checkbox.scriptName.empty())
                {
                    system->CallScriptFunction(entity, checkbox.scriptName, "OnPointerDown");
                }
            }

            if (mouseJustReleased && checkbox.wasPressedWhileHovered && isHovered)
            {
                checkbox.isChecked = !checkbox.isChecked;
                UpdateCheckboxVisual(entity);

                if (!checkbox.scriptName.empty())
                {
                    system->CallScriptFunction(entity, checkbox.scriptName, "OnToggle");
                    system->CallScriptFunction(entity, checkbox.scriptName, "OnPointerUp");
                }

                checkbox.wasPressedWhileHovered = false;
                checkbox.currentState = CheckboxState::Hovered;
            }
            else if (mouseJustReleased && checkbox.wasPressedWhileHovered && !isHovered)
            {
                if (!checkbox.scriptName.empty())
                {
                    system->CallScriptFunction(entity, checkbox.scriptName, "OnPointerUp");
                }
                checkbox.wasPressedWhileHovered = false;
            }

            checkbox.wasHoveredLastFrame = isHovered;
            UpdateCheckboxVisual(entity);
        }

        if (mouseJustReleased)
        {
            for (size_t i = 0; i < checkboxArray.Size(); ++i)
            {
                auto& checkbox = checkboxArray.GetComponentAt(i);
                checkbox.wasPressedWhileHovered = false;
            }
        }
    }

    void UISystem::EffectsPass(float dt)
    {
        auto& effectsArray = pCoordinator->GetComponentArray<Effects>();

        for (size_t i = 0; i < effectsArray.Size(); ++i)
        {
            Uma_ECS::Entity entity = effectsArray.GetEntity(i);
            auto& effects = effectsArray.GetComponentAt(i);

            if (effects.playOnEnable)
            {
                for (auto& clip : effects.clips)
                {
                    if (!clip.isPlaying && !clip.hasStarted)
                        clip.Play();
                }
            }

            for (auto& clip : effects.clips)
            {
                if (!clip.isPlaying)
                    continue;

                clip.currentTime += dt;

                if (clip.currentTime < clip.delay)
                    continue;

                clip.hasStarted = true;

                float progress = clip.GetProgress();
                float easedT = Easing::Apply(clip.easing, progress);

                ApplyEffect(entity, clip, easedT);

                if (clip.IsComplete())
                {
                    if (clip.loop)
                    {
                        clip.currentTime = 0.0f;
                        clip.hasStarted = false;
                    }
                    else
                    {
                        clip.isPlaying = false;
                    }
                }
            }
        }
    }

    void UISystem::ApplyEffect(Uma_ECS::Entity entity, EffectClip& clip, float easedT)
    {
        auto& rectTransformArray = pCoordinator->GetComponentArray<RectTransform>();
        auto& imageArray = pCoordinator->GetComponentArray<Image>();
        auto& textArray = pCoordinator->GetComponentArray<Text>();
        auto& transformArray = pCoordinator->GetComponentArray<Uma_ECS::Transform>();

        switch (clip.property)
        {
        case EffectProperty::Position:
            if (rectTransformArray.Has(entity))
            {
                auto& rt = rectTransformArray.GetData(entity);
                rt.anchoredPosition = LerpVec2(clip.startVec2, clip.endVec2, easedT);
                rt.isDirty = true;
            }
            break;

        case EffectProperty::Scale:
        {
            Vec2 currentScale = LerpVec2(clip.startVec2, clip.endVec2, easedT);
            static std::map<Uma_ECS::Entity, Vec2> originalSizes;
            static std::map<Uma_ECS::Entity, float> originalFontSizes;

            if (!clip.hasStarted || clip.currentTime <= clip.delay)
            {
                if (rectTransformArray.Has(entity))
                {
                    originalSizes[entity] = rectTransformArray.GetData(entity).sizeDelta;
                }
                if (textArray.Has(entity))
                {
                    originalFontSizes[entity] = textArray.GetData(entity).fontSize;
                }
            }

            if (rectTransformArray.Has(entity))
            {
                auto& rt = rectTransformArray.GetData(entity);

                if (originalSizes.find(entity) != originalSizes.end())
                {
                    rt.sizeDelta = Vec2(
                        originalSizes[entity].x * currentScale.x,
                        originalSizes[entity].y * currentScale.y
                    );
                }
                else
                {
                    originalSizes[entity] = rt.sizeDelta;
                    rt.sizeDelta = Vec2(
                        rt.sizeDelta.x * currentScale.x,
                        rt.sizeDelta.y * currentScale.y
                    );
                }

                rt.isDirty = true;
            }

            if (textArray.Has(entity))
            {
                auto& text = textArray.GetData(entity);

                if (originalFontSizes.find(entity) != originalFontSizes.end())
                {
                    float avgScale = (currentScale.x + currentScale.y) * 0.5f;
                    text.fontSize = originalFontSizes[entity] * avgScale;
                }
                else
                {
                    originalFontSizes[entity] = text.fontSize;
                    float avgScale = (currentScale.x + currentScale.y) * 0.5f;
                    text.fontSize = text.fontSize * avgScale;
                }
            }

            if (clip.applyToChildren && transformArray.Has(entity))
            {
                ApplyScaleToChildren(entity, currentScale, originalSizes, originalFontSizes);
            }

            if (!clip.loop && clip.IsComplete())
            {
                originalSizes.erase(entity);
                originalFontSizes.erase(entity);
            }
            break;
        }
        case EffectProperty::ColorTint:
            if (imageArray.Has(entity))
            {
                auto& image = imageArray.GetData(entity);
                image.color = LerpColor(clip.startColor, clip.endColor, easedT);
            }
            else if (textArray.Has(entity))
            {
                auto& text = textArray.GetData(entity);
                text.color = LerpColor(clip.startColor, clip.endColor, easedT);
            }
            break;

        case EffectProperty::Alpha:
            if (imageArray.Has(entity))
            {
                auto& image = imageArray.GetData(entity);
                image.color.a = LerpFloat(clip.startFloat, clip.endFloat, easedT);
            }
            else if (textArray.Has(entity))
            {
                auto& text = textArray.GetData(entity);
                text.color.a = LerpFloat(clip.startFloat, clip.endFloat, easedT);
            }
            break;

        case EffectProperty::FillAmount:
            if (imageArray.Has(entity))
            {
                auto& image = imageArray.GetData(entity);
                image.fillAmount = LerpFloat(clip.startFloat, clip.endFloat, easedT);
            }
            break;

        case EffectProperty::SpritesheetFrame:
            if (imageArray.Has(entity))
            {
                auto& image = imageArray.GetData(entity);
                image.SetFrame(clip.GetCurrentFrame());
            }
            break;

        default:
            break;
        }
    }

    void UISystem::ApplyScaleToChildren(Uma_ECS::Entity entity, const Vec2& scaleMultiplier, std::map<Uma_ECS::Entity, Vec2>& originalSizes, std::map<Uma_ECS::Entity, float>& originalFontSizes)
    {
        auto& transformArray = pCoordinator->GetComponentArray<Uma_ECS::Transform>();
        auto& rectTransformArray = pCoordinator->GetComponentArray<RectTransform>();
        auto& textArray = pCoordinator->GetComponentArray<Text>();

        if (!transformArray.Has(entity))
            return;

        auto& transform = transformArray.GetData(entity);

        for (Uma_ECS::Entity child : transform.children)
        {
            if (rectTransformArray.Has(child))
            {
                auto& childRT = rectTransformArray.GetData(child);

                if (originalSizes.find(child) == originalSizes.end())
                {
                    originalSizes[child] = childRT.sizeDelta;
                }

                childRT.sizeDelta = Vec2(
                    originalSizes[child].x * scaleMultiplier.x,
                    originalSizes[child].y * scaleMultiplier.y
                );
                childRT.isDirty = true;
            }

            if (textArray.Has(child))
            {
                auto& childText = textArray.GetData(child);

                if (originalFontSizes.find(child) == originalFontSizes.end())
                {
                    originalFontSizes[child] = childText.fontSize;
                }

                float avgScale = (scaleMultiplier.x + scaleMultiplier.y) * 0.5f;
                childText.fontSize = originalFontSizes[child] * avgScale;
            }

            ApplyScaleToChildren(child, scaleMultiplier, originalSizes, originalFontSizes);
        }
    }

    /*!
     * \brief Gets the current mouse position in screen pixel coordinates.
     * \return Mouse position as Vec2.
     */
    Vec2 UISystem::GetMousePosition() const
    {
        double x = 0.0, y = 0.0;
        glfwGetCursorPos(static_cast<GLFWwindow*>(pGraphics->GetWindow()), &x, &y);
        return Vec2(static_cast<float>(x), static_cast<float>(y));
    }

    /*!
     * \brief Marks all RectTransform components as dirty to force layout recomputation.
     */
    void UISystem::MarkAllDirty()
    {
        auto& rectTransform = pCoordinator->GetComponentArray<RectTransform>();
        for (size_t i = 0; i < rectTransform.Size(); ++i)
        {
            rectTransform.GetComponentAt(i).isDirty = true;
        }
    }

    /*!
     * \brief Marks an entity and all its descendants as dirty.
     * \param entity The root entity to mark dirty.
     *
     * Recursively traverses the Transform hierarchy and sets isDirty = true
     * for all RectTransform components found in the subtree.
     */
    void UISystem::MarkEntityAndChildrenDirty(Uma_ECS::Entity entity)
    {
        // Mark this entity's RectTransform as dirty if it exists
        auto& rectTransform = pCoordinator->GetComponentArray<RectTransform>();
        if (rectTransform.Has(entity))
        {
            rectTransform.GetComponentAt(entity).isDirty = true;
        }

        // Recursively mark all children
        auto& transformArray = pCoordinator->GetComponentArray<Uma_ECS::Transform>();
        if (transformArray.Has(entity))
        {
            const auto& transform = transformArray.GetComponentAt(entity);
            for (Uma_ECS::Entity child : transform.children)
            {
                MarkEntityAndChildrenDirty(child);
            }
        }
    }

    /*!
     * \brief Computes the NDC rectangle for a specific entity.
     * \param entity The entity to compute rect for.
     * \param canvasScale The canvas scale factor.
     * \return Computed NDC rectangle.
     */
    Rect UISystem::ComputeRectForEntity(Uma_ECS::Entity entity, float canvasScale)
    {
        if (!pCoordinator->GetComponentArray<RectTransform>().Has(entity))
        {
            return GetScreenRect();
        }

        auto& rectTransform = pCoordinator->GetComponent<RectTransform>(entity);
        Rect parentRect = GetParentRect(entity);
        return ComputeRectInNDC(rectTransform, parentRect, canvasScale, mScreenSize.x, mScreenSize.y);
    }

    /*!
     * \brief Gets the parent's computed NDC rectangle for a given entity.
     * \param entity The entity to find parent rect for.
     * \return Parent's NDC rectangle or screen rect if root.
     *
     * Uses Transform hierarchy to find parent, then gets parent's UI rect.
     */
    Rect UISystem::GetParentRect(Uma_ECS::Entity entity)
    {
        if (!pCoordinator->GetComponentArray<Uma_ECS::Transform>().Has(entity))
        {
            return GetScreenRect();
        }

        auto& transform = pCoordinator->GetComponent<Uma_ECS::Transform>(entity);

        // If no parent, return screen rect
        if (!transform.parent.has_value())
        {
            return GetScreenRect();
        }

        Uma_ECS::Entity parentEntity = transform.parent.value();

        // Parent must have RectTransform to provide a layout rect
        if (pCoordinator->GetComponentArray<RectTransform>().Has(parentEntity))
        {
            auto& parentRect = pCoordinator->GetComponent<RectTransform>(parentEntity);
            return parentRect.computedRect;
        }

        return GetParentRect(parentEntity);
    }

    /*!
     * \brief Updates the visual state of a button based on its current interaction state.
     * \param entity The button entity to update.
     */
    void UISystem::UpdateButtonVisual(Uma_ECS::Entity entity)
    {
        if (!pCoordinator->GetComponentArray<Image>().Has(entity))
        {
            return;
        }

        auto& button = pCoordinator->GetComponent<Button>(entity);
        auto& image = pCoordinator->GetComponent<Image>(entity);

        switch (button.currentState)
        {
        case ButtonState::Normal: image.color = button.normalColour; break;
        case ButtonState::Hovered: image.color = button.hoverColour; break;
        case ButtonState::Pressed: image.color = button.pressedColour; break;
        case ButtonState::Disabled: image.color = button.disabledColour; break;
        }
    }

    void UISystem::UpdateSliderVisual(Uma_ECS::Entity entity)
    {
        auto& rectTransformArray = pCoordinator->GetComponentArray<RectTransform>();
        auto& imageArray = pCoordinator->GetComponentArray<Image>();

        const auto& slider = pCoordinator->GetComponent<Slider>(entity);

        float range = slider.maxValue - slider.minValue;
        float normalizedValue = (range > 0.0f) ? (slider.value - slider.minValue) / range : 0.0f;
        Uma_UI::Color currentColour = (!slider.interactable) ? slider.disabledColour : (slider.isDragging || slider.isHovered) ? slider.highlightColour : slider.normalColour;

        if (slider.handle != static_cast<Uma_ECS::Entity>(-1) && rectTransformArray.Has(slider.handle))
        {
            auto& handleRT = rectTransformArray.GetData(slider.handle);

            if (slider.direction == SliderDirection::LeftToRight || slider.direction == SliderDirection::RightToLeft)
            {
                auto& sliderRT = rectTransformArray.GetData(entity);
                float sliderWidth = sliderRT.sizeDelta.x;
                float handleOffset = (normalizedValue - 0.5f) * sliderWidth;

                if (slider.direction == SliderDirection::RightToLeft) handleOffset = -handleOffset;

                handleRT.anchoredPosition.x = handleOffset;
            }
            else
            {
                auto& sliderRT = rectTransformArray.GetData(entity);
                float sliderHeight = sliderRT.sizeDelta.y;
                float handleOffset = (normalizedValue - 0.5f) * sliderHeight;

                if (slider.direction == SliderDirection::TopToBottom) handleOffset = -handleOffset;

                handleRT.anchoredPosition.y = handleOffset;
            }

            MarkEntityAndChildrenDirty(entity);

            if (imageArray.Has(slider.handle))
            {
                auto& handleImage = imageArray.GetData(slider.handle);
                handleImage.color = currentColour;
            }
        }

        if (slider.fill != static_cast<Uma_ECS::Entity>(-1) && imageArray.Has(slider.fill))
        {
            auto& fillImage = imageArray.GetData(slider.fill);
            fillImage.fillAmount = normalizedValue;

            switch (slider.direction)
            {
            case SliderDirection::LeftToRight:  fillImage.fillDirection = FillDirection::LeftToRight;  break;
            case SliderDirection::RightToLeft:  fillImage.fillDirection = FillDirection::RightToLeft;  break;
            case SliderDirection::BottomToTop:  fillImage.fillDirection = FillDirection::BottomToTop;  break;
            case SliderDirection::TopToBottom:  fillImage.fillDirection = FillDirection::TopToBottom;  break;
            default: break;
            }
        }
    }

    void UISystem::UpdateCheckboxVisual(Uma_ECS::Entity entity)
    {
        auto& imageArray = pCoordinator->GetComponentArray<Image>();

        const auto& checkbox = pCoordinator->GetComponent<Checkbox>(entity);

        Uma_UI::Color background = (!checkbox.interactable) ? checkbox.disabledColour : (checkbox.isChecked) ? checkbox.checkedColour :
            ((checkbox.currentState == CheckboxState::Normal) ? checkbox.normalColour :
                (checkbox.currentState == CheckboxState::Hovered) ? checkbox.hoverColour :
                (checkbox.currentState == CheckboxState::Pressed) ? checkbox.pressedColour :
                (checkbox.currentState == CheckboxState::Disabled) ? checkbox.disabledColour :
                checkbox.normalColour);

        Uma_UI::Color checkmark = (!checkbox.interactable) ? checkbox.checkmarkDisabledColour : checkbox.checkmarkNormalColour;

        if (checkbox.background != static_cast<Uma_ECS::Entity>(-1) && imageArray.Has(checkbox.background))
        {
            auto& bg = imageArray.GetData(checkbox.background);
            bg.color = background;
        }

        if (checkbox.checkmark != static_cast<Uma_ECS::Entity>(-1) && imageArray.Has(checkbox.checkmark))
        {
            auto& cm = imageArray.GetData(checkbox.checkmark);
            cm.visible = checkbox.isChecked;
            cm.color = checkmark;
        }
    }

    /*!
     * \brief Returns all UI entities sorted by canvas sorting order.
     * \return Vector of entity IDs in render order.
     */
    std::vector<Uma_ECS::Entity> UISystem::GetSortedUIEntities()
    {
        std::vector<std::pair<Uma_ECS::Entity, int>> entities;
        auto& rectTransform = pCoordinator->GetComponentArray<RectTransform>();

        for (size_t i = 0; i < rectTransform.Size(); ++i)
        {
            Uma_ECS::Entity entity = rectTransform.GetEntity(i);
            int sortingOrder = 0;
            auto& canvasArray = pCoordinator->GetComponentArray<Canvas>();

            // Check if this entity itself is a canvas
            if (canvasArray.Has(entity))
            {
                sortingOrder = pCoordinator->GetComponent<Canvas>(entity).sortingOrder;
            }
            else
            {
                // Walk up Transform hierarchy to find parent Canvas
                sortingOrder = FindCanvasSortingOrder(entity);
            }

            entities.push_back({ entity, sortingOrder });
        }

        // Sort by canvas order, then by entity ID for stability
        std::sort(entities.begin(), entities.end(), [](const auto& lhs, const auto& rhs) {
            if (lhs.second != rhs.second) return lhs.second < rhs.second;
            return lhs.first < rhs.first;
            });

        std::vector<Uma_ECS::Entity> result;
        result.reserve(entities.size());
        for (const auto& [entity, order] : entities)
        {
            result.push_back(entity);
        }

        return result;
    }

    /*!
     * \brief Finds the canvas sorting order by walking up the Transform hierarchy.
     * \param entity Entity to start search from.
     * \return Canvas sorting order, or 0 if no canvas found.
     */
    int UISystem::FindCanvasSortingOrder(Uma_ECS::Entity entity)
    {
        auto& canvasArray = pCoordinator->GetComponentArray<Canvas>();
        auto& transformArray = pCoordinator->GetComponentArray<Uma_ECS::Transform>();

        Uma_ECS::Entity current = entity;

        while (transformArray.Has(current))
        {
            // Check if current entity is a canvas
            if (canvasArray.Has(current))
            {
                return pCoordinator->GetComponent<Canvas>(current).sortingOrder;
            }

            // Move to parent
            auto& transform = pCoordinator->GetComponent<Uma_ECS::Transform>(current);
            if (!transform.parent.has_value())
            {
                break;
            }
            current = transform.parent.value();
        }

        return 0; // Default sorting order if no canvas found
    }

    /*!
     * \brief Loads a texture if not already loaded and returns its ID.
     * \param textureName Name of the texture to load.
     * \param fallbackPath Optional fallback path if texture is not found.
     * \return Texture ID or 0 if not available.
     */
    unsigned int UISystem::GetOrLoadTexture(const std::string& textureName, const std::string& fallbackPath)
    {
        if (!pResourcesManager)
        {
            return 0;
        }

        std::shared_ptr<Uma_Engine::Texture> texture = pResourcesManager->GetTexture(textureName);
        if (texture)
        {
            return texture->tex_id;
        }

        if (!fallbackPath.empty())
        {
            if (pResourcesManager->LoadTexture(textureName))
            {
                texture = pResourcesManager->GetTexture(textureName);
                if (texture)
                {
                    return texture->tex_id;
                }
            }
        }

        return 0;
    }

    /*!
     * \brief Ensures a font is loaded and ready for rendering.
     * \param fontName Name of the font to verify.
     * \return True if font is loaded and valid.
     */
    bool UISystem::EnsureFontLoaded(const std::string& fontName)
    {
        if (!pGraphics || fontName.empty())
        {
            return false;
        }

        Uma_Engine::FontData* uiFont = pResourcesManager->GetFont(fontName);
        float testWidth = pGraphics->MeasureText(*uiFont, "test", 24.0f);
        return testWidth > 0.0f;
    }
}