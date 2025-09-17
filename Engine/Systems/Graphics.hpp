#pragma once

#include "Core/SystemType.h" // Include the system interface
#include "Window.hpp"
#include "Math/Math.hpp"

#include <string>
#include <map>

// Forward declarations
struct GLFWwindow;          // from <GLFW/glfw3.h>
using GLuint = unsigned int; // from <glad/glad.h> (GLuint is just a typedef)

// Forward declare glm types you use
//namespace glm { struct vec2; }

namespace Uma_Engine
{
    class Graphics : public ISystem, public IWindowSystem
    {
    public:
        Graphics();
        ~Graphics();

        // ISystem interface implementation
        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;

        // IWindowSystem interface implementation
        void SetWindow(GLFWwindow* window) override;

        // Background operations
        void ClearBackground(float r = 0.0f, float g = 0.0f, float b = 0.0f);

        // Texture management
        unsigned int LoadTexture(const std::string& texturePath);
        void UnloadTexture(unsigned int textureID);

        // Sprite rendering
        void DrawSprite(unsigned int textureID,
            const Vec2& position,
            const Vec2& scale = Vec2(1.0f, 1.0f),
            float rotation = 0.0f);

        // Draw background that covers entire viewport
        void DrawBackground(unsigned int textureID);

        // Get texture dimensions
        Vec2 GetTextureSize(unsigned int textureID) const;

        void SetVSync(bool enabled);
        void SetViewport(int width, int height);

    private:
        bool mInitialized;
        GLFWwindow* mWindow; // Store window reference

        // Rendering resources
        GLuint mVAO, mVBO;
        GLuint mShaderProgram;

        // Viewport size
        int mViewportWidth, mViewportHeight;

        // Texture size storage
        std::map<unsigned int, Vec2> mTextureSizes;

        // Helper functions
        bool InitializeRenderer();
        void ShutdownRenderer();
        GLuint CreateShader(const std::string& vertexSource, const std::string& fragmentSource);
        void CheckOpenGLVersion();

        // Window resize callback (used by GLFW)
        void OnWindowResize(int width, int height);
        // Static callback function for GLFW
        static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    };
}