#pragma once
#include <iostream>
#include <GLFW/glfw3.h>
#include "Core/EventSystem.h"
#include "Core/EventTypes.h"
#include "Systems/InputSystem.h"

using namespace Uma_Engine;

class HybridInputSystem : public Uma_Engine::InputSystem
{
public:
    void Init() override
    {
        Uma_Engine::InputSystem::Init();
        std::cout << "HybridInputSystem: Initialized with strategic event dispatching" << std::endl;
        std::cout << "  - Critical events: DISPATCH immediately" << std::endl;
        std::cout << "  - High priority events: DISPATCH for responsiveness" << std::endl;
        std::cout << "  - Normal/Low events: EMIT to queue for stability" << std::endl;
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
        std::cout << "HybridInputSystem: Connected to EventSystem for hybrid processing" << std::endl;
    }

private:
    void HandleInputEvents(double prevMouseX, double prevMouseY)
    {
        if (KeyPressed(GLFW_KEY_ESCAPE))
        {
            std::cout << "HybridInputSystem: ESC pressed - DISPATCHING Critical WindowCloseEvent immediately" << std::endl;
            eventSystem->Dispatch(Events::WindowCloseEvent{}); // Critical priority
        }

        for (int button = 0; button <= GLFW_MOUSE_BUTTON_LAST; ++button)
        {
            if (MouseButtonPressed(button))
            {
                double x, y;
                GetMousePosition(x, y);
                std::cout << "HybridInputSystem: Mouse button " << button << " pressed - DISPATCHING High priority event" << std::endl;
                eventSystem->Dispatch(Events::MouseButtonEvent(button, GLFW_PRESS, 0, x, y)); // High priority
            }
            if (MouseButtonReleased(button))
            {
                double x, y;
                GetMousePosition(x, y);
                std::cout << "HybridInputSystem: Mouse button " << button << " released - DISPATCHING High priority event" << std::endl;
                eventSystem->Dispatch(Events::MouseButtonEvent(button, GLFW_RELEASE, 0, x, y)); // High priority
            }
        }

        if (KeyPressed(GLFW_KEY_F1) || KeyPressed(GLFW_KEY_F2) || KeyPressed(GLFW_KEY_F3))
        {
            for (int key = GLFW_KEY_F1; key <= GLFW_KEY_F12; ++key)
            {
                if (KeyPressed(key))
                {
                    std::cout << "HybridInputSystem: Function key F" << (key - GLFW_KEY_F1 + 1) << " - DISPATCHING High priority event" << std::endl;
                    eventSystem->Dispatch(Events::KeyPressEvent(key, GLFW_PRESS, 0)); // High priority
                }
            }
        }

        std::vector<int> movementKeys = { GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_LEFT, GLFW_KEY_RIGHT };

        for (int key : movementKeys)
        {
            if (KeyPressed(key))
            {
                std::cout << "HybridInputSystem: Movement key " << key << " pressed - EMITTING to queue (Normal priority)" << std::endl;
                eventSystem->Emit(Events::KeyPressEvent(key, GLFW_PRESS, 0)); // Normal priority (gets queued)
            }
            if (KeyReleased(key))
            {
                std::cout << "HybridInputSystem: Movement key " << key << " released - EMITTING to queue (Normal priority)" << std::endl;
                eventSystem->Emit(Events::KeyReleaseEvent(key, 0)); // High priority (immediate)
            }
        }

        std::vector<int> actionKeys = { GLFW_KEY_SPACE, GLFW_KEY_ENTER, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_LEFT_CONTROL };

        for (int key : actionKeys)
        {
            if (KeyPressed(key))
            {
                std::cout << "HybridInputSystem: Action key " << key << " pressed - DISPATCHING High priority event" << std::endl;
                eventSystem->Dispatch(Events::KeyPressEvent(key, GLFW_PRESS, 0)); // High priority
            }
        }

        double currentX = GetMouseX();
        double currentY = GetMouseY();

        if (abs(currentX - prevMouseX) > 1.0 || abs(currentY - prevMouseY) > 1.0)
        {
            double deltaX = currentX - prevMouseX;
            double deltaY = currentY - prevMouseY;
            // Only log significant movements to avoid spam
            if (abs(deltaX) > 1.0 || abs(deltaY) > 1.0)
            {
                std::cout << "HybridInputSystem: Mouse moved - EMITTING to queue (Normal priority)" << std::endl;
            }
            eventSystem->Emit(Events::MouseMoveEvent(currentX, currentY, deltaX, deltaY)); // Normal priority
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
        std::cout << "TestEventListener: Initializing...\n";
        EventListenerSystem::Init();
        std::cout << "TestEventListener: Ready to receive events\n";
    }

protected:
    void RegisterEventListeners() override
    {
        std::cout << "TestEventListener: Registering for input events...\n";

        SubscribeToEvent<Events::KeyPressEvent>([this](const Events::KeyPressEvent& event) {
            std::cout << "TestEventListener received KeyPress: key=" << event.key << "\n";
            });

        SubscribeToEvent<Events::KeyReleaseEvent>([this](const Events::KeyReleaseEvent& event) {
            std::cout << "TestEventListener received KeyRelease: key=" << event.key << "\n";
            });

        SubscribeToEvent<Events::MouseButtonEvent>([this](const Events::MouseButtonEvent& event) {
            std::string action = (event.action == GLFW_PRESS) ? "Press" : "Release";
            std::cout << "TestEventListener received MouseButton " << action
                << ": button=" << event.button << " at (" << event.x << ", " << event.y << ")\n";
            });

        SubscribeToEvent<Events::MouseMoveEvent>([this](const Events::MouseMoveEvent& event) {
            std::cout << "TestEventListener received MouseMove: (" << event.x << ", " << event.y
                << ") delta=(" << event.deltaX << ", " << event.deltaY << ")\n";
            });
    }
};