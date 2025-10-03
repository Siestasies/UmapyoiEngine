#pragma once
#include <iostream>
#include <GLFW/glfw3.h>
#include "Core/EventSystem.h"
#include "Core/InputEvents.h"
#include "Core/WindowEvents.h"
#include "Systems/InputSystem.h"

//#define _DEBUG_LOG

namespace Uma_Engine
{
    class HybridInputSystem : public Uma_Engine::InputSystem
    {
    public:
        void Init() override
        {
            Uma_Engine::InputSystem::Init();

#ifdef _DEBUG_LOG
            std::cout << "  - Critical events: DISPATCH immediately" << std::endl;
            std::cout << "  - High priority events: DISPATCH for responsiveness" << std::endl;
            std::cout << "  - Normal/Low events: EMIT to queue for stability" << std::endl;
#endif
        }

        void Update(float dt) override
        {
            double prevMouseX = GetMouseX();
            double prevMouseY = GetMouseY();

            if (eventSystem)
            {
                HandleInputEvents(prevMouseX, prevMouseY);
                Uma_Engine::InputSystem::Update(dt);
            }
        }

        void SetEventSystem(EventSystem* eventSys)
        {
            eventSystem = eventSys;

#ifdef _DEBUG_LOG
            std::cout << "HybridInputSystem: Connected to EventSystem for hybrid processing" << std::endl;
#endif

        }

    private:
        void HandleInputEvents(double prevMouseX, double prevMouseY)
        {
            if (KeyPressed(GLFW_KEY_ESCAPE))
            {
#ifdef _DEBUG_LOG
                std::cout << "HybridInputSystem: ESC pressed - DISPATCHING Critical WindowCloseEvent immediately" << std::endl;
#endif

                eventSystem->Dispatch(WindowCloseEvent{}); // Critical priority
            }

            for (int button = 0; button <= GLFW_MOUSE_BUTTON_LAST; button++)
            {
                if (MouseButtonPressed(button))
                {
                    double x, y;
                    GetMousePosition(x, y);

#ifdef _DEBUG_LOG
                    std::cout << "HybridInputSystem: Mouse button " << button << " pressed - DISPATCHING High priority event" << std::endl;
#else
                    (void)button;
#endif

                    eventSystem->Dispatch(MouseButtonEvent(button, GLFW_PRESS, 0, x, y)); // High priority
                }
                if (MouseButtonReleased(button))
                {
                    double x, y;
                    GetMousePosition(x, y);

#ifdef _DEBUG_LOG
                    std::cout << "HybridInputSystem: Mouse button " << button << " released - DISPATCHING High priority event" << std::endl;
#else
                    (void)button;
#endif

                    eventSystem->Dispatch(MouseButtonEvent(button, GLFW_RELEASE, 0, x, y)); // High priority
                }
            }

            if (KeyPressed(GLFW_KEY_F1) || KeyPressed(GLFW_KEY_F2) || KeyPressed(GLFW_KEY_F3))
            {
                for (int key = GLFW_KEY_F1; key <= GLFW_KEY_F12; key++)
                {
                    if (KeyPressed(key))
                    {
#ifdef _DEBUG_LOG
                        std::cout << "HybridInputSystem: Function key F" << (key - GLFW_KEY_F1 + 1) << " - DISPATCHING High priority event" << std::endl;
#else
                        (void)key;
#endif

                        eventSystem->Dispatch(KeyPressEvent(key, GLFW_PRESS, 0)); // High priority
                    }
                    if (KeyReleased(key))
                    {
#ifdef _DEBUG_LOG
                        std::cout << "HybridInputSystem: Movement key " << key << " released - EMITTING to queue (Normal priority)" << std::endl;
#else
                        (void)key;
#endif

                        eventSystem->Emit(KeyReleaseEvent(key, 0)); // High priority (immediate)
                    }
                }
            }

            std::vector<int> movementKeys = {GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_LEFT, GLFW_KEY_RIGHT};

            for (int key : movementKeys)
            {
                if (KeyPressed(key))
                {
#ifdef _DEBUG_LOG
                    std::cout << "HybridInputSystem: Movement key " << key << " pressed - EMITTING to queue (Normal priority)" << std::endl;
#else
                    (void)key;
#endif

                    eventSystem->Emit(KeyPressEvent(key, GLFW_PRESS, 0)); // Normal priority (gets queued)
                }
                if (KeyReleased(key))
                {
#ifdef _DEBUG_LOG
                    std::cout << "HybridInputSystem: Movement key " << key << " released - EMITTING to queue (Normal priority)" << std::endl;
#else
                    (void)key;
#endif

                    eventSystem->Emit(KeyReleaseEvent(key, 0)); // High priority (immediate)
                }
            }

            std::vector<int> actionKeys = {GLFW_KEY_SPACE, GLFW_KEY_ENTER, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_LEFT_CONTROL};

            for (int key : actionKeys)
            {
                if (KeyPressed(key))
                {
#ifdef _DEBUG_LOG
                    std::cout << "HybridInputSystem: Action key " << key << " pressed - DISPATCHING High priority event" << std::endl;
#else
                    (void)key;
#endif

                    eventSystem->Dispatch(KeyPressEvent(key, GLFW_PRESS, 0)); // High priority
                }
                if (KeyReleased(key))
                {
#ifdef _DEBUG_LOG
                    std::cout << "HybridInputSystem: Movement key " << key << " released - EMITTING to queue (Normal priority)" << std::endl;
#else
                    (void)key;
#endif

                    eventSystem->Emit(KeyReleaseEvent(key, 0)); // High priority (immediate)
                }
            }

            double currentX = GetMouseX();
            double currentY = GetMouseY();

            if (abs(currentX - prevMouseX) > 1.0 || abs(currentY - prevMouseY) > 1.0)
            {
                double deltaX = currentX - prevMouseX;
                double deltaY = currentY - prevMouseY;

#ifdef _DEBUG_LOG
                // Only log significant movements to avoid spam
                if (abs(deltaX) > 1.0 || abs(deltaY) > 1.0)
                {
                    std::cout << "HybridInputSystem: Mouse moved - EMITTING to queue (Normal priority)" << std::endl;
                }
#endif

                eventSystem->Emit(MouseMoveEvent(currentX, currentY, deltaX, deltaY)); // Normal priority
            }
        }

