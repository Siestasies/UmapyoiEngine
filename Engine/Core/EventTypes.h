#pragma once
#include <string>
#include <cstdint>
#include <typeindex>

namespace Uma_Engine
{
    // Base Event class, all events must inherit from this
    class Event
    {
    public:
        virtual ~Event() = default;
        bool handled = false;

        // Event priority for processing order
        enum class Priority
        {
            Low = 0,
            Normal = 1,
            High = 2,
            Critical = 3
        };

        Priority priority = Priority::Normal;
    };

    namespace Events
    {
        class KeyPressEvent : public Event
        {
        public:
            KeyPressEvent(int key, int action, int mods)
                : key(key), action(action), mods(mods)
            {
                priority = Priority::High;
            }

            int key, action, mods;
        };

        class KeyReleaseEvent : public Event
        {
        public:
            KeyReleaseEvent(int key, int mods)
                : key(key), mods(mods)
            {
                priority = Priority::High;
            }

            int key, mods;
        };

        class KeyRepeatEvent : public Event
        {
        public:
            KeyRepeatEvent(int key, int mods)
                : key(key), mods(mods)
            {
                priority = Priority::Normal; // Repeats can be queued
            }

            int key, mods;
        };

        class MouseButtonEvent : public Event
        {
        public:
            MouseButtonEvent(int button, int action, int mods, double x, double y)
                : button(button), action(action), mods(mods), x(x), y(y)
            {
                priority = Priority::High;
            }

            int button, action, mods;
            double x, y;
        };

        class MouseMoveEvent : public Event
        {
        public:
            MouseMoveEvent(double x, double y, double deltaX, double deltaY)
                : x(x), y(y), deltaX(deltaX), deltaY(deltaY)
            {
                priority = Priority::Normal;
            }

            double x, y, deltaX, deltaY;
        };

        class MouseScrollEvent : public Event
        {
        public:
            MouseScrollEvent(double xOffset, double yOffset)
                : xOffset(xOffset), yOffset(yOffset)
            {
                priority = Priority::Normal;
            }

            double xOffset, yOffset;
        };

        class TextInputEvent : public Event
        {
        public:
            TextInputEvent(unsigned int codepoint) : codepoint(codepoint)
            {
                priority = Priority::High;
            }

            unsigned int codepoint;
        };

        class WindowResizeEvent : public Event
        {
        public:
            WindowResizeEvent(int width, int height)
                : width(width), height(height)
            {
                priority = Priority::High;
            }

            int width, height;
        };

        class WindowCloseEvent : public Event
        {
        public:
            WindowCloseEvent()
            {
                priority = Priority::Critical;
            }
        };

        class WindowFocusEvent : public Event
        {
        public:
            WindowFocusEvent(bool focused) : focused(focused)
            {
                priority = Priority::High;
            }

            bool focused;
        };

        class WindowMoveEvent : public Event
        {
        public:
            WindowMoveEvent(int x, int y) : x(x), y(y)
            {
                priority = Priority::Normal;
            }

            int x, y;
        };

        class FramebufferResizeEvent : public Event
        {
        public:
            FramebufferResizeEvent(int width, int height)
                : width(width), height(height)
            {
                priority = Priority::High;
            }

            int width, height;
        };

        class EntityCreatedEvent : public Event
        {
        public:
            EntityCreatedEvent(uint32_t entityId) : entityId(entityId)
            {
                priority = Priority::Normal; // Safe to queue
            }

            uint32_t entityId;
        };

        class EntityDestroyedEvent : public Event
        {
        public:
            EntityDestroyedEvent(uint32_t entityId) : entityId(entityId)
            {
                priority = Priority::High;
            }

            uint32_t entityId;
        };

        class ComponentAddedEvent : public Event
        {
        public:
            ComponentAddedEvent(uint32_t entityId, std::type_index componentType)
                : entityId(entityId), componentType(componentType)
            {
                priority = Priority::Normal;
            }

            uint32_t entityId;
            std::type_index componentType;
        };

        class ComponentRemovedEvent : public Event
        {
        public:
            ComponentRemovedEvent(uint32_t entityId, std::type_index componentType)
                : entityId(entityId), componentType(componentType)
            {
                priority = Priority::Normal;
            }

