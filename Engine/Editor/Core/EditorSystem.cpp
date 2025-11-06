#include "EditorSystem.h"
#include "../../UI/Helpers/InputFilter.h"
#include <GLFW/glfw3.h>

#include <Debugging/Debugger.hpp>

namespace Uma_Engine
{
    EditorSystem::EditorSystem()
    {
        mState.enabled = true;
        mState.currentMode = EditorMode::Translate;
    }

    void EditorSystem::Init()
    {
        EventListenerSystem::Init();
        
        if (mConfig.autoSwitchMode)
        {
            mState.currentMode = EditorMode::Translate;
        }
    }

    void EditorSystem::Update(float dt)
    {
        (void)dt;
        
        if (mIsPlayMode) return;

        if (!mState.enabled) return;

        //if (isMouseOverUI) return;
        
        // Render selection highlight and gizmo
        if (mState.pickedEntity.has_value())
        {
            mGizmoRenderer.RenderSelectionHighlight(mState.pickedEntity.value(), mConfig);
            mGizmoRenderer.RenderGizmo(mState.pickedEntity.value(), mState, mConfig);
        }
    }

    void EditorSystem::Shutdown()
    {
        DropEntity();
    }

    void EditorSystem::SetCoordinator(Uma_ECS::Coordinator* coord)
    {
        pCoordinator = coord;
        
        // Propagate to subsystems
        mPickingSystem.SetCoordinator(coord);
        mGizmoRenderer.SetCoordinator(coord);
        mTransformManipulator.SetCoordinator(coord);
    }

    void EditorSystem::SetGraphics(Graphics* gfx)
    {
        pGraphics = gfx;
        
        // Propagate to subsystems
        mPickingSystem.SetGraphics(gfx);
        mGizmoRenderer.SetGraphics(gfx);
        mTransformManipulator.SetGraphics(gfx);
    }

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

    void EditorSystem::PickEntity(Uma_ECS::Entity entity)
    {
        if (!pCoordinator || entity == static_cast<Uma_ECS::Entity>(-1))
            return;

        // Verify entity exists
        if (!pCoordinator->HasActiveEntity(entity))
            return;

        // Auto-switch to translate mode
        if (mConfig.autoSwitchMode && mState.pickedEntity != entity)
        {
            mState.currentMode = EditorMode::Translate;
        }

        mState.pickedEntity = entity;
        
        // Emit event
        if (eventSystem)
        {
            eventSystem->Emit<EntityPickedEvent>(entity);
        }
    }

    void EditorSystem::DropEntity()
    {
        if (mState.pickedEntity.has_value())
        {
            Uma_ECS::Entity droppedEntity = mState.pickedEntity.value();
            
            mState.pickedEntity = std::nullopt;
            mState.isDragging = false;
            mState.activeAxis = GizmoAxis::None;

            // Emit event
            if (eventSystem)
            {
                eventSystem->Emit<EntityDroppedEvent>(droppedEntity);
            }
        }
    }

    void EditorSystem::SetEditorMode(EditorMode mode)
    {
        if (mState.isDragging)
        {
            mTransformManipulator.EndDrag(mState);
        }
        
        EditorMode previousMode = mState.currentMode;
        mState.currentMode = mode;

        // Emit event
        if (eventSystem && previousMode != mode)
        {
            eventSystem->Emit<EditorModeChangedEvent>(
                static_cast<int>(previousMode), 
                static_cast<int>(mode)
            );
        }
    }

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

    void EditorSystem::OnMouseButton(const MouseButtonEvent& event)
    {
        if (!mState.enabled || !pCoordinator || !pGraphics || isMouseOverUI || mIsPlayMode)
            return;

        // Left mouse button
        if (event.button == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (event.action == GLFW_PRESS)
            {
                // Check if UI consumed this click
                if (Uma_UI::InputFilter::ShouldBlockMouseInput())
                    return;

                Vec2 mousePos(event.x, event.y);

                // If we have a picked entity, check for gizmo interaction first
                if (mState.pickedEntity.has_value())
                {
                    // If gizmo was clicked, don't proceed to picking
                    if (mState.isDragging)
                        return;

                    HandleGizmoClick(mousePos);                    
                }

                // Try to pick an entity
                HandlePickingClick(mousePos);
            }
            else if (event.action == GLFW_RELEASE)
            {
                if (mState.isDragging)
                {
                    // Emit transformation event before ending drag
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

                    mTransformManipulator.EndDrag(mState);
                }
            }
        }
    }

    void EditorSystem::OnMouseMove(const MouseMoveEvent& event)
    {
        if (!mState.enabled || !pCoordinator || isMouseOverUI || mIsPlayMode)
            return;

        if (mState.isDragging && mState.pickedEntity.has_value())
        {
            Vec2 currentMouse(event.x, event.y);
            mTransformManipulator.UpdateDrag(currentMouse, mState, mConfig);
        }
    }

    void EditorSystem::OnKeyPress(const KeyPressEvent& event)
    {
        if (!mState.enabled)
            return;

        // Space - Drop entity
        if (event.key == GLFW_KEY_SPACE)
        {
            if (mState.pickedEntity.has_value())
            {
                DropEntity();
            }
        }
        // K - Translate mode
        else if (event.key == GLFW_KEY_K)
        {
            SetEditorMode(EditorMode::Translate);
        }
        // L - Rotate mode
        else if (event.key == GLFW_KEY_L)
        {
            SetEditorMode(EditorMode::Rotate);
        }
        // ; - Scale mode
        else if (event.key == GLFW_KEY_SEMICOLON)
        {
            SetEditorMode(EditorMode::Scale);
        }
        // P - Cycle modes
        else if (event.key == GLFW_KEY_P)
        {
            CycleMode();
        }
    }

    void EditorSystem::HandlePickingClick(const Vec2& screenPos)
    {
        Uma_ECS::Entity hit = mPickingSystem.RaycastEntity(screenPos, mConfig);
        
        if (hit != static_cast<Uma_ECS::Entity>(-1))
        {
            PickEntity(hit);
        }
        else
        {
            // Clicked empty space - drop current selection
            DropEntity();
        }
    }

    void EditorSystem::HandleGizmoClick(const Vec2& screenPos)
    {
        GizmoAxis hitAxis = mGizmoRenderer.HitTestGizmo(screenPos, 
                                                        mState.pickedEntity.value(), 
                                                        mState, mConfig);
        
        if (hitAxis != GizmoAxis::None)
        {
            // Start gizmo drag
            mTransformManipulator.StartDrag(mState.pickedEntity.value(), screenPos,
                                           hitAxis, mState);
        }
    }
}
