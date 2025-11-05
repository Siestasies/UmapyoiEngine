#include "UISystem.h"
#include "../Helpers/Layout.h"
#include "../Helpers/Input.h"
#include "../../Events/WindowEvents.h"
#include "Systems/ResourcesTypes.hpp"

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
    void UISystem::Init()
    {
        if (!pCoordinator)
        {
            std::cerr << "UISystem::Init - Warning: Coordinator not set!" << std::endl;
        }
        if (!pEventSystem)
        {
            std::cerr << "UISystem::Init - Warning: EventSystem not set!" << std::endl;
        }
        if (!pGraphics)
        {
            std::cerr << "UISystem::Init - Warning: Graphics not set!" << std::endl;
        }
        if (!pResourcesManager)
        {
            std::cerr << "UISystem::Init - Warning: ResourcesManager not set!" << std::endl;
        }

        // Cache initial screen size
        if (pGraphics)
        {
            mScreenSize = {static_cast<float>(pGraphics->GetViewportWidth()), static_cast<float>(pGraphics->GetViewportHeight())};
        }
        else
        {
            mScreenSize = {1280.f, 720.f};
        }

        pEventSystem->Subscribe<Uma_Engine::WindowResizeEvent>([this](const Uma_Engine::WindowResizeEvent& e) { mScreenSize.x = e.width, mScreenSize.y = e.height; });

        mHitTestCache.clear();
    }

    void UISystem::Update(float dt)
    {
        (void)dt;

        if (!pCoordinator || !pEventSystem || !pGraphics || !pResourcesManager)
        {
            return;
        }

        // Three explicit passes
        LayoutPass();
        InputPass();
        BuildDrawListPass();
    }

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

    void UISystem::LayoutPass()
    {
        // Get all Canvas entities sorted by sortingOrder
        auto& canvasArray = pCoordinator->GetComponentArray<Canvas>();

        std::vector<std::pair<Uma_ECS::Entity, int>> canvasEntities;
        for (size_t i = 0; i < canvasArray.Size(); ++i)
        {
            Uma_ECS::Entity entity = canvasArray.GetEntity(i);
            auto& canvas = canvasArray.GetComponentAt(i);
            canvasEntities.push_back({ entity, canvas.sortingOrder });
        }

        // Sort by sorting order
        std::sort(canvasEntities.begin(), canvasEntities.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });

        // Process each canvas
        for (const auto& [canvasEntity, sortOrder] : canvasEntities)
        {
            auto& canvas = pCoordinator->GetComponent<Canvas>(canvasEntity);

            // Compute canvas scale factor
            canvas.scaleFactor = ComputeCanvasScale(canvas, mScreenSize.x, mScreenSize.y);

            // Get all RectTransforms
            auto& rectArray = pCoordinator->GetComponentArray<RectTransform>();

            // Process all UI elements
            for (size_t i = 0; i < rectArray.Size(); ++i)
            {
                Uma_ECS::Entity entity = rectArray.GetEntity(i);
                auto& rectTransform = rectArray.GetComponentAt(i);

                // Skip if not dirty and has valid rect
                if (!rectTransform.isDirty && rectTransform.computedRect.width > 0.0f)
                {
                    continue;
                }

                // Compute rect
                Rect parentRect = GetParentRect(entity);
                rectTransform.computedRect = ComputeRectInNDC(
                    rectTransform,
                    parentRect,
                    canvas.scaleFactor,
                    mScreenSize.x,
                    mScreenSize.y
                );

                rectTransform.isDirty = false;
            }
        }
    }

    void UISystem::InputPass()
    {
        // Reset input consumption flag each frame
        mMouseConsumedThisFrame = false;

        // Get mouse position
        mMousePositionScreen = GetMousePosition();
        mMousePositionNDC = Uma_UI::ScreenToNDC(
            mMousePositionScreen.x,
            mMousePositionScreen.y,
            mScreenSize.x,
            mScreenSize.y
        );

        // Get mouse button state
        mMouseButtonDownLastFrame = mMouseButtonDown;

        if (pGraphics)
        {
            mMouseButtonDown = glfwGetMouseButton(pGraphics->GetWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        }
        else
        {
            mMouseButtonDown = false;
        }

        // Build hit test cache (all UI elements with RectTransform)
        mHitTestCache.clear();

        // Get aspect ratio (same as Graphics uses)
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

        // Raycast to find topmost hit
        Uma_ECS::Entity hitEntity = Uma_UI::RaycastUI(mMousePositionNDC, mHitTestCache);

        // If mouse hit any UI element, mark input as consumed
        if (hitEntity != static_cast<Uma_ECS::Entity>(-1))
        {
            mMouseConsumedThisFrame = true;
        }

        // Process buttons
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

            // State transitions
            if (mMouseButtonDown)
            {
                if (isHovered)
                {
                    button.currentState = Uma_UI::ButtonState::Pressed;

                    // Emit PointerDown event
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

                    // Emit PointerEnter event (first frame of hover)
                    if (!button.wasHoveredLastFrame)
                    {
                        pEventSystem->Emit<Uma_Engine::PointerEnterEvent>(entity, mMousePositionScreen);
                    }

                    // Check for click (button up while hovering)
                    if (mMouseButtonDownLastFrame && !mMouseButtonDown)
                    {
                        pEventSystem->Emit<Uma_Engine::PointerClickEvent>(entity, mMousePositionScreen);
                        pEventSystem->Emit<Uma_Engine::PointerUpEvent>(entity, mMousePositionScreen);

                        // Invoke callback if valid
                        if (button.onClick != nullptr)
                        {
                            button.onClick(entity);
                        }
                    }
                }
                else
                {
                    button.currentState = Uma_UI::ButtonState::Normal;

                    // Emit PointerExit event (first frame of un-hover)
                    if (button.wasHoveredLastFrame)
                    {
                        pEventSystem->Emit<Uma_Engine::PointerExitEvent>(entity, mMousePositionScreen);
                    }
                }
            }

            // Update hover tracking
            button.wasHoveredLastFrame = isHovered;

            // Update button visual (changes Image colour)
            UpdateButtonVisual(entity);
        }
    }


    void UISystem::BuildDrawListPass()
    {
        std::vector<SpriteWithColor> spritesWithColours;

        if (!pResourcesManager)
        {
            return;
        }

        // Get sorted entities by render order
        auto sortedEntities = GetSortedUIEntities();

        // Build sprite list for images
        for (Uma_ECS::Entity entity : sortedEntities)
        {
            // Check if entity has Image component
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

            // Get RectTransform
            if (!pCoordinator->GetComponentArray<RectTransform>().Has(entity))
            {
                continue;
            }

            auto& rectTransform = pCoordinator->GetComponent<RectTransform>(entity);

            // Get texture ID through ResourcesManager
            unsigned int texId = GetOrLoadTexture(image.textureName);
            if (texId == 0)
            {
                continue; // Skip if texture not available
            }

            // Build sprite info
            Uma_Engine::Sprite_Info sprite;
            sprite.pos = rectTransform.computedRect.Center();
            sprite.scale = rectTransform.computedRect.Size();
            sprite.rot = 0.0f;
            sprite.rot_speed = 0.0f;
            sprite.uvOffset = Vec2(0.0f, 0.0f);
            sprite.uvSize = Vec2(1.0f, 1.0f);
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
            pGraphics->DrawSpritesScreenInstanced(
                key.texId,
                sprites
            );
        }

        // Render text elements
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

            // Ensure font is loaded through ResourcesManager
            if (!text.fontName.empty())
            {
                EnsureFontLoaded(text.fontName);
            }

            // Get RectTransform
            if (!pCoordinator->GetComponentArray<RectTransform>().Has(entity))
            {
                continue;
            }

            auto& rectTransform = pCoordinator->GetComponent<RectTransform>(entity);

            Uma_Engine::FontData* uiFont = pResourcesManager->GetFont(text.fontName);

            // Compute alignment offset
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

            pGraphics->DrawTextScreen(
                *uiFont,
                text.text,
                alignX,     // NDC
                alignY,     // NDC
                text.fontSize,
                text.colour.r,
                text.colour.g,
                text.colour.b
            );
        }

        batches.clear();
        spritesWithColours.clear();
    }

    Vec2 UISystem::GetMousePosition() const
    {
        if (!pGraphics)
        {
            return Vec2(0.0f, 0.0f);
        }

        double xpos, ypos;
        glfwGetCursorPos(
            static_cast<GLFWwindow*>(pGraphics->GetWindow()),
            &xpos,
            &ypos
        );

        return Vec2(static_cast<float>(xpos), static_cast<float>(ypos));
    }

    void UISystem::MarkAllDirty()
    {
        auto& rectArray = pCoordinator->GetComponentArray<RectTransform>();
        for (size_t i = 0; i < rectArray.Size(); ++i)
        {
            rectArray.GetComponentAt(i).isDirty = true;
        }
    }

    Rect UISystem::ComputeRectForEntity(Uma_ECS::Entity entity, float canvasScale)
    {
        if (!pCoordinator->GetComponentArray<RectTransform>().Has(entity))
        {
            return GetScreenRect();
        }

        auto& rectTransform = pCoordinator->GetComponent<RectTransform>(entity);
        Rect parentRect = GetParentRect(entity);

        return ComputeRectInNDC(
            rectTransform,
            parentRect,
            canvasScale,
            mScreenSize.x,
            mScreenSize.y
        );
    }

    Rect UISystem::GetParentRect(Uma_ECS::Entity entity)
    {
        auto& rectTransform = pCoordinator->GetComponent<RectTransform>(entity);

        if (rectTransform.parent == static_cast<Uma_ECS::Entity>(-1))
        {
            // Root element - use screen rect
            return GetScreenRect();
        }

        // Get parent's computed rect
        if (pCoordinator->GetComponentArray<RectTransform>().Has(rectTransform.parent))
        {
            auto& parentRect = pCoordinator->GetComponent<RectTransform>(rectTransform.parent);
            return parentRect.computedRect;
        }

        // Fallback to screen rect
        return GetScreenRect();
    }

    void UISystem::UpdateButtonVisual(Uma_ECS::Entity entity)
    {
        // Check if button has an Image component
        if (!pCoordinator->GetComponentArray<Image>().Has(entity))
        {
            return;
        }

        auto& button = pCoordinator->GetComponent<Button>(entity);
        auto& image = pCoordinator->GetComponent<Image>(entity);

        // Update image colour based on button state
        switch (button.currentState)
        {
        case ButtonState::Normal:
            image.colour = button.normalColour;
            break;
        case ButtonState::Hovered:
            image.colour = button.hoverColour;
            break;
        case ButtonState::Pressed:
            image.colour = button.pressedColour;
            break;
        case ButtonState::Disabled:
            image.colour = button.disabledColour;
            break;
        }
    }

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
                // Use Coordinator to get component
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
                        // Use Coordinator to get component
                        sortingOrder = pCoordinator->GetComponent<Canvas>(current).sortingOrder;
                        break;
                    }

                    if (rectArray.Has(current))
                    {
                        // Use Coordinator to get component
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

        std::sort(entities.begin(), entities.end(),
            [](const auto& lhs, const auto& rhs) {
                if (lhs.second != rhs.second)
                    return lhs.second < rhs.second;
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

    unsigned int UISystem::GetOrLoadTexture(const std::string& textureName, const std::string& fallbackPath)
    {
        if (!pResourcesManager)
        {
            return 0;
        }

        // Check if texture already loaded
        Uma_Engine::Texture* texture = pResourcesManager->GetTexture(textureName);
        if (texture)
        {
            return texture->tex_id;
        }

        // If not loaded and we have a fallback path, try to load it
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

        // Texture not found and no fallback
        return 0;
    }

    bool UISystem::EnsureFontLoaded(const std::string& fontName)
    {
        if (!pGraphics || fontName.empty())
        {
            return false;
        }

        Uma_Engine::FontData* uiFont = pResourcesManager->GetFont(fontName);

        // If MeasureText returns 0, font might not be loaded
        float testWidth = pGraphics->MeasureText(*uiFont, "test", 24.0f);

        // If font appears unloaded and we have a fallback path, try to load it
        /* if (testWidth <= 0.0f && !fallbackPath.empty())
        {
            return pGraphics->LoadFont(fontName, fallbackPath, 48);
        }*/

        return testWidth > 0.0f;
    }
}