        EventSystem* eventSystem = nullptr;
    };

    // Simple test listener that just logs received events
    class TestEventListener : public EventListenerSystem
    {
    public:
        void Init() override
        {
            EventListenerSystem::Init();

#ifdef _DEBUG_LOG
            std::cout << "TestEventListener: Initializing...\n";
            std::cout << "TestEventListener: Ready to receive events\n";
#endif
        }

    protected:
        void RegisterEventListeners() override
        {
#ifdef _DEBUG_LOG
            std::cout << "TestEventListener: Registering for input events...\n";
#endif

            SubscribeToEvent<KeyPressEvent>([this](const KeyPressEvent& event){
#ifdef _DEBUG_LOG
                std::cout << "TestEventListener received KeyPress: key=" << event.key << "\n";
#else
                //std::stringstream ss{""};
                //ss << "TestEventListener received KeyPress: key=" << InputSystem::GetKeyName(event.key);
                //Debugger::Log(WarningLevel::eInfo, ss.str());
                (void)event;
#endif
                });

            SubscribeToEvent<KeyReleaseEvent>([this](const KeyReleaseEvent& event){
#ifdef _DEBUG_LOG
                std::cout << "TestEventListener received KeyRelease: key=" << event.key << "\n";
#else
                //std::stringstream ss{ "" };
                //ss << "TestEventListener received KeyRelease: key=" << InputSystem::GetKeyName(event.key);
                //Debugger::Log(WarningLevel::eInfo, ss.str());
                (void)event;
#endif
                });

            SubscribeToEvent<MouseButtonEvent>([this](const MouseButtonEvent& event){
#ifdef _DEBUG_LOG
                std::string action = (event.action == GLFW_PRESS) ? "Press" : "Release";
                std::cout << "TestEventListener received MouseButton " << action
                    << ": button=" << event.button << " at (" << event.x << ", " << event.y << ")\n";
#else
                //std::string action = (event.action == GLFW_PRESS) ? "Press" : "Release";
                //std::stringstream ss{ "" };
                //ss << "TestEventListener received MouseButton " << action
                //    << ": button=" << event.button << " at (" << event.x << ", " << event.y << ")";
                //Debugger::Log(WarningLevel::eInfo, ss.str());
                (void)event;
#endif
                });

            SubscribeToEvent<MouseMoveEvent>([this](const MouseMoveEvent& event){
#ifdef _DEBUG_LOG
                std::cout << "TestEventListener received MouseMove: (" << event.x << ", " << event.y
                    << ") delta=(" << event.deltaX << ", " << event.deltaY << ")\n";
#else
                //std::stringstream ss{ "" };
                //ss << "TestEventListener received MouseMove: (" << event.x << ", " << event.y
                //    << ") delta=(" << event.deltaX << ", " << event.deltaY << ")";
                //Debugger::Log(WarningLevel::eInfo, ss.str());
                (void)event;
#endif
                });
        }
    };
}