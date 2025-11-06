#include "../../ECS/Components/Transform.h"
#include "../../ECS/Components/Sprite.h"
#include "../../UI/Components/RectTransform.h"
#include "../../UI/Helpers/Input.h"
#include "../../UI/Helpers/Layout.h"
#include "../Core/EditorMath.h"
#include "GizmoRenderer.h"

#include <cmath>

namespace Uma_Engine
{
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

    void GizmoRenderer::RenderSelectionHighlight(Uma_ECS::Entity entity, const EditorConfig& config)
    {
        if (!pCoordinator || !pGraphics)
            return;

        std::vector<DebugLineInfo> lines;

        auto& transformArray = pCoordinator->GetComponentArray<Uma_ECS::Transform>();
        auto& spriteArray = pCoordinator->GetComponentArray<Uma_ECS::Sprite>();
        auto& rectTransformArray = pCoordinator->GetComponentArray<Uma_UI::RectTransform>();

        // Highlight game entities (already in world space)
        if (transformArray.Has(entity) && !rectTransformArray.Has(entity))
        {
            const auto& transform = transformArray.GetData(entity);

            Vec2 size(50.0f, 50.0f);  // Default
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
        // Highlight UI entities (convert NDC to world space)
        else if (rectTransformArray.Has(entity))
        {
            const auto& rectTransform = rectTransformArray.GetData(entity);

            // Convert NDC bounding box to screen space first
            int screenWidth = pGraphics->GetViewportWidth();
            int screenHeight = pGraphics->GetViewportHeight();

            Vec2 ndcCenter(rectTransform.computedRect.x, rectTransform.computedRect.y);
            Vec2 ndcSize(rectTransform.computedRect.width, rectTransform.computedRect.height);

            float screenHalfWidth = ndcSize.x * screenWidth * 0.5f;
            float screenHalfHeight = ndcSize.y * screenHeight * 0.5f;

            // Convert NDC center to screen coordinates
            Vec2 screenCenter = Uma_UI::NDCToScreen(
                ndcCenter.x, ndcCenter.y,
                static_cast<float>(screenWidth),
                static_cast<float>(screenHeight)
            );

            // Get world space corners for proper sizing
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

        // Draw all lines at once
        if (!lines.empty())
        {
            pGraphics->DrawDebugLinesInstanced(lines);
        }
    }

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

    Vec2 GizmoRenderer::GetEntityScreenPosition(Uma_ECS::Entity entity)
    {
        if (!pCoordinator || !pGraphics)
            return Vec2(0.0f, 0.0f);

        auto& transformArray = pCoordinator->GetComponentArray<Uma_ECS::Transform>();
        auto& rectTransformArray = pCoordinator->GetComponentArray<Uma_UI::RectTransform>();

        if (transformArray.Has(entity) && !rectTransformArray.Has(entity))
        {
            // Game entities: world space ? screen space
            const auto& transform = transformArray.GetData(entity);
            return pGraphics->WorldToScreen(transform.worldPosition);
        }
        else if (rectTransformArray.Has(entity))
        {
            // UI entities: NDC ? screen space
            const auto& rectTransform = rectTransformArray.GetData(entity);

            int screenWidth = pGraphics->GetViewportWidth();
            int screenHeight = pGraphics->GetViewportHeight();

            // computedRect stores center position in NDC (-1 to +1)
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

    void GizmoRenderer::RenderTranslateGizmo(const Vec2& screenPos, const EditorState& state, const EditorConfig& config)
    {
        if (!pGraphics)
            return;

        float size = config.gizmoSize;
        float handleSize = config.gizmoHandleSize;

        std::vector<DebugLineInfo> lines;

        // X axis (horizontal red line)
        Vec3 xColor = (state.activeAxis == GizmoAxis::X) ? config.colorHighlight : config.colorXAxis;
        Vec2 xStart = pGraphics->ScreenToWorld(screenPos);
        Vec2 xEnd = pGraphics->ScreenToWorld(Vec2(screenPos.x + size, screenPos.y));
        lines.push_back({ xStart, xEnd, xColor });

        // X handle
        Vec2 xHandleWorld = pGraphics->ScreenToWorld(Vec2(screenPos.x + size, screenPos.y));
        Graphics::AddDebugRect(xHandleWorld, Vec2(handleSize * 0.02f, handleSize * 0.02f),
            xColor.x, xColor.y, xColor.z, lines);

        // Y axis (vertical green line)
        Vec3 yColor = (state.activeAxis == GizmoAxis::Y) ? config.colorHighlight : config.colorYAxis;
        Vec2 yStart = pGraphics->ScreenToWorld(screenPos);
        Vec2 yEnd = pGraphics->ScreenToWorld(Vec2(screenPos.x, screenPos.y + size));
        lines.push_back({ yStart, yEnd, yColor });

        // Y handle
        Vec2 yHandleWorld = pGraphics->ScreenToWorld(Vec2(screenPos.x, screenPos.y + size));
        Graphics::AddDebugRect(yHandleWorld, Vec2(handleSize * 0.02f, handleSize * 0.02f),
            yColor.x, yColor.y, yColor.z, lines);

        // Center handle (blue square for XY movement)
        Vec3 xyColor = (state.activeAxis == GizmoAxis::XY) ? config.colorHighlight : config.colorXYHandle;
        Vec2 centerWorld = pGraphics->ScreenToWorld(screenPos);
        Graphics::AddDebugRect(centerWorld, Vec2(handleSize * 0.03f, handleSize * 0.03f),
            xyColor.x, xyColor.y, xyColor.z, lines);

        // Draw all lines at once
        pGraphics->DrawDebugLinesInstanced(lines);
    }

    void GizmoRenderer::RenderRotateGizmo(const Vec2& screenPos, const EditorState& state, const EditorConfig& config)
    {
        if (!pGraphics)
            return;

        float radius = config.gizmoSize;
        Vec3 color = state.isDragging ? config.colorHighlight : config.colorSelected;

        std::vector<DebugLineInfo> lines;
        Vec2 centerWorld = pGraphics->ScreenToWorld(screenPos);
        Graphics::AddDebugCircle(centerWorld, radius * 0.01f, color.x, color.y, color.z, lines);

        // Draw all lines at once
        pGraphics->DrawDebugLinesInstanced(lines);
    }

    void GizmoRenderer::RenderScaleGizmo(const Vec2& screenPos, const EditorState& state, const EditorConfig& config)
    {
        if (!pGraphics)
            return;

        float size = config.gizmoSize * 0.7f;
        float handleSize = config.gizmoHandleSize * 1.5f;

        std::vector<DebugLineInfo> lines;

        // X axis
        Vec3 xColor = (state.activeAxis == GizmoAxis::X) ? config.colorHighlight : config.colorXAxis;
        Vec2 xStart = pGraphics->ScreenToWorld(screenPos);
        Vec2 xEnd = pGraphics->ScreenToWorld(Vec2(screenPos.x + size, screenPos.y));
        lines.push_back({ xStart, xEnd, xColor });

        Vec2 xHandleWorld = pGraphics->ScreenToWorld(Vec2(screenPos.x + size, screenPos.y));
        Graphics::AddDebugRect(xHandleWorld, Vec2(handleSize * 0.02f, handleSize * 0.02f),
            xColor.x, xColor.y, xColor.z, lines);

        // Y axis
        Vec3 yColor = (state.activeAxis == GizmoAxis::Y) ? config.colorHighlight : config.colorYAxis;
        Vec2 yStart = pGraphics->ScreenToWorld(screenPos);
        Vec2 yEnd = pGraphics->ScreenToWorld(Vec2(screenPos.x, screenPos.y + size));
        lines.push_back({ yStart, yEnd, yColor });

        Vec2 yHandleWorld = pGraphics->ScreenToWorld(Vec2(screenPos.x, screenPos.y + size));
        Graphics::AddDebugRect(yHandleWorld, Vec2(handleSize * 0.02f, handleSize * 0.02f),
            yColor.x, yColor.y, yColor.z, lines);

        // Uniform scale handle (corner)
        Vec3 xyColor = (state.activeAxis == GizmoAxis::XY) ? config.colorHighlight : config.colorXYHandle;
        Vec2 xyHandleWorld = pGraphics->ScreenToWorld(Vec2(screenPos.x + size * 0.7f, screenPos.y + size * 0.7f));
        Graphics::AddDebugRect(xyHandleWorld, Vec2(handleSize * 0.025f, handleSize * 0.025f),
            xyColor.x, xyColor.y, xyColor.z, lines);

        // Draw all lines at once
        pGraphics->DrawDebugLinesInstanced(lines);
    }

    GizmoAxis GizmoRenderer::HitTestTranslateGizmo(const Vec2& mousePos, const Vec2& gizmoPos, const EditorConfig& config)
    {
        float size = config.gizmoSize;
        float handleSize = config.gizmoHandleSize * 2.0f;

        // Check center handle first (XY)
        if (std::abs(mousePos.x - gizmoPos.x) < handleSize &&
            std::abs(mousePos.y - gizmoPos.y) < handleSize)
        {
            return GizmoAxis::XY;
        }

        // Check X handle
        Vec2 xHandle(gizmoPos.x + size, gizmoPos.y);
        if (std::abs(mousePos.x - xHandle.x) < handleSize &&
            std::abs(mousePos.y - xHandle.y) < handleSize)
        {
            return GizmoAxis::X;
        }

        // Check Y handle
        Vec2 yHandle(gizmoPos.x, gizmoPos.y + size);
        if (std::abs(mousePos.x - yHandle.x) < handleSize &&
            std::abs(mousePos.y - yHandle.y) < handleSize)
        {
            return GizmoAxis::Y;
        }

        // Check X axis line
        Vec2 xStart = gizmoPos;
        Vec2 xEnd(gizmoPos.x + size, gizmoPos.y);
        if (DistanceToLineSegment(mousePos, xStart, xEnd) < handleSize * 0.5f)
        {
            return GizmoAxis::X;
        }

        // Check Y axis line
        Vec2 yStart = gizmoPos;
        Vec2 yEnd(gizmoPos.x, gizmoPos.y + size);
        if (DistanceToLineSegment(mousePos, yStart, yEnd) < handleSize * 0.5f)
        {
            return GizmoAxis::Y;
        }

        return GizmoAxis::None;
    }

    GizmoAxis GizmoRenderer::HitTestRotateGizmo(const Vec2& mousePos, const Vec2& gizmoPos, const EditorConfig& config)
    {
        float radius = config.gizmoSize;
        float thickness = config.gizmoHandleSize;

        float dx = mousePos.x - gizmoPos.x;
        float dy = mousePos.y - gizmoPos.y;
        float dist = std::sqrt(dx * dx + dy * dy);

        // Check if near circle
        if (std::abs(dist - radius) < thickness)
        {
            return GizmoAxis::XY;
        }

        return GizmoAxis::None;
    }

    GizmoAxis GizmoRenderer::HitTestScaleGizmo(const Vec2& mousePos, const Vec2& gizmoPos, const EditorConfig& config)
    {
        float size = config.gizmoSize * 0.7f;
        float handleSize = config.gizmoHandleSize * 2.0f;

        // Check uniform scale handle (corner)
        Vec2 xyHandle(gizmoPos.x + size * 0.7f, gizmoPos.y + size * 0.7f);
        if (std::abs(mousePos.x - xyHandle.x) < handleSize &&
            std::abs(mousePos.y - xyHandle.y) < handleSize)
        {
            return GizmoAxis::XY;
        }

        // Check X handle
        Vec2 xHandle(gizmoPos.x + size, gizmoPos.y);
        if (std::abs(mousePos.x - xHandle.x) < handleSize &&
            std::abs(mousePos.y - xHandle.y) < handleSize)
        {
            return GizmoAxis::X;
        }

        // Check Y handle
        Vec2 yHandle(gizmoPos.x, gizmoPos.y + size);
        if (std::abs(mousePos.x - yHandle.x) < handleSize &&
            std::abs(mousePos.y - yHandle.y) < handleSize)
        {
            return GizmoAxis::Y;
        }

        return GizmoAxis::None;
    }

    bool GizmoRenderer::IsGameEntity(Uma_ECS::Entity entity) const
    {
        if (!pCoordinator)
            return false;

        auto& transformArray = pCoordinator->GetComponentArray<Uma_ECS::Transform>();
        auto& rectTransformArray = pCoordinator->GetComponentArray<Uma_UI::RectTransform>();

        // Game entity: has Transform but NOT RectTransform
        return transformArray.Has(entity) && !rectTransformArray.Has(entity);
    }

    bool GizmoRenderer::IsUIEntity(Uma_ECS::Entity entity) const
    {
        if (!pCoordinator)
            return false;

        auto& rectTransformArray = pCoordinator->GetComponentArray<Uma_UI::RectTransform>();

        // UI entity: has RectTransform
        return rectTransformArray.Has(entity);
    }
}