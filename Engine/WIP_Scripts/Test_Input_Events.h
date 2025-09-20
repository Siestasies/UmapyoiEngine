#pragma once
#include <iostream>
#include <GLFW/glfw3.h>
#include "Core/EventSystem.h"
#include "Core/EventTypes.h"
#include "Systems/InputSystem.h"

using namespace Uma_Engine;

// Simple wrapper that adds event dispatching to existing InputSystem
class EventInputSystem : public Uma_Engine::InputSystem
{
public:
    void Init() override
    {
        Uma_Engine::InputSystem::Init();
        std::cout << "EventInputSystem: Adding event dispatching to InputSystem\n";
    }

    void Update(float dt) override
    {
        double prevMouseX = GetMouseX();
        double prevMouseY = GetMouseY();

        if (eventSystem)
        {
            DispatchInputEvents(prevMouseX, prevMouseY);
            Uma_Engine::InputSystem::Update(dt);
        }
    }

    void SetEventSystem(EventSystem* eventSys)
    {
        eventSystem = eventSys;
        std::cout << "EventInputSystem: Connected to EventSystem\n";
    }

private:
    void DispatchInputEvents(double prevMouseX, double prevMouseY)
    {
        for (int key = 0; key <= GLFW_KEY_LAST; key++)
        {
            if (KeyPressed(key))
            {
                eventSystem->Dispatch(Events::KeyPressEvent(key, GLFW_PRESS, 0));
            }
            if (KeyReleased(key))
            {
                eventSystem->Dispatch(Events::KeyReleaseEvent(key, 0));
            }
        }

        // Dispatch mouse button events
        for (int button = 0; button <= GLFW_MOUSE_BUTTON_LAST; button++)
        {
            if (MouseButtonPressed(button))
            {
                double x, y;
                GetMousePosition(x, y);
                eventSystem->Dispatch(Events::MouseButtonEvent(button, GLFW_PRESS, 0, x, y));
            }
            if (MouseButtonReleased(button))
            {
                double x, y;
                GetMousePosition(x, y);
                eventSystem->Dispatch(Events::MouseButtonEvent(button, GLFW_RELEASE, 0, x, y));
            }
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

        SubscribeToEvent<Events::KeyPressEvent>([this](const Events::KeyPressEvent& event){
            std::cout << "TestEventListener received KeyPress: key=" << event.key << "\n";
            });

        SubscribeToEvent<Events::KeyReleaseEvent>([this](const Events::KeyReleaseEvent& event){
            std::cout << "TestEventListener received KeyRelease: key=" << event.key << "\n";
            });

        SubscribeToEvent<Events::MouseButtonEvent>([this](const Events::MouseButtonEvent& event){
            std::string action = (event.action == GLFW_PRESS) ? "Press" : "Release";
            std::cout << "TestEventListener received MouseButton " << action
                << ": button=" << event.button << " at (" << event.x << ", " << event.y << ")\n";
            });

        SubscribeToEvent<Events::MouseMoveEvent>([this](const Events::MouseMoveEvent& event){
            std::cout << "TestEventListener received MouseMove: (" << event.x << ", " << event.y
                << ") delta=(" << event.deltaX << ", " << event.deltaY << ")\n";
            });
    }
};