/*!
\file   UISystem.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines the main UI system class coordinating all UI functionality.

This header declares the UISystem class, which manages UI components through
an explicit three-pass architecture: Layout, Input, and BuildDrawList. It
integrates with ECS, event system, graphics, and resource management subsystems.

CORRECTED: Transform is only for ECS organization. UISystem manages its own
UI-specific layout computations using Transform hierarchy for traversal.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../../Core/SystemType.h"
#include "FeedbackSystem.h"
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
#include "../Components/Slider.h"
#include "../Components/Checkbox.h"
#include "../Components/Effects.h"
#include <vector>
#include <map>

namespace Uma_UI
{
    /*!
     * \class UISystem
     * \brief Main UI system with explicit three-pass architecture for layout, input, and rendering.
     *
     * ARCHITECTURE NOTE:
     * - Transform component: Used for ECS organization and hierarchy traversal ONLY
     * - UISystem: Handles all UI-specific layout computations (rect positions, text alignment, etc.)
     * - Transform is NOT managed by TransformSystem for UI entities
     * - Transform.parent/children used to navigate UI tree, but UISystem does the rect math
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
         * \brief Third pass: Advances and applies all active UI effect animations.
         * \param dt Delta time in seconds.
         */
        void EffectsPass(float dt);

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
         * \brief Sets the font path used by the feedback subsystem.
         *        Must be called before Init(). Defaults to "Assets/Fonts/Fujimaru-Regular.ttf".
         * \param fontPath Relative path to the font file.
         */
        //void SetNumberFont(const std::string& fontPath) { mFeedbackSystem.SetNumberFont(fontPath); }

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
         * \brief Marks an entity and all its descendants as dirty.
         * \param entity The root entity to mark dirty.
         *
         * Recursively traverses the Transform hierarchy and sets isDirty = true
         * for all RectTransform components found in the subtree.
         */
        void MarkEntityAndChildrenDirty(Uma_ECS::Entity entity);

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

        void SimulateButtonAction(const Uma_ECS::Entity& buttonId, ButtonState state);

    private:
        Uma_ECS::Coordinator* pCoordinator = nullptr;
        Uma_Engine::EventSystem* pEventSystem = nullptr;
        Uma_Engine::Graphics* pGraphics = nullptr;
        Uma_Engine::ResourcesManager* pResourcesManager = nullptr;

        //FeedbackSystem mFeedbackSystem;

        // Screen state
        Vec2 mScreenSize{ 1280.f, 720.f };
        Vec2 mMousePositionScreen;
        Vec2 mMousePositionNDC;
        bool mMouseButtonDown = false;
        bool mMouseButtonDownLastFrame = false;
        bool mMouseConsumedThisFrame = false;
        Uma_ECS::Entity mDraggingSlider = static_cast<Uma_ECS::Entity>(-1);

        std::vector<std::pair<Uma_ECS::Entity, Rect>> mHitTestCache;

        /*!
         * \brief Recursively computes layout for entity and its children.
         * \param entity Current entity to process.
         * \param parentRect Parent's computed NDC rect.
         * \param canvasScale Canvas scale factor.
         *
         * Uses Transform.children to traverse hierarchy, but computes UI-specific rects.
         */
        void ComputeLayoutRecursive(Uma_ECS::Entity entity, const Rect& parentRect, float canvasScale);

        /*!
         * \brief Computes NDC rectangle for a specific entity.
         * \param entity The entity to compute rect for.
         * \param canvasScale The canvas scale factor.
         * \return Computed NDC rectangle.
         */
        Rect ComputeRectForEntity(Uma_ECS::Entity entity, float canvasScale);

        /*!
         * \brief Gets the parent's computed NDC rectangle.
         * \param entity The entity to find parent rect for.
         * \return Parent's NDC rectangle or screen rect if root.
         *
         * Uses Transform.parent to walk up hierarchy, returns first RectTransform found.
         */
        Rect GetParentRect(Uma_ECS::Entity entity);

        /*!
         * \brief Updates button visual state based on interaction state.
         * \param entity The button entity to update.
         */
        void UpdateButtonVisual(Uma_ECS::Entity entity);

        /*!
         * \brief Updates slider visual state based on interaction state.
         * \param entity The slider entity to update.
         */
        void UpdateSliderVisual(Uma_ECS::Entity entity);

        /*!
         * \brief Updates checkbox visual state based on interaction state.
         * \param entity The checkbox entity to update.
         */
        void UpdateCheckboxVisual(Uma_ECS::Entity entity);

        /*!
         * \brief Applies an effect clip's interpolated value to the target entity.
         * \param entity The entity to apply the effect to.
         * \param clip The effect clip being processed.
         * \param easedT The eased interpolation value [0,1].
         */
        void ApplyEffect(Uma_ECS::Entity entity, EffectClip& clip, float easedT);

        /*!
         * \brief Recursively applies a scale multiplier to an entity's children.
         * \param entity The parent entity whose children will be scaled.
         * \param scaleMultiplier Scale factor to apply to sizeDelta.
         * \param originalSizes Map to cache original sizes for restoration.
         * \param originalFontSizes Map to cache original font sizes for restoration.
         */
        void ApplyScaleToChildren(Uma_ECS::Entity entity, const Vec2& scaleMultiplier, std::map<Uma_ECS::Entity, Vec2>& originalSizes, std::map<Uma_ECS::Entity, float>& originalFontSizes);

        /*!
         * \brief Returns all UI entities sorted by canvas sorting order.
         * \return Vector of entity IDs in render order.
         */
        std::vector<Uma_ECS::Entity> GetSortedUIEntities();

        /*!
         * \brief Finds canvas sorting order by walking up Transform hierarchy.
         * \param entity Entity to start search from.
         * \return Canvas sorting order, or 0 if no canvas found.
         */
        int FindCanvasSortingOrder(Uma_ECS::Entity entity);

        /*!
         * \brief Loads a texture if not already loaded and returns its ID.
         * \param textureName Name of the texture to load.
         * \param fallbackPath Optional fallback path if texture is not found.
         * \return Texture ID or 0 if not available.
         */
        unsigned int GetOrLoadTexture(const std::string& textureName, const std::string& fallbackPath = "");

        /*!
         * \brief Ensures a font is loaded and ready for rendering.
         * \param fontName Name of the font to verify.
         * \return True if font is loaded and valid.
         */
        bool EnsureFontLoaded(const std::string& fontName);
    };
}