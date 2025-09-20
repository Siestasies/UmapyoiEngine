#include "EventSystem.h"


namespace Uma_Engine
{
    void EventSystem::Init()
    {
        // Clear any existing state
        listeners.clear();
        while (!queuedEvents.empty())
        {
            queuedEvents.pop();
        }
    }

    void EventSystem::Update(float dt)
    {
        ProcessQueuedEvents();
    }

    void EventSystem::Shutdown()
    {
        listeners.clear();
        while (!queuedEvents.empty())
        {
            queuedEvents.pop();
        }
    }

    void EventSystem::ClearAllListeners()
    {
        listeners.clear();
    }

    void EventSystem::ProcessQueuedEvents()
    {
        while (!queuedEvents.empty())
        {
            auto& queuedEvent = queuedEvents.front();

            // Execute the stored dispatch function
            queuedEvent.dispatchFunc();

            queuedEvents.pop();
        }
    }

    void EventListenerSystem::Init()
    {
        if (pSystemManager)
        {
            eventSystem = pSystemManager->GetSystem<EventSystem>();
            if (eventSystem)
            {
                RegisterEventListeners();
            }
        }
    }

    void EventListenerSystem::Update(float dt)
    {
        // Base implementation does nothing
    }

    void EventListenerSystem::Shutdown()
    {
        // Base implementation does nothing
        eventSystem = nullptr;
    }
}