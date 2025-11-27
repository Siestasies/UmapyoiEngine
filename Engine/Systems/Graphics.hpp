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
#include <map>
//#include "Systems/TMP_CameraSystem.hpp"

#include "Core/Coordinator.hpp"
#include "Collider.h"

// Forward declarations
struct GLFWwindow;
using GLuint = unsigned int;

namespace Uma_Engine
{
    enum class RenderTarget
    {
        Window,      // GLFW window
        Framebuffer  // Framebuffer for ImGui display
    };

    /**
     * \struct RectInfo
     * \brief Information to draw filled rectangle
     */
    struct RectInfo
    {
        Vec2 center;
        Vec2 size;
        Vec3 color;
        float alpha;
    };

    /**
     * \struct CircleInfo
     * \brief Information to draw filled circle
     */
    struct CircleInfo
    {
        Vec2 center;
        float radius;
        Vec3 color;
        float alpha;
    };

    /**
     * \struct TriangleInfo
     * \brief Information to draw filled Triangle
     */
    struct TriangleInfo
    {
        Vec2 p1, p2, p3;
        Vec3 color;
        float alpha;
    };

    /**
     * \struct DebugLineInfo
     * \brief Information to draw a debug line for instanced debug line drawing
     */
    struct DebugLineInfo
    {
        Vec2 start;
        Vec2 end;
        Vec3 color;
    };

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

        Vec2 uvOffset{ 0.0f, 0.0f };  // UV offset (default = full texture)
        Vec2 uvSize{ 1.0f, 1.0f };    // UV size (default = full texture)
        Vec3 tintColor{ 1.0f, 1.0f, 1.0f };  // RGB multiplier
        float alpha{ 1.0f };                 // Opacity
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

        // Rendering
        GLuint mVAO, mVBO;
        GLuint mShaderProgram;

        // Instanced rendering
        GLuint mInstanceVBO;
        GLuint mInstanceVAO;
        GLuint mInstanceShaderProgram;
        GLuint mInstanceUVVBO;
        GLuint mInstanceTintVBO;

        // Debug rendering
        GLuint mDebugLineVAO;
        GLuint mDebugLineVBO;
        GLuint mDebugLineInstanceVBO;
        GLuint mDebugLineColorVBO;
        GLuint mDebugLineShaderProgram;

        // Shapes rendering
        GLuint mShapeVAO;
        GLuint mShapeVBO;
        GLuint mShapeShaderProgram;

        // Text rendering
        GLuint mTextShaderProgram;

        // Frame buffer
        GLuint mSceneFramebuffer;
        GLuint mSceneTexture;
        GLuint mSceneDepthBuffer;
        int mSceneFBWidth, mSceneFBHeight;
        RenderTarget mRenderTarget;

        // Viewport size
        int mViewportWidth, mViewportHeight;

        // Helper functions

        /**
         * \brief Initializes FreeType text renderer and shader
         * \return true if initialization succeeded
         */
        bool InitializeTextRenderer();

        /**
         * \brief Shuts down the text renderer and releases its resources
         */
        void ShutdownTextRenderer();

        /**
         * \brief Initializes instanced debug line renderer and shader
         * \return true if initialization succeeded
         */
        bool InitializeDebugRenderer();

        /**
         * \brief Shuts down the debug line renderer and releases its resources
         */
        void ShutdownDebugRenderer();

        /**
         * \brief Initializes the solid shape renderer
         * \return true if initialization succeeded
         */
        bool InitializeShapeRenderer();

        /**
         * \brief Shuts down the solid shape renderer and releases its resources
         */
        void ShutdownShapeRenderer();

        /**
         * \brief Compiles and links the text rendering shader
         * \return OpenGL shader program ID, 0 if failed
         */
        GLuint CreateTextShader();

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

        /**
         * \brief Gets current render target dimensions
         *
         * \param[out] width Current render width in pixels
         * \param[out] height Current render height in pixels
         */
        void GetCurrentRenderDimensions(int& width, int& height) const;

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

