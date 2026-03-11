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

2D graphics rendering system

GLFW for window management
GLAD for OpenGL loading
GLM for math operations

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "Systems/Graphics.hpp"
#include "../Events/WindowEvents.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <cassert>
#include <cmath>
#include <fstream>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <ft2build.h>
#include <ResourcesManager.hpp>
#include FT_FREETYPE_H

namespace
{
    size_t MAX_INSTANCES = 10000;
}

namespace Uma_Engine
{
    Graphics::Graphics() : mInitialized(false), mWindow(nullptr), mVAO(0), mVBO(0),
        mShaderProgram(0), mInstanceVBO(0), mInstanceVAO(0), mInstanceShaderProgram(0), 
        mViewportWidth(800), mViewportHeight(600), mSceneFramebuffer(0), mSceneTexture(0),
        mSceneDepthBuffer(0), mSceneFBWidth(0), mSceneFBHeight(0), mRenderTarget(RenderTarget::Framebuffer) {}

    Graphics::~Graphics()
    {
        Shutdown();
    }

    void Graphics::Init()
    {
        // Prevent double initialization
        if (mInitialized)
        {
            std::cout << "Graphics already initialized!" << std::endl;
            return;
        }

        // Check if OpenGL context is available
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cerr << "GLAD not initialized" << std::endl;
            return;
        }

        // Get ResourcesManager reference
        mResourcesManager = pSystemManager->GetSystem<ResourcesManager>();
        assert(mResourcesManager != nullptr && "Error: ResourcesManager not initialized");

        // Enable blending for transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Set viewport
        if (mWindow)
        {
            int width, height;
            glfwGetFramebufferSize(mWindow, &width, &height);
            mViewportWidth = width;
            mViewportHeight = height;
        }
        glViewport(0, 0, mViewportWidth, mViewportHeight);

        // Initialize renderers
        if (!InitializeRenderer())
        {
            std::cerr << "Failed to initialize 2D renderer!" << std::endl;
            return;
        }

        if (!InitializeInstancedRenderer())
        {
            std::cerr << "Failed to initialize instanced renderer!" << std::endl;
            return;
        }

        if (!InitializeShapeRenderer())
        {
            std::cerr << "Failed to initialize shape renderer!" << std::endl;
            return;
        }

        if (!InitializeDebugRenderer())
        {
            std::cerr << "Failed to initialize debug renderer!" << std::endl;
            return;
        }

        if (!InitializeTextRenderer())
        {
            std::cerr << "Failed to initialize text renderer" << std::endl;
        }

        // init cam info
        cam =
        {
            .pos = {0,0},
            .zoom = 1.f
        };

        UpdateProjectionMatrix();
        InitSceneFramebuffer(1280, 720);

        std::cout << "Graphics system initialized successfully!" << std::endl;
        mInitialized = true;

