/*!
\file   InputSystem.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Javier Chua Dong Qing (100%)
\par    E-mail: javierdongqing.chua@digipen.edu
\par    DigiPen login: javierdongqing.chua

\brief
Declaration of a GLFW-based input handling system class that manages keyboard and mouse input through callbacks and query functions.
Supports detection of key/button down, pressed (single-frame), and released states.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once
#include <vector>

//#include <GLFW/glfw3.h>   // needed for GLFW_KEY_LAST, GLFW_MOUSE_BUTTON_LAST

#include "../Core/SystemType.h"
#include "Window.hpp"

namespace Uma_Engine
{
    class InputSystem : public ISystem, public IWindowSystem
    {
    public:
        /**
         * \brief Default constructor for InputSystem
         * Initializes the input system with no window attached
         */
        InputSystem();

        // ISystem interface

        /**
         * \brief Initializes the input system arrays and state
         */
        void Init() override;

        /**
         * \brief Updates the input system
         * \param dt Delta time
         *
         * Updates the current mouse cursor position by querying GLFW
         * Should be called once per frame before processing input
         */
        void Update(float dt) override;

        /**
         * \brief Shuts down the input system
         */
        void Shutdown() override;

        // IWindowSystem interface

        /**
         * \brief Sets the GLFW window for input processing and registers callbacks
         * \param window Pointer to the GLFWwindow
         * \throws std::runtime_error if window is nullptr
         */
        void SetWindow(GLFWwindow* window) override;

        // Static callbacks for GLFW

        /**
         * \brief GLFW callback for keyboard key events
         * \param window GLFW window
         * \param key Keyboard key that was pressed or released
         * \param scancode System-specific scancode of the key
         * \param action GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT
         * \param mods Bit field describing which modifier keys were held down
         *
         * Updates the internal key state arrays when keys are pressed or released
         */
        static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

        /**
         * \brief GLFW callback for mouse button events
         * \param window GLFW window
         * \param button Mouse button that was pressed or released
         * \param action GLFW_PRESS or GLFW_RELEASE
         * \param mods Bit field describing which modifier keys were held down
         *
         * Updates the internal mouse button state arrays when buttons are pressed or released
         */
        static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

        /**
         * \brief GLFW callback for cursor position changes
         * \param window GLFW window
         * \param xpos New x-coordinate of the cursor
         * \param ypos New y-coordinate of the cursor
         *
         * Updates the internal mouse position variables whenever the cursor moves
         */
        static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

        /**
         * \brief Updates the previous frame state for input detection
         */
        static void UpdatePreviousFrameState();

        // Input query methods

        /**
         * \brief Checks if a key is currently held down
         * \param key GLFW key code
         * \return true if the key is currently pressed, false otherwise
         */
        static bool KeyDown(int key);

        /**
         * \brief Checks if a key was just pressed this frame
         * \param key GLFW key code
         * \return true if the key is pressed now but wasn't pressed last frame, false otherwise
         */
        static bool KeyPressed(int key);

        /**
         * \brief Checks if a key was just released this frame
         * \param key GLFW key code
         * \return true if the key is not pressed now but was pressed last frame, false otherwise
         */
        static bool KeyReleased(int key);

        /**
         * \brief Checks if a mouse button is currently held down
         * \param button GLFW mouse button code
         * \return true if the button is currently pressed, false otherwise
         */
        static bool MouseButtonDown(int button);

        /**
         * \brief Checks if a mouse button was just pressed this frame
         * \param button GLFW mouse button code
         * \return true if the button is pressed now but wasn't pressed last frame, false otherwise
         */
        static bool MouseButtonPressed(int button);

        /**
         * \brief Checks if a mouse button was just released this frame
         * \param button GLFW mouse button code
         * \return true if the button is not pressed now but was pressed last frame, false otherwise
         */
        static bool MouseButtonReleased(int button);

        /**
         * \brief Gets the current mouse cursor position
         * \param x Reference to store the x-coordinate
         * \param y Reference to store the y-coordinate
         */
        static void GetMousePosition(double& x, double& y);

        /**
         * \brief Gets the current mouse cursor x-coordinate
         * \return x-coordinate of the mouse cursor
         */
        static double GetMouseX();

        /**
         * \brief Gets the current mouse cursor y-coordinate
         * \return y-coordinate of the mouse cursor
         */
        static double GetMouseY();

        /**
         * \brief Gets the key name
         * \return y-coordinate of the mouse cursor
         */
        static const char* GetKeyName(int key);

    private:
        // Static state
        static std::vector<bool> sKeys;
        static std::vector<bool> sKeysPrevFrame;
        static std::vector<bool> sMouseButtons;
        static std::vector<bool> sMouseButtonsPrevFrame;
        static double sMouseX, sMouseY;

        // Instance data
        GLFWwindow* mWindow;
    };
}