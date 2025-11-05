#pragma once
#include "Math/Math.h"

namespace Uma_Engine
{
    class HybridInputSystem;

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

        // Update camera
        void Update(HybridInputSystem* input, float dt);

        // Reset camera
        void Reset();

        // Getters
        Vec2 GetPosition() const { return m_Position; }
        float GetZoom() const { return m_Zoom; }
        bool IsActive() const { return m_IsActive; }

        // Setters
        void SetPosition(const Vec2& pos) { m_Position = pos; }
        void SetZoom(float zoom);
        void SetActive(bool active) { m_IsActive = active; }

        // Camera settings
        void SetPanSpeed(float speed) { m_PanSpeed = speed; }
        void SetZoomSpeed(float speed) { m_ZoomSpeed = speed; }
        void SetZoomLimits(float minZoom, float maxZoom)
        {
            m_MinZoom = minZoom;
            m_MaxZoom = maxZoom;
        }

    private:
        Vec2 m_Position;
        float m_Zoom;
        float m_PanSpeed;
        float m_ZoomSpeed;
        float m_MinZoom;
        float m_MaxZoom;
        bool m_IsActive;
    };
}