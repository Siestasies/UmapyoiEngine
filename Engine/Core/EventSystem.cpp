/*!
\file   EventSystem.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Implementation of the EventSystem defined in EventSystem.h.

This file provides the concrete logic for subscribing to, dispatching, and processing game events.
It implements listener management, immediate and deferred event dispatching with priority ordering,
and integrates with the ISystem lifecycle (Init, Update, Shutdown). Events are processed in priority order,
and propagation can be halted if an event is marked as handled.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

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
        (void)dt;

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
        while (!eventQueue.empty())
        {
            auto wrapper = std::move(eventQueue.front());

            eventQueue.erase(eventQueue.begin());

            wrapper->Dispatch(this);
        }
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
                ++processed;
            }
            else
            {
                ++it;
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
            ++processed;
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
                std::type_index typeIndex = std::type_index(typeid(*this));

#ifdef _DEBUG_LOG
                size_t oldCount = 0;
                for (const auto& [eventType, listenerList] : eventSystem->listeners)
                {
                    for (const auto& listener : listenerList)
                    {
                        if (listener->GetOwningSystemType() == typeIndex)
                        {
                            ++oldCount;
                        }
                    }
                }
                std::cout << "EventListenerSystem: Clearing " << oldCount << " old subscriptions for " << typeIndex.name() << std::endl;
#endif

                // Clear old subscriptions
                for (auto& [eventType, listenerList] : eventSystem->listeners)
                {
                    listenerList.erase(
                        std::remove_if(listenerList.begin(), listenerList.end(),
                            [typeIndex](const std::shared_ptr<IEventListener>& listener)
                            {
                                return listener->GetOwningSystemType() == typeIndex;
                            }),
                        listenerList.end()
                    );
                }

                RegisterEventListeners();

#ifdef _DEBUG_LOG
                size_t newCount = 0;
                for (const auto& [eventType, listenerList] : eventSystem->listeners)
                {
                    for (const auto& listener : listenerList)
                    {
                        if (listener->GetOwningSystemType() == typeIndex)
                        {
                            ++newCount;
                        }
                    }
                }
                std::cout << "EventListenerSystem: Registered " << newCount << " new subscriptions for " << typeIndex.name() << std::endl;
            }
            else
            {
                std::cout << "EventListenerSystem: EventSystem not found" << std::endl;
#endif
            }
        }
#ifdef _DEBUG_LOG
        else
        {
            std::cout << "EventListenerSystem: SystemManager not available" << std::endl;
        }
#endif
    }

    void EventListenerSystem::Update(float dt)
    {
        (void)dt;
        // Base implementation does nothing
        // Derived classes can override if they need per-frame updates beyond events
    }

    void EventListenerSystem::Shutdown()
    {
        if (eventSystem)
        {
            std::type_index typeIndex = std::type_index(typeid(*this));

#ifdef _DEBUG_LOG
            size_t count = 0;
            for (const auto& [eventType, listenerList] : eventSystem->listeners)
            {
                for (const auto& listener : listenerList)
                {
                    if (listener->GetOwningSystemType() == typeIndex)
                    {
                        count++;
                    }
                }
            }
            std::cout << "EventListenerSystem: Unsubscribing " << count << " subscriptions for " << typeIndex.name() << std::endl;
#endif

            for (auto& [eventType, listenerList] : eventSystem->listeners)
            {
                listenerList.erase(
                    std::remove_if(listenerList.begin(), listenerList.end(),
                        [typeIndex](const std::shared_ptr<IEventListener>& listener)
                        {
                            return listener->GetOwningSystemType() == typeIndex;
                        }),
                    listenerList.end()
                );
            }
        }

        eventSystem = nullptr;
    }
}