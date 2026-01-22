/*!
\file   EditorTypes.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines core types, enums, and structures used by the editor system.

This header provides EditorMode, GizmoAxis, EditorConfig, and EditorState structures
that configure and track the editor's runtime behavior.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "Math/Math.h"
#include "ECS/Core/Types.hpp"

#include "../EditorApp/Editor/Core/CommandHistory.h"

#include <optional>

namespace Uma_Engine
{
    /*!
     * \enum EditorMode
     * \brief Current editor manipulation mode.
     */
    enum class EditorMode
    {
        None = 0,
        Translate = 1,
        Rotate = 2,
        Scale = 3
    };

    /*!
     * \enum GizmoAxis
     * \brief Which axis or handle is being manipulated.
     */
    enum class GizmoAxis
    {
        None = 0,
        X = 1,
        Y = 2,
        XY = 3
    };

    /*!
     * \struct EditorConfig
     * \brief Configuration for editor behavior and appearance.
     */
    struct EditorConfig
    {
        float gamePickRadius = 10.0f;
        bool pickUIEntities = true;
        bool pickGameEntities = true;

        float gizmoSize = 50.0f;
        float gizmoLineWidth = 3.0f;
        float gizmoHandleSize = 8.0f;

        Vec3 colorXAxis = Vec3(1.0f, 0.0f, 0.0f);
        Vec3 colorYAxis = Vec3(0.0f, 1.0f, 0.0f);
        Vec3 colorXYHandle = Vec3(0.0f, 0.5f, 1.0f);
        Vec3 colorHighlight = Vec3(1.0f, 1.0f, 0.0f);
        Vec3 colorSelected = Vec3(0.0f, 1.0f, 1.0f);

        float snapGrid = 0.0f;
        bool autoSwitchMode = true;
    };

    /*!
     * \struct EditorState
     * \brief Runtime state of the editor.
     */
    struct EditorState
    {
        std::optional<Uma_ECS::Entity> pickedEntity;
        EditorMode currentMode = EditorMode::Translate;
        bool isDragging = false;
        GizmoAxis activeAxis = GizmoAxis::None;
        Vec2 dragStartMouse{};
        Vec2 dragStartPosition{};
        float dragStartRotation = 0.0f;
        Vec2 dragStartScale{};
        Vec2 dragPrevMouse{};
        bool enabled = true;
    };
}