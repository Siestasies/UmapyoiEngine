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

        /**
        * \brief Constructs a Window object with dimensions and title
        * \param width The width of the window in pixels
        * \param height The height of the window in pixels
        * \param title The title text to display in the window's title bar
        */
        Window(int width, int height, const std::string& title);

        /**
        * \brief Destructor that ensures proper cleanup of window resources
        */
        ~Window();

        /**
         * \brief Initializes OpenGL using GLAD and sets up the viewport
         * \return true if succeeded, false otherwise
         */
        bool InitializeOpenGL();

        /**
        * \brief Initializes the window system, GLFW, and OpenGL context
        * \return true if succeeded, false otherwise
        */
        bool Initialize();

        /**
         * \brief Updates the window by swapping buffers and polling events
         */
        void Update();

        /**
        * \brief Shuts down the window system and releases all resources
        */
        void Shutdown();

        /**
        * \brief Checks if the window should close
        * \return true if the window should close, false otherwise
        */
        bool ShouldClose() const;

        /**
        * \brief Signals the window to close on the next frame
        */
        void Close();

        /**
        * \brief Gets the width of the window
        * \return The window width in pixels
        */
        int GetWidth() const { return mWidth; }

        /**
        * \brief Gets the height of the window
        * \return The window height in pixels
        */
        int GetHeight() const { return mHeight; }

        /**
        * \brief Gets the GLFW window pointer
        * \return Pointer to the GLFWwindow object, nullptr if not initialized
        */
        GLFWwindow* GetGLFWWindow() const { return mWindow; }

        /**
        * \brief Changes the window's title text
        * \param newTitle The new title string to display
        */
        void SetTitle(std::string newTitle);
        
    private:
        GLFWwindow* mWindow;
        int mWidth;
        int mHeight;
        std::string mTitle;
        bool mInitialized;
    };
}