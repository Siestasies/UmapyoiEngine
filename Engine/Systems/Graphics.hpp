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
    /**
     * \struct Cam_Info
     * \brief Camera information containing position and zoom level
     */
    struct Cam_Info
    {
        Vec2 pos{};
        float zoom{};
    };

    /**
     * \struct Sprite_Info
     * \brief Sprite rendering information
     */
    struct Sprite_Info
    {
        GLuint tex_id{};
        //Vec2 tex_size{};

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

        /**
         * \brief Initializes the basic 2D sprite renderer
         * \return true if initialization succeeded, false otherwise
         *
         * Create shader program, set up quad vertices and configure VAO/VBO
         */
        bool InitializeRenderer();

        /**
         * \brief Shuts down the basic 2D renderer and releases resources
         *
         * Deletes VAO, VBO, and shader program
         */
        void ShutdownRenderer();

        /**
         * \brief Initializes the instanced sprite renderer
         * \return true if initialization succeeded, false otherwise
         *
         * Creates instanced shader program and sets up instance buffer
         */
        bool InitializeInstancedRenderer();

        /**
         * \brief Shuts down the instanced renderer and releases resources
         *
         * Deletes instance VAO, VBO, and shader program
         */
        void ShutdownInstancedRenderer();

        /**
         * \brief Compiles and links a shader program from vertex and fragment source
         * \param vertexSource Vertex shader source code
         * \param fragmentSource Fragment shader source code
         * \return OpenGL shader program ID, 0 if failed
         */
        GLuint CreateShader(const std::string& vertexSource, const std::string& fragmentSource);
        //void CheckOpenGLVersion();

        /**
         * \brief Handles window resize events
         * \param width New window width in pixels
         * \param height New window height in pixels
         *
         * Updates viewport and projection matrix when window is resized
         */
        void OnWindowResize(int width, int height);

        /**
         * \brief GLFW callback for framebuffer size changes
         * \param window GLFW window
         * \param width New framebuffer width
         * \param height New framebuffer height
         */
        static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);

        /**
         * \brief Updates the projection matrix based on camera
         *
         * Calculate orthographic projection using current camera position and zoom
         */
        void UpdateProjectionMatrix();

    public:
        /**
         * \brief Default constructor for Graphics system
         */
        Graphics();

        /**
         * \brief Destructor for cleanup
         */
        ~Graphics();

        // ISystem interface

        /**
         * \brief Initializes the graphics system 
         * OpenGL context, enable blending, initialize renderers, viewport and camera
         */
        void Init() override;

        /**
         * \brief Updates the graphics system each frame
         * \param dt Delta time
         *
         * Handles viewport resize and updates projection matrix
         */
        void Update(float dt) override;

        /**
         * \brief Shuts down the graphics system
         *
         * Release all rendering resources and marks system as uninitialized
         */
        void Shutdown() override;

        // IWindowSystem interface

        /**
         * \brief Sets GLFW window for rendering
         * \param window Pointer to GLFWwindow
         *
         * Associates the graphics system with a window and set resize callback
         */
        void SetWindow(GLFWwindow* window) override;

        // Background operations

        /**
         * \brief Clears the screen with specified color
         * \param r Red component (0.0 to 1.0)
         * \param g Green component (0.0 to 1.0)
         * \param b Blue component (0.0 to 1.0)
         */
        void ClearBackground(float r = 0.0f, float g = 0.0f, float b = 0.0f);

        // Texture loading

        /**
         * \brief Loads a texture from file
         * \param texturePath File path to the texture image
         * \return Texture struct containing texture ID and size
         */
        Texture LoadTextureFromFile(const std::string& texturePath);

        /**
         * \brief Unloads and deletes a texture
         * \param textureID OpenGL texture ID to delete
         */
        void UnloadTexture(unsigned int textureID);

        /**
         * \brief Sets the camera position and zoom level
         * \param pos Camera position in world space
         * \param zoom Camera zoom (1.0 = normal)
         */
        void SetCamInfo(const Vec2& pos, float zoom);

        // Sprite rendering

        /**
         * \brief Draws a single sprite
         * \param textureID OpenGL texture ID
         * \param position Sprite position in world space
         * \param scale Sprite scale
         * \param rotation Sprite rotation in degrees
         */
        void DrawSprite(unsigned int textureID,
            const Vec2& position,
            const Vec2& scale = Vec2(1.0f, 1.0f),
            float rotation = 0.0f);

        /**
         * \brief Draws multiple sprites in a single instanced draw call
         * \param textureID OpenGL texture ID shared by all sprites
         * \param sprites Vector of sprite information
         */
        void DrawSpritesInstanced(
            unsigned int textureID,
            //const Vec2& textureSize,
            std::vector<Sprite_Info> const& sprites);

        // Draw background image

        /**
         * \brief Draws a fullscreen background image
         * \param textureID OpenGL texture ID
         *
         * Renders texture as fullscreen quad in NDC
         */
        void DrawBackground(unsigned int textureID);

        /**
         * \brief Enables or disables VSync
         * \param enabled true to enable VSync, false to disable
         */
        void SetVSync(bool enabled);

        /**
         * \brief Sets the rendering viewport size
         * \param width Viewport width in pixels
         * \param height Viewport height in pixels
         */
        void SetViewport(int width, int height);

        // Camera access
        /*Camera2D& GetCamera() { return mCamera; }
        const Camera2D& GetCamera() const { return mCamera; }*/

        // Coordinate conversion

        /**
         * \brief Converts screen coordinates to world space
         * \param screenPos Screen position in pixels
         * \return World space position
         */
        Vec2 ScreenToWorld(const Vec2& screenPos) const;

        /**
         * \brief Converts world coordinates to screen space
         * \param worldPos World space position
         * \return Screen position in pixels
         */
        Vec2 WorldToScreen(const Vec2& worldPos) const;

        // Debug drawing

        /**
         * \brief Draws a point
         * \param position Point position in world space
         * \param r Red component (0.0 to 1.0)
         * \param g Green component (0.0 to 1.0)
         * \param b Blue component (0.0 to 1.0)
         */
        void DrawDebugPoint(const Vec2& position, float r = 1.0f, float g = 0.0f, float b = 0.0f);

        /**
         * \brief Draws a line
         * \param start Line start position in world space
         * \param end Line end position in world space
         * \param r Red component (0.0 to 1.0)
         * \param g Green component (0.0 to 1.0)
         * \param b Blue component (0.0 to 1.0)
         */
        void DrawDebugLine(const Vec2& start, const Vec2& end, float r = 1.0f, float g = 0.0f, float b = 0.0f);

        /**
         * \brief Draws a rectangle
         * \param center Rectangle center in world space
         * \param size Rectangle dimensions
         * \param r Red component (0.0 to 1.0)
         * \param g Green component (0.0 to 1.0)
         * \param b Blue component (0.0 to 1.0)
         */
        void DrawDebugRect(const Vec2& center, const Vec2& size, float r = 1.0f, float g = 0.0f, float b = 0.0f);

        /**
         * \brief Draws a rectangle using bounding box
         * \param bbox Bounding box to draw
         * \param r Red component (0.0 to 1.0)
         * \param g Green component (0.0 to 1.0)
         * \param b Blue component (0.0 to 1.0)
         */
        void DrawDebugRect(const Uma_ECS::BoundingBox& bbox, float r = 1.0f, float g = 0.0f, float b = 0.0f);

        /**
         * \brief Draws a circle
         * \param center Circle center in world space
         * \param radius Circle radius
         * \param r Red component (0.0 to 1.0)
         * \param g Green component (0.0 to 1.0)
         * \param b Blue component (0.0 to 1.0)
         */
        void DrawDebugCircle(const Vec2& center, float radius, float r = 1.0f, float g = 0.0f, float b = 0.0f);
    };
}