/*!
\file   TransformManipulator.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Implementation of the TransformManipulator class.

This file provides the concrete logic for applying transformations (translate, rotate, scale)
to entities during gizmo drag operations, supporting both game and UI entities.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "Editor/Systems/TransformManipulator.h"
#include "ECS/Components/Transform.h"
#include "UI/Components/RectTransform.h"
#include "UI/Helpers/Input.h"
#include "UI/Helpers/Layout.h"
#include "Editor/Core/EditorMath.h"
#include <cmath>
#include <algorithm>

namespace Uma_Engine
{
    /*!
     * \brief Initializes a drag operation on an entity.
     * \param entity Entity being manipulated.
     * \param startMouse Initial mouse position in screen pixels.
     * \param axis Which axis/handle is being dragged.
     * \param state Editor state to update with drag information.
     */
    void TransformManipulator::StartDrag(Uma_ECS::Entity entity, const Vec2& startMouse, GizmoAxis axis, EditorState& state)
    {
        state.isDragging = true;
        state.activeAxis = axis;
        state.dragStartMouse = startMouse;
        state.dragPrevMouse = startMouse;

        auto& transformArray = pCoordinator->GetComponentArray<Uma_ECS::Transform>();
        auto& rectTransformArray = pCoordinator->GetComponentArray<Uma_UI::RectTransform>();

        if (transformArray.Has(entity) && !rectTransformArray.Has(entity))
        {
            const auto& transform = transformArray.GetData(entity);
            state.dragStartPosition = transform.position;
            state.dragStartRotation = transform.rotation.x;
            state.dragStartScale = transform.scale;
        }
        else if (rectTransformArray.Has(entity))
        {
            const auto& rectTransform = rectTransformArray.GetData(entity);
            state.dragStartPosition = rectTransform.anchoredPosition;
            state.dragStartRotation = 0.f;
            state.dragStartScale = rectTransform.sizeDelta;
        }
    }

    /*!
     * \brief Updates the drag operation with the current mouse position.
     * \param currentMouse Current mouse position in screen pixels.
     * \param state Editor state with drag information.
     * \param config Editor configuration.
     */
    void TransformManipulator::UpdateDrag(const Vec2& currentMouse, EditorState& state, const EditorConfig& config)
    {
        if (!state.isDragging)
            return;

        Uma_ECS::Entity entity = state.pickedEntity.value();
        Vec2 mouseDelta = currentMouse - state.dragPrevMouse;
        state.dragPrevMouse = currentMouse;

        switch (state.currentMode)
        {
        case EditorMode::Translate:
        {
            Vec2 worldDelta(0.0f, 0.0f);

            if (state.activeAxis == GizmoAxis::X)
            {
                Vec2 startWorld = pGraphics->ScreenToWorld(state.dragStartMouse);
                Vec2 endWorld = pGraphics->ScreenToWorld(Vec2(currentMouse.x, state.dragStartMouse.y));
                worldDelta.x = endWorld.x - startWorld.x;
            }
            else if (state.activeAxis == GizmoAxis::Y)
            {
                Vec2 startWorld = pGraphics->ScreenToWorld(state.dragStartMouse);
                Vec2 endWorld = pGraphics->ScreenToWorld(Vec2(state.dragStartMouse.x, currentMouse.y));
                worldDelta.y = endWorld.y - startWorld.y;
            }
            else if (state.activeAxis == GizmoAxis::XY)
            {
                Vec2 startWorld = pGraphics->ScreenToWorld(state.dragStartMouse);
                Vec2 endWorld = pGraphics->ScreenToWorld(currentMouse);
                worldDelta = endWorld - startWorld;
            }

            ApplyTranslation(entity, state.dragStartPosition, worldDelta, config);
            break;
        }

        case EditorMode::Rotate:
        {
            Vec2 gizmoScreen = GetEntityScreenPosition(entity);

            Vec2 startDir = state.dragStartMouse - gizmoScreen;
            Vec2 currentDir = currentMouse - gizmoScreen;

            float startAngle = std::atan2(startDir.y, startDir.x);
            float currentAngle = std::atan2(currentDir.y, currentDir.x);
            float deltaAngle = currentAngle - startAngle;

            ApplyRotation(entity, deltaAngle);
            break;
        }

        case EditorMode::Scale:
        {
            Vec2 scaleFactor(1.0f, 1.0f);
            float scaleSensitivity = 0.01f;

            if (state.activeAxis == GizmoAxis::X)
            {
                scaleFactor.x = 1.0f + mouseDelta.x * scaleSensitivity;
            }
            else if (state.activeAxis == GizmoAxis::Y)
            {
                scaleFactor.y = 1.0f + mouseDelta.y * scaleSensitivity;
            }
            else if (state.activeAxis == GizmoAxis::XY)
            {
                float avgDelta = (mouseDelta.x + mouseDelta.y) * 0.5f;
                float scale = 1.0f + avgDelta * scaleSensitivity;
                scaleFactor = Vec2(scale, scale);
            }

            ApplyScale(entity, scaleFactor);
            break;
        }

        default:
            break;
        }
    }

    /*!
     * \brief Ends the current drag operation.
     * \param state Editor state to clear.
     */
    void TransformManipulator::EndDrag(EditorState& state)
    {
        state.isDragging = false;
        state.activeAxis = GizmoAxis::None;
    }

    /*!
     * \brief Applies translation to an entity.
     * \param entity Entity to translate.
     * \param start Starting position.
     * \param delta Translation delta.
     * \param config Editor configuration.
     */
    void TransformManipulator::ApplyTranslation(Uma_ECS::Entity entity, const Vec2& start, const Vec2& delta, const EditorConfig& config)
    {
        if (!pCoordinator || !pGraphics)
            return;

        auto& transformArray = pCoordinator->GetComponentArray<Uma_ECS::Transform>();
        auto& rectTransformArray = pCoordinator->GetComponentArray<Uma_UI::RectTransform>();

        if (transformArray.Has(entity) && !rectTransformArray.Has(entity))
        {
            auto& transform = transformArray.GetData(entity);
            Vec2 newPos = start + delta;

            if (config.snapGrid > 0.0f)
            {
                newPos = SnapToGrid(newPos, config.snapGrid);
            }

            transform.position = newPos;
            transform.isDirty = true;
        }
        else if (rectTransformArray.Has(entity))
        {
            auto& rectTransform = rectTransformArray.GetData(entity);

            int screenWidth = pGraphics->GetViewportWidth();
            int screenHeight = pGraphics->GetViewportHeight();

            Vec2 currentScreenPos = Uma_UI::NDCToScreen(
                rectTransform.computedRect.x,
                rectTransform.computedRect.y,
                static_cast<float>(screenWidth),
                static_cast<float>(screenHeight)
            );

            Vec2 currentWorld = pGraphics->ScreenToWorld(currentScreenPos);
            Vec2 newWorld = currentWorld + delta;
            Vec2 newScreenPos = pGraphics->WorldToScreen(newWorld);

            Vec2 screenDelta = newScreenPos - currentScreenPos;

            Vec2 ndcDelta(
                screenDelta.x / (screenWidth * 0.5f),
                -screenDelta.y / (screenHeight * 0.5f)
            );

            rectTransform.anchoredPosition = rectTransform.anchoredPosition + ndcDelta;

            Uma_UI::Rect parentRect = Uma_UI::GetScreenRect();
            if (rectTransform.parent != static_cast<Uma_ECS::Entity>(-1))
            {
                auto& parentRectTransform = rectTransformArray.GetData(rectTransform.parent);
                parentRect = parentRectTransform.computedRect;
            }

            float canvasScale = 1.0f;
            auto& canvasArray = pCoordinator->GetComponentArray<Uma_UI::Canvas>();
            Uma_ECS::Entity current = entity;
            while (current != static_cast<Uma_ECS::Entity>(-1))
            {
                if (canvasArray.Has(current))
                {
                    canvasScale = canvasArray.GetData(current).scaleFactor;
                    break;
                }
                if (rectTransformArray.Has(current))
                {
                    current = rectTransformArray.GetData(current).parent;
                }
                else
                {
                    break;
                }
            }

            rectTransform.computedRect = Uma_UI::ComputeRectInNDC(
                rectTransform,
                parentRect,
                canvasScale,
                static_cast<float>(screenWidth),
                static_cast<float>(screenHeight)
            );

            rectTransform.isDirty = true;
        }
    }

    /*!
     * \brief Applies rotation to an entity.
     * \param entity Entity to rotate.
     * \param deltaAngle Rotation delta in radians.
     */
    void TransformManipulator::ApplyRotation(Uma_ECS::Entity entity, float deltaAngle)
    {
        if (!pCoordinator)
            return;

        auto& transformArray = pCoordinator->GetComponentArray<Uma_ECS::Transform>();
        auto& rectTransformArray = pCoordinator->GetComponentArray<Uma_UI::RectTransform>();

        if (transformArray.Has(entity) && !rectTransformArray.Has(entity))
        {
            auto& transform = transformArray.GetData(entity);
            transform.rotation.x = deltaAngle;
            transform.worldRotation = transform.rotation.x;
        }
    }

    /*!
     * \brief Applies scaling to an entity.
     * \param entity Entity to scale.
     * \param scaleFactor Scale factor to apply.
     */
    void TransformManipulator::ApplyScale(Uma_ECS::Entity entity, const Vec2& scaleFactor)
    {
        if (!pCoordinator)
            return;

        auto& transformArray = pCoordinator->GetComponentArray<Uma_ECS::Transform>();
        auto& rectTransformArray = pCoordinator->GetComponentArray<Uma_UI::RectTransform>();

        if (transformArray.Has(entity) && !rectTransformArray.Has(entity))
        {
            auto& transform = transformArray.GetData(entity);
            Vec2 newScale(
                transform.scale.x * scaleFactor.x,
                transform.scale.y * scaleFactor.y
            );

            newScale.x = std::max(0.01f, newScale.x);
            newScale.y = std::max(0.01f, newScale.y);

            transform.scale = newScale;
            transform.isDirty = true;
        }
        else if (rectTransformArray.Has(entity))
        {
            auto& rectTransform = rectTransformArray.GetData(entity);
            rectTransform.sizeDelta.x *= scaleFactor.x;
            rectTransform.sizeDelta.y *= scaleFactor.y;
            rectTransform.isDirty = true;
        }
    }

    /*!
     * \brief Gets the screen position of an entity.
     * \param entity Entity to get screen position for.
     * \return Screen position as Vec2.
     */
    Vec2 TransformManipulator::GetEntityScreenPosition(Uma_ECS::Entity entity)
    {
        if (!pCoordinator || !pGraphics)
            return Vec2(0.0f, 0.0f);

        auto& transformArray = pCoordinator->GetComponentArray<Uma_ECS::Transform>();
        auto& rectTransformArray = pCoordinator->GetComponentArray<Uma_UI::RectTransform>();

        if (transformArray.Has(entity) && !rectTransformArray.Has(entity))
        {
            const auto& transform = transformArray.GetData(entity);
            return pGraphics->WorldToScreen(transform.worldPosition);
        }
        else if (rectTransformArray.Has(entity))
        {
            const auto& rectTransform = rectTransformArray.GetData(entity);

            int screenWidth = pGraphics->GetViewportWidth();
            int screenHeight = pGraphics->GetViewportHeight();

            Vec2 screenPos = Uma_UI::NDCToScreen(
                rectTransform.computedRect.x,
                rectTransform.computedRect.y,
                static_cast<float>(screenWidth),
                static_cast<float>(screenHeight)
            );

            return screenPos;
        }

        return Vec2(0.0f, 0.0f);
    }

    /*!
     * \brief Checks if an entity is a game entity.
     * \param entity Entity to check.
     * \return True if it's a game entity.
     */
    bool TransformManipulator::IsGameEntity(Uma_ECS::Entity entity) const
    {
        if (!pCoordinator)
            return false;

        auto& transformArray = pCoordinator->GetComponentArray<Uma_ECS::Transform>();
        auto& rectTransformArray = pCoordinator->GetComponentArray<Uma_UI::RectTransform>();

        return transformArray.Has(entity) && !rectTransformArray.Has(entity);
    }

    /*!
     * \brief Checks if an entity is a UI entity.
     * \param entity Entity to check.
     * \return True if it's a UI entity.
     */
    bool TransformManipulator::IsUIEntity(Uma_ECS::Entity entity) const
    {
        if (!pCoordinator)
            return false;

        auto& rectTransformArray = pCoordinator->GetComponentArray<Uma_UI::RectTransform>();

        return rectTransformArray.Has(entity);
    }
}