#include "TransformManipulator.h"
#include "../../ECS/Components/Transform.h"
#include "../../UI/Components/RectTransform.h"
#include "../../UI/Helpers/Input.h"
#include "../../UI/Helpers/Layout.h"
#include "../Core/EditorMath.h"

#include <cmath>
#include <algorithm>

namespace Uma_Engine
{
    void TransformManipulator::StartDrag(Uma_ECS::Entity entity, const Vec2& mousePos, GizmoAxis axis, EditorState& state)
    {
        state.isDragging = true;
        state.activeAxis = axis;
        state.dragStartMouse = mousePos;
        state.dragPrevMouse = mousePos;

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
            state.dragStartRotation = 0.f;  // UI typically doesn't rotate
            state.dragStartScale = rectTransform.sizeDelta;
        }
    }

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

    void TransformManipulator::EndDrag(EditorState& state)
    {
        state.isDragging = false;
        state.activeAxis = GizmoAxis::None;
    }

    void TransformManipulator::ApplyTranslation(Uma_ECS::Entity entity, const Vec2& start, const Vec2& delta, const EditorConfig& config)
    {
        if (!pCoordinator || !pGraphics)
            return;

        auto& transformArray = pCoordinator->GetComponentArray<Uma_ECS::Transform>();
        auto& rectTransformArray = pCoordinator->GetComponentArray<Uma_UI::RectTransform>();

        // Game entities: world space translation
        if (transformArray.Has(entity) && !rectTransformArray.Has(entity))
        {
            auto& transform = transformArray.GetData(entity);
            Vec2 newPos = start + delta;

            // Apply grid snapping in world space
            if (config.snapGrid > 0.0f)
            {
                newPos = SnapToGrid(newPos, config.snapGrid);
            }

            transform.position = newPos;
            transform.isDirty = true;
        }
        // UI entities: convert world delta to NDC delta
        else if (rectTransformArray.Has(entity))
        {
            auto& rectTransform = rectTransformArray.GetData(entity);

            // Get current position in screen space
            int screenWidth = pGraphics->GetViewportWidth();
            int screenHeight = pGraphics->GetViewportHeight();

            Vec2 currentScreenPos = Uma_UI::NDCToScreen(
                rectTransform.computedRect.x,
                rectTransform.computedRect.y,
                static_cast<float>(screenWidth),
                static_cast<float>(screenHeight)
            );

            // Convert world delta to screen delta
            Vec2 currentWorld = pGraphics->ScreenToWorld(currentScreenPos);
            Vec2 newWorld = currentWorld + delta;
            Vec2 newScreenPos = pGraphics->WorldToScreen(newWorld);

            Vec2 screenDelta = newScreenPos - currentScreenPos;

            // Convert screen delta to NDC delta
            Vec2 ndcDelta(
                screenDelta.x / (screenWidth * 0.5f),
                -screenDelta.y / (screenHeight * 0.5f)
            );

            rectTransform.anchoredPosition = rectTransform.anchoredPosition + ndcDelta;

            // FIX: Immediately update computedRect for visual feedback
            // This ensures Text and Image components see the new position right away

            // Get parent rect for computation
            Uma_UI::Rect parentRect = Uma_UI::GetScreenRect(); // Default to screen
            if (rectTransform.parent != static_cast<Uma_ECS::Entity>(-1))
            {
                auto& parentRectTransform = rectTransformArray.GetData(rectTransform.parent);
                parentRect = parentRectTransform.computedRect;
            }

            // Check for canvas scale
            float canvasScale = 1.0f;
            auto& canvasArray = pCoordinator->GetComponentArray<Uma_UI::Canvas>();
            // Find canvas in hierarchy (traverse up parents)
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

            // Clamp to prevent zero/negative scale
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

    bool TransformManipulator::IsGameEntity(Uma_ECS::Entity entity) const
    {
        if (!pCoordinator)
            return false;

        auto& transformArray = pCoordinator->GetComponentArray<Uma_ECS::Transform>();
        auto& rectTransformArray = pCoordinator->GetComponentArray<Uma_UI::RectTransform>();

        return transformArray.Has(entity) && !rectTransformArray.Has(entity);
    }

    bool TransformManipulator::IsUIEntity(Uma_ECS::Entity entity) const
    {
        if (!pCoordinator)
            return false;

        auto& rectTransformArray = pCoordinator->GetComponentArray<Uma_UI::RectTransform>();

        return rectTransformArray.Has(entity);
    }
}