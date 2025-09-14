#pragma once
#include <array>
#include <GLFW/glfw3.h>
#include "Core/SystemType.h"

namespace UmapyoiEngine
{
    class InputSystem : public EngineSystem::ISystem, public EngineSystem::IWindowSystem
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

    private:
        static const char* GetKeyName(int key);

        // Static state - accessible from callbacks
        static std::array<bool, GLFW_KEY_LAST + 1> sKeys;
        static std::array<bool, GLFW_KEY_LAST + 1> sKeysPrevFrame;
        static std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> sMouseButtons;
        static std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> sMouseButtonsPrevFrame;
        static double sMouseX, sMouseY;

        // Instance data
        GLFWwindow* mWindow;
    };
}