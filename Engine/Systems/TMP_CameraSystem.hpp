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
        /**
         * \brief Default constructor
         *
         * Initializes camera at origin (0,0) with default zoom (1.0)
         */
        Camera2D();

        /**
         * \brief Constructs camera with position and zoom
         * \param position Initial camera position in world space
         * \param zoom Initial zoom level
         */
        Camera2D(const Vec2& position, float zoom = 1.0f);

        /**
         * \brief Sets the camera position
         * \param position New camera position in world space
         */
        void SetPosition(const Vec2& position) { mPosition = position; }

        /**
         * \brief Gets the current camera position
         * \return Camera position in world space
         */
        Vec2 GetPosition() const { return mPosition; }

        /**
         * \brief Sets the camera zoom level
         * \param zoom New zoom level
         */
        void SetZoom(float zoom) { mZoom = std::max(0.1f, std::min(10.0f, zoom)); }

        /**
         * \brief Gets the current zoom level
         * \return Current zoom level
         */
        float GetZoom() const { return mZoom; }

        /**
         * \brief Calculates the view-projection matrix for rendering
         * \param viewportWidth Width of the viewport in pixels
         * \param viewportHeight Height of the viewport in pixels
         * \return Orthographic projection matrix
         *
         * Generates an orthographic projection matrix based on camera
         */
        glm::mat4 GetViewProjectionMatrix(int viewportWidth, int viewportHeight) const;

    private:
        Vec2 mPosition;
        float mZoom;
    };
}