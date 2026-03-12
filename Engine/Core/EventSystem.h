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

The system supports optional predicate functions (std::function<bool(const T&)>) during subscription,
allowing listeners to filter events based on custom criteria without requiring additional event dispatching overhead.
This enables fine-grained control over which events trigger specific callbacks.

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
    /*!
    \brief Interface for type-erased event listeners.

    Provides a common base for all templated EventListener instances,
    enabling storage in homogeneous containers.
    */
    class IEventListener
    {
    public:
        virtual ~IEventListener() = default;

        /*!
        \brief Gets the type index of the system that owns this listener.
        \return The std::type_index identifying the owning system.
        */
        virtual std::type_index GetOwningSystemType() const = 0;
    };

    /*!
    \brief Templated event listener providing type-safe event subscription and dispatch.
    \tparam T The event type this listener handles. Must inherit from Event.
    */
    template<typename T>
    class EventListener : public IEventListener
    {
    public:
        using EventCallback = std::function<void(const T&)>;
        using EventPredicate = std::function<bool(const T&)>;

        /*!
        \brief Constructs an EventListener with a callback, optional predicate, and owning system type.
        \param callback The function to invoke when an event is received.
        \param predicate Optional filter predicate; if set, the callback is only invoked when this returns true.
        \param systemType The type index of the system that owns this listener.
        */
        EventListener(EventCallback callback, EventPredicate predicate, std::type_index systemType) : callback(callback), predicate(predicate), systemType(systemType) {}

        /*!
        \brief Handles an incoming event by invoking the callback if the predicate passes.
        \param event The event to handle.
        */
        void OnEvent(const T& event)
        {
            if (!predicate || predicate(event))
            {
                callback(event);
            }
        }

        /*!
        \brief Checks whether this listener would receive the given event based on its predicate.
        \param event The event to test against.
        \return True if the predicate is not set or returns true for the event.
        */
        bool ShouldReceiveEvent(const T& event) const
        {
            return !predicate || predicate(event);
        }

        /*!
        \brief Gets the type index of the system that owns this listener.
        \return The std::type_index identifying the owning system.
        */
        std::type_index GetOwningSystemType() const override { return systemType; }

    private:
        EventCallback callback;
        EventPredicate predicate;
        std::type_index systemType;
    };

    /*!
    \brief Hybrid event system supporting both immediate and queued event processing.

    Provides type-safe event subscription, immediate dispatch for critical events,
    and priority-based queued dispatch for deferred processing.
    */
    class EventSystem : public ISystem
    {
    public:
        EventSystem() = default;
        ~EventSystem() = default;

        /*!
        \brief Initializes the event system.
        */
        void Init() override;

        /*!
        \brief Updates the event system, processing queued events.
        \param dt Delta time since the last frame.
        */
        void Update(float dt) override;

        /*!
        \brief Shuts down the event system, clearing all listeners and queued events.
        */
        void Shutdown() override;

        /*!
        \brief Subscribes to an event type with a callback.
        \tparam T The event type to subscribe to. Must inherit from Event.
        \tparam U The type of the subscribing system (used for ownership tracking).
        \param callback The function to invoke when the event is dispatched.
        */
        template<typename T, typename U>
        void Subscribe(std::function<void(const T&)> callback)
        {
            static_assert(std::is_base_of_v<Event, T>, "T must inherit from Event");

            std::type_index eventType = std::type_index(typeid(T));
            std::type_index systemType = std::type_index(typeid(U));

            auto listener = std::make_shared<EventListener<T>>(callback, nullptr, systemType);
            listeners[eventType].push_back(listener);
        }

        /*!
        \brief Subscribes to an event type with a callback and predicate-based filtering.
        \tparam T The event type to subscribe to. Must inherit from Event.
        \tparam U The type of the subscribing system (used for ownership tracking).
        \param callback The function to invoke when the event is dispatched and the predicate passes.
        \param predicate A filter function; the callback is only invoked when this returns true.
        */
        template<typename T, typename U>
        void Subscribe(std::function<void(const T&)> callback,
            std::function<bool(const T&)> predicate)
        {
            static_assert(std::is_base_of_v<Event, T>, "T must inherit from Event");

            std::type_index eventType = std::type_index(typeid(T));
            std::type_index systemType = std::type_index(typeid(U));

            auto listener = std::make_shared<EventListener<T>>(callback, predicate, systemType);
            listeners[eventType].push_back(listener);
        }

        //// Unsubscribe a specific listener
        //template<typename T>
        //void Unsubscribe(std::shared_ptr<EventListener<T>> listener)
        //{
        //    std::type_index typeIndex = std::type_index(typeid(T));
        //    auto& listenerList = listeners[typeIndex];
        //    listenerList.erase(std::remove(listenerList.begin(), listenerList.end(), listener), listenerList.end());
        //}

        //// Unsubscribe 
        //void UnsubscribeListener(std::shared_ptr<IEventListener> listener)
        //{
        //    for (auto& [typeIndex, listenerList] : listeners)
        //    {
        //        auto it = std::find(listenerList.begin(), listenerList.end(), listener);
        //        if (it != listenerList.end())
        //        {
        //            listenerList.erase(it);
        //            return;
        //        }
        //    }
        //}

        /*!
        \brief Unsubscribes all event listeners owned by the specified system type.
        \tparam T The type of the system whose listeners should be removed.
        */
        template <typename T>
        void UnsubscribeSystem()
        {
            std::type_index typeIndex = std::type_index(typeid(T));

            for (auto& [eventType, listenerList] : listeners)
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

        /*!
        \brief Unsubscribes a specific system from a specific event type.
        \tparam T The event type to unsubscribe from.
        \tparam U The type of the system whose listener should be removed.
        */
        template <typename T, typename U>
        void UnsubscribeEvent()
        {
            std::type_index eventType = std::type_index(typeid(T));
            std::type_index systemType = std::type_index(typeid(U));

            auto it = listeners.find(eventType);
            if (it != listeners.end());
            {
                auto& listenerList = it->second;
                listenerList.erase(
                    std::remove_if(listenerList.begin(), listenerList.end(),
                        [systemType](const std::shared_ptr<IEventListener>& listener)
                        {
                            return listener->GetOwningSystemType() == systemType;
                        }),
                    listenerList.end()
                );
            }
        }

        /*!
        \brief Checks whether the specified system type has any active event subscriptions.
        \tparam T The type of the system to check.
        \return True if the system has at least one subscription.
        */
        template <typename T>
        void HasSubscriptions() const
        {
            std::type_index typeIndex = std::type_index(typeid(T));

            for (const auto& [eventType, listenerList] : listeners)
            {
                for (const auto& listener : listenerList)
                {
                    if (listener->GetOwningSystemType == typeIndex) return true;
                }
            }

            return false;
        }

        /*!
        \brief Gets the number of active subscriptions for the specified system type.
        \tparam T The type of the system to count subscriptions for.
        \return The number of subscriptions owned by the system.
        */
        template <typename T>
        size_t GetSubscriptionCount() const
        {
            std::type_index typeIndex = std::type_index(typeid(T));

            size_t count = 0;
            for (const auto& [eventType, listenerList] : listeners)
            {
                for (const auto& listener : listenerList)
                {
                    if (listener->GetOwningSystemType == typeIndex) ++count;
                }
            }

            return count;
        }

        /*!
        \brief Checks whether any listener would receive the given event (accounting for predicates).
        \tparam T The event type. Must inherit from Event.
        \param event The event to test against registered listeners.
        \return True if at least one listener would receive the event.
        */
        template <typename T>
        bool WouldReceive(const T& event) const
        {
            static_assert(std::is_base_of_v<Event, T>, "T must inherit from Event");

            std::type_index typeIndex = std::type_index(typeid(T));
            auto it = listeners.find(typeIndex);
            if (it != listeners.end())
            {
                for (const auto& listener : it->second)
                {
                    if (auto typedListener = std::dynamic_pointer_cast<EventListener<T>>(listener))
                    {
                        if (typedListener->ShouldReceiveEvent(event)) return true;
                    }
                }
            }
            return false;
        }

        /*!
        \brief Counts the number of listeners that would receive the given event (accounting for predicates).
        \tparam T The event type. Must inherit from Event.
        \param event The event to test against registered listeners.
        \return The number of listeners whose predicates pass for this event.
        */
        template <typename T>
        size_t GetReceiverCount(const T& event) const
        {
            static_assert(std::is_base_of_v<Event, T>, "T must inherit from Event");

            std::type_index typeIndex = std::type_index(typeid(T));
            auto it = listeners.find(typeIndex);
            if (it == listeners.end()) return 0;

            size_t count = 0;
            for (const auto& listener : it->second)
            {
                if (auto typedListener = std::dynamic_pointer_cast<EventListener<T>>(listener))
                {
                    if (typedListener->ShouldReceiveEvent(event)) ++count;
                }
            }
            return count;
        }

        /*!
        \brief Immediately dispatches an event to all matching listeners. Use for critical/real-time events.
        \tparam T The event type. Must inherit from Event.
        \param event The event to dispatch immediately.
        */
        template<typename T>
        void Dispatch(const T& event)
        {
            static_assert(std::is_base_of_v<Event, T>, "T must inherit from Event");
            DispatchImmediate(event);
        }

        /*!
        \brief Emits an event to be processed later via the priority queue. Critical events are dispatched immediately.
        \tparam T The event type. Must inherit from Event.
        \param event The event to queue or dispatch.
        */
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

        /*!
        \brief Emits an event constructed in-place with perfect forwarding.
        \tparam T The event type. Must inherit from Event.
        \tparam Args The constructor argument types for the event.
        \param args Arguments forwarded to the event constructor.
        */
        template<typename T, typename... Args>
        void Emit(Args&&... args)
        {
            static_assert(std::is_base_of_v<Event, T>, "T must inherit from Event");
#pragma warning(push)
#pragma warning(disable: 4244 4267)
            T event(std::forward<Args>(args)...);
#pragma warning(pop)
            Emit(event);
        }

        /*!
        \brief Processes all queued events. Intended to be called at frame boundaries.
        */
        void ProcessEvents();

        /*!
        \brief Processes only high priority events from the queue.
        */
        void ProcessHighPriorityEvents();

        /*!
        \brief Processes up to a limited number of queued events.
        \param maxEvents The maximum number of events to process this call.
        */
        void ProcessEvents(size_t maxEvents);

        /*!
        \brief Clears all listeners for a specific event type.
        \tparam T The event type whose listeners should be removed.
        */
        template<typename T>
        void ClearListeners()
        {
            std::type_index typeIndex = std::type_index(typeid(T));
            listeners.erase(typeIndex);
        }

        /*!
        \brief Clears all listeners and all queued events.
        */
        void ClearAll();

        /*!
        \brief Gets the number of listeners registered for a specific event type.
        \tparam T The event type to query.
        \return The number of listeners for this event type.
        */
        template<typename T>
        size_t GetListenerCount() const
        {
            std::type_index typeIndex = std::type_index(typeid(T));
            auto it = listeners.find(typeIndex);
            return (it != listeners.end()) ? it->second.size() : 0;
        }

        /*!
        \brief Gets the number of events currently in the queue.
        \return The number of queued events.
        */
        size_t GetQueuedEventCount() const;

        /*!
        \brief Checks whether there are any high priority events in the queue.
        \return True if at least one high priority event is queued.
        */
        bool HasHighPriorityEvents() const;

    private:
        /*!
        \brief Type-erased event wrapper interface for storing heterogeneous events in the queue.
        */
        struct IEventWrapper
        {
            virtual ~IEventWrapper() = default;

            /*!
            \brief Gets the type index of the wrapped event.
            \return The std::type_index of the event type.
            */
            virtual std::type_index GetType() const = 0;

            /*!
            \brief Gets a pointer to the raw event data.
            \return A const void pointer to the event data.
            */
            virtual const void* GetData() const = 0;

            /*!
            \brief Gets the priority of the wrapped event.
            \return The priority level of the event.
            */
            virtual Event::Priority GetPriority() const = 0;

            /*!
            \brief Dispatches the wrapped event through the given event system.
            \param system The event system to dispatch through.
            */
            virtual void Dispatch(EventSystem* system) const = 0;
        };

        /*!
        \brief Templated event wrapper that stores a concrete event for deferred dispatch.
        \tparam T The event type stored in this wrapper.
        */
        template<typename T>
        struct EventWrapper : IEventWrapper
        {
            /*!
            \brief Constructs an EventWrapper by copying the given event.
            \param e The event to store.
            */
            EventWrapper(const T& e) : event(e) {}

            /*!
            \brief Gets the type index of the stored event.
            \return The std::type_index of the event type T.
            */
            std::type_index GetType() const override { return std::type_index(typeid(T)); }

            /*!
            \brief Gets a pointer to the stored event data.
            \return A const void pointer to the event.
            */
            const void* GetData() const override { return &event; }

            /*!
            \brief Gets the priority of the stored event.
            \return The priority level of the stored event.
            */
            Event::Priority GetPriority() const override { return event.priority; }

            /*!
            \brief Dispatches the stored event through the given event system.
            \param system The event system to dispatch through.
            */
            void Dispatch(EventSystem* system) const override
            {
                system->DispatchImmediate(event);
            }

            T event;
        };

        /*!
        \brief Internal immediate dispatch implementation. Sends the event to all matching listeners.
        \tparam T The event type. Must inherit from Event.
        \param event The event to dispatch. Propagation stops if event.handled is set to true by a listener.
        */
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

        // Allow EventListenerSystem to access listeners map
        friend class EventListenerSystem;
    };

    /*!
    \brief Helper base class for systems that listen to events.

    Provides convenience methods for event subscription and automatically
    retrieves the EventSystem pointer during initialization.
    */
    class EventListenerSystem : public ISystem
    {
    public:
        /*!
        \brief Initializes the listener system by obtaining the EventSystem pointer and registering event listeners.
        */
        void Init() override;

        /*!
        \brief Updates the listener system.
        \param dt Delta time since the last frame.
        */
        void Update(float dt) override;

        /*!
        \brief Shuts down the listener system.
        */
        void Shutdown() override;

    protected:
        /*!
        \brief Registers all event listeners for this system. Must be implemented by derived classes.
        */
        virtual void RegisterEventListeners() = 0;
        EventSystem* eventSystem = nullptr;

        /*!
        \brief Subscribes to an event type using this system's type as the owner.
        \tparam T The event type to subscribe to. Must inherit from Event.
        \param callback The function to invoke when the event is dispatched.
        */
        template<typename T>
        void SubscribeToEvent(std::function<void(const T&)> callback)
        {
            if (eventSystem)
            {
                std::type_index typeIndex = std::type_index(typeid(*this));
                SubscribeInternal<T>(callback, nullptr, typeIndex);
            }
        }

        /*!
        \brief Subscribes to an event type with predicate-based filtering using this system's type as the owner.
        \tparam T The event type to subscribe to. Must inherit from Event.
        \param callback The function to invoke when the event is dispatched and the predicate passes.
        \param predicate A filter function; the callback is only invoked when this returns true.
        */
        template<typename T>
        void SubscribeToEvent(std::function<void(const T&)> callback, std::function<bool(const T&)> predicate)
        {
            if (eventSystem)
            {
                std::type_index typeIndex = std::type_index(typeid(*this));
                SubscribeInternal<T>(callback, predicate, typeIndex);
            }
        }

    private:
        /*!
        \brief Internal helper that creates and registers an event listener on the event system.
        \tparam T The event type to subscribe to.
        \param callback The function to invoke when the event is dispatched.
        \param predicate Optional filter function for the listener.
        \param systemType The type index of the owning system.
        */
        template<typename T>
        void SubscribeInternal(std::function<void(const T&)> callback,
            std::function<bool(const T&)> predicate,
            std::type_index systemType)
        {
            std::type_index typeIndex = std::type_index(typeid(T));
            auto listener = std::make_shared<EventListener<T>>(callback, predicate, systemType);
            eventSystem->listeners[typeIndex].push_back(listener);
        }
    };
}