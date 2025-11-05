#pragma once
#include <iostream>
#include <GLFW/glfw3.h>
#include "../Core/EventSystem.h"
#include "../Events/InputEvents.h"
#include "../Events/WindowEvents.h"
#include "InputSystem.h"
#include "../UI/Helpers/InputFilter.h"

//#define _DEBUG_LOG

namespace Uma_Engine
{
    class HybridInputSystem : public Uma_Engine::InputSystem
    {
    public:
        void Init() override
        {
            Uma_Engine::InputSystem::Init();

            prevMouseX = GetMouseX();
            prevMouseY = GetMouseY();

#ifdef _DEBUG_LOG
            std::cout << "HybridInputSystem: Initialized with UI input filtering" << std::endl;
            std::cout << "  - UI layer: HIGHEST priority (blocks game input)" << std::endl;
            std::cout << "  - Critical events: DISPATCH immediately" << std::endl;
            std::cout << "  - High priority events: DISPATCH for responsiveness" << std::endl;
            std::cout << "  - Normal/Low events: EMIT to queue for stability" << std::endl;
#endif
        }

        void Update(float dt) override
        {
            double currMouseX = GetMouseX();
            double currMouseY = GetMouseY();

            if (eventSystem)
            {
                HandleInputEvents(prevMouseX, prevMouseY);
                Uma_Engine::InputSystem::Update(dt);
            }

            prevMouseX = currMouseX;
            prevMouseY = currMouseY;
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
            // ================================================================
            // CRITICAL PRIORITY: Window close (never blocked)
            // ================================================================
            if (KeyPressed(GLFW_KEY_ESCAPE))
            {
#ifdef _DEBUG_LOG
                std::cout << "HybridInputSystem: ESC pressed - DISPATCHING Critical WindowCloseEvent immediately" << std::endl;
#endif
                eventSystem->Dispatch(WindowCloseEvent{});
                return; // Skip other input processing
            }

            // ================================================================
            // HIGH PRIORITY: Mouse buttons (check UI consumption)
            // ================================================================
            for (int button = 0; button <= GLFW_MOUSE_BUTTON_LAST; button++)
            {
                if (MouseButtonPressed(button))
                {
                    // Check if UI consumed the click
                    if (Uma_UI::InputFilter::ShouldBlockMouseInput())
                    {
#ifdef _DEBUG_LOG
                        std::cout << "HybridInputSystem: Mouse button " << button
                            << " pressed - BLOCKED by UI" << std::endl;
#endif
                        continue; // Skip game event - UI handled it
                    }

                    double x, y;
                    GetMousePosition(x, y);

#ifdef _DEBUG_LOG
                    std::cout << "HybridInputSystem: Mouse button " << button
                        << " pressed - DISPATCHING High priority event to game" << std::endl;
#else
                    (void)button;
#endif
                    eventSystem->Dispatch(MouseButtonEvent(button, GLFW_PRESS, 0, x, y));
                }

                if (MouseButtonReleased(button))
                {
                    double x, y;
                    GetMousePosition(x, y);

#ifdef _DEBUG_LOG
                    std::cout << "HybridInputSystem: Mouse button " << button
                        << " released - DISPATCHING High priority event" << std::endl;
#else
                    (void)button;
#endif
                    eventSystem->Dispatch(MouseButtonEvent(button, GLFW_RELEASE, 0, x, y));
                }
            }

            // ================================================================
            // HIGH PRIORITY: Function keys (not blocked by UI)
            // ================================================================
            for (int key = GLFW_KEY_F1; key <= GLFW_KEY_F12; key++)
            {
                if (KeyPressed(key))
                {
#ifdef _DEBUG_LOG
                    std::cout << "HybridInputSystem: Function key F" << (key - GLFW_KEY_F1 + 1)
                        << " - DISPATCHING High priority event" << std::endl;
#else
                    (void)key;
#endif
                    eventSystem->Dispatch(KeyPressEvent(key, GLFW_PRESS, 0));
                }
                if (KeyReleased(key))
                {
#ifdef _DEBUG_LOG
                    std::cout << "HybridInputSystem: Function key F" << (key - GLFW_KEY_F1 + 1)
                        << " released - EMITTING to queue" << std::endl;
#else
                    (void)key;
#endif
                    eventSystem->Emit(KeyReleaseEvent(key, 0));
                }
            }

            // ================================================================
            // NORMAL PRIORITY: Movement keys (can be blocked by future UI text input)
            // ================================================================
            std::vector<int> movementKeys = {
                GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D,
                GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_LEFT, GLFW_KEY_RIGHT
            };

            for (int key : movementKeys)
            {
                if (KeyPressed(key))
                {
                    // Future: Block if InputField has focus
                    if (Uma_UI::InputFilter::ShouldBlockKeyboardInput())
                    {
#ifdef _DEBUG_LOG
                        std::cout << "HybridInputSystem: Movement key " << key
                            << " pressed - BLOCKED by UI text input" << std::endl;
#endif
                        continue;
                    }

#ifdef _DEBUG_LOG
                    std::cout << "HybridInputSystem: Movement key " << key
                        << " pressed - EMITTING to queue (Normal priority)" << std::endl;
#else
                    (void)key;
#endif
                    eventSystem->Emit(KeyPressEvent(key, GLFW_PRESS, 0));
                }
                if (KeyReleased(key))
                {
#ifdef _DEBUG_LOG
                    std::cout << "HybridInputSystem: Movement key " << key
                        << " released - EMITTING to queue (Normal priority)" << std::endl;
#else
                    (void)key;
#endif
                    eventSystem->Emit(KeyReleaseEvent(key, 0));
                }
            }

            // ================================================================
            // HIGH PRIORITY: Action keys (SPACE, ENTER, SHIFT, CTRL)
            // ================================================================
            std::vector<int> actionKeys = {
                GLFW_KEY_SPACE, GLFW_KEY_ENTER,
                GLFW_KEY_LEFT_SHIFT, GLFW_KEY_LEFT_CONTROL,
                GLFW_KEY_K, GLFW_KEY_L,
                GLFW_KEY_SEMICOLON, GLFW_KEY_P
            };

            for (int key : actionKeys)
            {
                if (KeyPressed(key))
                {
                    // Future: Block if InputField has focus
                    if (Uma_UI::InputFilter::ShouldBlockKeyboardInput())
                    {
#ifdef _DEBUG_LOG
                        std::cout << "HybridInputSystem: Action key " << key
                            << " pressed - BLOCKED by UI text input" << std::endl;
#endif
                        continue;
                    }

#ifdef _DEBUG_LOG
                    std::cout << "HybridInputSystem: Action key " << key
                        << " pressed - DISPATCHING High priority event" << std::endl;
#else
                    (void)key;
#endif
                    eventSystem->Dispatch(KeyPressEvent(key, GLFW_PRESS, 0));
                }
                if (KeyReleased(key))
                {
#ifdef _DEBUG_LOG
                    std::cout << "HybridInputSystem: Action key " << key
                        << " released - EMITTING to queue" << std::endl;
#else
                    (void)key;
#endif
                    eventSystem->Emit(KeyReleaseEvent(key, 0));
                }
            }

            // ================================================================
            // NORMAL PRIORITY: Mouse movement (blocked if over UI)
            // ================================================================
            double currMouseX = GetMouseX();
            double currMouseY = GetMouseY();

            double deltaX = currMouseX - prevMouseX;
            double deltaY = currMouseY - prevMouseY;

            if (std::abs(deltaX) > 0.1 || std::abs(deltaY) > 0.1)
            {
                // Block mouse move events when over UI (prevents camera rotation, etc.)
                if (Uma_UI::InputFilter::IsMouseOverUI())
                {
#ifdef _DEBUG_LOG
                    std::cout << "HybridInputSystem: Mouse move BLOCKED - mouse is over UI" << std::endl;
#endif
                    return;  // Don't emit mouse move event
                }

#ifdef _DEBUG_LOG
                // Only log significant movements to avoid spam
                if (abs(deltaX) > 1.0 || abs(deltaY) > 1.0)
                {
                    std::cout << "HybridInputSystem: Mouse moved - EMITTING to queue (Normal priority)" << std::endl;
                }
#endif
                eventSystem->Emit<MouseMoveEvent>(currMouseX, currMouseY, deltaX, deltaY);
            }
        }
    private:
        EventSystem* eventSystem = nullptr;

