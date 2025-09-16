#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

namespace Uma_Engine
{
    // Optional interface for systems that need window access
    class IWindowSystem
    {
    public:
        virtual ~IWindowSystem() = default;
        virtual void SetWindow(GLFWwindow* window) = 0;
    };

    class Window
    {
    public:
        Window(int width, int height, const std::string& title);
        ~Window();

        bool Initialize();
        void Update();
        void Shutdown();

        bool ShouldClose() const;
        void Close();

        int GetWidth() const { return mWidth; }
        int GetHeight() const { return mHeight; }

        GLFWwindow* GetGLFWWindow() const { return mWindow; }

        void SetTitle(std::string newTitle);
        
    private:
        GLFWwindow* mWindow;
        int mWidth;
        int mHeight;
        std::string mTitle;
        bool mInitialized;

        bool InitializeOpenGL();
    };
}