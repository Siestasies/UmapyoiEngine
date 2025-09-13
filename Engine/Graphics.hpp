#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>  // Need this for glfwGetProcAddress
#include <string>

namespace UmapyoiEngine
{
    class Graphics
    {
    public:
        Graphics();
        ~Graphics();

        // Initialize graphics system
        bool Initialize();
        void Shutdown();

        // Background color
        void ClearBackground(float r = 0.0f, float g = 0.0f, float b = 0.0f);

    private:
        bool mInitialized;
    };
}