/*!
\file   EditorSystem.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines the main editor system class for entity picking and manipulation.

This header declares the EditorSystem class which coordinates picking, gizmo rendering,
and transformation manipulation subsystems to provide a unified editing interface.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../Core/SystemType.h"
#include "../ECS/Core/Coordinator.hpp"
#include "../Core/EventSystem.h"
#include "../Systems/Graphics.hpp"
#include "../Events/InputEvents.h"
#include "../Events/EditorEvents.h"
#include "../Core/EditorTypes.h"
#include "../Systems/PickingSystem.h"
#include "../Systems/GizmoRenderer.h"
#include "../Systems/TransformManipulator.h"
#include "../WIP_Scripts/ImguiManager.h"

namespace Uma_Engine
{
    /*!
     * \class EditorSystem
     * \brief Main runtime editor system for entity picking and manipulation.
     *
     * Coordinates between PickingSystem, GizmoRenderer, and TransformManipulator subsystems.
     */
    class EditorSystem : public EventListenerSystem
    {
    public:
        /*!
         * \brief Constructs the editor system.
         */
        EditorSystem();
        
        /*!
         * \brief Destroys the editor system.
         */
        ~EditorSystem() override = default;

        /*!
         * \brief Initializes the editor system.
         */
        void Init() override;

        /*!
         * \brief Updates the editor system.
         * \param dt Delta time in seconds.
         */
        void Update(float dt) override;

        /*!
         * \brief Shuts down the editor system.
         */
        void Shutdown() override;

        /*!
         * \brief Sets the ECS coordinator dependency.
         * \param coord Pointer to the coordinator.
         */
        void SetCoordinator(Uma_ECS::Coordinator* coord);

        /*!
         * \brief Sets the graphics system dependency.
         * \param gfx Pointer to the graphics system.
         */
        void SetGraphics(Graphics* gfx);

        /*!
         * \brief Selects an entity for manipulation.
         * \param entity Entity to pick.
         */
        void PickEntity(Uma_ECS::Entity entity);

        /*!
         * \brief Deselects the currently picked entity.
         */
        void DropEntity();

        /*!
         * \brief Gets the currently picked entity ID.
         * \return Entity ID or -1 if none is selected.
         */
        Uma_ECS::Entity GetPickedEntity() const
        {
            return mState.pickedEntity.value_or(static_cast<Uma_ECS::Entity>(-1));
        }

        /*!
         * \brief Checks if an entity is currently picked.
         * \return True if an entity is selected.
         */
        bool HasPickedEntity() const { return mState.pickedEntity.has_value(); }

        /*!
         * \brief Sets the editor manipulation mode.
         * \param mode The new editor mode.
         */
        void SetEditorMode(EditorMode mode);

        /*!
         * \brief Gets the current editor mode.
         * \return Current editor mode.
         */
        EditorMode GetEditorMode() const { return mState.currentMode; }

        /*!
         * \brief Cycles through editor modes in order.
         */
        void CycleMode();

        /*!
         * \brief Gets mutable editor configuration.
         * \return Reference to editor config.
         */
        EditorConfig& GetConfig() { return mConfig; }

        /*!
         * \brief Gets const editor configuration.
         * \return Const reference to editor config.
         */
        const EditorConfig& GetConfig() const { return mConfig; }

        /*!
         * \brief Enables or disables the editor.
         * \param enabled Whether to enable the editor.
         */
        void SetEnabled(bool enabled) { mState.enabled = enabled; }

        /*!
         * \brief Checks if the editor is enabled.
         * \return True if editor is enabled.
         */
        bool IsEnabled() const { return mState.enabled; }

        /*!
         * \brief Sets play mode state (disables editor during gameplay).
         * \param isPlaying True if game is playing.
         */
        void SetPlayMode(bool isPlaying) { mIsPlayMode = isPlaying; }

        /*!
         * \brief Checks if the game is in play mode.
         * \return True if in play mode.
         */
        bool IsPlayMode() const { return mIsPlayMode; }

        void SetImguiManager(ImguiManager* imgui);

    protected:
        /*!
         * \brief Registers event listeners for editor input.
         */
        void RegisterEventListeners() override;

    private:
        Uma_ECS::Coordinator* pCoordinator = nullptr;
        Graphics* pGraphics = nullptr;
        ImguiManager* pImguiManager = nullptr;

        EditorState mState;
        EditorConfig mConfig;
        bool mIsPlayMode = false;
        bool isMouseOverUI = false;

        PickingSystem mPickingSystem;
        GizmoRenderer mGizmoRenderer;
        TransformManipulator mTransformManipulator;

        /*!
         * \brief Handles mouse button events.
         * \param event Mouse button event.
         */
        void OnMouseButton(const MouseButtonEvent& event);

        /*!
         * \brief Handles mouse move events.
         * \param event Mouse move event.
         */
        void OnMouseMove(const MouseMoveEvent& event);

        /*!
         * \brief Handles key press events.
         * \param event Key press event.
         */
        void OnKeyPress(const KeyPressEvent& event);

        /*!
         * \brief Handles entity picking via raycasting.
         * \param screenPos Mouse position in screen pixels.
         */
        void HandlePickingClick(const Vec2& screenPos);

        /*!
         * \brief Handles gizmo click detection.
         * \param screenPos Mouse position in screen pixels.
         */
        void HandleGizmoClick(const Vec2& screenPos);

        Vec2 GetAdjustedMousePosition(float rawX, float rawY) const;
    };
}