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

void main()
{
    color = texture(image, TexCoords);
}
)";

    Graphics::Graphics() : mInitialized(false), mWindow(nullptr), mVAO(0), mVBO(0),
        mShaderProgram(0), mViewportWidth(800), mViewportHeight(600) {}

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
        glViewport(0, 0, mViewportWidth, mViewportHeight);

        // Initialize 2D renderer
        if (!InitializeRenderer())
        {
            std::cerr << "Failed to initialize 2D renderer!" << std::endl;
            return;
        }

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
    }

    void Graphics::Shutdown()
    {
        if (mInitialized)
        {
            std::cout << "Shutting down graphics system..." << std::endl;
            ShutdownRenderer();
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
        model = glm::scale(model, glm::vec3(textureSize.x * scale.x, textureSize.y * scale.y, 1.0f));

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
        if (textureSize.x == 0 || textureSize.y == 0) return;

        // Scale sprite to cover entire viewport
        Vec2 scale = Vec2(
            (float)mViewportWidth / textureSize.x,
            (float)mViewportHeight / textureSize.y
        );

        // Position the background's center at the viewport's center
        Vec2 position = Vec2(mViewportWidth / 2.0f, mViewportHeight / 2.0f);

        DrawSprite(textureID, textureSize, position, scale, 0.0f);
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
            -0.5f,  0.5f,      0.0f, 1.0f,  // Top-left
             0.5f, -0.5f,      1.0f, 0.0f,  // Bottom-right
            -0.5f, -0.5f,      0.0f, 0.0f,  // Bottom-left

            // Triangle 2
           -0.5f,  0.5f,      0.0f, 1.0f,  // Top-left
            0.5f,  0.5f,      1.0f, 1.0f,  // Top-right
            0.5f, -0.5f,      1.0f, 0.0f   // Bottom-right
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
        glm::mat4 projection = glm::ortho(0.0f, (float)mViewportWidth, (float)mViewportHeight, 0.0f, -1.0f, 1.0f);
        GLint projLoc = glGetUniformLocation(mShaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        // Set texture sampler
        glUniform1i(glGetUniformLocation(mShaderProgram, "image"), 0);

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
        glm::mat4 projection = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
        GLint projLoc = glGetUniformLocation(mShaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
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
}