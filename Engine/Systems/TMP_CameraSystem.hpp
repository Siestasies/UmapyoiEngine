/*!
\file   TMP_CameraSystem.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Javier Chua Dong Qing (100%)
\par    E-mail: javierdongqing.chua@digipen.edu
\par    DigiPen login: javierdongqing.chua

\brief
Class for 2D camera state (position and zoom) to generate view-projection matrices.
The camera centers the view on its position and scales the visible area based on zoom level.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once
#include "Math/Math.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Uma_Engine
{
	class Camera2D
    {
    public:
        Camera2D();
        Camera2D(const Vec2& position, float zoom = 1.0f);

        void SetPosition(const Vec2& position) { mPosition = position; }
        Vec2 GetPosition() const { return mPosition; }

        void SetZoom(float zoom) { mZoom = std::max(0.1f, std::min(10.0f, zoom)); }
        float GetZoom() const { return mZoom; }

        glm::mat4 GetViewProjectionMatrix(int viewportWidth, int viewportHeight) const;

    private:
        Vec2 mPosition;
        float mZoom;
    };
}