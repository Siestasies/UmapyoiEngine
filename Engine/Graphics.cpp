#include "Graphics.hpp"
#include <iostream>

namespace UmapyoiEngine
{
    Graphics::Graphics() : mInitialized(false) {}

    Graphics::~Graphics()
    {
        Shutdown();
    }

    bool Graphics::Initialize()
    {
        if (mInitialized)
        {
            std::cout << "Graphics already initialized!" << std::endl;
            return true;
        }

        // Check if OpenGL context is available
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cerr << "GLAD not initialized! Make sure Window::Initialize() was called first." << std::endl;
            return false;
        }

        // Check OpenGL version
        std::cout << "Graphics initializing..." << std::endl;
        std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
        std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;

        // Enable blending for transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        std::cout << "Graphics system initialized successfully!" << std::endl;

        mInitialized = true;
        return true;
    }

    void Graphics::Shutdown()
    {
        if (mInitialized)
        {
            std::cout << "Shutting down graphics system..." << std::endl;
            mInitialized = false;
        }
    }

    void Graphics::ClearBackground(float r, float g, float b)
    {
        if (!mInitialized) return;

        // Clear the screen with the specified color
        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
}