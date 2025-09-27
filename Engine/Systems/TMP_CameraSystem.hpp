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