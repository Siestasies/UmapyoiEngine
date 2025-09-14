#include "Window.hpp"
#include <iostream>

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
        
        // Create window
        mWindow = glfwCreateWindow(mWidth, mHeight, mTitle.c_str(), nullptr, nullptr);
        if (!mWindow) 
        {
            std::cerr << "Failed to create GLFW window!" << std::endl;
            glfwTerminate();
            return false;
        }
        
        // Make the window's context current
        glfwMakeContextCurrent(mWindow);
        
        std::cout << "Uma_Engine window created: " << mTitle << " (" << mWidth << "x" << mHeight << ")" << std::endl;
        
        mInitialized = true;
        return true;
    }
    
    void Window::Update() 
    {
        if (!mInitialized) return;
        
        // Clear the screen to purple colour
        glClearColor(0.6f, 0.3f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
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
}