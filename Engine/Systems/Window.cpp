/*!
\file   Window.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Javier Chua Dong Qing (90%) - Initial GLFW window and GLAD setup
\par    E-mail: javierdongqing.chua@digipen.edu
\par    DigiPen login: javierdongqing.chua

\author Muhammad Shahir Bin Rasid (10%) - IWindowSystem interface
\par    E-mail: b.muhammadshahir@digipen.edu
\par    DigiPen login: b.muhammadshahir

\brief
Handles creation and management of the application window using GLFW,
including OpenGL context setup using GLAD. Manages window events,
buffer swapping and shutdown.

Inherits from ISystem for engine management and IWindowSystem for
GLFW window integration and resize handling.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "Window.hpp"
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Uma_Engine
{
    Window::Window(int width, int height, const std::string& title)
        : mWindow(nullptr), mWidth(width), mHeight(height), mTitle(title), mInitialized(false) {}

    Window::~Window()
    {
        Shutdown();
    }

    bool Window::Initialize()
    {
        std::cout << "Initializing Uma_Engine..." << std::endl;
        
        // Initialize GLFW
        if (!glfwInit())
        {
            std::cerr << "Failed to initialize GLFW!" << std::endl;
            return false;
        }

        // Configure GLFW for OpenGL 4.5 Core
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Create window
        mWindow = glfwCreateWindow(mWidth, mHeight, mTitle.c_str(), nullptr, nullptr);
        if (!mWindow)
        {
            std::cerr << "Failed to create GLFW window!" << std::endl;
            glfwTerminate();
            return false;
        }

        // Make the window's context current BEFORE initializing GLAD
        glfwMakeContextCurrent(mWindow);

        // Initialize OpenGL with GLAD
        if (!InitializeOpenGL())
        {
            std::cerr << "Failed to initialize OpenGL!" << std::endl;
            glfwDestroyWindow(mWindow);
            glfwTerminate();
            return false;
        }

        std::cout << "Window created successfully: " << mTitle << " (" << mWidth << "x" << mHeight << ")" << std::endl;
        std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

        mInitialized = true;
        return true;
    }

    bool Window::InitializeOpenGL()
    {
        // Initialize GLAD
        std::cout << "Initializing GLAD..." << std::endl;

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cerr << "Failed to initialize GLAD!" << std::endl;
            return false;
        }

        // Set viewport
        glViewport(0, 0, mWidth, mHeight);

        return true;
    }

    void Window::Update()
    {
        if (!mInitialized) return;

        // Swap front and back buffers
        glfwSwapBuffers(mWindow);

        // Poll for and process events
        glfwPollEvents();
    }

    void Window::Shutdown()
    {
        if (mInitialized)
        {
            std::cout << "Shutting down Uma_Engine..." << std::endl;
            
            if (mWindow) 
            {
                glfwDestroyWindow(mWindow);
                mWindow = nullptr;
            }

            glfwTerminate();
            mInitialized = false;
        }
    }

    void Window::Close()
    {
        if (mWindow)
        {
            glfwSetWindowShouldClose(mWindow, GLFW_TRUE);
        }
    }

    bool Window::ShouldClose() const
    {
        if (!mInitialized || !mWindow) return true;
        return glfwWindowShouldClose(mWindow);
    }

    void Window::SetTitle(std::string newTitle)
    {
        glfwSetWindowTitle(mWindow, newTitle.c_str());

        mTitle = newTitle;
    }
}