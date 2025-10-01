#include "Systems/Graphics.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <cassert>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Uma_Engine
{
    // Vertex shader for 2D sprite rendering
    const std::string vertexShaderSource = R"(
#version 450 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 projection;

void main()
{
    TexCoords = vertex.zw;
    gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}
)";

    // Fragment shader for 2D sprite rendering
    const std::string fragmentShaderSource = R"(
#version 450 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform vec3 debugColor;
uniform int useDebugColor;

void main()
{
    if (useDebugColor == 1) {
        color = vec4(debugColor, 1.0);
    } else {
        color = texture(image, TexCoords);
    }
}
)";

    // Vertex shader for 2D sprite instanced rendering
    const std::string instancedVertexShaderSource = R"(
#version 450 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
layout (location = 1) in mat4 instanceModel; // Takes locations 1-4

out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    TexCoords = vertex.zw;
    gl_Position = projection * instanceModel * vec4(vertex.xy, 0.0, 1.0);
}
)";

    // Fragment shader for 2D sprite instanced rendering
    const std::string instancedFragmentShaderSource = R"(
#version 450 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;

void main()
{
    color = texture(image, TexCoords);
}
)";

    Graphics::Graphics() : mInitialized(false), mWindow(nullptr), mVAO(0), mVBO(0),
        mShaderProgram(0), mInstanceVBO(0), mInstanceVAO(0), mInstanceShaderProgram(0), 
        mViewportWidth(800), mViewportHeight(600) {}

    Graphics::~Graphics()
    {
        Shutdown();
    }

    void Graphics::Init()
    {
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

        // Set camera
        //mCamera = Camera2D(Vec2(mViewportWidth * 0.5f, mViewportHeight * 0.5f), 1.0f);

        // V sync 
        //SetVSync(true);

        // Initialize 2D renderer
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

        // init cam info
        cam = {
            .pos = {0,0},
            .zoom = 1.f
        };

        std::cout << "Graphics system initialized successfully!" << std::endl;
        mInitialized = true;
    }

    void Graphics::Update(float dt)
    {
        // Handle window resize if needed
        if (mWindow)
        {
            int width, height;
            glfwGetFramebufferSize(mWindow, &width, &height);

            if (width != mViewportWidth || height != mViewportHeight)
            {
                SetViewport(width, height);
            }
        }
        UpdateProjectionMatrix();
    }

    void Graphics::Shutdown()
    {
        if (mInitialized)
        {
            std::cout << "Shutting down graphics system..." << std::endl;
            ShutdownRenderer();
            ShutdownInstancedRenderer();
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

        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Load image
        int width, height, nrChannels;
        unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, 0);

        if (data)
        {
            GLenum format = GL_RGB;
            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 3)
                format = GL_RGB;
            else if (nrChannels == 4)
                format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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
        cam.pos = pos;
        cam.zoom = zoom;
    }

    void Graphics::DrawSprite(unsigned int textureID, const Vec2& textureSize,
        const Vec2& position, const Vec2& scale, float rotation)
    {
        if (!mInitialized || textureID == 0) return;

        // Use shader program
        glUseProgram(mShaderProgram);

        // Create transformation matrix
        glm::mat4 model = glm::mat4(1.0f);

        glm::vec2 pos{ position.x, position.y };

        model = glm::translate(model, glm::vec3(pos, 0.0f));
        model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(/*textureSize.x * */scale.x, /*textureSize.y * */scale.y, 1.0f));

        // Set model uniform
        GLint modelLoc = glGetUniformLocation(mShaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

        // Bind texture and render
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glBindVertexArray(mVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    void Graphics::DrawBackground(unsigned int textureID, const Vec2& textureSize)
    {
        if (!mInitialized || textureID == 0) return;

        glUseProgram(mShaderProgram);

        // Scale and flip Y to match NDC properly
        glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f));

        GLint modelLoc = glGetUniformLocation(mShaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

        // Projection matrix: identity for NDC
        glm::mat4 identity = glm::mat4(1.0f);
        GLint projLoc = glGetUniformLocation(mShaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &identity[0][0]);

        // Render
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
        // Create shader program
        mShaderProgram = CreateShader(vertexShaderSource, fragmentShaderSource);
        if (mShaderProgram == 0) return false;

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

        glGenVertexArrays(1, &mVAO);
        glGenBuffers(1, &mVBO);

        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindVertexArray(mVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // Set projection matrix
        glUseProgram(mShaderProgram);
        UpdateProjectionMatrix();

        // Set uniforms
        glUniform1i(glGetUniformLocation(mShaderProgram, "image"), 0);
        glUniform1i(glGetUniformLocation(mShaderProgram, "useDebugColor"), 0);

        return true;
    }

    void Graphics::ShutdownRenderer()
    {
        if (mVAO != 0) glDeleteVertexArrays(1, &mVAO);
        if (mVBO != 0) glDeleteBuffers(1, &mVBO);
        if (mShaderProgram != 0) glDeleteProgram(mShaderProgram);

        mVAO = mVBO = mShaderProgram = 0;
    }

    GLuint Graphics::CreateShader(const std::string& vertexSource, const std::string& fragmentSource)
    {
        // Compile vertex shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const char* vSource = vertexSource.c_str();
        glShaderSource(vertexShader, 1, &vSource, NULL);
        glCompileShader(vertexShader);

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

        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            std::cerr << "Shader linking failed: " << infoLog << std::endl;
            return 0;
        }

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
        SetViewport(width, height);
    }

    void Graphics::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
    {
        Graphics* graphics = static_cast<Graphics*>(glfwGetWindowUserPointer(window));
        if (graphics)
        {
            graphics->OnWindowResize(width, height);
        }
    }

    void Graphics::UpdateProjectionMatrix()
    {
        if (!mInitialized) return;

        // testing
        float halfWidth = (mViewportWidth * 0.5f) / cam.zoom;
        float halfHeight = (mViewportHeight * 0.5f) / cam.zoom;

        float left = cam.pos.x - halfWidth;
        float right = cam.pos.x + halfWidth;
        float bottom = cam.pos.y - halfHeight;
        float top = cam.pos.y + halfHeight;

        glm::mat4 projMat =  glm::ortho(left, right, bottom, top, -1.0f, 1.0f);

        glUseProgram(mShaderProgram);
        glm::mat4 projection = projMat;
        GLint projLoc = glGetUniformLocation(mShaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
    }

    Vec2 Graphics::ScreenToWorld(const Vec2& screenPos) const
    {
        // Convert screen coordinates to NDC
        float ndcX = (2.0f * screenPos.x) / mViewportWidth - 1.0f;
        float ndcY = 1.0f - (2.0f * screenPos.y) / mViewportHeight;

        // Calculate  projection matrix
        float halfWidth = (mViewportWidth * 0.5f) / cam.zoom;
        float halfHeight = (mViewportHeight * 0.5f) / cam.zoom;
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
        // Calculate projection matrix
        float halfWidth = (mViewportWidth * 0.5f) / cam.zoom;
        float halfHeight = (mViewportHeight * 0.5f) / cam.zoom;
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
        float screenX = (ndcX + 1.0f) * 0.5f * mViewportWidth;
        float screenY = (1.0f - ndcY) * 0.5f * mViewportHeight;

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
        model = glm::scale(model, glm::vec3(length, 2.0f, 1.0f)); // 2 pixel thick line

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
        // Create instanced shader program
        mInstanceShaderProgram = CreateShader(instancedVertexShaderSource, instancedFragmentShaderSource);
        if (mInstanceShaderProgram == 0) 
        {
            std::cerr << "Failed to create instanced shader program!" << std::endl;
            return false;
        }

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

        // Allocate space for up to 10000 instances
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * 10000, nullptr, GL_DYNAMIC_DRAW);

        // Set up instance attributes
        for (int i = 0; i < 4; ++i) 
        {
            glEnableVertexAttribArray(1 + i);
            glVertexAttribPointer(1 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
            glVertexAttribDivisor(1 + i, 1);
        }

        // Unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        return true;
    }

    void Graphics::DrawSpritesInstanced(
        unsigned int textureID,
        const Vec2& textureSize,
        std::vector<Sprite_Info> const& sprites)
    {
        if (!mInitialized || textureID == 0 || sprites.empty()) return;

        size_t instanceCount = sprites.size();

        // Build model matrices for all instances
        std::vector<glm::mat4> models;
        models.reserve(instanceCount);

        for (size_t i = 0; i < instanceCount; ++i)
        {
            const Sprite_Info& sprite = sprites[i];
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(sprite.pos.x, sprite.pos.y, 0.0f));
            model = glm::rotate(model, glm::radians(sprite.rot), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, glm::vec3(sprite.scale.x, sprite.scale.y, 1.0f));

            models.push_back(model);
        }

        if (models.empty()) return;

        glBindBuffer(GL_ARRAY_BUFFER, mInstanceVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::mat4) * models.size(), models.data());
        glUseProgram(mInstanceShaderProgram);

        // Set projection matrix uniform
        GLint projLoc = glGetUniformLocation(mInstanceShaderProgram, "projection");

        // Calculate projection matrix
        float halfWidth = (mViewportWidth * 0.5f) / cam.zoom;
        float halfHeight = (mViewportHeight * 0.5f) / cam.zoom;
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
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, instanceCount);

        // Cleanup
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Graphics::ShutdownInstancedRenderer()
    {
        if (mInstanceVAO != 0) {
            glDeleteVertexArrays(1, &mInstanceVAO);
            mInstanceVAO = 0;
        }
        if (mInstanceVBO != 0) {
            glDeleteBuffers(1, &mInstanceVBO);
            mInstanceVBO = 0;
        }
        if (mInstanceShaderProgram != 0) {
            glDeleteProgram(mInstanceShaderProgram);
            mInstanceShaderProgram = 0;
        }
    }
}