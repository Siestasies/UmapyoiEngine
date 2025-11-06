/*!
\file    EditorCamera.h
\par     Project: GAM200
\par     Course: CSD2401
\par     Section A
\par     Software Engineering Project 3

\author Javier Chua Dong Qing (100%)
\par     E-mail: javierdongqing.chua@digipen.edu
\par     DigiPen login: javierdongqing.chua

\brief
Defines the EditorCamera class, a 2D camera controller for use within
the editor environment. It provides functionality for panning (WASD,
middle-mouse drag) and zooming (Mouse scroll, Q/E keys)

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once
#include "Math/Math.h"

namespace Uma_Engine
{
    // Forward declaration
    class HybridInputSystem;

    /**
     * \class EditorCamera
     * \brief A 2D camera controller for the editor
     */
    class EditorCamera
    {
    public:
        EditorCamera()
            : m_Position(0.0f, 0.0f)
            , m_Zoom(10.0f)
            , m_PanSpeed(500.0f)
            , m_ZoomSpeed(1.0f)
            , m_MinZoom(0.1f)
            , m_MaxZoom(20.0f)
            , m_IsActive(false)
        {
        }

        /**
         * \brief Updates camera position and zoom based on input
         * \param input Pointer to the HybridInputSystem
         * \param dt Delta time
         */
        void Update(HybridInputSystem* input, float dt);

        /**
         * \brief Resets the camera to default position and zoom
         */
        void Reset();

        // Getters
        /**
         * \brief Gets the current camera position in world space
         * \return Vec2 The camera's center position
         */
        Vec2 GetPosition() const { return m_Position; }

        /**
         * \brief Gets the current camera zoom level
         * \return float The zoom level
         */
        float GetZoom() const { return m_Zoom; }

        /**
         * \brief Checks if the editor camera controls are active
         * \return true if the camera is active and processing input
         */
        bool IsActive() const { return m_IsActive; }

        // Setters
        /**
         * \brief Sets the camera's world space position
         * \param pos The new center position for the camera
         */
        void SetPosition(const Vec2& pos) { m_Position = pos; }

        /**
         * \brief Sets the camera's zoom level and clamping it within min/max limits
         * \param zoom The new zoom level
         */
        void SetZoom(float zoom);

        /**
         * \brief Activates or deactivates the editor camera controls
         * \param active The new active state
         */
        void SetActive(bool active) { m_IsActive = active; }

        // Camera settings
        /**
         * \brief Sets the camera's panning speed
         * \param speed The speed for WASD and middle-mouse panning
         */
        void SetPanSpeed(float speed) { m_PanSpeed = speed; }

        /**
         * \brief Sets the camera's zoom speed
         * \param speed The speed/sensitivity for scroll wheel and Q/E zooming
         */
        void SetZoomSpeed(float speed) { m_ZoomSpeed = speed; }

        /**
         * \brief Sets the minimum and maximum zoom levels
         * \param minZoom The minimum zoom value
         * \param maxZoom The maximum zoom value
         */
        void SetZoomLimits(float minZoom, float maxZoom)
        {
            m_MinZoom = minZoom;
            m_MaxZoom = maxZoom;
        }

    private:
        Vec2 m_Position;   // Center position of camera in world space
        float m_Zoom;      // Zoom level
        float m_PanSpeed;  // Camera move speed
        float m_ZoomSpeed; // Camera zoom speed
        float m_MinZoom;   // Minimum zoom level
        float m_MaxZoom;   // Maximum zoom level
        bool m_IsActive;   // If true, camera will respond to input
    };
}