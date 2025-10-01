/*!
\file   Graphics.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Javier Chua Dong Qing (100%)
\par    E-mail: javierdongqing.chua@digipen.edu
\par    DigiPen login: javierdongqing.chua

\brief
Manages OpenGL resources, shaders, and rendering operations for 2D sprite-based
graphics.

Inherits from ISystem for engine management and IWindowSystem for
GLFW window integration and resize handling.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once
#include "Core/SystemType.h" 
#include "Window.hpp"
#include "Math/Math.h"
#include "ResourcesTypes.hpp"
#include <string>
//#include "Systems/TMP_CameraSystem.hpp"

#include "Core/Coordinator.hpp"
#include "Collider.h"

// Forward declarations
struct GLFWwindow;
using GLuint = unsigned int;

namespace Uma_Engine
{
    struct Cam_Info
    {
        Vec2 pos{};
        float zoom{};
    };

    struct Sprite_Info
    {
        GLuint tex_id{};
        Vec2 tex_size{};

        Vec2 pos{};
        Vec2 scale{};
        float rot{};
        float rot_speed{};
    };

    class Graphics : public ISystem, public IWindowSystem
    {
    private:
        bool mInitialized;
        GLFWwindow* mWindow;

        // gCoordinator
        //Camera2D mCamera;
        //mat4 mprojectionMatrix;
        Cam_Info cam;

        // Rendering resources
        GLuint mVAO, mVBO;
        GLuint mShaderProgram;

        // Instanced rendering resources
        GLuint mInstanceVBO;
        GLuint mInstanceVAO;
        GLuint mInstanceShaderProgram;

        // Viewport size
        int mViewportWidth, mViewportHeight;

        // Helper functions
        bool InitializeRenderer();
        void ShutdownRenderer();
        bool InitializeInstancedRenderer();
        void ShutdownInstancedRenderer();
        GLuint CreateShader(const std::string& vertexSource, const std::string& fragmentSource);
        //void CheckOpenGLVersion();

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

        // setting cam info
        void SetCamInfo(const Vec2& pos, float zoom);

        // Sprite rendering
        void DrawSprite(unsigned int textureID,
            const Vec2& position,
            const Vec2& scale = Vec2(1.0f, 1.0f),
            float rotation = 0.0f);

        void DrawSpritesInstanced(
            unsigned int textureID,
            //const Vec2& textureSize,
            std::vector<Sprite_Info> const& sprites);

        // Draw background image
        void DrawBackground(unsigned int textureID);
        void SetVSync(bool enabled);
        void SetViewport(int width, int height);

        // Camera access
        /*Camera2D& GetCamera() { return mCamera; }
        const Camera2D& GetCamera() const { return mCamera; }*/

        // Coordinate conversion
        Vec2 ScreenToWorld(const Vec2& screenPos) const;
        Vec2 WorldToScreen(const Vec2& worldPos) const;

        // Debug drawing
        void DrawDebugPoint(const Vec2& position, float r = 1.0f, float g = 0.0f, float b = 0.0f);
        void DrawDebugLine(const Vec2& start, const Vec2& end, float r = 1.0f, float g = 0.0f, float b = 0.0f);
        void DrawDebugRect(const Vec2& center, const Vec2& size, float r = 1.0f, float g = 0.0f, float b = 0.0f);
        void DrawDebugRect(const Uma_ECS::BoundingBox& bbox, float r = 1.0f, float g = 0.0f, float b = 0.0f);
        void DrawDebugCircle(const Vec2& center, float radius, float r = 1.0f, float g = 0.0f, float b = 0.0f);
    };
}