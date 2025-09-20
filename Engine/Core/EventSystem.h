#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <memory>
#include <queue>
#include "EventTypes.h"
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

    // Event System - manages event dispatch and listener registration
    class EventSystem : public ISystem
    {
    public:
        EventSystem() = default;
        ~EventSystem() = default;

        // ISystem interface
        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;

        // Subscribe to an event type
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
            listenerList.erase(std::remove(listenerList.begin(), listenerList.end(), listener),listenerList.end());
        }

        // Immediately dispatch an event
        template<typename T>
        void Dispatch(const T& event)
        {
            static_assert(std::is_base_of_v<Event, T>, "T must inherit from Event");

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

        // Queue an event for processing in the next update
        template<typename T>
        void QueueEvent(std::unique_ptr<T> event)
        {
            static_assert(std::is_base_of_v<Event, T>, "T must inherit from Event");

            // Store dispatch function with the event for type-safe queued dispatch
            auto dispatchFunc = [this, event = event.get()]()
            {
                this->Dispatch(*static_cast<T*>(event));
            };

            queuedEvents.emplace(std::move(event), dispatchFunc);
        }

        // Queue an event using perfect forwarding
        template<typename T, typename... Args>
        void QueueEvent(Args&&... args)
        {
            static_assert(std::is_base_of_v<Event, T>, "T must inherit from Event");
            auto event = std::make_unique<T>(std::forward<Args>(args)...);
            QueueEvent(std::move(event));
        }

        // Clear all listeners for a specific event type
        template<typename T>
        void ClearListeners()
        {
            std::type_index typeIndex = std::type_index(typeid(T));
            listeners.erase(typeIndex);
        }

        // Clear all listeners
        void ClearAllListeners();

        // Get number of listeners for an event type
        template<typename T>
        size_t GetListenerCount() const
        {
            std::type_index typeIndex = std::type_index(typeid(T));
            auto it = listeners.find(typeIndex);
            return (it != listeners.end()) ? it->second.size() : 0;
        }

    private:
        void ProcessQueuedEvents();

        std::unordered_map<std::type_index, std::vector<std::shared_ptr<IEventListener>>> listeners;

        // Queued events storage for type-safe dispatch
        struct QueuedEvent
        {
            std::unique_ptr<Event> event;
            std::function<void()> dispatchFunc;

            QueuedEvent(std::unique_ptr<Event> e, std::function<void()> f) : event(std::move(e)), dispatchFunc(f) {}
        };

        std::queue<QueuedEvent> queuedEvents;
    };

    // Helper class for systems that want to listen to events
    class EventListenerSystem : public ISystem
    {
    public:
        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;

    protected:
        virtual void RegisterEventListeners() = 0;
        EventSystem* eventSystem = nullptr;

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