/*!
\file   EditorSystem.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Implementation of the EditorSystem class.

This file provides the concrete logic for the editor's main coordination system,
handling entity picking, mode switching, and integration with gizmo and manipulation subsystems.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "Editor/Core/EditorSystem.h"
#include "UI/Core/InputFilter.h"

#include "InputSystem.h"
#include "Editor/Cmds/EntityDeleteCmd.h"
#include <memory>

#include <GLFW/glfw3.h>
#include <Debugging/Debugger.hpp>

#include "Systems/SceneManager.h"

#include "Systems/TilemapEditorManager.h"

//events
#include "Events/SceneEvents.h"

namespace Uma_Engine
{
    /*!
     * \brief Constructs the editor system with default enabled state and translate mode.
     */
    EditorSystem::EditorSystem()
    {
        mState.enabled = true;
        mState.currentMode = EditorMode::Translate;
    }

    /*!
     * \brief Initializes the editor system and sets initial mode if auto-switching is enabled.
     */
    void EditorSystem::Init()
    {
        EventListenerSystem::Init();

        if (pSystemManager)
        {
            auto* imguiManager = pSystemManager->GetSystem<ImguiManager>();
            SetImguiManager(imguiManager);
        }
        
        if (mConfig.autoSwitchMode)
        {
            mState.currentMode = EditorMode::Translate;
        }

        SetGraphics(pSystemManager->GetSystem<Graphics>());

        eventSystem->Subscribe<SceneLoadedEvent, EditorSystem>(
            [this](const SceneLoadedEvent& e)
            {
                // set coordinator
                (void)e;
                DropEntity();
                SetCoordinator(pSystemManager->GetSystem<SceneManager>()->GetActiveSceneCoordinator());
            }
        );

        eventSystem->Subscribe<SceneUnloadedEvent, EditorSystem>(
            [this](const SceneUnloadedEvent& e)
            {
                (void)e;
                DropEntity();
                SetCoordinator(nullptr);
            }
        );

        eventSystem->Subscribe<PlaySceneRequest, EditorSystem>(
            [&](const PlaySceneRequest& e) {
                (void)e;
                SetPlayMode(true);
            }
        );

        eventSystem->Subscribe<PauseSceneRequest, EditorSystem>(
            [&](const PauseSceneRequest& e) {
                (void)e;
                SetPlayMode(false);
            }
        );

        eventSystem->Subscribe<StopSceneRequest, EditorSystem>(
            [&](const StopSceneRequest& e) {
                (void)e;
                SetPlayMode(false);
            }
        );
    }

    /*!
     * \brief Updates the editor system, rendering selection highlights and gizmos when active.
     * \param dt Delta time in seconds.
     */
    void EditorSystem::Update(float dt)
    {
        (void)dt;
        
        if (mIsPlayMode) return;

        if (!mState.enabled) return;

        if (mState.pickedEntity.has_value())
        {
            mGizmoRenderer.RenderSelectionHighlight(mState.pickedEntity.value(), mConfig);
            mGizmoRenderer.RenderGizmo(mState.pickedEntity.value(), mState, mConfig);
        }
    }

    /*!
     * \brief Shuts down the editor system and drops any selected entity.
     */
    void EditorSystem::Shutdown()
    {
        DropEntity();
    }

    /*!
     * \brief Sets the ECS coordinator dependency and propagates it to subsystems.
     * \param coord Pointer to the coordinator.
     */
    void EditorSystem::SetCoordinator(Uma_ECS::Coordinator* coord)
    {
        pCoordinator = coord;
        mPickingSystem.SetCoordinator(coord);
        mGizmoRenderer.SetCoordinator(coord);
        mTransformManipulator.SetCoordinator(coord);
    }

    /*!
     * \brief Sets the graphics system dependency and propagates it to subsystems.
     * \param gfx Pointer to the graphics system.
     */
    void EditorSystem::SetGraphics(Graphics* gfx)
    {
        pGraphics = gfx;
        
        mPickingSystem.SetGraphics(gfx);
        mGizmoRenderer.SetGraphics(gfx);
        mTransformManipulator.SetGraphics(gfx);
    }

    /*!
     * \brief Registers event listeners for mouse button, mouse move, key press, and UI events.
     */
    void EditorSystem::RegisterEventListeners()
    {
        SubscribeToEvent<MouseButtonEvent>([this](const MouseButtonEvent& e) {
            OnMouseButton(e);
        });

        SubscribeToEvent<MouseMoveEvent>([this](const MouseMoveEvent& e) {
            OnMouseMove(e);
        });

        SubscribeToEvent<KeyPressEvent>([this](const KeyPressEvent& e) {
            OnKeyPress(e);
        });

        SubscribeToEvent<UpdateMouseOverUIEvent>([this](const UpdateMouseOverUIEvent& e) {
            isMouseOverUI = e.isFocus;
        });

        SubscribeToEvent<EntityPickedEvent>([this](const EntityPickedEvent& e) {
            mState.pickedEntity = e.entity;
        });
    }

    /*!
     * \brief Selects an entity for manipulation.
     * \param entity Entity to pick.
     */
    void EditorSystem::PickEntity(Uma_ECS::Entity entity)
    {
        if (!pCoordinator || entity == static_cast<Uma_ECS::Entity>(-1))
            return;

        if (!pCoordinator->HasActiveEntity(entity))
            return;

        if (mConfig.autoSwitchMode && mState.pickedEntity != entity)
        {
            mState.currentMode = EditorMode::Translate;
        }

        mState.pickedEntity = entity;
        
        if (eventSystem)
        {
            eventSystem->Emit<EntityPickedEvent>(entity);
        }
    }

    /*!
     * \brief Deselects the currently picked entity.
     */
    void EditorSystem::DropEntity()
    {
        if (mState.pickedEntity.has_value())
        {
            Uma_ECS::Entity droppedEntity = mState.pickedEntity.value();
            
            mState.pickedEntity = std::nullopt;
            mState.isDragging = false;
            mState.activeAxis = GizmoAxis::None;

            if (eventSystem)
            {
                eventSystem->Emit<EntityDroppedEvent>(droppedEntity);
            }
        }
    }

    /*!
     * \brief Sets the editor manipulation mode.
     * \param mode The new editor mode (Translate/Rotate/Scale).
     */
    void EditorSystem::SetEditorMode(EditorMode mode)
    {
        if (mState.isDragging)
        {

            mTransformManipulator.EndDrag(mState);
        }
        
        EditorMode previousMode = mState.currentMode;
        mState.currentMode = mode;

        if (eventSystem && previousMode != mode)
        {
            eventSystem->Emit<EditorModeChangedEvent>(
                static_cast<int>(previousMode), 
                static_cast<int>(mode)
            );
        }
    }

    /*!
     * \brief Cycles through editor modes in order: Translate -> Rotate -> Scale -> Translate.
     */
    void EditorSystem::CycleMode()
    {
        switch (mState.currentMode)
        {
            case EditorMode::Translate:
                SetEditorMode(EditorMode::Rotate);
                break;
            case EditorMode::Rotate:
                SetEditorMode(EditorMode::Scale);
                break;
            case EditorMode::Scale:
                SetEditorMode(EditorMode::Translate);
                break;
            default:
                SetEditorMode(EditorMode::Translate);
                break;
        }
    }

    /*!
     * \brief Handles mouse button events for picking and gizmo interaction.
     * \param event Mouse button event.
     */
    void EditorSystem::OnMouseButton(const MouseButtonEvent& event)
    {
        if (!mState.enabled || !pCoordinator || !pGraphics || mIsPlayMode)
            return;

        // In Scene View mode, ignore isMouseOverUI since we handle it differently
        bool shouldBlockInput = false;
        if (pGraphics->GetRenderTarget() == Uma_Engine::RenderTarget::Window)
        {
            shouldBlockInput = isMouseOverUI;
        }

        if (shouldBlockInput)
            return;

        const auto& tilemapEditorManager = pSystemManager->GetSystem<TilemapEditorManager>();

        if (tilemapEditorManager->IsEditing()) return;

        if (event.button == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (event.action == GLFW_PRESS)
            {
                if (Uma_UI::InputFilter::ShouldBlockMouseInput())
                    return;

                // Get adjusted mouse position
                Vec2 mousePos = GetAdjustedMousePosition(
                    static_cast<float>(event.x),
                    static_cast<float>(event.y)
                );

                // Check if mouse position is valid
                if (mousePos.x < 0 || mousePos.y < 0)
                    return;

                if (mState.pickedEntity.has_value())
                {
                    if (mState.isDragging)
                        return;

                    HandleGizmoClick(mousePos);
                }

                HandlePickingClick(mousePos);
            }
            else if (event.action == GLFW_RELEASE)
            {
                if (mState.isDragging)
                {
                    if (eventSystem && mState.pickedEntity.has_value())
                    {
                        int transformType = 0;
                        switch (mState.currentMode)
                        {
                        case EditorMode::Translate: transformType = 0; break;
                        case EditorMode::Rotate: transformType = 1; break;
                        case EditorMode::Scale: transformType = 2; break;
                        default: break;
                        }

                        eventSystem->Emit<EntityTransformedEvent>(
                            mState.pickedEntity.value(),
                            transformType
                        );
                    }
                    pImguiManager->HasUnsavedChanges() = true;
                    pImguiManager->EndComponentEdit(mState.pickedEntity.value(), *pCoordinator, "Transform", true);

                    mTransformManipulator.EndDrag(mState);
                }
            }
        }
    }

    /*!
     * \brief Handles mouse move events for gizmo dragging.
     * \param event Mouse move event.
     */
    void EditorSystem::OnMouseMove(const MouseMoveEvent& event)
    {
        if (!mState.enabled || !pCoordinator || mIsPlayMode)
            return;

        // In Scene View mode
        bool shouldBlockInput = false;
        if (pGraphics && pGraphics->GetRenderTarget() == Uma_Engine::RenderTarget::Window)
        {
            shouldBlockInput = isMouseOverUI;
        }

        if (shouldBlockInput)
            return;

        if (mState.isDragging && mState.pickedEntity.has_value())
        {
            // Get adjusted mouse position
            Vec2 currentMouse = GetAdjustedMousePosition(
                static_cast<float>(event.x),
                static_cast<float>(event.y)
            );

            // Check if mouse position is valid
            if (currentMouse.x < 0 || currentMouse.y < 0)
                return;

            mTransformManipulator.UpdateDrag(currentMouse, mState, mConfig);
        }
    }

    /*!
     * \brief Handles key press events for editor shortcuts.
     * \param event Key press event.
     */
    void EditorSystem::OnKeyPress(const KeyPressEvent& event)
    {
        if (!mState.enabled || isMouseOverUI)
            return;

        if (event.key == GLFW_KEY_SPACE)
        {
            if (mState.pickedEntity.has_value())
            {
                DropEntity();
            }
        }
        else if (event.key == GLFW_KEY_K)
        {
            SetEditorMode(EditorMode::Translate);
        }
        else if (event.key == GLFW_KEY_L)
        {
            SetEditorMode(EditorMode::Rotate);
        }
        else if (event.key == GLFW_KEY_SEMICOLON)
        {
            SetEditorMode(EditorMode::Scale);
        }
        else if (event.key == GLFW_KEY_P)
        {
            CycleMode();
        }
        else if (event.key == GLFW_KEY_DELETE)
        {
            if (mState.pickedEntity.has_value() && mState.pickedEntity.value() != static_cast<Entity>(-1))
            {
                //pCoordinator->DestroyEntityAndChildren(mState.pickedEntity.value());

                auto cmd = std::make_unique<Uma_Editor::EntityDeleteCmd>(
                    pCoordinator,
                    mState.pickedEntity.value(),
                    true,
                    "Delete entity"
                );

                pImguiManager->GetCommandHistory()->ExecuteCommand(std::move(cmd));

                if (mState.pickedEntity.value() != static_cast<Entity>(-1))
                {
                    mState.pickedEntity.value() = static_cast<Entity>(-1);
                }
            }
        }
    }

    /*!
     * \brief Handles entity picking via raycasting at the mouse position.
     * \param screenPos Mouse position in screen pixels.
     */
    void EditorSystem::HandlePickingClick(const Vec2& screenPos)
    {
        Uma_ECS::Entity hit = mPickingSystem.RaycastEntity(screenPos, mConfig);
        
        if (hit != static_cast<Uma_ECS::Entity>(-1))
        {
            PickEntity(hit);
        }
        else
        {
            DropEntity();
        }
    }

    /*!
     * \brief Handles gizmo click detection and initiates dragging if a handle is hit.
     * \param screenPos Mouse position in screen pixels.
     */
    void EditorSystem::HandleGizmoClick(const Vec2& screenPos)
    {
        GizmoAxis hitAxis = mGizmoRenderer.HitTestGizmo(screenPos, 
                                                        mState.pickedEntity.value(), 
                                                        mState, mConfig);
        
        if (hitAxis != GizmoAxis::None)
        {
            pImguiManager->BeginComponentEdit(mState.pickedEntity.value(), *pCoordinator);

            Debugger::Log(Uma_Engine::WarningLevel::eInfo, "Started drging");

            mTransformManipulator.StartDrag(mState.pickedEntity.value(), screenPos,
                                           hitAxis, mState);
        }
    }

    void EditorSystem::SetImguiManager(ImguiManager* imgui)
    {
        pImguiManager = imgui;
    }

    Vec2 EditorSystem::GetAdjustedMousePosition(float rawX, float rawY) const
    {
        if (!pGraphics || !pImguiManager)
            return Vec2(rawX, rawY);

        // Check if in framebuffer (Scene View) mode
        if (pGraphics->GetRenderTarget() == Uma_Engine::RenderTarget::Framebuffer)
        {
            // In Scene View mode
            if (!pImguiManager->IsMouseInSceneView())
            {
                return Vec2(-1.0f, -1.0f); // Mouse not in Scene View
            }
            return pImguiManager->GetSceneViewMousePos();
        }
        else
        {
            // In Window mode
            return Vec2(rawX, rawY);
        }
    }
}