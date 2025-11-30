/*!
\file   Window.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Javier Chua Dong Qing (100%)
\par    E-mail: javierdongqing.chua@digipen.edu
\par    DigiPen login: javierdongqing.chua

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
    Window::Window(int width, int height, const std::string& title, bool isEditorMode, WindowMode mode) : mWindow(nullptr),
        mWidth(width), mHeight(height), mWindowedWidth(width), mWindowedHeight(height), mTitle(title),
        mInitialized(false), mMode(mode), mIsEditorMode(isEditorMode) {}

    Window::~Window()
    {
        Shutdown();
    }

    bool Window::Initialize()
    {
        std::cout << "Initializing Uma_Engine..." << std::endl;

        if (!glfwInit())
        {
            std::cerr << "Failed to initialize GLFW!" << std::endl;
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Set resizable based on editor mode
        if (mIsEditorMode)
        {
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        }
        else
        {
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        }

        GLFWmonitor* monitor = nullptr;
        if (mMode == WindowMode::Fullscreen)
        {
            monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            mWidth = mode->width;
            mHeight = mode->height;
            glfwWindowHint(GLFW_RED_BITS, mode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        }
        else if (mMode == WindowMode::Maximized)
        {
            glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
        }

        mWindow = glfwCreateWindow(mWidth, mHeight, mTitle.c_str(), monitor, nullptr);
        if (!mWindow)
        {
            std::cerr << "Failed to create GLFW window!" << std::endl;
            glfwTerminate();
            return false;
        }

        // Center window if windowed mode
        if (mMode == WindowMode::Windowed)
        {
            GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
            int xpos = (videoMode->width - mWidth) / 2;
            int ypos = (videoMode->height - mHeight) / 2;
            glfwSetWindowPos(mWindow, xpos, ypos);
        }

        glfwMakeContextCurrent(mWindow);

        if (!InitializeOpenGL())
        {
            std::cerr << "Failed to initialize OpenGL!" << std::endl;
            glfwDestroyWindow(mWindow);
            glfwTerminate();
            return false;
        }

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
        //glfwSwapBuffers(mWindow);

        // Poll for and process events
        glfwPollEvents();
    }

    void Window::Shutdown()
    {
        // Only shut down if initialized
        if (mInitialized)
        {
            std::cout << "Shutting down Uma_Engine..." << std::endl;
            
            // Destroy the window and free its resources
            if (mWindow) 
            {
                glfwDestroyWindow(mWindow);
                mWindow = nullptr;
            }

            // Terminate GLFW and clean up all GLFW resources
            glfwTerminate();

            // Mark as no longer initialized
            mInitialized = false;
        }
    }

    void Window::Close()
    {
        // Only set close flag if window exists
        if (mWindow)
        {
            // Set the window's close flag to true
            glfwSetWindowShouldClose(mWindow, GLFW_TRUE);
        }
    }

    bool Window::ShouldClose() const
    {
        // If not initialized or window doesn't exist, should close
        if (!mInitialized || !mWindow) return true;

        // Check GLFW's window close flag
        return glfwWindowShouldClose(mWindow);
    }

    void Window::SetTitle(std::string newTitle)
    {
        // Update the window's title bar
        glfwSetWindowTitle(mWindow, newTitle.c_str());

        // Store the new title
        mTitle = newTitle;
    }

    void Window::SetWindowMode(WindowMode mode)
    {
        if (!mWindow || mMode == mode) return;

        mMode = mode;

        if (mMode == WindowMode::Fullscreen)
        {
            // Save current windowed dimensions
            glfwGetWindowSize(mWindow, &mWindowedWidth, &mWindowedHeight);

            // Get monitor
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);

            // Switch to fullscreen
            glfwSetWindowMonitor(mWindow, monitor, 0, 0,
                videoMode->width, videoMode->height,
                videoMode->refreshRate);

            mWidth = videoMode->width;
            mHeight = videoMode->height;
        }
        else if (mMode == WindowMode::Maximized)
        {
            // Switch to windowed mode first
            glfwSetWindowMonitor(mWindow, nullptr, 0, 0,
                mWindowedWidth, mWindowedHeight,
                GLFW_DONT_CARE);

            // Then maximize
            glfwMaximizeWindow(mWindow);

            glfwGetWindowSize(mWindow, &mWidth, &mHeight);
        }
        else
        {
            // Calculate centered position
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
            int xpos = (videoMode->width - mWindowedWidth) / 2;
            int ypos = (videoMode->height - mWindowedHeight) / 2;

            // Switch to windowed mode
            glfwSetWindowMonitor(mWindow, nullptr, xpos, ypos,
                mWindowedWidth, mWindowedHeight,
                GLFW_DONT_CARE);

            mWidth = mWindowedWidth;
            mHeight = mWindowedHeight;
        }

        // Update OpenGL viewport
        glViewport(0, 0, mWidth, mHeight);

        // Ensure cursor is visible after window mode change
        // This must be set AFTER glfwSetWindowMonitor to prevent it from being reset
        glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    void Window::ToggleFullscreen()
    {
        SetWindowMode(mMode == WindowMode::Fullscreen ? WindowMode::Windowed : WindowMode::Fullscreen);
    }
}