        /**
         * \brief Gets the GLFW window pointer
         * \return Const pointer to the GLFWwindow
         */
        GLFWwindow* GetWindow() const { return mWindow; }

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
         * \param position World space position
         * \param scale World space scale
         * \param rotation Rotation in degrees
         * \param tint RGB tint color
         * \param alpha Opacity
         */
        void DrawSprite(unsigned int textureID,
            const Vec2& position,
            const Vec2& scale = Vec2(1.0f, 1.0f),
            float rotation = 0.0f,
            const Vec3& tint = Vec3(1.0f, 1.0f, 1.0f),
            float alpha = 1.0f);

        /**
         * \brief Draws a batch of sprites using instanced rendering
         * \param textureID OpenGL texture ID of batch
         * \param sprites Vector of Sprite_Info
         */
        void DrawSpritesInstanced(
            unsigned int textureID,
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

        /**
         * \brief Get current viewport width
         * \return Viewport width in pixels
         */
        int GetViewportWidth() const { return mViewportWidth; }

        /**
         * \brief Get current viewport height
         * \return Viewport height in pixels
         */
        int GetViewportHeight() const { return mViewportHeight; }

        /**
         * \brief Gets current scene viewport dimensions
         * \return Vec2 containing width and height
         */
        Vec2 GetSceneViewport() const 
        {
            Vec2 size = Vec2(static_cast<float>(mViewportWidth), static_cast<float>(mViewportHeight));
            
            if (mRenderTarget == RenderTarget::Framebuffer)
            {
                size = Vec2(static_cast<float>(mSceneFBWidth), static_cast<float>(mSceneFBHeight));
            }

            return size;
        }


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

        /**
         * \brief Draws a batch of debug lines using instanced rendering
         * \param lines Vector of DebugLineInfo
         */
        void DrawDebugLinesInstanced(const std::vector<DebugLineInfo>& lines);

        /**
         * \brief Helper to add 4 lines for a rect to DebugLineInfo vector
         * \param center Rectangle center
         * \param size Rectangle size
         * \param r Red component
         * \param g Green component
         * \param b Blue component
         * \param outLines Vector to add DebugLineInfo
         */
        static void AddDebugRect(const Vec2& center, const Vec2& size, float r, float g, float b, std::vector<DebugLineInfo>& outLines);

        /**
         * \brief Helper to add 4 lines for bounding box to DebugLineInfo vector
         * \param bbox Bounding box
         * \param r Red component
         * \param g Green component
         * \param b Blue component
         * \param outLines Vector to add DebugLineInfo
         */
        static void AddDebugRect(const Uma_ECS::BoundingBox& bbox, float r, float g, float b, std::vector<DebugLineInfo>& outLines);

        /**
         * \brief Helper to add circle line segments to DebugLineInfo vector
         * \param center Circle center
         * \param radius Circle radius
         * \param r Red component
         * \param g Green component
         * \param b Blue component
         * \param outLines Vector to add DebugLineInfo
         */
        static void AddDebugCircle(const Vec2& center, float radius, float r, float g, float b, std::vector<DebugLineInfo>& outLines);

        /**
         * \brief Helper to add a debug point to DebugLineInfo vector
         * \param position Point position
         * \param r Red component
         * \param g Green component
         * \param b Blue component
         * \param outLines Vector to add DebugLineInfo
         */
        static void AddDebugPoint(const Vec2& position, float r, float g, float b, std::vector<DebugLineInfo>& outLines);

        /**
         * \brief Loads a font from a file using FreeType
         * \param fontPath File path to the font
         * \param fontSize Font size in pixels
         * \return FontData struct with character map and properties
         */
        FontData LoadFontFromFile(const std::string& fontPath, unsigned int fontSize = 48);

        /**
         * \brief Unloads and releases resources of FontData
         * \param fontData FontData object to unload
         */
        void UnloadFontData(FontData& fontData);

        /**
         * \brief Draws text in screen space
         * \param font The loaded FontData
         * \param text The string to render
         * \param x X position in screen pixels
         * \param y Y position in screen pixels
         * \param scale Text scale multiplier
         * \param r Red component
         * \param g Green component
         * \param b Blue component
         */
        void DrawTextScreen(const FontData& font, const std::string& text, float x, float y, float scale = 1.0f, float r = 1.0f, float g = 1.0f, float b = 1.0f);

        /**
         * \brief Calculates the width of a text string in pixels
         * \param font The loaded FontData
         * \param text The string to measure
         * \param scale Text scale multiplier
         * \return The total width of the text in pixels
         */
        float MeasureText(const FontData& font, const std::string& text, float scale = 1.0f);

        /**
         * \brief Draws text in world space
         * \param font The loaded FontData
         * \param text The string to render
         * \param x X position in world space
         * \param y Y position in world space
         * \param scale Text scale multiplier
         * \param r Red component
         * \param g Green component
         * \param b Blue component
         */
        void DrawTextWorld(const FontData& font, const std::string& text, float x, float y, float scale = 1.0f, float r = 1.0f, float g = 1.0f, float b = 1.0f);

        /**
         * \brief Draws a single sprite in screen space
         * \param textureID OpenGL texture ID
         * \param position Screen space position
         * \param size Screen space size in pixels
         * \param rotation Rotation in degrees
         * \param uvOffset UV coordinate offset
         * \param uvSize UV coordinate size
         * \param tint RGB tint color
         * \param alpha Opacity
         */
        void DrawSpriteScreen(unsigned int textureID, const Vec2& position, const Vec2& size, float rotation = 0.0f, const Vec2& uvOffset = Vec2(0.0f, 0.0f),
            const Vec2& uvSize = Vec2(1.0f, 1.0f), const Vec3& tint = Vec3(1.0f, 1.0f, 1.0f), float alpha = 1.0f);

        /**
         * \brief Draws a batch of sprites in screen space using instanced rendering
         * \param textureID OpenGL texture ID of batch
         * \param sprites Vector of Sprite_Info
         */
        void DrawSpritesScreenInstanced(unsigned int textureID, std::vector<Sprite_Info> const& sprites);

        /**
         * \brief Draws a solid filled rectangle in world space
         * \param center Rectangle center in world space
         * \param size Rectangle dimensions
         * \param r Red component
         * \param g Green component
         * \param b Blue component
         * \param alpha Opacity
         */
        void DrawFilledRect(const Vec2& center, const Vec2& size, float r, float g, float b, float alpha = 1.0f);

        /**
         * \brief Draws a solid filled circle in world space
         * \param center Circle center in world space
         * \param radius Circle radius
         * \param r Red component
         * \param g Green component
         * \param b Blue component
         * \param alpha Opacity
         * \param segments Number of triangles to approximate circle
         */
        void DrawFilledCircle(const Vec2& center, float radius, float r, float g, float b, float alpha = 1.0f,
            int segments = 24);

        /**
         * \brief Draws a solid filled triangle in world space
         * \param p1 Vertex 1 position
         * \param p2 Vertex 2 position
         * \param p3 Vertex 3 position
         * \param r Red component
         * \param g Green component
         * \param b Blue component
         * \param alpha Opacity
         */
        void DrawFilledTriangle(const Vec2& p1, const Vec2& p2, const Vec2& p3, float r, float g, float b, float alpha = 1.0f);

        /**
         * \brief Sets the current render target
         * \param target Render target to use (Framebuffer/Viewport)
         */
        void SetRenderTarget(RenderTarget target) { mRenderTarget = target; }

        /**
         * \brief Gets the current render target
         * \return The active render target (Framebuffer or Viewport)
         */
        RenderTarget GetRenderTarget() const { return mRenderTarget; }

        /**
         * \brief Initializes scene view framebuffer
         * \param width Framebuffer width in pixels
         * \param height Framebuffer height in pixels
         */
        void InitSceneFramebuffer(int width, int height);

        /**
         * \brief Resizes scene view framebuffer
         * \param width New framebuffer width in pixels
         * \param height New framebuffer height in pixels
         */
        void ResizeSceneFramebuffer(int width, int height);

        /**
         * \brief Gets OpenGL texture ID of scene view framebuffer
         * \return OpenGL texture handle
         */
        GLuint GetSceneTexture() const { return mSceneTexture; }

        /**
         * \brief Unbinds the scene view framebuffer to clear viewport
         */
        void UnbindFramebuffer();

        /**
         * \brief Gets scene view framebuffer width
         * \return Framebuffer width in pixels
         */
        int GetSceneFBWidth() const { return mSceneFBWidth; }

        /**
         * \brief Gets scene view framebuffer height
         * \return Framebuffer height in pixels
         */
        int GetSceneFBHeight() const { return mSceneFBHeight; }
    };
}