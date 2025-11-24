/*!
\file   GizmoRenderer.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Implementation of the GizmoRenderer class.

This file provides the concrete logic for rendering interactive gizmos and handle hit-testing
for translate, rotate, and scale manipulation modes.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "ECS/Components/Transform.h"
#include "ECS/Components/Sprite.h"
#include "UI/Components/RectTransform.h"
#include "UI/Helpers/Input.h"
#include "UI/Helpers/Layout.h"
#include "Editor/Core/EditorMath.h"
#include "Editor/Systems/GizmoRenderer.h"
#include <cmath>

namespace Uma_Engine
{
    /*!
     * \brief Renders the appropriate gizmo for the current editor mode.
     * \param entity The entity to render gizmo for.
     * \param state Current editor state.
     * \param config Editor configuration.
     */
    void GizmoRenderer::RenderGizmo(Uma_ECS::Entity entity, const EditorState& state, const EditorConfig& config)
    {
        if (!pGraphics)
            return;

        Vec2 screenPos = GetEntityScreenPosition(entity);

        switch (state.currentMode)
        {
        case EditorMode::Translate:
            RenderTranslateGizmo(screenPos, state, config);
            break;
        case EditorMode::Rotate:
            RenderRotateGizmo(screenPos, state, config);
            break;
        case EditorMode::Scale:
            RenderScaleGizmo(screenPos, state, config);
            break;
        default:
            break;
        }
    }

    /*!
     * \brief Renders selection highlight around the specified entity.
     * \param entity Entity to highlight.
     * \param config Editor configuration.
     */
    void GizmoRenderer::RenderSelectionHighlight(Uma_ECS::Entity entity, const EditorConfig& config)
    {
        if (!pCoordinator || !pGraphics)
            return;

        std::vector<DebugLineInfo> lines;

        auto& transformArray = pCoordinator->GetComponentArray<Uma_ECS::Transform>();
        auto& spriteArray = pCoordinator->GetComponentArray<Uma_ECS::Sprite>();
        auto& rectTransformArray = pCoordinator->GetComponentArray<Uma_UI::RectTransform>();

        if (transformArray.Has(entity) && !rectTransformArray.Has(entity))
        {
            const auto& transform = transformArray.GetData(entity);

            Vec2 size(50.0f, 50.0f);
            if (spriteArray.Has(entity))
            {
                const auto& sprite = spriteArray.GetData(entity);
                if (sprite.texture)
                {
                    size.x = static_cast<float>(sprite.texture->tex_size.x)/static_cast<float>(sprite.texture->pixelsPerUnit);
                    size.y = static_cast<float>(sprite.texture->tex_size.y)/static_cast<float>(sprite.texture->pixelsPerUnit);
                }
            }

            Graphics::AddDebugRect(transform.worldPosition, size * transform.scale,
                config.colorSelected.x, config.colorSelected.y, config.colorSelected.z, lines);
        }
        else if (rectTransformArray.Has(entity))
        {
            const auto& rectTransform = rectTransformArray.GetData(entity);

            int screenWidth = pGraphics->GetViewportWidth();
            int screenHeight = pGraphics->GetViewportHeight();

            Vec2 ndcCenter(rectTransform.computedRect.x, rectTransform.computedRect.y);
            Vec2 ndcSize(rectTransform.computedRect.width, rectTransform.computedRect.height);

            float screenHalfWidth = ndcSize.x * screenWidth * 0.5f;
            float screenHalfHeight = ndcSize.y * screenHeight * 0.5f;

            Vec2 screenCenter = Uma_UI::NDCToScreen(
                ndcCenter.x, ndcCenter.y,
                static_cast<float>(screenWidth),
                static_cast<float>(screenHeight)
            );

            Vec2 screenTopLeft = screenCenter - Vec2(screenHalfWidth, screenHalfHeight);
            Vec2 screenBottomRight = screenCenter + Vec2(screenHalfWidth, screenHalfHeight);

            Vec2 worldTopLeft = pGraphics->ScreenToWorld(screenTopLeft);
            Vec2 worldBottomRight = pGraphics->ScreenToWorld(screenBottomRight);

            Vec2 worldCenter = (worldTopLeft + worldBottomRight) * 0.5f;
            Vec2 worldSize = worldBottomRight - worldTopLeft;
            worldSize.x = std::abs(worldSize.x);
            worldSize.y = std::abs(worldSize.y);

            Graphics::AddDebugRect(worldCenter, worldSize,
                config.colorSelected.x, config.colorSelected.y, config.colorSelected.z, lines);

        }

        if (!lines.empty())
        {
            pGraphics->DrawDebugLinesInstanced(lines);
        }
    }

    /*!
     * \brief Performs hit-testing on gizmo handles to determine which axis was clicked.
     * \param mousePos Mouse position in screen pixels.
     * \param entity Entity with the gizmo.
     * \param state Current editor state.
     * \param config Editor configuration.
     * \return The axis that was hit, or GizmoAxis::None.
     */
    GizmoAxis GizmoRenderer::HitTestGizmo(const Vec2& mousePos, Uma_ECS::Entity entity,
        const EditorState& state, const EditorConfig& config)
    {
        Vec2 gizmoPos = GetEntityScreenPosition(entity);

        switch (state.currentMode)
        {
        case EditorMode::Translate:
            return HitTestTranslateGizmo(mousePos, gizmoPos, config);
        case EditorMode::Rotate:
            return HitTestRotateGizmo(mousePos, gizmoPos, config);
        case EditorMode::Scale:
            return HitTestScaleGizmo(mousePos, gizmoPos, config);
        default:
            return GizmoAxis::None;
        }
    }

    /*!
     * \brief Gets the screen position of an entity for gizmo placement.
     * \param entity The entity to get screen position for.
     * \return Screen position as Vec2.
     */
    Vec2 GizmoRenderer::GetEntityScreenPosition(Uma_ECS::Entity entity)
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
     * \brief Renders the translation gizmo (X/Y axes with handles).
     * \param screenPos Gizmo position in screen pixels.
     * \param state Current editor state.
     * \param config Editor configuration.
     */
    void GizmoRenderer::RenderTranslateGizmo(const Vec2& screenPos, const EditorState& state, const EditorConfig& config)
    {
        if (!pGraphics)
            return;

        float size = config.gizmoSize;
        float handleSize = config.gizmoHandleSize;

        std::vector<DebugLineInfo> lines;

        Vec3 xColor = (state.activeAxis == GizmoAxis::X) ? config.colorHighlight : config.colorXAxis;
        Vec2 xStart = pGraphics->ScreenToWorld(screenPos);
        Vec2 xEnd = pGraphics->ScreenToWorld(Vec2(screenPos.x + size, screenPos.y));
        lines.push_back({ xStart, xEnd, xColor });

        Vec2 xHandleWorld = pGraphics->ScreenToWorld(Vec2(screenPos.x + size, screenPos.y));
        Graphics::AddDebugRect(xHandleWorld, Vec2(handleSize * 0.02f, handleSize * 0.02f),
            xColor.x, xColor.y, xColor.z, lines);

        Vec3 yColor = (state.activeAxis == GizmoAxis::Y) ? config.colorHighlight : config.colorYAxis;
        Vec2 yStart = pGraphics->ScreenToWorld(screenPos);
        Vec2 yEnd = pGraphics->ScreenToWorld(Vec2(screenPos.x, screenPos.y + size));
        lines.push_back({ yStart, yEnd, yColor });

        Vec2 yHandleWorld = pGraphics->ScreenToWorld(Vec2(screenPos.x, screenPos.y + size));
        Graphics::AddDebugRect(yHandleWorld, Vec2(handleSize * 0.02f, handleSize * 0.02f),
            yColor.x, yColor.y, yColor.z, lines);

        Vec3 xyColor = (state.activeAxis == GizmoAxis::XY) ? config.colorHighlight : config.colorXYHandle;
        Vec2 centerWorld = pGraphics->ScreenToWorld(screenPos);
        Graphics::AddDebugRect(centerWorld, Vec2(handleSize * 0.03f, handleSize * 0.03f),
            xyColor.x, xyColor.y, xyColor.z, lines);

        pGraphics->DrawDebugLinesInstanced(lines);
    }

    /*!
     * \brief Renders the rotation gizmo (circular handle).
     * \param screenPos Gizmo position in screen pixels.
     * \param state Current editor state.
     * \param config Editor configuration.
     */
    void GizmoRenderer::RenderRotateGizmo(const Vec2& screenPos, const EditorState& state, const EditorConfig& config)
    {
        if (!pGraphics)
            return;

        float radius = config.gizmoSize;
        Vec3 color = state.isDragging ? config.colorHighlight : config.colorSelected;

        std::vector<DebugLineInfo> lines;
        Vec2 centerWorld = pGraphics->ScreenToWorld(screenPos);
        Graphics::AddDebugCircle(centerWorld, radius * 0.01f, color.x, color.y, color.z, lines);

        pGraphics->DrawDebugLinesInstanced(lines);
    }

    /*!
     * \brief Renders the scale gizmo (X/Y axes with box handles).
     * \param screenPos Gizmo position in screen pixels.
     * \param state Current editor state.
     * \param config Editor configuration.
     */
    void GizmoRenderer::RenderScaleGizmo(const Vec2& screenPos, const EditorState& state, const EditorConfig& config)
    {
        if (!pGraphics)
            return;

        float size = config.gizmoSize * 0.7f;
        float handleSize = config.gizmoHandleSize * 1.5f;

        std::vector<DebugLineInfo> lines;

        Vec3 xColor = (state.activeAxis == GizmoAxis::X) ? config.colorHighlight : config.colorXAxis;
        Vec2 xStart = pGraphics->ScreenToWorld(screenPos);
        Vec2 xEnd = pGraphics->ScreenToWorld(Vec2(screenPos.x + size, screenPos.y));
        lines.push_back({ xStart, xEnd, xColor });

        Vec2 xHandleWorld = pGraphics->ScreenToWorld(Vec2(screenPos.x + size, screenPos.y));
        Graphics::AddDebugRect(xHandleWorld, Vec2(handleSize * 0.02f, handleSize * 0.02f),
            xColor.x, xColor.y, xColor.z, lines);

        Vec3 yColor = (state.activeAxis == GizmoAxis::Y) ? config.colorHighlight : config.colorYAxis;
        Vec2 yStart = pGraphics->ScreenToWorld(screenPos);
        Vec2 yEnd = pGraphics->ScreenToWorld(Vec2(screenPos.x, screenPos.y + size));
        lines.push_back({ yStart, yEnd, yColor });

        Vec2 yHandleWorld = pGraphics->ScreenToWorld(Vec2(screenPos.x, screenPos.y + size));
        Graphics::AddDebugRect(yHandleWorld, Vec2(handleSize * 0.02f, handleSize * 0.02f),
            yColor.x, yColor.y, yColor.z, lines);

        Vec3 xyColor = (state.activeAxis == GizmoAxis::XY) ? config.colorHighlight : config.colorXYHandle;
        Vec2 xyHandleWorld = pGraphics->ScreenToWorld(Vec2(screenPos.x + size * 0.7f, screenPos.y + size * 0.7f));
        Graphics::AddDebugRect(xyHandleWorld, Vec2(handleSize * 0.025f, handleSize * 0.025f),
            xyColor.x, xyColor.y, xyColor.z, lines);

        pGraphics->DrawDebugLinesInstanced(lines);
    }

    /*!
     * \brief Hit-tests the translation gizmo handles.
     * \param mousePos Mouse position in screen pixels.
     * \param gizmoPos Gizmo position in screen pixels.
     * \param config Editor configuration.
     * \return The axis that was hit, or GizmoAxis::None.
     */
    GizmoAxis GizmoRenderer::HitTestTranslateGizmo(const Vec2& mousePos, const Vec2& gizmoPos, const EditorConfig& config)
    {
        float size = config.gizmoSize;
        float handleSize = config.gizmoHandleSize * 2.0f;

        if (std::abs(mousePos.x - gizmoPos.x) < handleSize &&
            std::abs(mousePos.y - gizmoPos.y) < handleSize)
        {
            return GizmoAxis::XY;
        }

        Vec2 xHandle(gizmoPos.x + size, gizmoPos.y);
        if (std::abs(mousePos.x - xHandle.x) < handleSize &&
            std::abs(mousePos.y - xHandle.y) < handleSize)
        {
            return GizmoAxis::X;
        }

        Vec2 yHandle(gizmoPos.x, gizmoPos.y + size);
        if (std::abs(mousePos.x - yHandle.x) < handleSize &&
            std::abs(mousePos.y - yHandle.y) < handleSize)
        {
            return GizmoAxis::Y;
        }

        Vec2 xStart = gizmoPos;
        Vec2 xEnd(gizmoPos.x + size, gizmoPos.y);
        if (DistanceToLineSegment(mousePos, xStart, xEnd) < handleSize * 0.5f)
        {
            return GizmoAxis::X;
        }

        Vec2 yStart = gizmoPos;
        Vec2 yEnd(gizmoPos.x, gizmoPos.y + size);
        if (DistanceToLineSegment(mousePos, yStart, yEnd) < handleSize * 0.5f)
        {
            return GizmoAxis::Y;
        }

        return GizmoAxis::None;
    }

    /*!
     * \brief Hit-tests the rotation gizmo (circular handle).
     * \param mousePos Mouse position in screen pixels.
     * \param gizmoPos Gizmo position in screen pixels.
     * \param config Editor configuration.
     * \return The axis that was hit, or GizmoAxis::None.
     */
    GizmoAxis GizmoRenderer::HitTestRotateGizmo(const Vec2& mousePos, const Vec2& gizmoPos, const EditorConfig& config)
    {
        float radius = config.gizmoSize;
        float thickness = config.gizmoHandleSize;

        float dx = mousePos.x - gizmoPos.x;
        float dy = mousePos.y - gizmoPos.y;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (std::abs(dist - radius) < thickness)
        {
            return GizmoAxis::XY;
        }

        return GizmoAxis::None;
    }

    /*!
     * \brief Hit-tests the scale gizmo handles.
     * \param mousePos Mouse position in screen pixels.
     * \param gizmoPos Gizmo position in screen pixels.
     * \param config Editor configuration.
     * \return The axis that was hit, or GizmoAxis::None.
     */
    GizmoAxis GizmoRenderer::HitTestScaleGizmo(const Vec2& mousePos, const Vec2& gizmoPos, const EditorConfig& config)
    {
        float size = config.gizmoSize * 0.7f;
        float handleSize = config.gizmoHandleSize * 2.0f;

        Vec2 xyHandle(gizmoPos.x + size * 0.7f, gizmoPos.y + size * 0.7f);
        if (std::abs(mousePos.x - xyHandle.x) < handleSize &&
            std::abs(mousePos.y - xyHandle.y) < handleSize)
        {
            return GizmoAxis::XY;
        }

        Vec2 xHandle(gizmoPos.x + size, gizmoPos.y);
        if (std::abs(mousePos.x - xHandle.x) < handleSize &&
            std::abs(mousePos.y - xHandle.y) < handleSize)
        {
            return GizmoAxis::X;
        }

        Vec2 yHandle(gizmoPos.x, gizmoPos.y + size);
        if (std::abs(mousePos.x - yHandle.x) < handleSize &&
            std::abs(mousePos.y - yHandle.y) < handleSize)
        {
            return GizmoAxis::Y;
        }

        return GizmoAxis::None;
    }

    /*!
     * \brief Checks if an entity is a game entity (has Transform but not RectTransform).
     * \param entity Entity to check.
     * \return True if it's a game entity.
     */
    bool GizmoRenderer::IsGameEntity(Uma_ECS::Entity entity) const
    {
        if (!pCoordinator)
            return false;

        auto& transformArray = pCoordinator->GetComponentArray<Uma_ECS::Transform>();
        auto& rectTransformArray = pCoordinator->GetComponentArray<Uma_UI::RectTransform>();

        return transformArray.Has(entity) && !rectTransformArray.Has(entity);
    }

    /*!
     * \brief Checks if an entity is a UI entity (has RectTransform).
     * \param entity Entity to check.
     * \return True if it's a UI entity.
     */
    bool GizmoRenderer::IsUIEntity(Uma_ECS::Entity entity) const
    {
        if (!pCoordinator)
            return false;

        auto& rectTransformArray = pCoordinator->GetComponentArray<Uma_UI::RectTransform>();

        return rectTransformArray.Has(entity);
    }
}