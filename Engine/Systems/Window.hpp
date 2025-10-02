/*!
\file   Window.hpp
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

#pragma once

#include <string>

struct GLFWwindow;       // forward declaration

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