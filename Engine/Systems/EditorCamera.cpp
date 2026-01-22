/*!
\file    EditorCamera.cpp
\par     Project: GAM200
\par     Course: CSD2401
\par     Section A
\par     Software Engineering Project 3

\author Javier Chua Dong Qing (100%)
\par     E-mail: javierdongqing.chua@digipen.edu
\par     DigiPen login: javierdongqing.chua

\brief
Implements the EditorCamera class logic. Handles input polling for
panning (WASD, MMB drag) and zooming (Scroll, Q/E) and update the
camera position and zoom

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "EditorCamera.h"
#include "Systems/HybridInputSystem.h"
#include "UI/Core/InputFilter.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace Uma_Engine
{
    void EditorCamera::Update(HybridInputSystem* input, float dt)
    {
        //std::cout << "editor cam : " << (m_IsActive ? "turned on" : "turned off") << " " << m_Position << std::endl;
        if (!input || !m_IsActive) return;

        // WASD for panning
        Vec2 movement(0, 0);
        if (input->KeyDown(GLFW_KEY_W)) movement.y += 1;
        if (input->KeyDown(GLFW_KEY_S)) movement.y -= 1;
        if (input->KeyDown(GLFW_KEY_A)) movement.x -= 1;
        if (input->KeyDown(GLFW_KEY_D)) movement.x += 1;

        // Speed boost with shift
        float currentSpeed = m_PanSpeed;
        if (input->KeyDown(GLFW_KEY_LEFT_SHIFT))
        {
            currentSpeed *= 2.0f;
        }

        // Apply movement
        if (movement.x != 0 || movement.y != 0)
        {
            float length = std::sqrt(movement.x * movement.x + movement.y * movement.y);
            if (length > 0)
            {
                movement.x /= length;
                movement.y /= length;
            }

            m_Position += movement * currentSpeed * dt / m_Zoom;
        }

        // Store previous mouse position for delta calculation
        static double prevMouseX = 0;
        static double prevMouseY = 0;

        double mouseX = input->GetMouseX();
        double mouseY = input->GetMouseY();

        // Middle mouse button drag to pan
        if (input->MouseButtonDown(GLFW_MOUSE_BUTTON_MIDDLE))
        {
            double deltaX = mouseX - prevMouseX;
            double deltaY = mouseY - prevMouseY;

            m_Position.x -= static_cast<float>(deltaX) / m_Zoom;
            m_Position.y += static_cast<float>(deltaY) / m_Zoom;
        }

        // Update previous mouse position
        prevMouseX = mouseX;
        prevMouseY = mouseY;

        // Reset camera with R key
        if (input->KeyPressed(GLFW_KEY_R))
        {
            Reset();
            std::cout << "[EditorCamera] Reset to origin" << std::endl;
        }

        // Q to zoom out, E to zoom in
        if (input->KeyDown(GLFW_KEY_Q))
        {
            SetZoom(m_Zoom - m_ZoomSpeed * 5.0f * dt);
        }
        if (input->KeyDown(GLFW_KEY_E))
        {
            SetZoom(m_Zoom + m_ZoomSpeed * 5.0f * dt);
        }


        float scrollDelta = static_cast<float>(input->GetScrollOffsetY());
        if (std::abs(scrollDelta) > 0.01f)
        {
            if (!Uma_UI::InputFilter::IsMouseOverUI())
            {
                SetZoom(m_Zoom + scrollDelta * m_ZoomSpeed);
            }
        }
    }

    void EditorCamera::Reset()
    {
        m_Position = Vec2(0.0f, 0.0f);
        m_Zoom = 10.0f;
    }

    void EditorCamera::SetZoom(float zoom)
    {
        m_Zoom = std::clamp(zoom, m_MinZoom, m_MaxZoom);
    }
}