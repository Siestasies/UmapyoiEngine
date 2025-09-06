#include "Window.hpp"
#include <iostream>

namespace UmapyoiEngine 
{
    
    Window::Window(int width, int height, const std::string& title)
        : m_Window(nullptr), m_Width(width), m_Height(height), m_Title(title), m_Initialized(false) {}
    
    Window::~Window() 
    {
        Shutdown();
    }
    
    bool Window::Initialize() 
    {
        std::cout << "Initializing UmapyoiEngine..." << std::endl;
        
        // Initialize GLFW
        if (!glfwInit()) 
        {
            std::cerr << "Failed to initialize GLFW!" << std::endl;
            return false;
        }
        
        // Create window
        m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
        if (!m_Window) 
        {
            std::cerr << "Failed to create GLFW window!" << std::endl;
            glfwTerminate();
            return false;
        }
        
        // Make the window's context current
        glfwMakeContextCurrent(m_Window);
        
        std::cout << "UmapyoiEngine window created: " << m_Title << " (" << m_Width << "x" << m_Height << ")" << std::endl;
        
        m_Initialized = true;
        return true;
    }
    
    void Window::Update() 
    {
        if (!m_Initialized) return;
        
        // Clear the screen to purple colour
        glClearColor(0.6f, 0.3f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Swap front and back buffers
        glfwSwapBuffers(m_Window);
        
        // Poll for and process events
        glfwPollEvents();
    }
    
    void Window::Shutdown() 
    {
        if (m_Initialized)
        {
            std::cout << "Shutting down UmapyoiEngine..." << std::endl;
            
            if (m_Window) 
            {
                glfwDestroyWindow(m_Window);
                m_Window = nullptr;
            }
            
            glfwTerminate();
            m_Initialized = false;
        }
    }
    
    bool Window::ShouldClose() const 
    {
        if (!m_Initialized || !m_Window) return true;
        return glfwWindowShouldClose(m_Window);
    }
    
    bool Window::IsKeyPressed(int key) const 
    {
        if (!m_Initialized || !m_Window) return false;
        return glfwGetKey(m_Window, key) == GLFW_PRESS;
    }
    
}