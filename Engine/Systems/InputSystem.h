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
        InputSystem();

        // ISystem interface
        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;

        // IWindowSystem interface
        void SetWindow(GLFWwindow* window) override;

        // Static callbacks for GLFW
        static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
        static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

        // Input query methods
        static bool KeyDown(int key);
        static bool KeyPressed(int key);
        static bool KeyReleased(int key);
        static bool MouseButtonDown(int button);
        static bool MouseButtonPressed(int button);
        static bool MouseButtonReleased(int button);
        static void GetMousePosition(double& x, double& y);
        static double GetMouseX();
        static double GetMouseY();

        static void UpdatePreviousFrameState();

    private:
        static const char* GetKeyName(int key);

        // Static state - accessible from callbacks
        static std::vector<bool> sKeys;
        static std::vector<bool> sKeysPrevFrame;
        static std::vector<bool> sMouseButtons;
        static std::vector<bool> sMouseButtonsPrevFrame;
        static double sMouseX, sMouseY;

        // Instance data
        GLFWwindow* mWindow;
    };
}