        inline static double prevMouseX = 0.0, prevMouseY = 0.0;
    };

    // Simple test listener that logs received events
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

            SubscribeToEvent<KeyPressEvent>([this](const KeyPressEvent& event) {
#ifdef _DEBUG_LOG
                std::cout << "TestEventListener received KeyPress: key=" << event.key << "\n";
#else
                (void)event;
#endif
                });

            SubscribeToEvent<KeyReleaseEvent>([this](const KeyReleaseEvent& event) {
#ifdef _DEBUG_LOG
                std::cout << "TestEventListener received KeyRelease: key=" << event.key << "\n";
#else
                (void)event;
#endif
                });

            SubscribeToEvent<MouseButtonEvent>([this](const MouseButtonEvent& event) {
#ifdef _DEBUG_LOG
                std::string action = (event.action == GLFW_PRESS) ? "Press" : "Release";
                std::cout << "TestEventListener received MouseButton " << action
                    << ": button=" << event.button << " at (" << event.x << ", " << event.y << ")\n";
#else
                (void)event;
#endif
                });

            SubscribeToEvent<MouseMoveEvent>([this](const MouseMoveEvent& event) {
#ifdef _DEBUG_LOG
                std::cout << "TestEventListener received MouseMove: (" << event.x << ", " << event.y
                    << ") delta=(" << event.deltaX << ", " << event.deltaY << ")\n";
#else
                (void)event;
#endif
                });
        }
    };
}