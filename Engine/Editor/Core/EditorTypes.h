#pragma once

#include "../../Math/Math.h"
#include "../../ECS/Core/Types.hpp"

#include <optional>

namespace Uma_Engine
{
    /**
     * \enum EditorMode
     * \brief Current editor manipulation mode
     */
    enum class EditorMode
    {
        None = 0,      // No editing mode active
        Translate = 1, // Move entity
        Rotate = 2,    // Rotate entity
        Scale = 3      // Scale entity
    };

    /**
     * \enum GizmoAxis
     * \brief Which axis is being manipulated
     */
    enum class GizmoAxis
    {
        None = 0,
        X = 1,
        Y = 2,
        XY = 3  // Center handle for 2D movement
    };

    /**
     * \struct EditorConfig
     * \brief Configuration for editor behavior and appearance
     */
    struct EditorConfig
    {
        // === Picking Settings ===
        float gamePickRadius = 10.0f;  // Pixels around cursor to consider for game entities
        bool pickUIEntities = true;
        bool pickGameEntities = true;

        // === Gizmo Appearance ===
        float gizmoSize = 50.0f;        // Gizmo size in pixels
        float gizmoLineWidth = 3.0f;
        float gizmoHandleSize = 8.0f;

        // === Gizmo Colors ===
        Vec3 colorXAxis = Vec3(1.0f, 0.0f, 0.0f);      // Red
        Vec3 colorYAxis = Vec3(0.0f, 1.0f, 0.0f);      // Green
        Vec3 colorXYHandle = Vec3(0.0f, 0.5f, 1.0f);   // Blue
        Vec3 colorHighlight = Vec3(1.0f, 1.0f, 0.0f);  // Yellow (active)
        Vec3 colorSelected = Vec3(0.0f, 1.0f, 1.0f);   // Cyan (selection)

        // === Behavior Settings ===
        float snapGrid = 0.0f;          // 0 = no snapping, >0 = snap to grid
        bool autoSwitchMode = true;     // Auto-switch to translate on pick
    };

    /**
     * \struct EditorState
     * \brief Runtime state of the editor
     */
    struct EditorState
    {
        // Current selection
        std::optional<Uma_ECS::Entity> pickedEntity;

        // Current mode
        EditorMode currentMode = EditorMode::Translate;

        // Interaction state
        bool isDragging = false;
        GizmoAxis activeAxis = GizmoAxis::None;

        // Drag start values (for relative manipulation)
        Vec2 dragStartMouse{};
        Vec2 dragStartPosition{};
        float dragStartRotation = 0.0f;
        Vec2 dragStartScale{};
        Vec2 dragPrevMouse{};

        // System state
        bool enabled = true;
    };
}
