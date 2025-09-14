#include "InputSystem.h"
#include <stdexcept>
#include <iostream>

namespace UmapyoiEngine
{
    // Static member definitions
    std::array<bool, GLFW_KEY_LAST + 1> InputSystem::sKeys;
    std::array<bool, GLFW_KEY_LAST + 1> InputSystem::sKeysPrevFrame;
    std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> InputSystem::sMouseButtons;
    std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> InputSystem::sMouseButtonsPrevFrame;
    double InputSystem::sMouseX = 0.0;
    double InputSystem::sMouseY = 0.0;

    InputSystem::InputSystem() : mWindow(nullptr)
    {
    }

    void InputSystem::Init()
    {
        // Initialize arrays
        sKeys.fill(false);
        sKeysPrevFrame.fill(false);
        sMouseButtons.fill(false);
        sMouseButtonsPrevFrame.fill(false);

        std::cout << "InputSystem initialized" << std::endl;
    }

    void InputSystem::SetWindow(GLFWwindow* window)
    {
        if (!window) {
            throw std::runtime_error("InputSystem requires a valid GLFWwindow.");
        }

        mWindow = window;

        // Set GLFW input callbacks
        glfwSetKeyCallback(mWindow, KeyCallback);
        glfwSetMouseButtonCallback(mWindow, MouseButtonCallback);
        glfwSetCursorPosCallback(mWindow, CursorPositionCallback);

        std::cout << "InputSystem window set and callbacks registered" << std::endl;
    }

    void InputSystem::Update(float dt)
    {
        // Only process input if window is set
        if (!mWindow) return;

        // Optional: Print input state changes (you may want to remove this in production)
        
        for (int i = 0; i <= GLFW_KEY_LAST; ++i) {
            const char* name = GetKeyName(i);
            if (name && strcmp(name, "UNKNOWN") != 0) {
                if (sKeys[i] && !sKeysPrevFrame[i]) {
                    std::cout << name << " pressed" << std::endl;
                }
                else if (!sKeys[i] && sKeysPrevFrame[i]) {
                    std::cout << name << " released" << std::endl;
                }
            }
        }

        for (int i = 0; i <= GLFW_MOUSE_BUTTON_LAST; ++i) {
            const char* name = (i == 0) ? "LEFT_MOUSE" : (i == 1) ? "RIGHT_MOUSE" : (i == 2) ? "MIDDLE_MOUSE" : nullptr;
            if (name) {
                if (sMouseButtons[i] && !sMouseButtonsPrevFrame[i]) {
                    std::cout << name << " pressed" << std::endl;
                }
                else if (!sMouseButtons[i] && sMouseButtonsPrevFrame[i]) {
                    std::cout << name << " released" << std::endl;
                }
            }
        }
        
        // Update previous frame state
        sKeysPrevFrame = sKeys;
        sMouseButtonsPrevFrame = sMouseButtons;
    }

    void InputSystem::Shutdown()
    {
        // Clear callbacks if window is still valid
        if (mWindow) {
            glfwSetKeyCallback(mWindow, nullptr);
            glfwSetMouseButtonCallback(mWindow, nullptr);
            glfwSetCursorPosCallback(mWindow, nullptr);
        }

        std::cout << "InputSystem shut down" << std::endl;
    }

