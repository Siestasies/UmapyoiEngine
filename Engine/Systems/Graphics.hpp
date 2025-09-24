#pragma once
#include "Core/SystemType.h" 
#include "Window.hpp"
#include "Math/Math.h"
#include "ResourcesTypes.hpp"
#include <string>
#include "Systems/Camera.hpp"

// Forward declarations
struct GLFWwindow;
using GLuint = unsigned int;

namespace Uma_Engine
{
    class Graphics : public ISystem, public IWindowSystem
    {
    private:
        bool mInitialized;
        GLFWwindow* mWindow;

        // Camera
        Camera2D mCamera;

        // Rendering resources
        GLuint mVAO, mVBO;
        GLuint mShaderProgram;

        // Viewport size
        int mViewportWidth, mViewportHeight;

        // Helper functions
        bool InitializeRenderer();
        void ShutdownRenderer();
        GLuint CreateShader(const std::string& vertexSource, const std::string& fragmentSource);
        void CheckOpenGLVersion();

        // Window resize callback (used by GLFW)
        void OnWindowResize(int width, int height);
        // Static callback function for GLFW
        static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);

        void UpdateProjectionMatrix();

    public:
        Graphics();
        ~Graphics();

        // ISystem interface
        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;

        // IWindowSystem interface
        void SetWindow(GLFWwindow* window) override;

        // Background operations
        void ClearBackground(float r = 0.0f, float g = 0.0f, float b = 0.0f);

        // Texture loading
        Texture LoadTextureFromFile(const std::string& texturePath);
        void UnloadTexture(unsigned int textureID);

        // Sprite rendering
        void DrawSprite(unsigned int textureID,
            const Vec2& textureSize,
            const Vec2& position,
            const Vec2& scale = Vec2(1.0f, 1.0f),
            float rotation = 0.0f);

        // Draw background image
        void DrawBackground(unsigned int textureID, const Vec2& textureSize);
        void SetVSync(bool enabled);
        void SetViewport(int width, int height);

        // Camera access
        Camera2D& GetCamera() { return mCamera; }
        const Camera2D& GetCamera() const { return mCamera; }

        // Coordinate conversion
        Vec2 ScreenToWorld(const Vec2& screenPos) const;
        Vec2 WorldToScreen(const Vec2& worldPos) const;

        // Debug drawing
        void DrawDebugPoint(const Vec2& position, float r = 1.0f, float g = 0.0f, float b = 0.0f);
        void DrawDebugLine(const Vec2& start, const Vec2& end, float r = 1.0f, float g = 0.0f, float b = 0.0f);
        void DrawDebugRect(const Vec2& center, const Vec2& size, float r = 1.0f, float g = 0.0f, float b = 0.0f);
        void DrawDebugCircle(const Vec2& center, float radius, float r = 1.0f, float g = 0.0f, float b = 0.0f);
    };
}