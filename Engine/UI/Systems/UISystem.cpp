/*!
\file   UISystem.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

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
#include "Systems/ResourcesTypes.hpp"

#include "Systems/LuaScriptingSystem.hpp"

// events
#include "../Events/WindowEvents.h"
#include "../Events/AudioEvents.h"
#include "../Events/IMGUIEvents.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <map>

namespace
{
    struct SpriteWithColor
    {
        Uma_Engine::Sprite_Info sprite;
        Uma_UI::Colour colour;
    };

    struct BatchKey
    {
        unsigned int texId;
        Uma_UI::Colour colour;

        bool operator<(const BatchKey& other) const
        {
            if (texId != other.texId) return texId < other.texId;
            if (colour.r != other.colour.r) return colour.r < other.colour.r;
            if (colour.g != other.colour.g) return colour.g < other.colour.g;
            if (colour.b != other.colour.b) return colour.b < other.colour.b;
            return colour.a < other.colour.a;
        }
    };
}

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

        if (pGraphics)
        {
            mScreenSize = {static_cast<float>(pGraphics->GetViewportWidth()), static_cast<float>(pGraphics->GetViewportHeight())};
        }
        else
        {
            mScreenSize = {1280.f, 720.f};
        }

        pEventSystem->Subscribe<Uma_Engine::WindowResizeEvent, UISystem>([this](const Uma_Engine::WindowResizeEvent& e) 
            { 
                mScreenSize.x = static_cast<float>(e.width), mScreenSize.y = static_cast<float>(e.height);
            });

        // cache callback
        // TEMP SOLUTION FOR UI
        callbacks["UI_CALLBACK_PLAY_SOUND"] = ([this](Uma_ECS::Entity btn)
            {
                (void)btn;
                pEventSystem->Emit<Uma_Engine::PlaySoundEvent>("explosion", 1.0f, 0.0f);
            });
        callbacks["UI_CALLBACK_LOAD_SCENE"] = ([this](Uma_ECS::Entity btn)
            {
                (void)btn;
                pEventSystem->Emit<Uma_Engine::LoadSceneRequestEvent>("test_default.scn");
            });

        mHitTestCache.clear();
    }

    /*!
     * \brief Updates the UI system through three explicit passes: Layout, Input, and BuildDrawList.
     * \param dt Delta time in seconds.
     */
    void UISystem::Update(float dt)
    {
        (void)dt;

        if (!pCoordinator || !pEventSystem || !pGraphics || !pResourcesManager)
        {
            return;
        }

        LayoutPass();
        //InputPass();
        BuildDrawListPass();
    }

    /*!
     * \brief Shuts down the UI system and clears all cached data.
     */
    void UISystem::Shutdown()
    {
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
     */
    void UISystem::LayoutPass()
    {
        auto& canvasArray = pCoordinator->GetComponentArray<Canvas>();
        std::vector<std::pair<Uma_ECS::Entity, int>> canvasEntities;

        for (size_t i = 0; i < canvasArray.Size(); ++i)
        {
            Uma_ECS::Entity entity = canvasArray.GetEntity(i);
            auto& canvas = canvasArray.GetComponentAt(i);
            canvasEntities.push_back({ entity, canvas.sortingOrder });
        }

        std::sort(canvasEntities.begin(), canvasEntities.end(), [](const auto& a, const auto& b) { return a.second < b.second; });

        for (const auto& [canvasEntity, sortOrder] : canvasEntities)
        {
            auto& canvas = pCoordinator->GetComponent<Canvas>(canvasEntity);
            canvas.scaleFactor = ComputeCanvasScale(canvas, mScreenSize.x, mScreenSize.y);
            auto& rectArray = pCoordinator->GetComponentArray<RectTransform>();

            for (size_t i = 0; i < rectArray.Size(); ++i)
            {
                Uma_ECS::Entity entity = rectArray.GetEntity(i);
                auto& rectTransform = rectArray.GetComponentAt(i);

                if (!rectTransform.isDirty && rectTransform.computedRect.width > 0.0f)
                {
                    continue;
                }

                Rect parentRect = GetParentRect(entity);
                rectTransform.computedRect = ComputeRectInNDC(rectTransform, parentRect, canvas.scaleFactor, mScreenSize.x, mScreenSize.y);
                rectTransform.isDirty = false;
            }
        }
    }

    /*!
     * \brief Second pass: Handles mouse input, updates button states, and emits UI events.
     */
    void UISystem::InputPass()
    {
        mMouseConsumedThisFrame = false;
        mMousePositionScreen = GetMousePosition();
        mMousePositionNDC = Uma_UI::ScreenToNDC(mMousePositionScreen.x, mMousePositionScreen.y, mScreenSize.x, mScreenSize.y);
        mMouseButtonDownLastFrame = mMouseButtonDown;

        if (pGraphics)
        {
            mMouseButtonDown = glfwGetMouseButton(pGraphics->GetWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        }
        else
        {
            mMouseButtonDown = false;
        }

        mHitTestCache.clear();
        float aspect = mScreenSize.x / mScreenSize.y;
        auto sortedEntities = GetSortedUIEntities();

        for (Uma_ECS::Entity entity : sortedEntities)
        {
            if (!pCoordinator->GetComponentArray<RectTransform>().Has(entity))
                continue;

            bool hasButton = pCoordinator->GetComponentArray<Button>().Has(entity);
            if (!hasButton)
                continue;

            auto& rectTransform = pCoordinator->GetComponent<RectTransform>(entity);
            Uma_UI::Rect hitRect = rectTransform.computedRect;
            hitRect.width /= aspect;
            mHitTestCache.push_back({entity, hitRect});
        }

        Uma_ECS::Entity hitEntity = Uma_UI::RaycastUI(mMousePositionNDC, mHitTestCache);

        if (hitEntity != static_cast<Uma_ECS::Entity>(-1))
        {
            mMouseConsumedThisFrame = true;
        }

        auto& buttonArray = pCoordinator->GetComponentArray<Button>();
        for (size_t i = 0; i < buttonArray.Size(); ++i)
        {
            Uma_ECS::Entity entity = buttonArray.GetEntity(i);
            auto& button = buttonArray.GetComponentAt(i);

            if (!button.interactable)
            {
                button.currentState = Uma_UI::ButtonState::Disabled;
                continue;
            }

            bool isHovered = (entity == hitEntity);

            if (mMouseButtonDown)
            {
                if (isHovered)
                {
                    button.currentState = Uma_UI::ButtonState::Pressed;
                    if (!mMouseButtonDownLastFrame)
                    {
                        pEventSystem->Emit<Uma_Engine::PointerDownEvent>(entity, mMousePositionScreen);
                    }
                }
                else
                {
                    button.currentState = Uma_UI::ButtonState::Normal;
                }
            }
            else
            {
                if (isHovered)
                {
                    button.currentState = Uma_UI::ButtonState::Hovered;
                    if (!button.wasHoveredLastFrame)
                    {
                        pEventSystem->Emit<Uma_Engine::PointerEnterEvent>(entity, mMousePositionScreen);
                    }
                    if (mMouseButtonDownLastFrame && !mMouseButtonDown)
                    {
                        pEventSystem->Emit<Uma_Engine::PointerClickEvent>(entity, mMousePositionScreen);
                        pEventSystem->Emit<Uma_Engine::PointerUpEvent>(entity, mMousePositionScreen);

                        // Invoke callback if valid
                        // 
                        /*if (button.onClick != nullptr)
                        {
                            button.onClick(entity);
                        }*/

                        // temp solution
                        if (!button.functionName.empty())
                        {
                            //button.onClick = callbacks[button.functionName];
                            //button.onClick(entity);
                            ButtonOnClicked(entity);
                        }
                    }
                }
                else
                {
                    button.currentState = Uma_UI::ButtonState::Normal;
                    if (button.wasHoveredLastFrame)
                    {
                        pEventSystem->Emit<Uma_Engine::PointerExitEvent>(entity, mMousePositionScreen);
                    }
                }
            }

            button.wasHoveredLastFrame = isHovered;
            UpdateButtonVisual(entity);
        }
    }

    /*!
     * \brief Third pass: Generates draw lists for rendering images and text.
     */
    void UISystem::BuildDrawListPass()
    {
        std::vector<SpriteWithColor> spritesWithColours;

        if (!pResourcesManager)
        {
            return;
        }

        auto sortedEntities = GetSortedUIEntities();

        for (Uma_ECS::Entity entity : sortedEntities)
        {
            auto& imageArray = pCoordinator->GetComponentArray<Image>();
            if (!imageArray.Has(entity))
            {
                continue;
            }

            auto& image = pCoordinator->GetComponent<Image>(entity);
            if (!image.visible || image.textureName.empty())
            {
                continue;
            }

            if (!pCoordinator->GetComponentArray<RectTransform>().Has(entity))
            {
                continue;
            }

            auto& rectTransform = pCoordinator->GetComponent<RectTransform>(entity);
            unsigned int texId = GetOrLoadTexture(image.textureName);
            if (texId == 0)
            {
                continue;
            }

            Uma_Engine::Sprite_Info sprite;
            sprite.pos = rectTransform.computedRect.Center();
            sprite.scale = rectTransform.computedRect.Size();
            sprite.rot = 0.0f;
            sprite.rot_speed = 0.0f;
            sprite.uvOffset = Vec2(0.0f, 0.0f);
            sprite.uvSize = Vec2(1.0f, 1.0f);
            sprite.tintColor = image.colour.ToVec3();
            sprite.alpha = image.colour.a;
            sprite.tex_id = texId;
            spritesWithColours.push_back({sprite, image.colour});
        }

        std::map<BatchKey, std::vector<Uma_Engine::Sprite_Info>> batches;
        for (const auto& swc : spritesWithColours)
        {
            BatchKey key{ swc.sprite.tex_id, swc.colour };
            batches[key].push_back(swc.sprite);
        }

        for (const auto& [key, sprites] : batches)
        {
            pGraphics->DrawSpritesScreenInstanced(key.texId, sprites);
        }

        for (Uma_ECS::Entity entity : sortedEntities)
        {
            auto& textArray = pCoordinator->GetComponentArray<Text>();
            if (!textArray.Has(entity))
            {
                continue;
            }

            auto& text = pCoordinator->GetComponent<Text>(entity);
            if (!text.visible || text.text.empty())
            {
                continue;
            }

            if (!text.fontName.empty())
            {
                EnsureFontLoaded(text.fontName);
            }

            if (!pCoordinator->GetComponentArray<RectTransform>().Has(entity))
            {
                continue;
            }

            auto& rectTransform = pCoordinator->GetComponent<RectTransform>(entity);
            Uma_Engine::FontData* uiFont = pResourcesManager->GetFont(text.fontName);
            float textWidthNDC = pGraphics->MeasureText(*uiFont, text.text, text.fontSize);
            float alignX = 0.0f;

            switch (text.alignment)
            {
            case TextAlignment::Left:
                alignX = rectTransform.computedRect.Left();
                break;
            case TextAlignment::Center:
                alignX = rectTransform.computedRect.Center().x - textWidthNDC * 0.5f;
                break;
            case TextAlignment::Right:
                alignX = rectTransform.computedRect.Right() - textWidthNDC;
                break;
            }

            float fontHeightNDC = (text.fontSize * 48.f) / static_cast<float>(mScreenSize.y) * 2.0f;
            float alignY = rectTransform.computedRect.Center().y - fontHeightNDC * 0.15f;

            pGraphics->DrawTextScreen(*uiFont, text.text, alignX, alignY, text.fontSize, text.colour.r, text.colour.g, text.colour.b);
        }

        batches.clear();
        spritesWithColours.clear();
    }

    /*!
     * \brief Gets the current mouse position in screen pixel coordinates.
     * \return Mouse position as Vec2.
     */
    Vec2 UISystem::GetMousePosition() const
    {
        if (!pGraphics)
        {
            return Vec2(0.0f, 0.0f);
        }

        double xpos, ypos;
        glfwGetCursorPos(static_cast<GLFWwindow*>(pGraphics->GetWindow()), &xpos, &ypos);
        return Vec2(static_cast<float>(xpos), static_cast<float>(ypos));
    }

    /*!
     * \brief Marks all RectTransform components as dirty to force layout recomputation.
     */
    void UISystem::MarkAllDirty()
    {
        auto& rectArray = pCoordinator->GetComponentArray<RectTransform>();
        for (size_t i = 0; i < rectArray.Size(); ++i)
        {
            rectArray.GetComponentAt(i).isDirty = true;
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
     */
    Rect UISystem::GetParentRect(Uma_ECS::Entity entity)
    {
        auto& rectTransform = pCoordinator->GetComponent<RectTransform>(entity);
        if (rectTransform.parent == static_cast<Uma_ECS::Entity>(-1))
        {
            return GetScreenRect();
        }

        if (pCoordinator->GetComponentArray<RectTransform>().Has(rectTransform.parent))
        {
            auto& parentRect = pCoordinator->GetComponent<RectTransform>(rectTransform.parent);
            return parentRect.computedRect;
        }

        return GetScreenRect();
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
        case ButtonState::Normal: image.colour = button.normalColour; break;
        case ButtonState::Hovered: image.colour = button.hoverColour; break;
        case ButtonState::Pressed: image.colour = button.pressedColour; break;
        case ButtonState::Disabled: image.colour = button.disabledColour; break;
        }
    }

    /*!
     * \brief Returns all UI entities sorted by canvas sorting order.
     * \return Vector of entity IDs in render order.
     */
    std::vector<Uma_ECS::Entity> UISystem::GetSortedUIEntities()
    {
        std::vector<std::pair<Uma_ECS::Entity, int>> entities;
        auto& rectArray = pCoordinator->GetComponentArray<RectTransform>();

        for (size_t i = 0; i < rectArray.Size(); ++i)
        {
            Uma_ECS::Entity entity = rectArray.GetEntity(i);
            int sortingOrder = 0;
            auto& canvasArray = pCoordinator->GetComponentArray<Canvas>();

            if (canvasArray.Has(entity))
            {
                sortingOrder = pCoordinator->GetComponent<Canvas>(entity).sortingOrder;
            }
            else
            {
                auto& rect = rectArray.GetComponentAt(i);
                Uma_ECS::Entity current = rect.parent;
                while (current != static_cast<Uma_ECS::Entity>(-1))
                {
                    if (canvasArray.Has(current))
                    {
                        sortingOrder = pCoordinator->GetComponent<Canvas>(current).sortingOrder;
                        break;
                    }

                    if (rectArray.Has(current))
                    {
                        current = pCoordinator->GetComponent<RectTransform>(current).parent;
                    }
                    else
                    {
                        break;
                    }
                }
            }

            entities.push_back({ entity, sortingOrder });
        }

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
            if (pResourcesManager->LoadTexture(textureName, fallbackPath))
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

    void UISystem::ButtonOnClicked(Uma_ECS::Entity entity)
    {
        pEventSystem->Emit<Uma_Engine::ButtonOnClcikedEvent>(entity, 0);
    }
}