    // Static callback functions
    void InputSystem::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (key >= 0 && key <= GLFW_KEY_LAST)
        {
            if (action == GLFW_PRESS) {
                sKeys[key] = true;
            }
            else if (action == GLFW_RELEASE) {
                sKeys[key] = false;
            }
            // Note: GLFW_REPEAT action maintains the current state
        }
    }

    void InputSystem::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        if (button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST)
        {
            if (action == GLFW_PRESS) {
                sMouseButtons[button] = true;
            }
            else if (action == GLFW_RELEASE) {
                sMouseButtons[button] = false;
            }
        }
    }

    void InputSystem::CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
    {
        sMouseX = xpos;
        sMouseY = ypos;
        std::cout << "Mouse position: (" << sMouseX << ", " << sMouseY << ")" << std::endl;
    }

    // Static input query methods
    bool InputSystem::KeyDown(int key)
    {
        return (key >= 0 && key <= GLFW_KEY_LAST) ? sKeys[key] : false;
    }

    bool InputSystem::KeyPressed(int key)
    {
        return (key >= 0 && key <= GLFW_KEY_LAST) ? sKeys[key] && !sKeysPrevFrame[key] : false;
    }

    bool InputSystem::KeyReleased(int key)
    {
        return (key >= 0 && key <= GLFW_KEY_LAST) ? !sKeys[key] && sKeysPrevFrame[key] : false;
    }

    bool InputSystem::MouseButtonDown(int button)
    {
        return (button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST) ? sMouseButtons[button] : false;
    }

    bool InputSystem::MouseButtonPressed(int button)
    {
        return (button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST) ? sMouseButtons[button] && !sMouseButtonsPrevFrame[button] : false;
    }

    bool InputSystem::MouseButtonReleased(int button)
    {
        return (button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST) ? !sMouseButtons[button] && sMouseButtonsPrevFrame[button] : false;
    }

    void InputSystem::GetMousePosition(double& x, double& y)
    {
        x = sMouseX;
        y = sMouseY;
    }

    double InputSystem::GetMouseX()
    {
        return sMouseX;
    }

    double InputSystem::GetMouseY()
    {
        return sMouseY;
    }

    const char* InputSystem::GetKeyName(int key)
    {
        switch (key) {
            // Numbers
        case GLFW_KEY_0: return "0"; case GLFW_KEY_1: return "1"; case GLFW_KEY_2: return "2";
        case GLFW_KEY_3: return "3"; case GLFW_KEY_4: return "4"; case GLFW_KEY_5: return "5";
        case GLFW_KEY_6: return "6"; case GLFW_KEY_7: return "7"; case GLFW_KEY_8: return "8";
        case GLFW_KEY_9: return "9";

            // Letters
        case GLFW_KEY_A: return "A"; case GLFW_KEY_B: return "B"; case GLFW_KEY_C: return "C";
        case GLFW_KEY_D: return "D"; case GLFW_KEY_E: return "E"; case GLFW_KEY_F: return "F";
        case GLFW_KEY_G: return "G"; case GLFW_KEY_H: return "H"; case GLFW_KEY_I: return "I";
        case GLFW_KEY_J: return "J"; case GLFW_KEY_K: return "K"; case GLFW_KEY_L: return "L";
        case GLFW_KEY_M: return "M"; case GLFW_KEY_N: return "N"; case GLFW_KEY_O: return "O";
        case GLFW_KEY_P: return "P"; case GLFW_KEY_Q: return "Q"; case GLFW_KEY_R: return "R";
        case GLFW_KEY_S: return "S"; case GLFW_KEY_T: return "T"; case GLFW_KEY_U: return "U";
        case GLFW_KEY_V: return "V"; case GLFW_KEY_W: return "W"; case GLFW_KEY_X: return "X";
        case GLFW_KEY_Y: return "Y"; case GLFW_KEY_Z: return "Z";

            // Function Keys
        case GLFW_KEY_F1: return "F1"; case GLFW_KEY_F2: return "F2"; case GLFW_KEY_F3: return "F3";
        case GLFW_KEY_F4: return "F4"; case GLFW_KEY_F5: return "F5"; case GLFW_KEY_F6: return "F6";
        case GLFW_KEY_F7: return "F7"; case GLFW_KEY_F8: return "F8"; case GLFW_KEY_F9: return "F9";
        case GLFW_KEY_F10: return "F10"; case GLFW_KEY_F11: return "F11"; case GLFW_KEY_F12: return "F12";

            // Special Keys
        case GLFW_KEY_SPACE: return "SPACE";
        case GLFW_KEY_APOSTROPHE: return "'";
        case GLFW_KEY_COMMA: return ",";
        case GLFW_KEY_MINUS: return "-";
        case GLFW_KEY_PERIOD: return ".";
        case GLFW_KEY_SLASH: return "/";
        case GLFW_KEY_SEMICOLON: return ";";
        case GLFW_KEY_EQUAL: return "=";
        case GLFW_KEY_LEFT_BRACKET: return "[";
        case GLFW_KEY_BACKSLASH: return "\\";
        case GLFW_KEY_RIGHT_BRACKET: return "]";
        case GLFW_KEY_GRAVE_ACCENT: return "`";

            // Control Keys
        case GLFW_KEY_ESCAPE: return "ESC";
        case GLFW_KEY_ENTER: return "ENTER";
        case GLFW_KEY_TAB: return "TAB";
        case GLFW_KEY_BACKSPACE: return "BACKSPACE";
        case GLFW_KEY_INSERT: return "INSERT";
        case GLFW_KEY_DELETE: return "DELETE";
        case GLFW_KEY_HOME: return "HOME";
        case GLFW_KEY_END: return "END";
        case GLFW_KEY_PAGE_UP: return "PAGE_UP";
        case GLFW_KEY_PAGE_DOWN: return "PAGE_DOWN";

            // Arrow Keys
        case GLFW_KEY_RIGHT: return "RIGHT";
        case GLFW_KEY_LEFT: return "LEFT";
        case GLFW_KEY_DOWN: return "DOWN";
        case GLFW_KEY_UP: return "UP";

            // Modifier Keys
        case GLFW_KEY_LEFT_SHIFT: return "LSHIFT";
        case GLFW_KEY_RIGHT_SHIFT: return "RSHIFT";
        case GLFW_KEY_LEFT_CONTROL: return "LCTRL";
        case GLFW_KEY_RIGHT_CONTROL: return "RCTRL";
        case GLFW_KEY_LEFT_ALT: return "LALT";
        case GLFW_KEY_RIGHT_ALT: return "RALT";
        case GLFW_KEY_LEFT_SUPER: return "LSUPER";
        case GLFW_KEY_RIGHT_SUPER: return "RSUPER";
        case GLFW_KEY_MENU: return "MENU";

            // Numpad Keys
        case GLFW_KEY_KP_0: return "KP_0";
        case GLFW_KEY_KP_1: return "KP_1";
        case GLFW_KEY_KP_2: return "KP_2";
        case GLFW_KEY_KP_3: return "KP_3";
        case GLFW_KEY_KP_4: return "KP_4";
        case GLFW_KEY_KP_5: return "KP_5";
        case GLFW_KEY_KP_6: return "KP_6";
        case GLFW_KEY_KP_7: return "KP_7";
        case GLFW_KEY_KP_8: return "KP_8";
        case GLFW_KEY_KP_9: return "KP_9";
        case GLFW_KEY_KP_DECIMAL: return "KP_DECIMAL";
        case GLFW_KEY_KP_DIVIDE: return "KP_DIVIDE";
        case GLFW_KEY_KP_MULTIPLY: return "KP_MULTIPLY";
        case GLFW_KEY_KP_SUBTRACT: return "KP_SUBTRACT";
        case GLFW_KEY_KP_ADD: return "KP_ADD";
        case GLFW_KEY_KP_ENTER: return "KP_ENTER";
        case GLFW_KEY_KP_EQUAL: return "KP_EQUAL";

        default: return "UNKNOWN";
        }
    }
}