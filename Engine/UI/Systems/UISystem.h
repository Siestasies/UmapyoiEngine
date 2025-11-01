#pragma once

#include "../../Core/SystemType.h"
#include "../../ECS/Core/Coordinator.hpp"
#include "../../Core/EventSystem.h"
#include "../../Systems/Graphics.hpp"
#include "../../Systems/ResourcesTypes.hpp"
#include "../../Systems/ResourcesManager.hpp"
#include "../Core/UITypes.h"
#include "../../Events/UIEvents.h"
#include "../Components/Canvas.h"
#include "../Components/RectTransform.h"
#include "../Components/Image.h"
#include "../Components/Text.h"
#include "../Components/Button.h"

#include <vector>
#include <map>

namespace Uma_UI
{
    /**
     * \class UISystem
     * \brief Main UI system with explicit three-pass architecture:
     *        1. LayoutPass - compute NDC rects from anchors
     *        2. InputPass - raycast mouse, update button states, emit events
     *        3. BuildDrawListPass - generate sprite draw list for renderer
     */
    class UISystem : public Uma_Engine::ISystem
    {
    public:
        UISystem() = default;
        ~UISystem() = default;

        // ISystem interface
        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;

        // Pass 1: Layout computation
        void LayoutPass();

        // Pass 2: Input handling
        void InputPass();

        // Pass 3: Build draw list for renderer
        void BuildDrawListPass();

        // Accessors
        void SetCoordinator(Uma_ECS::Coordinator* coord) { pCoordinator = coord; }
        void SetEventSystem(Uma_Engine::EventSystem* events) { pEventSystem = events; }
        void SetGraphics(Uma_Engine::Graphics* gfx) { pGraphics = gfx; }
        void SetResourcesManager(Uma_Engine::ResourcesManager* resources) { pResourcesManager = resources; }

        // Get mouse position in screen pixels
        Vec2 GetMousePosition() const;

        // Get current screen dimensions
        Vec2 GetScreenSize() const { return mScreenSize; }

        // Force all UI elements to recompute layout
        void MarkAllDirty();

        // Input consumption queries (for HybridInputSystem integration)
        bool IsMouseConsumedByUI() const { return mMouseConsumedThisFrame; }
        bool IsUIHovered() const { return !mHitTestCache.empty(); }

    private:
        // Dependencies
        Uma_ECS::Coordinator* pCoordinator = nullptr;
        Uma_Engine::EventSystem* pEventSystem = nullptr;
        Uma_Engine::Graphics* pGraphics = nullptr;
        Uma_Engine::ResourcesManager* pResourcesManager = nullptr;

        // Screen state
        Vec2 mScreenSize{1280.f, 720.f};

        // Input state
        Vec2 mMousePositionScreen;
        Vec2 mMousePositionNDC;
        bool mMouseButtonDown = false;
        bool mMouseButtonDownLastFrame = false;
        bool mMouseConsumedThisFrame = false;

        // Hit testing cache (sorted by render order)
        std::vector<std::pair<Uma_ECS::Entity, Uma_UI::Rect>> mHitTestCache;

        // Draw list (output for renderer)
        std::vector<Uma_Engine::Sprite_Info> mDrawList;

        // Helper: Compute rect for entity
        Uma_UI::Rect ComputeRectForEntity(Uma_ECS::Entity entity, float canvasScale);

        // Helper: Get parent's computed rect
        Uma_UI::Rect GetParentRect(Uma_ECS::Entity entity);

        // Helper: Update button visual state
        void UpdateButtonVisual(Uma_ECS::Entity entity);

        // Helper: Sort entities by canvas sorting order
        std::vector<Uma_ECS::Entity> GetSortedUIEntities();

        unsigned int GetOrLoadTexture(const std::string& textureName, const std::string& fallbackPath = "");

        bool EnsureFontLoaded(const std::string& fontName, const std::string& fallbackPath = "");
    };
}