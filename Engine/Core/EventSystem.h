/*!
\file   EventSystem.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Implementation of a hybrid event system for managing game events with both immediate and queued dispatching.

This file defines a flexible, type-safe event dispatch and subscription system, enabling systems to respond to
events in real-time or defer handling through prioritised event queues. It supports listener registration,
event emission with priority-based ordering, and safe type-erased storage for runtime event handling.
The system also includes facilities for managing event listeners, limiting event processing per frame,
and integrating cleanly with the game's system architecture. Designed for use in modular ECS-style engines.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <queue>

#include "EventType.h"
#include "SystemManager.h"
#include "SystemType.h"

namespace Uma_Engine
{
    // Event listener interface
    class IEventListener
    {
    public:
        virtual ~IEventListener() = default;
    };

    // Templated event listener for type safety
    template<typename T>
    class EventListener : public IEventListener
    {
    public:
        using EventCallback = std::function<void(const T&)>;

        EventListener(EventCallback callback) : callback(callback) {}

        void OnEvent(const T& event)
        {
            callback(event);
        }

    private:
        EventCallback callback;
    };

    // Hybrid Event System - supports both immediate and queued processing
    class EventSystem : public ISystem
    {
    public:
        EventSystem() = default;
        ~EventSystem() = default;

        // ISystem interface
        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;

        // Subscribe to an event type with callback
        template<typename T>
        void Subscribe(std::function<void(const T&)> callback)
        {
            static_assert(std::is_base_of_v<Event, T>, "T must inherit from Event");

            std::type_index typeIndex = std::type_index(typeid(T));
            auto listener = std::make_shared<EventListener<T>>(callback);
            listeners[typeIndex].push_back(listener);
        }

        // Unsubscribe a specific listener
        template<typename T>
        void Unsubscribe(std::shared_ptr<EventListener<T>> listener)
        {
            std::type_index typeIndex = std::type_index(typeid(T));
            auto& listenerList = listeners[typeIndex];
            listenerList.erase(std::remove(listenerList.begin(), listenerList.end(), listener), listenerList.end());
        }

        // Immediately dispatch an event for critical/real-time events
        template<typename T>
        void Dispatch(const T& event)
        {
            static_assert(std::is_base_of_v<Event, T>, "T must inherit from Event");
            DispatchImmediate(event);
        }

        // Emit an event to be processed later (safer, use for most game events)
        template<typename T>
        void Emit(const T& event)
        {
            static_assert(std::is_base_of_v<Event, T>, "T must inherit from Event");

            // Critical events are always processed immediately
            if (event.priority == Event::Priority::Critical)
            {
                DispatchImmediate(event);
                return;
            }

            // Queue non-critical events
            auto wrapper = std::make_shared<EventWrapper<T>>(event);

            // Insert by priority (higher priority first)
            auto insertPos = eventQueue.end();
            for (auto it = eventQueue.begin(); it != eventQueue.end(); it++)
            {
                if ((*it)->GetPriority() < event.priority)
                {
                    insertPos = it;
                    break;
                }
            }
            eventQueue.insert(insertPos, wrapper);
        }

        // Emit with perfect forwarding
        template<typename T, typename... Args>
        void Emit(Args&&... args)
        {
            static_assert(std::is_base_of_v<Event, T>, "T must inherit from Event");
            T event(std::forward<Args>(args)...);
            Emit(event);
        }

        // Process all queued events (call this at frame boundaries)
        void ProcessEvents();

        // Process only high priority events
        void ProcessHighPriorityEvents();

        // Process a limited number of events
        void ProcessEvents(size_t maxEvents);

        // Clear all listeners for a specific event type
        template<typename T>
        void ClearListeners()
        {
            std::type_index typeIndex = std::type_index(typeid(T));
            listeners.erase(typeIndex);
        }

        // Clear all listeners and queued events
        void ClearAll();

        // Get number of listeners for an event type
        template<typename T>
        size_t GetListenerCount() const
        {
            std::type_index typeIndex = std::type_index(typeid(T));
            auto it = listeners.find(typeIndex);
            return (it != listeners.end()) ? it->second.size() : 0;
        }

        // Get number of queued events
        size_t GetQueuedEventCount() const;

        // Check if there are any high priority events queued
        bool HasHighPriorityEvents() const;

    private:
        // Type-erased event wrapper for queuing
        struct IEventWrapper
        {
            virtual ~IEventWrapper() = default;
            virtual std::type_index GetType() const = 0;
            virtual const void* GetData() const = 0;
            virtual Event::Priority GetPriority() const = 0;
            virtual void Dispatch(EventSystem* system) const = 0;
        };

        template<typename T>
        struct EventWrapper : IEventWrapper
        {
            EventWrapper(const T& e) : event(e) {}

            std::type_index GetType() const override { return std::type_index(typeid(T)); }
            const void* GetData() const override { return &event; }
            Event::Priority GetPriority() const override { return event.priority; }

            void Dispatch(EventSystem* system) const override
            {
                system->DispatchImmediate(event);
            }

            T event;
        };

        // Immediate dispatch implementation
        template<typename T>
        void DispatchImmediate(const T& event)
        {
            std::type_index typeIndex = std::type_index(typeid(T));
            auto it = listeners.find(typeIndex);
            if (it != listeners.end())
            {
                for (auto& listener : it->second)
                {
                    if (auto typedListener = std::dynamic_pointer_cast<EventListener<T>>(listener))
                    {
                        typedListener->OnEvent(event);
                        if (event.handled) break; // Stop propagation if event is handled
                    }
                }
            }
        }

    private:
        std::unordered_map<std::type_index, std::vector<std::shared_ptr<IEventListener>>> listeners;
        std::vector<std::shared_ptr<IEventWrapper>> eventQueue;
    };

    // Helper base class for systems that listen to events
    class EventListenerSystem : public ISystem
    {
    public:
        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;

    protected:
        virtual void RegisterEventListeners() = 0;
        EventSystem* eventSystem = nullptr;

        // Helper method to subscribe to events
        template<typename T>
        void SubscribeToEvent(std::function<void(const T&)> callback)
        {
            if (eventSystem)
            {
                eventSystem->Subscribe<T>(callback);
            }
        }
    };
}