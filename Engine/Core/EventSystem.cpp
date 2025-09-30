#include <iostream>

#include "EventSystem.h"

//#define _DEBUG_LOG

namespace Uma_Engine
{
    void EventSystem::Init()
    {
        listeners.clear();
        eventQueue.clear();
    }

    void EventSystem::Update(float dt)
    {
        ProcessEvents();
    }

    void EventSystem::Shutdown()
    {
        ClearAll();
    }

    void EventSystem::ProcessEvents()
    {
        if (eventQueue.empty()) return;

#ifdef _DEBUG_LOG
        std::cout << "EventSystem: Processing " << eventQueue.size() << " queued events" << std::endl;
#endif

        // Process all queued events in priority order
        for (auto& wrapper : eventQueue)
        {
            wrapper->Dispatch(this);
        }

        eventQueue.clear();
    }

    void EventSystem::ProcessHighPriorityEvents()
    {
        if (eventQueue.empty()) return;

#ifdef _DEBUG_LOG
        std::cout << "EventSystem: Processing high-priority events" << std::endl;
#endif

        // Process only high and critical priority events
        auto it = eventQueue.begin();
        size_t processed = 0;

        while (it != eventQueue.end())
        {
            if ((*it)->GetPriority() >= Event::Priority::High)
            {
                (*it)->Dispatch(this);
                it = eventQueue.erase(it);
                processed++;
            }
            else
            {
                it++;
            }
        }

#ifdef _DEBUG_LOG
        if (processed > 0)
        {
            std::cout << "EventSystem: Processed " << processed << " high-priority events" << std::endl;
        }
#endif
    }

    void EventSystem::ProcessEvents(size_t maxEvents)
    {
        if (eventQueue.empty() || maxEvents == 0) return;

        size_t processed = 0;
        auto it = eventQueue.begin();

#ifdef _DEBUG_LOG
        std::cout << "EventSystem: Processing up to " << maxEvents << " events" << std::endl;
#endif

        while (it != eventQueue.end() && processed < maxEvents)
        {
            (*it)->Dispatch(this);
            it = eventQueue.erase(it);
            processed++;
        }

#ifdef _DEBUG_LOG
        if (processed > 0)
        {
            std::cout << "EventSystem: Processed " << processed << " events (" << eventQueue.size() << " remaining in queue)" << std::endl;
        }
#endif
    }

    void EventSystem::ClearAll()
    {
        listeners.clear();
        eventQueue.clear();
#ifdef _DEBUG_LOG
        std::cout << "EventSystem: Cleared all listeners and queued events" << std::endl;
#endif
    }

    size_t EventSystem::GetQueuedEventCount() const
    {
        return eventQueue.size();
    }

    bool EventSystem::HasHighPriorityEvents() const
    {
        for (const auto& wrapper : eventQueue)
        {
            if (wrapper->GetPriority() >= Event::Priority::High)
            {
                return true;
            }
        }
        return false;
    }

    void EventListenerSystem::Init()
    {
        if (pSystemManager)
        {
            eventSystem = pSystemManager->GetSystem<EventSystem>();

            if (eventSystem)
            {
                RegisterEventListeners();
#ifdef _DEBUG_LOG
                std::cout << "EventListenerSystem: Connected to EventSystem and registered listeners" << std::endl;

            }
            else
            {
                std::cout << "EventListenerSystem: Warning - EventSystem not found in SystemManager" << std::endl;
#endif
            }
        }
#ifdef _DEBUG_LOG
        else
        {
            std::cout << "EventListenerSystem: Warning - SystemManager not available" << std::endl;
        }
#endif
    }

    void EventListenerSystem::Update(float dt)
    {
        // Base implementation does nothing
        // Derived classes can override if they need per-frame updates beyond events
    }

    void EventListenerSystem::Shutdown()
    {
#ifdef _DEBUG_LOG
        if (eventSystem)
        {
            std::cout << "EventListenerSystem: Disconnecting from EventSystem" << std::endl;
        }
#endif
        eventSystem = nullptr;
    }
}