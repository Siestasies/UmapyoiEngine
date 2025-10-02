/*!
\file   Tmp_CameraSystem.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Javier Chua Dong Qing (100%)
\par    E-mail: javierdongqing.chua@digipen.edu
\par    DigiPen login: javierdongqing.chua

\brief
Simple 2D camera with position and zoom control. Generates orthographic
projection matrices centered on the camera position with zoom applied uniformly.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "Systems/TMP_CameraSystem.hpp"
#include <algorithm>

namespace Uma_Engine
{
    Camera2D::Camera2D() : mPosition(0.0f, 0.0f), mZoom(1.0f) {}

    Camera2D::Camera2D(const Vec2& position, float zoom) : mPosition(position), mZoom(std::max(0.1f, std::min(10.0f, zoom))) {}

    glm::mat4 Camera2D::GetViewProjectionMatrix(int viewportWidth, int viewportHeight) const
    {
        float halfWidth = (viewportWidth * 0.5f) / mZoom;
        float halfHeight = (viewportHeight * 0.5f) / mZoom;

        float left = mPosition.x - halfWidth;
        float right = mPosition.x + halfWidth;
        float bottom = mPosition.y - halfHeight;
        float top = mPosition.y + halfHeight;

        return glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
    }
}