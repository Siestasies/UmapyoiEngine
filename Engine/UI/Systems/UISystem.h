/*!
\file   UISystem.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines the main UI system class coordinating all UI functionality.

This header declares the UISystem class, which manages UI components through
an explicit three-pass architecture: Layout, Input, and BuildDrawList. It
integrates with ECS, event system, graphics, and resource management subsystems.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

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
    /*!
     * \class UISystem
     * \brief Main UI system with explicit three-pass architecture for layout, input, and rendering.
     */
    class UISystem : public Uma_ECS::ECSSystem
    {
    public:
        UISystem() = default;
        ~UISystem() = default;

        /*!
         * \brief Initializes the UI system.
         */
        void Init();

        /*!
         * \brief Updates the UI system.
         * \param dt Delta time in seconds.
         */
        void Update(float dt);

        /*!
         * \brief Shuts down the UI system.
         */
        void Shutdown();

        /*!
         * \brief First pass: Computes NDC rectangles from layout data.
         */
        void LayoutPass();

        /*!
         * \brief Second pass: Processes input and updates interaction states.
         */
        void InputPass();

        /*!
         * \brief Third pass: Builds draw lists for rendering.
         */
        void BuildDrawListPass();

        /*!
         * \brief Injects the ECS coordinator dependency.
         * \param coord Pointer to the coordinator.
         */
        void SetCoordinator(Uma_ECS::Coordinator* coord) { pCoordinator = coord; }

        /*!
         * \brief Injects the event system dependency.
         * \param events Pointer to the event system.
         */
        void SetEventSystem(Uma_Engine::EventSystem* events) { pEventSystem = events; }

        /*!
         * \brief Injects the graphics system dependency.
         * \param gfx Pointer to the graphics system.
         */
        void SetGraphics(Uma_Engine::Graphics* gfx) { pGraphics = gfx; }

        /*!
         * \brief Injects the resources manager dependency.
         * \param resources Pointer to the resources manager.
         */
        void SetResourcesManager(Uma_Engine::ResourcesManager* resources) { pResourcesManager = resources; }

        /*!
         * \brief Gets the current mouse position in screen pixel coordinates.
         * \return Mouse position as Vec2.
         */
        Vec2 GetMousePosition() const;

        /*!
         * \brief Gets the current screen dimensions.
         * \return Screen size as Vec2 (width, height).
         */
        Vec2 GetScreenSize() const { return mScreenSize; }

        /*!
         * \brief Marks all RectTransform components as dirty to force layout recomputation.
         */
        void MarkAllDirty();

        /*!
         * \brief Queries if mouse input is being consumed by UI this frame.
         * \return True if UI is consuming mouse input.
         */
        bool IsMouseConsumedByUI() const { return mMouseConsumedThisFrame; }

        /*!
         * \brief Queries if any UI element is currently hovered.
         * \return True if mouse is over any UI element.
         */
        bool IsUIHovered() const { return !mHitTestCache.empty(); }


        // button related
        void ButtonOnClicked(Uma_ECS::Entity entity);

    private:
        Uma_ECS::Coordinator* pCoordinator = nullptr;
        Uma_Engine::EventSystem* pEventSystem = nullptr;
        Uma_Engine::Graphics* pGraphics = nullptr;
        Uma_Engine::ResourcesManager* pResourcesManager = nullptr;

        // temp solution to cache callback
        std::unordered_map<std::string, Uma_UI::UICallback> callbacks;

        // Screen state
        Vec2 mScreenSize{1280.f, 720.f};
        Vec2 mMousePositionScreen;
        Vec2 mMousePositionNDC;
        bool mMouseButtonDown = false;
        bool mMouseButtonDownLastFrame = false;
        bool mMouseConsumedThisFrame = false;

        std::vector<std::pair<Uma_ECS::Entity, Rect>> mHitTestCache;

        /*!
         * \brief Helper: Computes NDC rectangle for a specific entity.
         * \param entity The entity to compute rect for.
         * \param canvasScale The canvas scale factor.
         * \return Computed NDC rectangle.
         */
        Rect ComputeRectForEntity(Uma_ECS::Entity entity, float canvasScale);

        /*!
         * \brief Helper: Gets the parent's computed NDC rectangle.
         * \param entity The entity to find parent rect for.
         * \return Parent's NDC rectangle or screen rect if root.
         */
        Rect GetParentRect(Uma_ECS::Entity entity);

        /*!
         * \brief Helper: Updates button visual state based on interaction state.
         * \param entity The button entity to update.
         */
        void UpdateButtonVisual(Uma_ECS::Entity entity);

        /*!
         * \brief Helper: Returns all UI entities sorted by canvas sorting order.
         * \return Vector of entity IDs in render order.
         */
        std::vector<Uma_ECS::Entity> GetSortedUIEntities();

        /*!
         * \brief Helper: Loads a texture if not already loaded and returns its ID.
         * \param textureName Name of the texture to load.
         * \param fallbackPath Optional fallback path if texture is not found.
         * \return Texture ID or 0 if not available.
         */
        unsigned int GetOrLoadTexture(const std::string& textureName, const std::string& fallbackPath = "");

        /*!
         * \brief Helper: Ensures a font is loaded and ready for rendering.
         * \param fontName Name of the font to verify.
         * \return True if font is loaded and valid.
         */
        bool EnsureFontLoaded(const std::string& fontName);
    };
}