#include "Window.hpp"
#include "Debugging/Debugger.hpp"
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
        Debugger::Log(WarningLevel::eInfo, "Initializing Uma_Engine...");
        
        // Initialize GLFW
        if (!glfwInit())
        {
            Debugger::Log(WarningLevel::eError, "Failed to initialize GLFW!");
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
            Debugger::Log(WarningLevel::eError, "Failed to create GLFW window!");
            glfwTerminate();
            return false;
        }

        // Make the window's context current BEFORE initializing GLAD
        glfwMakeContextCurrent(mWindow);

        // Initialize OpenGL with GLAD
        if (!InitializeOpenGL())
        {
            Debugger::Log(WarningLevel::eError, "Failed to initialize OpenGL!");
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
            Debugger::Log(WarningLevel::eError, "Failed to initialize GLAD!");
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
            Debugger::Log(WarningLevel::eInfo, "Shutting down Uma_Engine...");
            
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