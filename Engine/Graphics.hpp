#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <map>
#include "Core/SystemType.h" // Include the system interface

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
            const glm::vec2& position,
            const glm::vec2& scale = glm::vec2(1.0f),
            float rotation = 0.0f);

        // Draw background that covers entire viewport
        void DrawBackground(unsigned int textureID);

        // Get texture dimensions
        glm::vec2 GetTextureSize(unsigned int textureID) const;

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
        std::map<unsigned int, glm::vec2> mTextureSizes;

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