        // Adaptive sync: syncs to VBlank when frame is on time, tears instead of waiting when late.
        // Eliminates the fixed center-screen tear caused by 120 FPS on 60Hz without adding VSync lag.
        if (glfwExtensionSupported("WGL_EXT_swap_control_tear") ||
            glfwExtensionSupported("GLX_EXT_swap_control_tear"))
        {
            glfwSwapInterval(-1);
            std::cout << "Adaptive sync enabled." << std::endl;
        }
        else
        {
            glfwSwapInterval(0);
            std::cout << "Adaptive sync not supported, running without VSync." << std::endl;
        }
    }

    void Graphics::Update(float dt)
    {
        UNREFERENCED_PARAMETER(dt);

        // Handle window resize if needed
        if (mWindow)
        {
            int width, height;
            glfwGetFramebufferSize(mWindow, &width, &height);

            if (width == 0 || height == 0) return;

            // Only update if size changed
            if (width != mViewportWidth || height != mViewportHeight)
            {
                SetViewport(width, height);
            }
        }

        // Bind framebuffer if in viewport mode
        if (mRenderTarget == RenderTarget::Framebuffer)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, mSceneFramebuffer);
            glViewport(0, 0, mSceneFBWidth, mSceneFBHeight);
        }

        ClearBackground(0.18f, 0.02f, 0.02f);
        UpdateProjectionMatrix();
    }

    void Graphics::Shutdown()
    {
        if (mInitialized)
        {
            std::cout << "Shutting down graphics system..." << std::endl;

            if (mSceneFramebuffer != 0)
            {
                glDeleteFramebuffers(1, &mSceneFramebuffer);
                glDeleteTextures(1, &mSceneTexture);
                glDeleteRenderbuffers(1, &mSceneDepthBuffer);
            }

            // Clean up renderers
            ShutdownTextRenderer();
            ShutdownRenderer();
            ShutdownInstancedRenderer();
            ShutdownDebugRenderer();
            ShutdownShapeRenderer();

            mInitialized = false;
        }
    }

    void Graphics::SetWindow(GLFWwindow* window)
    {
        mWindow = window;

        if (window)
        {
            // Get initial window size
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            mViewportWidth = width;
            mViewportHeight = height;

            // Set this Graphics instance as user pointer
            glfwSetWindowUserPointer(window, this);

            // Set the framebuffer size callback
            glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

            // Update viewport if graphics is initialised
            if (mInitialized)
            {
                SetViewport(width, height);
            }
        }
    }

    void Graphics::ClearBackground(float r, float g, float b)
    {
        if (!mInitialized) return;
        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    Texture Graphics::LoadTextureFromFile(const std::string& texturePath)
    {
        assert(mInitialized && "Error: Graphics System is not initialized.");

        Texture tex = {}; // Initialize to zero

        // Generate OpenGL texture object
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // Load image
        int width, height, nrChannels;
        unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, 0);

        if (data)
        {
            // Determine image format
            GLenum format = GL_RGB;
            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 3)
                format = GL_RGB;
            else if (nrChannels == 4)
                format = GL_RGBA;

            // Upload texture data to GPU
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            // Generate mipmaps
            glGenerateMipmap(GL_TEXTURE_2D);

            // Fill texture struct
            tex.tex_id = textureID;
            tex.tex_size = Vec2(static_cast<float>(width), static_cast<float>(height));
            tex.filePath = texturePath;

            std::cout << "Texture loaded: " << texturePath << " (" << width << "x" << height << ") ID: " << textureID << std::endl;
        }
        else
        {
            std::cerr << "Failed to load texture: " << texturePath << std::endl;
            glDeleteTextures(1, &textureID);
        }

        // Free image data
        stbi_image_free(data);
        return tex;
    }

    void Graphics::UnloadTexture(unsigned int textureID)
    {
        if (textureID != 0)
        {
            GLuint id = textureID;
            glDeleteTextures(1, &id);
        }
    }

    void Graphics::SetCamInfo(const Vec2& pos, float zoom)
    {
        // Update camera position and zoom level
        cam.pos = pos;
        cam.zoom = zoom;
    }

    void Graphics::DrawSprite(unsigned int textureID, const Vec2& position, const Vec2& scale, float rotation, const Vec3& tint, float alpha)
    {
        if (!mInitialized || textureID == 0) return;

        // Use shader program
        glUseProgram(mShaderProgram);

        GLint tintLoc = glGetUniformLocation(mShaderProgram, "tintColor");
        glUniform3f(tintLoc, tint.x, tint.y, tint.z);

        GLint alphaLoc = glGetUniformLocation(mShaderProgram, "alpha");
        glUniform1f(alphaLoc, alpha);

        // Create transformation matrix
        glm::mat4 model = glm::mat4(1.0f);
        glm::vec2 pos{ position.x, position.y };

        // Translate to position
        model = glm::translate(model, glm::vec3(pos, 0.0f));
        // Rotate around Z-axis
        model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
        // Scale the sprite
        model = glm::scale(model, glm::vec3(/*textureSize.x * */scale.x, /*textureSize.y * */scale.y, 1.0f));

        // Set model uniform
        GLint modelLoc = glGetUniformLocation(mShaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

        // Bind texture and render
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Draw the quad
        glBindVertexArray(mVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    void Graphics::DrawBackground(unsigned int textureID)
    {
        if (!mInitialized || textureID == 0) return;

        glUseProgram(mShaderProgram);

        GLint tintLoc = glGetUniformLocation(mShaderProgram, "tintColor");
        glUniform3f(tintLoc, 1.0f, 1.0f, 1.0f);

        GLint alphaLoc = glGetUniformLocation(mShaderProgram, "alpha");
        glUniform1f(alphaLoc, 1.0f);

        // Scale quad to fill entire NDC space
        glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f));

        GLint modelLoc = glGetUniformLocation(mShaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

        // Use identity projection matrix for NDC rendering
        glm::mat4 identity = glm::mat4(1.0f);
        GLint projLoc = glGetUniformLocation(mShaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &identity[0][0]);

        // Render fullscreen quad
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glBindVertexArray(mVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Restore camera projection
        UpdateProjectionMatrix();
    }

    bool Graphics::InitializeRenderer()
    {
        // Load shader through ResourcesManager
        if (!mResourcesManager->LoadShader(SHADER_SPRITE,
            "Assets/Shaders/sprite.vert",
            "Assets/Shaders/sprite.frag"))
        {
            std::cerr << "Failed to load sprite shader!" << std::endl;
            return false;
        }

        auto shader = mResourcesManager->GetShader(SHADER_SPRITE);
        if (!shader || shader->shaderProgramID == 0)
        {
            std::cerr << "Failed to retrieve sprite shader!" << std::endl;
            return false;
        }

        mShaderProgram = shader->shaderProgramID;

        // Set up quad vertices
        float vertices[] = {
            // pos             // tex
            // Triangle 1
            -0.5f,  0.5f,      0.0f, 0.0f,  // Top-left
             0.5f, -0.5f,      1.0f, 1.0f,  // Bottom-right
            -0.5f, -0.5f,      0.0f, 1.0f,  // Bottom-left

            // Triangle 2
           -0.5f,  0.5f,      0.0f, 0.0f,  // Top-left
            0.5f,  0.5f,      1.0f, 0.0f,  // Top-right
            0.5f, -0.5f,      1.0f, 1.0f   // Bottom-right
        };

        // Generate and configure VAO and VBO
        glGenVertexArrays(1, &mVAO);
        glGenBuffers(1, &mVBO);

        // Upload vertex data
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Configure vertex attributes
        glBindVertexArray(mVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // Set up shader uniforms
        glUseProgram(mShaderProgram);
        UpdateProjectionMatrix();

        // Set uniforms
        glUniform1i(glGetUniformLocation(mShaderProgram, "image"), 0);
        glUniform1i(glGetUniformLocation(mShaderProgram, "useDebugColor"), 0);
        glUniform3f(glGetUniformLocation(mShaderProgram, "tintColor"), 1.0f, 1.0f, 1.0f);
        glUniform1f(glGetUniformLocation(mShaderProgram, "alpha"), 1.0f);

        return true;
    }

    void Graphics::ShutdownRenderer()
    {
        // Clean up OpenGL resources
        if (mVAO != 0) glDeleteVertexArrays(1, &mVAO);
        if (mVBO != 0) glDeleteBuffers(1, &mVBO);

        mVAO = mVBO = 0;
        mShaderProgram = 0; // Don't delete, ResourcesManager owns it
    }

    GLuint Graphics::CreateShader(const std::string& vertexSource, const std::string& fragmentSource)
    {
        // Compile vertex shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const char* vSource = vertexSource.c_str();
        glShaderSource(vertexShader, 1, &vSource, NULL);
        glCompileShader(vertexShader);

        // Check vertex shader compilation
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
            return 0;
        }

        // Compile fragment shader
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const char* fSource = fragmentSource.c_str();
        glShaderSource(fragmentShader, 1, &fSource, NULL);
        glCompileShader(fragmentShader);

        // Check fragment shader compilation
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
            return 0;
        }

        // Link program
        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        // Check program linking
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            std::cerr << "Shader linking failed: " << infoLog << std::endl;
            return 0;
        }

        // Delete shader objects
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return program;
    }

    void Graphics::SetVSync(bool enabled)
    {
        if (!mInitialized) return;
        glfwSwapInterval(enabled ? 1 : 0);
    }

    void Graphics::SetViewport(int width, int height)
    {
        if (!mInitialized) return;

        // Update stored viewport dimensions
        mViewportWidth = width;
        mViewportHeight = height;
        glViewport(0, 0, width, height);

        // Update projection matrix
        glUseProgram(mShaderProgram);
        //mCamera.SetPosition(Vec2(width * 0.5f, height * 0.5f));
        UpdateProjectionMatrix();
    }

    void Graphics::OnWindowResize(int width, int height)
    {
        if (!mInitialized) return;
        if (width == 0 || height == 0) return;
        pSystemManager->GetSystem<EventSystem>()->Emit<WindowResizeEvent>(width, height);
        SetViewport(width, height);
    }

    void Graphics::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
    {
        Graphics* graphics = static_cast<Graphics*>(glfwGetWindowUserPointer(window));
        if (graphics)
        {
            // Resize
            graphics->OnWindowResize(width, height);
        }
    }

    void Graphics::UpdateProjectionMatrix()
    {
        if (!mInitialized) return;

        int width, height;
        GetCurrentRenderDimensions(width, height);

        // Calculate orthographic projection bounds based on camera zoom
        float halfWidth = (width * 0.5f) / cam.zoom;
        float halfHeight = (height * 0.5f) / cam.zoom;

        // Snap camera position to pixel grid to prevent sub-pixel seams in tilemaps
        float pixelSize = 1.0f / cam.zoom;
        float snappedX = std::round(cam.pos.x / pixelSize) * pixelSize;
        float snappedY = std::round(cam.pos.y / pixelSize) * pixelSize;

        // Calculate projection bounds centered on snapped camera position
        float left = snappedX - halfWidth;
        float right = snappedX + halfWidth;
        float bottom = snappedY - halfHeight;
        float top = snappedY + halfHeight;

        // Create orthographic projection matrix
        glm::mat4 projMat = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);

        // Upload projection matrix to shader
        glUseProgram(mShaderProgram);
        GLint projLoc = glGetUniformLocation(mShaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projMat[0][0]);
    }

    Vec2 Graphics::ScreenToWorld(const Vec2& screenPos) const
    {
        int width, height;
        GetCurrentRenderDimensions(width, height);

        // Convert screen coordinates to NDC
        float ndcX = (2.0f * screenPos.x) / width - 1.0f;
        float ndcY = 1.0f - (2.0f * screenPos.y) / height;

        // Calculate projection matrix
        float halfWidth = (width * 0.5f) / cam.zoom;
        float halfHeight = (height * 0.5f) / cam.zoom;
        float left = cam.pos.x - halfWidth;
        float right = cam.pos.x + halfWidth;
        float bottom = cam.pos.y - halfHeight;
        float top = cam.pos.y + halfHeight;

        glm::mat4 projectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
        glm::mat4 invProjectionMatrix = glm::inverse(projectionMatrix);

        // Transform NDC to world space
        glm::vec4 worldPos = invProjectionMatrix * glm::vec4(ndcX, ndcY, 0.0f, 1.0f);
        return Vec2(worldPos.x, worldPos.y);
    }

    Vec2 Graphics::WorldToScreen(const Vec2& worldPos) const
    {
        int width, height;
        GetCurrentRenderDimensions(width, height);

        // Calculate projection matrix
        float halfWidth = (width * 0.5f) / cam.zoom;
        float halfHeight = (height * 0.5f) / cam.zoom;
        float left = cam.pos.x - halfWidth;
        float right = cam.pos.x + halfWidth;
        float bottom = cam.pos.y - halfHeight;
        float top = cam.pos.y + halfHeight;

        glm::mat4 projectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);

        // Transform world space to clip space
        glm::vec4 clipPos = projectionMatrix * glm::vec4(worldPos.x, worldPos.y, 0.0f, 1.0f);

        // Convert clip space to NDC
        float ndcX = clipPos.x / clipPos.w;
        float ndcY = clipPos.y / clipPos.w;

        // Convert NDC to screen coordinates
        float screenX = (ndcX + 1.0f) * 0.5f * width;
        float screenY = (1.0f - ndcY) * 0.5f * height;

        return Vec2(screenX, screenY);
    }

    void Graphics::DrawDebugPoint(const Vec2& position, float r, float g, float b)
    {
        if (!mInitialized) return;

        glUseProgram(mShaderProgram);

        // Enable debug color mode
        glUniform1i(glGetUniformLocation(mShaderProgram, "useDebugColor"), 1);
        glUniform3f(glGetUniformLocation(mShaderProgram, "debugColor"), r, g, b);

        // Draw a small quad as point
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(position.x, position.y, 0.0f));
        model = glm::scale(model, glm::vec3(6.0f, 6.0f, 1.0f)); // 6x6 pixel point

        GLint modelLoc = glGetUniformLocation(mShaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

        // Draw using existing VAO
        glBindVertexArray(mVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Disable debug color mode
        glUniform1i(glGetUniformLocation(mShaderProgram, "useDebugColor"), 0);
    }

    void Graphics::DrawDebugLine(const Vec2& start, const Vec2& end, float r, float g, float b)
    {
        if (!mInitialized) return;

        // Calculate line properties
        float dx = end.x - start.x;
        float dy = end.y - start.y;
        float length = sqrtf(dx * dx + dy * dy);
        if (length < 0.001f) return;

        float angle = atan2f(dy, dx) * 180.0f / 3.14159265f;
        Vec2 center = Vec2((start.x + end.x) * 0.5f, (start.y + end.y) * 0.5f);

        glUseProgram(mShaderProgram);

        // Enable debug color mode
        glUniform1i(glGetUniformLocation(mShaderProgram, "useDebugColor"), 1);
        glUniform3f(glGetUniformLocation(mShaderProgram, "debugColor"), r, g, b);

        // Draw line as thin rectangle
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(center.x, center.y, 0.0f));
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(length, 0.3f, 0.3f)); // 2 pixel thick line

        GLint modelLoc = glGetUniformLocation(mShaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

        glBindVertexArray(mVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Disable debug color mode
        glUniform1i(glGetUniformLocation(mShaderProgram, "useDebugColor"), 0);
    }

    void Graphics::DrawDebugRect(const Vec2& center, const Vec2& size, float r, float g, float b)
    {
        if (!mInitialized) return;

        float halfW = size.x * 0.5f;
        float halfH = size.y * 0.5f;

        // Draw 4 lines
        DrawDebugLine(Vec2(center.x - halfW, center.y - halfH), Vec2(center.x + halfW, center.y - halfH), r, g, b); // Bottom
        DrawDebugLine(Vec2(center.x + halfW, center.y - halfH), Vec2(center.x + halfW, center.y + halfH), r, g, b); // Right
        DrawDebugLine(Vec2(center.x + halfW, center.y + halfH), Vec2(center.x - halfW, center.y + halfH), r, g, b); // Top
        DrawDebugLine(Vec2(center.x - halfW, center.y + halfH), Vec2(center.x - halfW, center.y - halfH), r, g, b); // Left
    }

    void Graphics::DrawDebugRect(const Uma_ECS::BoundingBox& bbox, float r, float g, float b)
    {
        if (!mInitialized) return;

        // Draw 4 lines using bounding box min and max
        DrawDebugLine(Vec2(bbox.min.x, bbox.min.y), Vec2(bbox.max.x, bbox.min.y), r, g, b); // Bottom
        DrawDebugLine(Vec2(bbox.max.x, bbox.min.y), Vec2(bbox.max.x, bbox.max.y), r, g, b); // Right
        DrawDebugLine(Vec2(bbox.max.x, bbox.max.y), Vec2(bbox.min.x, bbox.max.y), r, g, b); // Top
        DrawDebugLine(Vec2(bbox.min.x, bbox.max.y), Vec2(bbox.min.x, bbox.min.y), r, g, b); // Left
    }

    void Graphics::DrawDebugCircle(const Vec2& center, float radius, float r, float g, float b)
    {
        if (!mInitialized) return;

        const int segments = 24;
        const float angleStep = 2.0f * 3.14159f / segments;

        // Draw circle as connected lines
        for (int i = 0; i < segments; ++i)
        {
            float angle1 = i * angleStep;
            float angle2 = (i + 1) * angleStep;

            Vec2 p1 = Vec2(center.x + cosf(angle1) * radius, center.y + sinf(angle1) * radius);
            Vec2 p2 = Vec2(center.x + cosf(angle2) * radius, center.y + sinf(angle2) * radius);

            DrawDebugLine(p1, p2, r, g, b);
        }
    }

    bool Graphics::InitializeInstancedRenderer()
    {
        // Load instanced shader through ResourcesManager
        if (!mResourcesManager->LoadShader(SHADER_INSTANCED,
            "Assets/Shaders/instanced.vert",
            "Assets/Shaders/instanced.frag"))
        {
            std::cerr << "Failed to load instanced shader!" << std::endl;
            return false;
        }

        auto shader = mResourcesManager->GetShader(SHADER_INSTANCED);
        if (!shader || shader->shaderProgramID == 0)
        {
            std::cerr << "Failed to retrieve instanced shader!" << std::endl;
            return false;
        }

        mInstanceShaderProgram = shader->shaderProgramID;

        // Create instance VAO
        glGenVertexArrays(1, &mInstanceVAO);
        glBindVertexArray(mInstanceVAO);

        // Bind the existing VBO with quad vertices
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        // Create instance VBO for model matrices
        glGenBuffers(1, &mInstanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, mInstanceVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * MAX_INSTANCES, nullptr, GL_DYNAMIC_DRAW);

        // Set up instance attributes
        for (int i = 0; i < 4; ++i)
        {
            glEnableVertexAttribArray(1 + i);
            glVertexAttribPointer(1 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
            glVertexAttribDivisor(1 + i, 1);
        }

        // Create instance VBO for UVs
        glGenBuffers(1, &mInstanceUVVBO);
        glBindBuffer(GL_ARRAY_BUFFER, mInstanceUVVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * MAX_INSTANCES, nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
        glVertexAttribDivisor(5, 1);

        // Create instance VBO for Tint
        glGenBuffers(1, &mInstanceTintVBO);
        glBindBuffer(GL_ARRAY_BUFFER, mInstanceTintVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * MAX_INSTANCES, nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
        glVertexAttribDivisor(6, 1);

        // Unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // Set default uniforms
        glUseProgram(mInstanceShaderProgram);
        glUniform1i(glGetUniformLocation(mInstanceShaderProgram, "image"), 0);

        return true;
    }

    bool Graphics::InitializeDebugRenderer()
    {
        // Load debug shader through ResourcesManager
        if (!mResourcesManager->LoadShader(SHADER_DEBUG,
            "Assets/Shaders/debug.vert",
            "Assets/Shaders/debug.frag"))
        {
            std::cerr << "Failed to load debug shader!" << std::endl;
            return false;
        }

        auto shader = mResourcesManager->GetShader(SHADER_DEBUG);
        if (!shader || shader->shaderProgramID == 0)
        {
            std::cerr << "Failed to retrieve debug shader!" << std::endl;
            return false;
        }

        mDebugLineShaderProgram = shader->shaderProgramID;

        float lineVertices[] =
        {
            -0.5f, 0.0f,
             0.5f, 0.0f
        };

        // Create VAO and VBO for line geometry
        glGenVertexArrays(1, &mDebugLineVAO);
        glGenBuffers(1, &mDebugLineVBO);

        glBindBuffer(GL_ARRAY_BUFFER, mDebugLineVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);

        glBindVertexArray(mDebugLineVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

        // Create instance VBO for matrices
        glGenBuffers(1, &mDebugLineInstanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, mDebugLineInstanceVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * MAX_INSTANCES, nullptr, GL_DYNAMIC_DRAW);

        for (int i = 0; i < 4; ++i)
        {
            glEnableVertexAttribArray(1 + i);
            glVertexAttribPointer(1 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
                (void*)(sizeof(glm::vec4) * i));
            glVertexAttribDivisor(1 + i, 1);
        }

        // Create instance VBO for colors
        glGenBuffers(1, &mDebugLineColorVBO);
        glBindBuffer(GL_ARRAY_BUFFER, mDebugLineColorVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * MAX_INSTANCES, nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glVertexAttribDivisor(5, 1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        std::cout << "Debug renderer initialized" << std::endl;
        return true;
    }

    bool Graphics::InitializeShapeRenderer()
    {
        // Load shape shader through ResourcesManager
        if (!mResourcesManager->LoadShader(SHADER_SHAPE,
            "Assets/Shaders/shape.vert",
            "Assets/Shaders/shape.frag"))
        {
            std::cerr << "Failed to load shape shader!" << std::endl;
            return false;
        }

        auto shader = mResourcesManager->GetShader(SHADER_SHAPE);
        if (!shader || shader->shaderProgramID == 0)
        {
            std::cerr << "Failed to retrieve shape shader!" << std::endl;
            return false;
        }

        mShapeShaderProgram = shader->shaderProgramID;

        // Create VAO and VBO
        glGenVertexArrays(1, &mShapeVAO);
        glGenBuffers(1, &mShapeVBO);

        glBindVertexArray(mShapeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, mShapeVBO);

        // Reserve space for shapes
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * 1000, nullptr, GL_DYNAMIC_DRAW);

        // Position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        std::cout << "Shape renderer initialized" << std::endl;
        return true;
    }

    void Graphics::DrawSpritesInstanced(
        unsigned int textureID,
        std::vector<Sprite_Info> const& sprites)
    {
        if (!mInitialized || textureID == 0 || sprites.empty()) return;

        if (sprites.size() > MAX_INSTANCES)
        {
            std::cerr << "Warning: Clamping " << sprites.size() << " instances to " << MAX_INSTANCES << std::endl;
        }

        size_t instanceCount = std::min(sprites.size(), MAX_INSTANCES);

        // Build model matrices and UV data
        std::vector<glm::mat4> models;
        std::vector<glm::vec4> uvData;
        std::vector<glm::vec4> tintData;
        models.reserve(instanceCount);
        uvData.reserve(instanceCount);
        tintData.reserve(instanceCount);

        for (size_t i = 0; i < instanceCount; ++i)
        {
            const Sprite_Info& sprite = sprites[i];

            // Build model matrix
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(sprite.pos.x, sprite.pos.y, 0.0f));
            model = glm::rotate(model, glm::radians(sprite.rot), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, glm::vec3(sprite.scale.x, sprite.scale.y, 1.0f));
            models.push_back(model);

            // Build UV data
            uvData.push_back(glm::vec4(sprite.uvOffset.x, sprite.uvOffset.y,
                sprite.uvSize.x, sprite.uvSize.y));

            // Build tint data
            tintData.push_back(glm::vec4(sprite.tintColor.x, sprite.tintColor.y,
                sprite.tintColor.z, sprite.alpha));
        }

        if (models.empty()) return;

        // Upload model matrices
        glBindBuffer(GL_ARRAY_BUFFER, mInstanceVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::mat4) * models.size(), models.data());

        // Upload UV data
        glBindBuffer(GL_ARRAY_BUFFER, mInstanceUVVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec4) * uvData.size(), uvData.data());

        // Upload tint data
        glBindBuffer(GL_ARRAY_BUFFER, mInstanceTintVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec4) * tintData.size(), tintData.data());

        glUseProgram(mInstanceShaderProgram);

        // Set projection matrix uniform
        GLint projLoc = glGetUniformLocation(mInstanceShaderProgram, "projection");

        int width, height;
        GetCurrentRenderDimensions(width, height);

        // Calculate projection matrix
        float halfWidth = (width * 0.5f) / cam.zoom;
        float halfHeight = (height * 0.5f) / cam.zoom;
        float left = cam.pos.x - halfWidth;
        float right = cam.pos.x + halfWidth;
        float bottom = cam.pos.y - halfHeight;
        float top = cam.pos.y + halfHeight;
        glm::mat4 projection = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);

        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        // Set texture uniform
        glUniform1i(glGetUniformLocation(mInstanceShaderProgram, "image"), 0);

        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Draw all instances in one call
        glBindVertexArray(mInstanceVAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<GLsizei>(instanceCount));

        // Cleanup
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Graphics::ShutdownInstancedRenderer()
    {
        if (mInstanceVAO != 0)
        {
            glDeleteVertexArrays(1, &mInstanceVAO);
            mInstanceVAO = 0;
        }
        if (mInstanceVBO != 0)
        {
            glDeleteBuffers(1, &mInstanceVBO);
            mInstanceVBO = 0;
        }
        if (mInstanceUVVBO != 0)
        {
            glDeleteBuffers(1, &mInstanceUVVBO);
            mInstanceUVVBO = 0;
        }
        if (mInstanceTintVBO != 0)
        {
            glDeleteBuffers(1, &mInstanceTintVBO);
            mInstanceTintVBO = 0;
        }

        mInstanceShaderProgram = 0; // Don't delete, ResourcesManager owns it
    }

    void Graphics::ShutdownDebugRenderer()
    {
        if (mDebugLineVAO != 0)
        {
            glDeleteVertexArrays(1, &mDebugLineVAO);
            mDebugLineVAO = 0;
        }
        if (mDebugLineVBO != 0)
        {
            glDeleteBuffers(1, &mDebugLineVBO);
            mDebugLineVBO = 0;
        }
        if (mDebugLineInstanceVBO != 0)
        {
            glDeleteBuffers(1, &mDebugLineInstanceVBO);
            mDebugLineInstanceVBO = 0;
        }
        if (mDebugLineColorVBO != 0)
        {
            glDeleteBuffers(1, &mDebugLineColorVBO);
            mDebugLineColorVBO = 0;
        }

        mDebugLineShaderProgram = 0; // Don't delete, ResourcesManager owns it
    }

    bool Graphics::InitializeTextRenderer()
    {
        // Load text shader through ResourcesManager
        if (!mResourcesManager->LoadShader(SHADER_TEXT,
            "Assets/Shaders/text.vert",
            "Assets/Shaders/text.frag"))
        {
            std::cerr << "Failed to load text shader!" << std::endl;
            return false;
        }

        auto shader = mResourcesManager->GetShader(SHADER_TEXT);
        if (!shader || shader->shaderProgramID == 0)
        {
            std::cerr << "Failed to retrieve text shader!" << std::endl;
            return false;
        }

        mTextShaderProgram = shader->shaderProgramID;

        std::cout << "Text renderer initialized successfully" << std::endl;
        return true;
    }

    void Graphics::ShutdownTextRenderer()
    {
        mTextShaderProgram = 0; // Don't delete, ResourcesManager owns it
    }

    void Graphics::UnloadFontData(FontData& fontData)
    {
        // Clean up specified font resources
        for (auto& [c, character] : fontData.characters)
        {
            glDeleteTextures(1, &character.textureID);
        }
        glDeleteVertexArrays(1, &fontData.VAO);
        glDeleteBuffers(1, &fontData.VBO);

        // Clear data
        fontData.characters.clear();
        fontData.VAO = 0;
        fontData.VBO = 0;
    }

    FontData Graphics::LoadFontFromFile(const std::string& fontPath, unsigned int fontSize)
    {
        FontData newFont = {};
        newFont.fontSize = fontSize;
        newFont.filePath = fontPath;

        // Initialize FreeType
        FT_Library ft;
        if (FT_Init_FreeType(&ft)) 
        {
            std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            return newFont;
        }

        // Load font face
        FT_Face face;
        if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) 
        {
            std::cerr << "ERROR::FREETYPE: Failed to load font: " << fontPath << std::endl;
            FT_Done_FreeType(ft);
            return newFont;
        }

        // Set font size
        FT_Set_Pixel_Sizes(face, 0, fontSize);

        // Disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glCreateVertexArrays(1, &newFont.VAO);
        glCreateBuffers(1, &newFont.VBO);
        glNamedBufferStorage(newFont.VBO, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_STORAGE_BIT);
        glEnableVertexArrayAttrib(newFont.VAO, 0);
        glVertexArrayAttribFormat(newFont.VAO, 0, 4, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(newFont.VAO, 0, 0);
        glVertexArrayVertexBuffer(newFont.VAO, 0, newFont.VBO, 0, 4 * sizeof(float));

        // Load ASCII characters (32-126)
        for (unsigned char c = 32; c < 127; c++)
        {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cerr << "WARNING: Failed to load Glyph '" << c << "'" << std::endl;
                continue;
            }

            GLuint texture;
            glCreateTextures(GL_TEXTURE_2D, 1, &texture);
            glTextureStorage2D(texture, 1, GL_R8, face->glyph->bitmap.width, face->glyph->bitmap.rows);
            if (face->glyph->bitmap.buffer) {
                glTextureSubImage2D(texture, 0, 0, 0, face->glyph->bitmap.width, face->glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
            }
            glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // Store character
            Character character =
            {
                texture,
                Vec2(static_cast<float>(face->glyph->bitmap.width), static_cast<float>(face->glyph->bitmap.rows)),
                Vec2(static_cast<float>(face->glyph->bitmap_left), static_cast<float>(face->glyph->bitmap_top)),
                static_cast<float>(face->glyph->advance.x >> 6)
            };
            // Store in the newFont's map
            newFont.characters[c] = character;
        }

        // Clean up FreeType
        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        std::cout << "Font loaded from file: " << fontPath << " (size: " << fontSize << ")" << std::endl;
        return newFont;
    }

    void Graphics::DrawTextScreen(const FontData& font, const std::string& text,
        float x, float y, float scale, float r, float g, float b)
    {
        if (font.VAO == 0)
        {
            std::cerr << "ERROR: Invalid FontData passed to DrawTextScreen" << std::endl;
            return;
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glUseProgram(mTextShaderProgram);
        glBindVertexArray(font.VAO);

        glUniform3f(glGetUniformLocation(mTextShaderProgram, "textColor"), r, g, b);

        glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
        GLint projLoc = glGetUniformLocation(mTextShaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        // Calculate aspect ratio
        float aspect = static_cast<float>(mViewportWidth) / static_cast<float>(mViewportHeight);
        float normalizedScale = scale / static_cast<float>(mViewportHeight);

        for (char c : text)
        {
            auto char_it = font.characters.find(c);
            if (char_it == font.characters.end()) continue;

            Character ch = char_it->second;

            // Divide X by aspect
            float xpos = x + (ch.bearing.x * normalizedScale) / aspect;
            float ypos = y - ((ch.size.y - ch.bearing.y) * normalizedScale);
            float w = (ch.size.x * normalizedScale) / aspect;
            float h = ch.size.y * normalizedScale;

            float vertices[6][4] =
            {
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos,     ypos,       0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos + w, ypos + h,   1.0f, 0.0f }
            };

            glBindTextureUnit(0, ch.textureID);
            glNamedBufferSubData(font.VBO, 0, sizeof(vertices), vertices);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            x += (ch.advance * normalizedScale) / aspect;
        }

        glBindVertexArray(0);
        glBindTextureUnit(0, 0);
    }

    float Graphics::MeasureText(const FontData& font, const std::string& text, float scale)
    {
        if (font.VAO == 0) 
        {
            return 0.0f;
        }

        float aspect = static_cast<float>(mViewportWidth) / static_cast<float>(mViewportHeight);
        float normalizedScale = scale / static_cast<float>(mViewportHeight);
        float width = 0.0f;

        for (char c : text)
        {
            auto char_it = font.characters.find(c);
            if (char_it != font.characters.end())
            {
                width += (char_it->second.advance * normalizedScale) / aspect;
            }
        }

        return width;
    }

    void Graphics::DrawTextWorld(const FontData& font, const std::string& text,
        float x, float y, float scale, float r, float g, float b)
    {
        if (font.VAO == 0) 
        {
            std::cerr << "ERROR: Invalid FontData passed to DrawTextWorld" << std::endl;
            return;
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glUseProgram(mTextShaderProgram);
        glBindVertexArray(font.VAO);

        // Set text color
        glUniform3f(glGetUniformLocation(mTextShaderProgram, "textColor"), r, g, b);

        int width, height;
        GetCurrentRenderDimensions(width, height);

        // Use camera-based projection
        float halfWidth = (width * 0.5f) / cam.zoom;
        float halfHeight = (height * 0.5f) / cam.zoom;
        float left = cam.pos.x - halfWidth;
        float right = cam.pos.x + halfWidth;
        float bottom = cam.pos.y - halfHeight;
        float top = cam.pos.y + halfHeight;
        glm::mat4 projection = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);

        GLint projLoc = glGetUniformLocation(mTextShaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        // Render each character in world space
        for (char c : text) {
            auto char_it = font.characters.find(c);
            if (char_it == font.characters.end()) continue;

            Character ch = char_it->second;

            // Calculate positions in world space
            float xpos = x + ch.bearing.x * scale;
            float ypos = y - (ch.size.y - ch.bearing.y) * scale;
            float w = ch.size.x * scale;
            float h = ch.size.y * scale;

            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos,     ypos,       0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos + w, ypos + h,   1.0f, 0.0f }
            };

            glBindTextureUnit(0, ch.textureID);
            glNamedBufferSubData(font.VBO, 0, sizeof(vertices), vertices);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Advance to next character position
            x += ch.advance * scale;
        }

        glBindVertexArray(0);
        glBindTextureUnit(0, 0);
    }

    void Graphics::DrawSpriteScreen(unsigned int textureID, const Vec2& position,
        const Vec2& size, float rotation, const Vec2& uvOffset, const Vec2& uvSize, const Vec3& tint, float alpha, int fillDirection, float fillAmount)
    {
        (void)uvSize;
        (void)uvOffset;
        if (!mInitialized || textureID == 0) return;

        glUseProgram(mShaderProgram);

        // Set fill uniforms
        GLint fillDirLoc = glGetUniformLocation(mShaderProgram, "fillDirection");
        glUniform1i(fillDirLoc, fillDirection);

        GLint fillAmtLoc = glGetUniformLocation(mShaderProgram, "fillAmount");
        glUniform1f(fillAmtLoc, fillAmount);

        // Set tint uniform
        GLint tintLoc = glGetUniformLocation(mShaderProgram, "tintColor");
        glUniform3f(tintLoc, tint.x, tint.y, tint.z);

        GLint alphaLoc = glGetUniformLocation(mShaderProgram, "alpha");
        glUniform1f(alphaLoc, alpha);

        // Calculate current aspect ratio
        //float aspect = static_cast<float>(mViewportWidth) / static_cast<float>(mViewportHeight);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(position.x, position.y, 0.0f));

        if (rotation != 0.0f)
        {
            model = glm::translate(model, glm::vec3(size.x * 0.5f, size.y * 0.5f, 0.0f));
            model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::translate(model, glm::vec3(-size.x * 0.5f, -size.y * 0.5f, 0.0f));
        }

        model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));

        GLint modelLoc = glGetUniformLocation(mShaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

        glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
        GLint projLoc = glGetUniformLocation(mShaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glBindVertexArray(mVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        UpdateProjectionMatrix();
    }

    void Graphics::DrawSpritesScreenInstanced(unsigned int textureID, std::vector<Sprite_Info> const& sprites)
    {
        if (!mInitialized || textureID == 0 || sprites.empty()) return;

        if (sprites.size() > MAX_INSTANCES)
        {
            std::cerr << "Warning: Clamping " << sprites.size() << " instances to " << MAX_INSTANCES << std::endl;
        }

        size_t instanceCount = std::min(sprites.size(), MAX_INSTANCES);

        std::vector<glm::mat4> models;
        std::vector<glm::vec4> uvData;
        std::vector<glm::vec4> tintData;
        models.reserve(instanceCount);
        uvData.reserve(instanceCount);
        tintData.reserve(instanceCount);

        for (size_t i = 0; i < instanceCount; ++i)
        {
            const Sprite_Info& sprite = sprites[i];

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(sprite.pos.x, sprite.pos.y, 0.0f));
            model = glm::rotate(model, glm::radians(sprite.rot), glm::vec3(0.0f, 0.0f, 1.0f));

            // Divide X by aspect to make squares square, otherwise don't
            model = glm::scale(model, glm::vec3(sprite.scale.x, sprite.scale.y, 1.0f));
            models.push_back(model);

            uvData.push_back(glm::vec4(sprite.uvOffset.x, sprite.uvOffset.y,
                sprite.uvSize.x, sprite.uvSize.y));
            tintData.push_back(glm::vec4(sprite.tintColor.x, sprite.tintColor.y,
                sprite.tintColor.z, sprite.alpha));
        }

        if (models.empty()) return;

        glBindBuffer(GL_ARRAY_BUFFER, mInstanceVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::mat4) * models.size(), models.data());

        glBindBuffer(GL_ARRAY_BUFFER, mInstanceUVVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec4) * uvData.size(), uvData.data());

        glBindBuffer(GL_ARRAY_BUFFER, mInstanceTintVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec4) * tintData.size(), tintData.data());

        glUseProgram(mInstanceShaderProgram);

        glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
        GLint projLoc = glGetUniformLocation(mInstanceShaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        glUniform1i(glGetUniformLocation(mInstanceShaderProgram, "image"), 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glBindVertexArray(mInstanceVAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<GLsizei>(instanceCount));

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        UpdateProjectionMatrix();
    }

    void Graphics::DrawDebugLinesInstanced(const std::vector<DebugLineInfo>& lines)
    {
        if (!mInitialized || lines.empty()) return;

        size_t instanceCount = std::min(lines.size(), MAX_INSTANCES);

        // Build model matrices and colors for each line
        std::vector<glm::mat4> models;
        std::vector<glm::vec3> colors;
        models.reserve(instanceCount);
        colors.reserve(instanceCount);

        for (size_t i = 0; i < instanceCount; ++i)
        {
            const DebugLineInfo& line = lines[i];

            // Calculate line properties
            float dx = line.end.x - line.start.x;
            float dy = line.end.y - line.start.y;
            float length = sqrtf(dx * dx + dy * dy);

            if (length < 0.001f) continue; // Skip zero-length lines

            float angle = atan2f(dy, dx);
            Vec2 center = Vec2((line.start.x + line.end.x) * 0.5f,
                (line.start.y + line.end.y) * 0.5f);

            // Build transformation matrix
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(center.x, center.y, 0.0f));
            model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, glm::vec3(length, 2.0f, 1.0f));

            models.push_back(model);
            colors.push_back(glm::vec3(line.color.x, line.color.y, line.color.z));
        }

        if (models.empty()) return;

        // Upload model matrices to GPU
        glBindBuffer(GL_ARRAY_BUFFER, mDebugLineInstanceVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::mat4) * models.size(), models.data());

        // Upload colors to GPU
        glBindBuffer(GL_ARRAY_BUFFER, mDebugLineColorVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * colors.size(), colors.data());

        // Set line thickness
        glLineWidth(2.0f);

        // Use debug shader
        glUseProgram(mDebugLineShaderProgram);

        int width, height;
        GetCurrentRenderDimensions(width, height);

        // Calculate projection matrix
        float halfWidth = (width * 0.5f) / cam.zoom;
        float halfHeight = (height * 0.5f) / cam.zoom;
        float left = cam.pos.x - halfWidth;
        float right = cam.pos.x + halfWidth;
        float bottom = cam.pos.y - halfHeight;
        float top = cam.pos.y + halfHeight;
        glm::mat4 projection = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);

        GLint projLoc = glGetUniformLocation(mDebugLineShaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        // Draw all lines in one call
        glBindVertexArray(mDebugLineVAO);
        glDrawArraysInstanced(GL_LINES, 0, 2, static_cast<GLsizei>(models.size()));
        glBindVertexArray(0);
    }

    void Graphics::AddDebugRect(const Vec2& center, const Vec2& size, float r, float g, float b,
        std::vector<DebugLineInfo>& outLines)
    {
        Vec3 color(r, g, b);
        float halfW = size.x * 0.5f;
        float halfH = size.y * 0.5f;

        // Bottom line
        outLines.push_back({ Vec2(center.x - halfW, center.y - halfH), Vec2(center.x + halfW, center.y - halfH), color });
        // Right line
        outLines.push_back({ Vec2(center.x + halfW, center.y - halfH), Vec2(center.x + halfW, center.y + halfH), color });
        // Top line
        outLines.push_back({ Vec2(center.x + halfW, center.y + halfH), Vec2(center.x - halfW, center.y + halfH), color });
        // Left line
        outLines.push_back({ Vec2(center.x - halfW, center.y + halfH), Vec2(center.x - halfW, center.y - halfH), color });
    }

    void Graphics::AddDebugRect(const Uma_ECS::BoundingBox& bbox, float r, float g, float b,
        std::vector<DebugLineInfo>& outLines)
    {
        Vec3 color(r, g, b);

        // Bottom line
        outLines.push_back({ Vec2(bbox.min.x, bbox.min.y), Vec2(bbox.max.x, bbox.min.y), color });
        // Right line
        outLines.push_back({ Vec2(bbox.max.x, bbox.min.y), Vec2(bbox.max.x, bbox.max.y), color });
        // Top line
        outLines.push_back({ Vec2(bbox.max.x, bbox.max.y), Vec2(bbox.min.x, bbox.max.y), color });
        // Left line
        outLines.push_back({ Vec2(bbox.min.x, bbox.max.y), Vec2(bbox.min.x, bbox.min.y), color });
    }

    void Graphics::AddDebugCircle(const Vec2& center, float radius, float r, float g, float b,
        std::vector<DebugLineInfo>& outLines)
    {
        Vec3 color(r, g, b);
        const int segments = 24;
        const float angleStep = 2.0f * 3.14159f / segments;

        for (int i = 0; i < segments; ++i)
        {
            float angle1 = i * angleStep;
            float angle2 = (i + 1) * angleStep;

            Vec2 p1 = Vec2(center.x + cosf(angle1) * radius, center.y + sinf(angle1) * radius);
            Vec2 p2 = Vec2(center.x + cosf(angle2) * radius, center.y + sinf(angle2) * radius);

            outLines.push_back({ p1, p2, color });
        }
    }

    void Graphics::AddDebugPoint(const Vec2& position, float r, float g, float b,
        std::vector<DebugLineInfo>& outLines)
    {
        AddDebugRect(position, Vec2(6.0f, 6.0f), r, g, b, outLines);
    }

    void Graphics::DrawFilledRect(const Vec2& center, const Vec2& size,
        float r, float g, float b, float alpha)
    {
        if (!mInitialized) return;

        // Define rectangle vertices
        float halfW = size.x * 0.5f;
        float halfH = size.y * 0.5f;

        float vertices[] = {
            // Triangle 1
            -halfW, -halfH,  // Bottom-left
             halfW, -halfH,  // Bottom-right
            -halfW,  halfH,  // Top-left

            // Triangle 2
            -halfW,  halfH,  // Top-left
             halfW, -halfH,  // Bottom-right
             halfW,  halfH   // Top-right
        };

        // Upload vertex data
        glBindBuffer(GL_ARRAY_BUFFER, mShapeVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        // Use shape shader
        glUseProgram(mShapeShaderProgram);

        // Set color uniform
        GLint colorLoc = glGetUniformLocation(mShapeShaderProgram, "color");
        glUniform4f(colorLoc, r, g, b, alpha);

        // Set model matrix
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(center.x, center.y, 0.0f));

        GLint modelLoc = glGetUniformLocation(mShapeShaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

        int width, height;
        GetCurrentRenderDimensions(width, height);

        // Set projection matrix
        float halfWidth = (width * 0.5f) / cam.zoom;
        float halfHeight = (height * 0.5f) / cam.zoom;
        float left = cam.pos.x - halfWidth;
        float right = cam.pos.x + halfWidth;
        float bottom = cam.pos.y - halfHeight;
        float top = cam.pos.y + halfHeight;
        glm::mat4 projection = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);

        GLint projLoc = glGetUniformLocation(mShapeShaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        glBindVertexArray(mShapeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    void Graphics::DrawFilledCircle(const Vec2& center, float radius,
        float r, float g, float b, float alpha,
        int segments)
    {
        if (!mInitialized) return;

        // Generate circle vertices using triangle fan
        std::vector<float> vertices;
        vertices.reserve((segments + 2) * 2);

        // Center point
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);

        // Generate points around circle
        for (int i = 0; i <= segments; i++)
        {
            float angle = (float)i / (float)segments * 2.0f * 3.14159265f;
            vertices.push_back(cosf(angle) * radius);
            vertices.push_back(sinf(angle) * radius);
        }

        // Upload vertex data
        glBindBuffer(GL_ARRAY_BUFFER, mShapeVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());

        // Use shape shader
        glUseProgram(mShapeShaderProgram);

        // Set color uniform
        GLint colorLoc = glGetUniformLocation(mShapeShaderProgram, "color");
        glUniform4f(colorLoc, r, g, b, alpha);

        // Set model matrix
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(center.x, center.y, 0.0f));

        GLint modelLoc = glGetUniformLocation(mShapeShaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

        int width, height;
        GetCurrentRenderDimensions(width, height);

        // Set projection matrix
        float halfWidth = (width * 0.5f) / cam.zoom;
        float halfHeight = (height * 0.5f) / cam.zoom;
        float left = cam.pos.x - halfWidth;
        float right = cam.pos.x + halfWidth;
        float bottom = cam.pos.y - halfHeight;
        float top = cam.pos.y + halfHeight;
        glm::mat4 projection = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);

        GLint projLoc = glGetUniformLocation(mShapeShaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        glBindVertexArray(mShapeVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 2);
        glBindVertexArray(0);
    }

    void Graphics::DrawFilledTriangle(const Vec2& p1, const Vec2& p2, const Vec2& p3,
        float r, float g, float b, float alpha)
    {
        if (!mInitialized) return;

        // Triangle vertices
        float vertices[] = {
            p1.x, p1.y,
            p2.x, p2.y,
            p3.x, p3.y
        };

        // Upload vertex data
        glBindBuffer(GL_ARRAY_BUFFER, mShapeVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        // Use shape shader
        glUseProgram(mShapeShaderProgram);

        // Set color uniform
        GLint colorLoc = glGetUniformLocation(mShapeShaderProgram, "color");
        glUniform4f(colorLoc, r, g, b, alpha);

        glm::mat4 model = glm::mat4(1.0f);
        GLint modelLoc = glGetUniformLocation(mShapeShaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

        int width, height;
        GetCurrentRenderDimensions(width, height);

        // Set projection matrix
        float halfWidth = (width * 0.5f) / cam.zoom;
        float halfHeight = (height * 0.5f) / cam.zoom;
        float left = cam.pos.x - halfWidth;
        float right = cam.pos.x + halfWidth;
        float bottom = cam.pos.y - halfHeight;
        float top = cam.pos.y + halfHeight;
        glm::mat4 projection = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);

        GLint projLoc = glGetUniformLocation(mShapeShaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        glBindVertexArray(mShapeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
    }

    void Graphics::ShutdownShapeRenderer()
    {
        if (mShapeVAO != 0)
        {
            glDeleteVertexArrays(1, &mShapeVAO);
            mShapeVAO = 0;
        }
        if (mShapeVBO != 0)
        {
            glDeleteBuffers(1, &mShapeVBO);
            mShapeVBO = 0;
        }

        mShapeShaderProgram = 0; // Don't delete, ResourcesManager owns it
    }

    void Graphics::InitSceneFramebuffer(int width, int height)
    {
        if (width <= 0 || height <= 0) return;

        mSceneFBWidth = width;
        mSceneFBHeight = height;

        glGenFramebuffers(1, &mSceneFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, mSceneFramebuffer);

        glGenTextures(1, &mSceneTexture);
        glBindTexture(GL_TEXTURE_2D, mSceneTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mSceneTexture, 0);

        glGenRenderbuffers(1, &mSceneDepthBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, mSceneDepthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mSceneDepthBuffer);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cerr << "ERROR: Scene framebuffer is not complete" << std::endl;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Graphics::ResizeSceneFramebuffer(int width, int height)
    {
        if (width == mSceneFBWidth && height == mSceneFBHeight)
            return;

        if (mSceneFramebuffer != 0)
        {
            glDeleteFramebuffers(1, &mSceneFramebuffer);
            glDeleteTextures(1, &mSceneTexture);
            glDeleteRenderbuffers(1, &mSceneDepthBuffer);
        }

        InitSceneFramebuffer(width, height);
    }

    void Graphics::UnbindFramebuffer()
    {
        if (mRenderTarget == RenderTarget::Framebuffer)
        {
            // Unbind framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // Restore viewport
            glViewport(0, 0, mViewportWidth, mViewportHeight);

            // Clear viewport
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }

    void Graphics::GetCurrentRenderDimensions(int& width, int& height) const
    {
        if (mRenderTarget == RenderTarget::Framebuffer)
        {
            width = mSceneFBWidth;
            height = mSceneFBHeight;
        }
        else
        {
            width = mViewportWidth;
            height = mViewportHeight;
        }
    }

    void Graphics::UnloadShader(unsigned int shaderID)
    {
        if (shaderID != 0)
        {
            glDeleteProgram(shaderID);
        }
    }

    Shader Graphics::LoadShaderFromFile(const std::string& vertexPath, const std::string& fragmentPath)
    {
        Shader shader = { 0, vertexPath, fragmentPath };

        auto ReadFile = [](const std::string& path) -> std::string {
            std::ifstream file(path);
            if (!file.is_open()) {
                std::cerr << "Failed to open shader file: " << path << std::endl;
                return "";
            }
            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
            };

        std::string vertexCode = ReadFile(vertexPath);
        std::string fragmentCode = ReadFile(fragmentPath);

        if (vertexCode.empty() || fragmentCode.empty()) {
            return shader;
        }

        shader.shaderProgramID = CreateShader(vertexCode, fragmentCode);

        if (shader.shaderProgramID != 0) {
            std::cout << "Shader loaded: " << vertexPath << std::endl;
        }

        return shader;
    }
}
