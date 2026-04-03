/*!
\file   InputSystem.cpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Javier Chua Dong Qing (50%)
\par    E-mail: javierdongqing.chua@digipen.edu
\par    DigiPen login: javierdongqing.chua

\author Leong Wai Men (50%, all controller input)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Definition of a GLFW-based input handling system class that manages keyboard and mouse input through callbacks and query functions.
Supports detection of key/button down, pressed (single-frame), and released states.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "InputSystem.h"
#include "Debugging/Debugger.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <fstream>
#include <GLFW/glfw3.h>

#include "../Core/FilePaths.h"

// Include ImGui for input checking
#include "../EditorApp/imgui/imgui.h"

// comment and uncomment this line below to enable/ disable console debug log
#if defined(_DEBUG) || defined(DEBUG)
//#define _DEBUG_LOG
#endif

namespace Uma_Engine
{
    struct ControllerInput
    {
        GLFWgamepadstate currState;
        GLFWgamepadstate prevState; 
        bool anyButtonPressed = false;
        bool anyAxisChanged = false;
        float axisThreshold = 0.1f;

        void UpdateState(const GLFWgamepadstate& state)
        {
            prevState = currState;
            currState = state;

            anyButtonPressed = false;
            for (int i = 0; i < GLFW_GAMEPAD_BUTTON_LAST; i++)
            {
                if (currState.buttons[i] == GLFW_PRESS &&
                    prevState.buttons[i] == GLFW_RELEASE)
                {
                    anyButtonPressed = true;
                    break;
                }
            }

            anyAxisChanged = false;
            for (int i = 0; i < GLFW_GAMEPAD_AXIS_LAST; i++)
            {
                if (std::abs(currState.axes[i] - prevState.axes[i]) > axisThreshold)
                {
                    anyAxisChanged = true;
                    break;
                }
            }
        }
    };

    // Static member definitions
    std::vector<bool> Uma_Engine::InputSystem::sKeys(GLFW_KEY_LAST + 1);
    std::vector<bool> Uma_Engine::InputSystem::sKeysPrevFrame(GLFW_KEY_LAST + 1);
    std::vector<bool> Uma_Engine::InputSystem::sMouseButtons(GLFW_MOUSE_BUTTON_LAST + 1);
    std::vector<bool> Uma_Engine::InputSystem::sMouseButtonsPrevFrame(GLFW_MOUSE_BUTTON_LAST + 1);

    double InputSystem::sMouseX = 0.0;
    double InputSystem::sMouseY = 0.0;
    double InputSystem::sScrollX = 0.0;
    double InputSystem::sScrollY = 0.0;

    int InputSystem::sCurrInputMethod = 0;

    InputSystem::InputSystem() : mWindow(nullptr) {}

    std::unordered_map<int, std::unique_ptr<ControllerInput>> InputSystem::sActiveController;

    void InputSystem::Init()
    {
        // Initialize arrays
        sKeys.assign(GLFW_KEY_LAST + 1, false);
        sKeysPrevFrame.assign(GLFW_KEY_LAST + 1, false);
        sMouseButtons.assign(GLFW_MOUSE_BUTTON_LAST + 1, false);
        sMouseButtonsPrevFrame.assign(GLFW_MOUSE_BUTTON_LAST + 1, false);

        SetUpSDLControllerDB();

#ifdef _DEBUG_LOG
        std::cout << "InputSystem initialized" << std::endl;
#endif // !_DEBUG_LOG
    }

    void InputSystem::ResetInputState()
    {
        // Reset Key States
        std::fill(sKeys.begin(), sKeys.end(), false);
        std::fill(sKeysPrevFrame.begin(), sKeysPrevFrame.end(), false);

        // Reset Mouse Button States
        std::fill(sMouseButtons.begin(), sMouseButtons.end(), false);
        std::fill(sMouseButtonsPrevFrame.begin(), sMouseButtonsPrevFrame.end(), false);

        // Reset Scroll
        sScrollX = 0.0;
        sScrollY = 0.0;
    }

    void InputSystem::SetWindow(GLFWwindow* window)
    {
        if (!window) {
            throw std::runtime_error("InputSystem requires a valid GLFWwindow.");
        }

        mWindow = window;

        // Set callbacks for inputs
        glfwSetKeyCallback(mWindow, KeyCallback);
        glfwSetMouseButtonCallback(mWindow, MouseButtonCallback);
        glfwSetCursorPosCallback(mWindow, CursorPositionCallback);
        glfwSetScrollCallback(mWindow, MouseScrollCallback);

        // check all joystick slots for controllers already connected (including BT)
        for (int jid = 0; jid <= GLFW_JOYSTICK_LAST; ++jid)
        {
            if (glfwJoystickPresent(jid))
            {
                ControllerConnectionCallback(jid, GLFW_CONNECTED);
            }
        }

        // controller
        glfwSetJoystickCallback(ControllerConnectionCallback);
    }

    void InputSystem::Update(float dt)
    {
        (void)dt;

        // Only process input if window is set
        if (!mWindow) return;

        // Get ImGui IO to check if ImGui wants input
        //ImGuiIO& io = ImGui::GetIO();

        // Update mouse position
        double xpos, ypos;
        glfwGetCursorPos(mWindow, &xpos, &ypos);

        if (xpos != sMouseX || ypos != sMouseY)
        {
            sCurrInputMethod = 0;
        }

        sMouseX = xpos;
        sMouseY = ypos;

        // Update previous frame state
        // sKeysPrevFrame = sKeys;
        // sMouseButtonsPrevFrame = sMouseButtons;

#ifdef _DEBUG_LOG
        // Log held keys
        for (int i = 0; i <= GLFW_KEY_LAST; ++i) {
            if (sKeys[i] && sKeysPrevFrame[i]) {
                //std::cout << "Key HELD: " << GetKeyName(i) << " (" << i << ")" << std::endl;

                std::stringstream ss{""};
                ss << "Key HELD: " << GetKeyName(i) << " (" << i << ")";
                Debugger::Log(WarningLevel::eInfo, ss.str());
            }
        }

        // Log held mouse buttons
        for (int i = 0; i <= GLFW_MOUSE_BUTTON_LAST; ++i) {
            if (sMouseButtons[i] && sMouseButtonsPrevFrame[i]) {
                //std::cout << "Mouse button HELD: " << i << std::endl;
                std::stringstream ss{ "" };
                ss << "Mouse button HELD: " << i;
                Debugger::Log(WarningLevel::eInfo, ss.str());
            }
        }
#endif

        static float pollTimer = 0.f;
        pollTimer += dt;

        if (pollTimer >= 1.0f) // check once per second
        {
            pollTimer = 0.f;
            for (int jid = 0; jid <= GLFW_JOYSTICK_LAST; ++jid)
            {
                bool present = glfwJoystickPresent(jid);
                bool tracked = sActiveController.count(jid);

                if (present && !tracked)
                    ControllerConnectionCallback(jid, GLFW_CONNECTED); // BT controller appeared
                else if (!present && tracked)
                    ControllerConnectionCallback(jid, GLFW_DISCONNECTED);    // BT controller dropped
            }
        }

        // controller polling
        for (auto& [id, controller] : sActiveController)
        {
            // --- Gamepad mapped state ---
            GLFWgamepadstate state;

            if (glfwGetGamepadState(id, &state))
            {
                controller->UpdateState(state);

                if (controller->anyAxisChanged || controller->anyButtonPressed)
                    sCurrInputMethod = 1;
            }
        }

#ifdef _DEBUG_LOG
        // DEBUGGING 

        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_DPAD_UP, GLFW_PRESS, 0);
        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_DPAD_UP, GLFW_REPEAT, 0);
        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_DPAD_UP, GLFW_RELEASE, 0);

        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_DPAD_DOWN, GLFW_PRESS, 0);
        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_DPAD_DOWN, GLFW_REPEAT, 0);
        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_DPAD_DOWN, GLFW_RELEASE, 0);

        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_DPAD_LEFT, GLFW_PRESS, 0);
        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_DPAD_LEFT, GLFW_REPEAT, 0);
        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_DPAD_LEFT, GLFW_RELEASE, 0);

        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, GLFW_PRESS, 0);
        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, GLFW_REPEAT, 0);
        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, GLFW_RELEASE, 0);

        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_A, GLFW_PRESS, 0);
        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_A, GLFW_REPEAT, 0);
        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_A, GLFW_RELEASE, 0);

        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_B, GLFW_PRESS, 0);
        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_B, GLFW_REPEAT, 0);
        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_B, GLFW_RELEASE, 0);

        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_X, GLFW_PRESS, 0);
        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_X, GLFW_REPEAT, 0);
        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_X, GLFW_RELEASE, 0);

        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_Y, GLFW_PRESS, 0);
        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_Y, GLFW_REPEAT, 0);
        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_Y, GLFW_RELEASE, 0);

        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, GLFW_PRESS, 0);
        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, GLFW_REPEAT, 0);
        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, GLFW_RELEASE, 0);

        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, GLFW_PRESS, 0);
        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, GLFW_REPEAT, 0);
        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, GLFW_RELEASE, 0);

        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_LEFT_THUMB, GLFW_PRESS, 0);
        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_LEFT_THUMB, GLFW_REPEAT, 0);
        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_LEFT_THUMB, GLFW_RELEASE, 0);

        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, GLFW_PRESS, 0);
        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, GLFW_REPEAT, 0);
        GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, GLFW_RELEASE, 0);

        GetControllerAxesInput(GLFW_GAMEPAD_AXIS_LEFT_X, 0);
        GetControllerAxesInput(GLFW_GAMEPAD_AXIS_LEFT_Y, 0);
        GetControllerAxesInput(GLFW_GAMEPAD_AXIS_RIGHT_X, 0);
        GetControllerAxesInput(GLFW_GAMEPAD_AXIS_RIGHT_Y, 0);
        GetControllerAxesInput(GLFW_GAMEPAD_AXIS_LEFT_TRIGGER, 0);
        GetControllerAxesInput(GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER, 0);

#endif
    }

    void InputSystem::Shutdown()
    {
        if (mWindow)
        {
            glfwSetKeyCallback(mWindow, nullptr);
            glfwSetMouseButtonCallback(mWindow, nullptr);
            glfwSetCursorPosCallback(mWindow, nullptr);
        }


#ifdef _DEBUG_LOG
        std::cout << "InputSystem shut down" << std::endl;
#endif // !_DEBUG_LOG
    }

    // Keep callback functions for potential future use or chaining
    void InputSystem::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        (void)window; (void)mods; (void)scancode;

        sCurrInputMethod = 0;

        if (key >= 0 && key <= GLFW_KEY_LAST)
        {
            if (action == GLFW_PRESS) {
                sKeys[key] = true;
#ifdef _DEBUG_LOG
                //std::cout << "Key pressed: " << GetKeyName(key) << " (" << key << ")" << std::endl;
                std::stringstream ss{ "" };
                ss << "Key pressed: " << GetKeyName(key) << " (" << key << ")";
                Debugger::Log(WarningLevel::eInfo, ss.str());
#endif
            }
            else if (action == GLFW_RELEASE) {
                sKeys[key] = false;
#ifdef _DEBUG_LOG
                //std::cout << "Key released: " << GetKeyName(key) << " (" << key << ")" << std::endl;
                std::stringstream ss{ "" };
                ss << "Key released: " << GetKeyName(key) << " (" << key << ")";
                Debugger::Log(WarningLevel::eInfo, ss.str());
#endif
            }
        }
    }

    void InputSystem::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        (void)window; (void)mods;

        sCurrInputMethod = 0;

        if (button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST)
        {
            if (action == GLFW_PRESS) {
                sMouseButtons[button] = true;
#ifdef _DEBUG_LOG
                //std::cout << "Mouse button pressed: " << button << std::endl;
                std::stringstream ss{ "" };
                ss << "Mouse button pressed: " << button;
                Debugger::Log(WarningLevel::eInfo, ss.str());
#endif
            }
            else if (action == GLFW_RELEASE) {
                sMouseButtons[button] = false;
#ifdef _DEBUG_LOG
                //std::cout << "Mouse button released: " << button << std::endl;
                std::stringstream ss{ "" };
                ss << "Mouse button released: " << button;
                Debugger::Log(WarningLevel::eInfo, ss.str());
#endif
            }
        }
    }

    void InputSystem::MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        (void)window;
        sScrollX = xoffset;
        sScrollY = yoffset;
    }

    void InputSystem::CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
    {
        (void)window;

        sMouseX = xpos;
        sMouseY = ypos;

#ifdef _DEBUG_LOG
        //std::cout << "Mouse position: (" << sMouseX << ", " << sMouseY << ")" << std::endl;
        std::stringstream ss{ "" };
        ss << "Mouse pos: (" << sMouseX << ", " << sMouseY << ")";
        Debugger::Log(WarningLevel::eInfo, ss.str());
#endif // !_DEBUG_LOG
    }

    bool InputSystem::KeyDown(int key) { return (key >= 0 && key <= GLFW_KEY_LAST) ? sKeys[key] : false; }
    bool InputSystem::KeyPressed(int key) { return (key >= 0 && key <= GLFW_KEY_LAST) ? sKeys[key] && !sKeysPrevFrame[key] : false; }
    bool InputSystem::KeyReleased(int key) { return (key >= 0 && key <= GLFW_KEY_LAST) ? !sKeys[key] && sKeysPrevFrame[key] : false; }

    bool InputSystem::MouseButtonDown(int button) { return (button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST) ? sMouseButtons[button] : false; }
    bool InputSystem::MouseButtonPressed(int button) { return (button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST) ? sMouseButtons[button] && !sMouseButtonsPrevFrame[button] : false; }
    bool InputSystem::MouseButtonReleased(int button) { return (button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST) ? !sMouseButtons[button] && sMouseButtonsPrevFrame[button] : false; }

    void InputSystem::GetMousePosition(double& x, double& y) { x = sMouseX; y = sMouseY; }
    Vec2 InputSystem::GetMousePosition() { return Vec2(static_cast<float>(sMouseX), static_cast<float>(sMouseY)); }
    double InputSystem::GetMouseX() { return sMouseX; }
    double InputSystem::GetMouseY() { return sMouseY; }

    double InputSystem::GetScrollOffsetX()
    {
        return sScrollX;
    }

    double InputSystem::GetScrollOffsetY()
    {
        return sScrollY;
    }

    void InputSystem::UpdatePreviousFrameState()
    {
        sKeysPrevFrame = sKeys;
        sMouseButtonsPrevFrame = sMouseButtons;
        sScrollX = 0.0;
        sScrollY = 0.0;
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

    const char* InputSystem::GetControllerButtonName(int btn)
    {
        switch (btn)
        {
        
        case GLFW_GAMEPAD_BUTTON_B: return "B";
        case GLFW_GAMEPAD_BUTTON_A: return "A";
        case GLFW_GAMEPAD_BUTTON_Y: return "Y";
        case GLFW_GAMEPAD_BUTTON_X: return "X";

        case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER: return "LB"; case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER: return "RB";
        case GLFW_GAMEPAD_BUTTON_LEFT_THUMB: return "LEFT_THUMB"; case GLFW_GAMEPAD_BUTTON_RIGHT_THUMB: return "RIGHT_THUMB";

        case GLFW_GAMEPAD_BUTTON_BACK: return "BACK";
        case GLFW_GAMEPAD_BUTTON_START: return "START";
        case GLFW_GAMEPAD_BUTTON_GUIDE: return "GUIDE";

        case GLFW_GAMEPAD_BUTTON_DPAD_UP: return "DPAD_UP";
        case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT: return "DPAD_RIGHT";
        case GLFW_GAMEPAD_BUTTON_DPAD_DOWN: return "DPAD_DOWN";
        case GLFW_GAMEPAD_BUTTON_DPAD_LEFT: return "DPAD_LEFT";

        default: return "UNKNOWN";
        }
    }

    const char* InputSystem::GetControllerAxisName(int axis)
    {
        switch (axis)
        {

        case GLFW_GAMEPAD_AXIS_LEFT_X: return "LEFT_X";
        case GLFW_GAMEPAD_AXIS_LEFT_Y: return "LEFT_Y";
        case GLFW_GAMEPAD_AXIS_RIGHT_X: return "RIGHT_X";
        case GLFW_GAMEPAD_AXIS_RIGHT_Y: return "RIGHT_Y";

        case GLFW_GAMEPAD_AXIS_LEFT_TRIGGER: return "LEFT_TRIGGER";
        case GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER: return "RIGHT_TRIGGER";

        default: return "UNKNOWN";
        }
    }

    void InputSystem::SetUpSDLControllerDB()
    {
        std::ifstream ifs(Uma_FilePath::CONFIG_ROOT + "/gamecontrollerdb.txt");

        std::string content((std::istreambuf_iterator<char>(ifs)),
                            std::istreambuf_iterator<char>());

        if (glfwUpdateGamepadMappings(content.c_str()))
        {
            std::cout << "Mappings loaded!\n";
        }
        else
        {
            std::cout << "Failed to load mappings\n";
        }
    }


    void InputSystem::ControllerConnectionCallback(int id, int event)
    {
        std::stringstream ss;
        ss << "Controller " << id << " has " << (event == GLFW_CONNECTED ? "connected" : "disconnected");
        Debugger::Log(WarningLevel::eInfo, ss.str());

        //pSystemManager


        if (event == GLFW_CONNECTED)
        {
            if (!sActiveController.contains(id))
            {
                const char* name = glfwGetJoystickName(id);
                const char* guid = glfwGetJoystickGUID(id);
                bool isGamepad   = glfwJoystickIsGamepad(id);
                const char* gpName = isGamepad ? glfwGetGamepadName(id) : "N/A";

                ss.str("");
                ss << "[Diag] Controller " << id
                   << " | Name: " << (name ? name : "null")
                   << " | GUID: " << (guid ? guid : "null")
                   << " | IsGamepad: " << (isGamepad ? "YES" : "NO")
                   << " | GamepadName: " << (gpName ? gpName : "null");
                Debugger::Log(WarningLevel::eInfo, ss.str());

                // new controller connected
                sActiveController[id] = std::make_unique<ControllerInput>();
            }
        }
        else if (event == GLFW_DISCONNECTED)
        {
            if (sActiveController.contains(id))
            {
                sActiveController.erase(id);
            }
        }
    }

    const bool InputSystem::IsControllerConnected(int id)
    {
        return sActiveController.contains(id);
    }

    bool InputSystem::GetControllerButtonInput(int key, int action, int controllerId)
    {
        if (!sActiveController.contains(controllerId)) 
            return false;

        const char* buttonNames[] = {
                                "A", "B", "X", "Y",
                                "LB", "RB", "BACK", "START", "GUIDE",
                                "LEFT_THUMB", "RIGHT_THUMB",
                                "DPAD_UP", "DPAD_RIGHT", "DPAD_DOWN", "DPAD_LEFT"
                            };

        ControllerInput* con = sActiveController[controllerId].get();

        bool result = false;

        switch (action)
        {
        case GLFW_PRESS:
        {
            if (con->currState.buttons[key] == GLFW_PRESS && con->prevState.buttons[key] == GLFW_RELEASE)
            {
#ifdef _DEBUG_LOG
                std::stringstream ss{ "" };
                ss << "Controller " << controllerId << " Button [" << buttonNames[key] << "]: PRESSED";
                Debugger::Log(WarningLevel::eInfo, ss.str());
#endif
                result = true;
            }
            break;
        }
        case GLFW_REPEAT:
        {
            if (con->currState.buttons[key] == GLFW_PRESS && con->prevState.buttons[key] == GLFW_PRESS)
            {
#ifdef _DEBUG_LOG
                std::stringstream ss{ "" };
                ss << "Controller " << controllerId << " Button [" << buttonNames[key] << "]: HOLD";
                Debugger::Log(WarningLevel::eInfo, ss.str());
#endif
                result = true;
            }
            break;
        }
        case GLFW_RELEASE:
        {
            if (con->prevState.buttons[key] == GLFW_PRESS && con->currState.buttons[key] == GLFW_RELEASE)
            {
#ifdef _DEBUG_LOG
                std::stringstream ss{ "" };
                ss << "Controller " << controllerId << " Button [" << buttonNames[key] << "]: RELEASED";
                Debugger::Log(WarningLevel::eInfo, ss.str());
#endif
                result = true;
            }
            break;
        }
        }

        return result;
    }
    float InputSystem::GetControllerAxesInput(int axis, int controllerId)
    {
        if (!sActiveController.contains(controllerId))
            return false;

        const char* axisNames[] = 
        {
            "LEFT_X", "LEFT_Y", "RIGHT_X", "RIGHT_Y", "LEFT_TRIGGER", "RIGHT_TRIGGER"
        };

        ControllerInput* con = sActiveController[controllerId].get();

        if (axis <= GLFW_GAMEPAD_AXIS_RIGHT_Y && std::abs(con->currState.axes[axis]) > 0.15f)  // only log if outside deadzone
        {
#ifdef _DEBUG_LOG
            std::stringstream ss{ "" };
            ss << "Controller " << controllerId << " Axis [" << axisNames[axis] << "]: " << con->currState.axes[axis];
            Debugger::Log(WarningLevel::eInfo, ss.str());
#endif
        }
        else if (axis > GLFW_GAMEPAD_AXIS_RIGHT_Y && con->currState.axes[axis] > -1)
        {
#ifdef _DEBUG_LOG
            std::stringstream ss{ "" };
            ss << "Controller " << controllerId << " Axis [" << axisNames[axis] << "]: " << con->currState.axes[axis];
            Debugger::Log(WarningLevel::eInfo, ss.str());
#endif
        }

        return con->currState.axes[axis];
    }

    int InputSystem::GetCurrentInputMethod()
    {
        return sCurrInputMethod;
    }
}
