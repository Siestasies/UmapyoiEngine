#pragma once

#include "../../Core/SystemType.h"
#include "../../ECS/Core/Coordinator.hpp"
#include "../../Core/EventSystem.h"
#include "../../Systems/Graphics.hpp"
#include "../../Events/InputEvents.h"
#include "../Core/EditorTypes.h"
#include "../Events/EditorEvents.h"
#include "../Systems/PickingSystem.h"
#include "../Systems/GizmoRenderer.h"
#include "../Systems/TransformManipulator.h"

namespace Uma_Engine
{
    /**
     * \class EditorSystem
     * \brief Main runtime editor system for entity picking and manipulation
     *
     * Coordinates subsystems:
     * - PickingSystem: Entity raycasting and selection
     * - GizmoRenderer: Visual gizmo drawing and hit testing
     * - TransformManipulator: Entity transformation application
     */
    class EditorSystem : public EventListenerSystem
    {
    public:
        EditorSystem();
        ~EditorSystem() override = default;

        // === ISystem Interface ===
        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;

        // === Dependencies ===
        void SetCoordinator(Uma_ECS::Coordinator* coord);
        void SetGraphics(Graphics* gfx);

        // === Picking API ===

        /**
         * \brief Pick an entity by ID
         * \param entity Entity to select
         */
        void PickEntity(Uma_ECS::Entity entity);

        /**
         * \brief Drop current selection
         */
        void DropEntity();

        /**
         * \brief Get currently picked entity
         * \return Entity ID or -1 if none
         */
        Uma_ECS::Entity GetPickedEntity() const
        {
            return mState.pickedEntity.value_or(static_cast<Uma_ECS::Entity>(-1));
        }

        /**
         * \brief Check if an entity is currently picked
         */
        bool HasPickedEntity() const { return mState.pickedEntity.has_value(); }

        // === Mode Control ===

        /**
         * \brief Set editor mode
         * \param mode Translate/Rotate/Scale
         */
        void SetEditorMode(EditorMode mode);

        /**
         * \brief Get current editor mode
         */
        EditorMode GetEditorMode() const { return mState.currentMode; }

        /**
         * \brief Cycle through modes (Translate -> Rotate -> Scale -> Translate)
         */
        void CycleMode();

        // === Configuration ===

        /**
         * \brief Get mutable configuration reference
         */
        EditorConfig& GetConfig() { return mConfig; }

        /**
         * \brief Get const configuration reference
         */
        const EditorConfig& GetConfig() const { return mConfig; }

        // === Enable/Disable ===

        /**
         * \brief Enable or disable editor
         */
        void SetEnabled(bool enabled) { mState.enabled = enabled; }

        /**
         * \brief Check if editor is enabled
         */
        bool IsEnabled() const { return mState.enabled; }

        // === Play Mode ===

        /**
         * \brief Set play mode state (editor disabled during play)
         * \param isPlaying true if game is playing, false if editing
         */
        void SetPlayMode(bool isPlaying) { mIsPlayMode = isPlaying; }

        /**
         * \brief Check if game is in play mode
         */
        bool IsPlayMode() const { return mIsPlayMode; }

    protected:
        void RegisterEventListeners() override;

    private:
        // === Dependencies ===
        Uma_ECS::Coordinator* pCoordinator = nullptr;
        Graphics* pGraphics = nullptr;

        // === State ===
        EditorState mState;
        EditorConfig mConfig;
        bool mIsPlayMode = false;  // Track play mode to disable editor during gameplay

        // === Systems ===
        PickingSystem mPickingSystem;
        GizmoRenderer mGizmoRenderer;
        TransformManipulator mTransformManipulator;

        // === Event Handlers ===
        void OnMouseButton(const MouseButtonEvent& event);
        void OnMouseMove(const MouseMoveEvent& event);
        void OnKeyPress(const KeyPressEvent& event);

        // === Internal Helpers ===
        void HandlePickingClick(const Vec2& screenPos);
        void HandleGizmoClick(const Vec2& screenPos);
    };
}