            uint32_t entityId;
            std::type_index componentType;
        };

        class CollisionBeginEvent : public Event
        {
        public:
            CollisionBeginEvent(uint32_t entityA, uint32_t entityB)
                : entityA(entityA), entityB(entityB)
            {
                priority = Priority::Normal;
            }

            uint32_t entityA, entityB;
        };

        class CollisionEndEvent : public Event
        {
        public:
            CollisionEndEvent(uint32_t entityA, uint32_t entityB)
                : entityA(entityA), entityB(entityB)
            {
                priority = Priority::Normal;
            }

            uint32_t entityA, entityB;
        };

        class TriggerEnterEvent : public Event
        {
        public:
            TriggerEnterEvent(uint32_t trigger, uint32_t entity)
                : trigger(trigger), entity(entity)
            {
                priority = Priority::Normal;
            }

            uint32_t trigger, entity;
        };

        class TriggerExitEvent : public Event
        {
        public:
            TriggerExitEvent(uint32_t trigger, uint32_t entity)
                : trigger(trigger), entity(entity)
            {
                priority = Priority::Normal;
            }

            uint32_t trigger, entity;
        };

        class PlaySoundEvent : public Event
        {
        public:
            PlaySoundEvent(const std::string& soundName, float volume = 1.0f, bool loop = false)
                : soundName(soundName), volume(volume), loop(loop)
            {
                priority = Priority::Low;
            }

            std::string soundName;
            float volume;
            bool loop;
        };

        class StopSoundEvent : public Event
        {
        public:
            StopSoundEvent(const std::string& soundName) : soundName(soundName)
            {
                priority = Priority::Low;
            }

            std::string soundName;
        };

        class PlayMusicEvent : public Event
        {
        public:
            PlayMusicEvent(const std::string& musicName, float volume = 1.0f, bool loop = true)
                : musicName(musicName), volume(volume), loop(loop)
            {
                priority = Priority::Low;
            }

            std::string musicName;
            float volume;
            bool loop;
        };

        class StopMusicEvent : public Event
        {
        public:
            StopMusicEvent()
            {
                priority = Priority::Low;
            }
        };

        class SceneChangeEvent : public Event
        {
        public:
            SceneChangeEvent(const std::string& sceneName) : sceneName(sceneName)
            {
                priority = Priority::High;
            }

            std::string sceneName;
        };

        class SceneLoadedEvent : public Event
        {
        public:
            SceneLoadedEvent(const std::string& sceneName) : sceneName(sceneName)
            {
                priority = Priority::High;
            }

            std::string sceneName;
        };

        class SceneUnloadedEvent : public Event
        {
        public:
            SceneUnloadedEvent(const std::string& sceneName) : sceneName(sceneName)
            {
                priority = Priority::Normal;
            }

            std::string sceneName;
        };

        class ResourceLoadedEvent : public Event
        {
        public:
            ResourceLoadedEvent(const std::string& resourcePath, const std::string& resourceType)
                : resourcePath(resourcePath), resourceType(resourceType)
            {
                priority = Priority::Normal;
            }

            std::string resourcePath;
            std::string resourceType;
        };

        class ResourceLoadFailedEvent : public Event
        {
        public:
            ResourceLoadFailedEvent(const std::string& resourcePath, const std::string& error)
                : resourcePath(resourcePath), error(error)
            {
                priority = Priority::High;
            }

            std::string resourcePath;
            std::string error;
        };

        class GamePausedEvent : public Event
        {
        public:
            GamePausedEvent()
            {
                priority = Priority::High;
            }
        };

        class GameResumedEvent : public Event
        {
        public:
            GameResumedEvent()
            {
                priority = Priority::High;
            }
        };

        class GameOverEvent : public Event
        {
        public:
            GameOverEvent(bool victory, int score = 0) : victory(victory), score(score)
            {
                priority = Priority::High;
            }

            bool victory;
            int score;
        };

        class DebugToggleEvent : public Event
        {
        public:
            DebugToggleEvent(const std::string& feature) : feature(feature)
            {
                priority = Priority::Low;
            }

            std::string feature;
        };
    }
}