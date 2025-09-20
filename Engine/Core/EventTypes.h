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
    };

    // Common Event Types
    namespace Events
    {
        // Input Events
        class KeyPressEvent : public Event
        {
        public:
            KeyPressEvent(int key, int action, int mods) : key(key), action(action), mods(mods) {}

        public:
            int key;
            int action;
            int mods;
        };

        class KeyReleaseEvent : public Event
        {
        public:
            KeyReleaseEvent(int key, int mods) : key(key), mods(mods) {}

        public:
            int key;
            int mods;
        };

        class KeyRepeatEvent : public Event
        {
        public:
            KeyRepeatEvent(int key, int mods) : key(key), mods(mods) {}

        public:
            int key;
            int mods;
        };

        class MouseButtonEvent : public Event
        {
        public:
            MouseButtonEvent(int button, int action, int mods, double x, double y) : button(button), action(action), mods(mods), x(x), y(y) {}

        public:
            int button;
            int action;
            int mods;
            double x, y;
        };

        class MouseMoveEvent : public Event
        {
        public:
            MouseMoveEvent(double x, double y, double deltaX, double deltaY) : x(x), y(y), deltaX(deltaX), deltaY(deltaY) {}

        public:
            double x, y;
            double deltaX, deltaY;
        };

        class MouseScrollEvent : public Event
        {
        public:
            MouseScrollEvent(double xOffset, double yOffset) : xOffset(xOffset), yOffset(yOffset) {}

        public:
            double xOffset, yOffset;
        };

        class TextInputEvent : public Event
        {
        public:
            TextInputEvent(unsigned int codepoint) : codepoint(codepoint) {}

        public:
            unsigned int codepoint;
        };

        // Window Events
        class WindowResizeEvent : public Event
        {
        public:
            WindowResizeEvent(int width, int height) : width(width), height(height) {}
        
        public:
            int width, height;
        };

        class WindowCloseEvent : public Event
        {
        public:
            WindowCloseEvent() = default;
        };

        class WindowFocusEvent : public Event
        {
        public:
            WindowFocusEvent(bool focused) : focused(focused) {}

        public:
            bool focused;
        };

        class WindowMoveEvent : public Event
        {
        public:
            WindowMoveEvent(int x, int y) : x(x), y(y) {}
        
        public:
            int x, y;
        };

        class FramebufferResizeEvent : public Event
        {
        public:
            FramebufferResizeEvent(int width, int height) : width(width), height(height) {}

        public:
            int width, height;
        };

        // ECS/Game Events
        class EntityCreatedEvent : public Event
        {
        public:
            EntityCreatedEvent(uint32_t entityId) : entityId(entityId) {}

        public:
            uint32_t entityId;
        };

        class EntityDestroyedEvent : public Event
        {
        public:
            EntityDestroyedEvent(uint32_t entityId) : entityId(entityId) {}

        public:
            uint32_t entityId;
        };

        class ComponentAddedEvent : public Event
        {
        public:
            ComponentAddedEvent(uint32_t entityId, std::type_index componentType) : entityId(entityId), componentType(componentType) {}

        public:
            uint32_t entityId;
            std::type_index componentType;
        };

        class ComponentRemovedEvent : public Event
        {
        public:
            ComponentRemovedEvent(uint32_t entityId, std::type_index componentType) : entityId(entityId), componentType(componentType) {}

        public:
            uint32_t entityId;
            std::type_index componentType;
        };

        // Physics/Collision Events
        class CollisionBeginEvent : public Event
        {
        public:
            CollisionBeginEvent(uint32_t entityA, uint32_t entityB) : entityA(entityA), entityB(entityB) {}

        public:
            uint32_t entityA, entityB;
        };

        class CollisionEndEvent : public Event
        {
        public:
            CollisionEndEvent(uint32_t entityA, uint32_t entityB) : entityA(entityA), entityB(entityB) {}

        public:
            uint32_t entityA, entityB;
        };

        class TriggerEnterEvent : public Event
        {
        public:
            TriggerEnterEvent(uint32_t trigger, uint32_t entity) : trigger(trigger), entity(entity) {}

        public:
            uint32_t trigger, entity;
        };

        class TriggerExitEvent : public Event
        {
        public:
            TriggerExitEvent(uint32_t trigger, uint32_t entity) : trigger(trigger), entity(entity) {}

        public:
            uint32_t trigger, entity;
        };

        // Audio Events
        class PlaySoundEvent : public Event
        {
        public:
            PlaySoundEvent(const std::string& soundName, float volume = 1.0f, bool loop = false) : soundName(soundName), volume(volume), loop(loop) {}

        public:
            std::string soundName;
            float volume;
            bool loop;
        };

        class StopSoundEvent : public Event
        {
        public:
            StopSoundEvent(const std::string& soundName) : soundName(soundName) {}

        public:
            std::string soundName;
        };

        class PlayMusicEvent : public Event
        {
        public:
            PlayMusicEvent(const std::string& musicName, float volume = 1.0f, bool loop = true) : musicName(musicName), volume(volume), loop(loop) {}

        public:
            std::string musicName;
            float volume;
            bool loop;
        };

        class StopMusicEvent : public Event
        {
        public:
            StopMusicEvent() = default;
        };

        // Scene Management Events
        class SceneChangeEvent : public Event
        {
        public:
            SceneChangeEvent(const std::string& sceneName) : sceneName(sceneName) {}

        public:
            std::string sceneName;
        };

        class SceneLoadedEvent : public Event
        {
        public:
            SceneLoadedEvent(const std::string& sceneName) : sceneName(sceneName) {}

        public:
            std::string sceneName;
        };

        class SceneUnloadedEvent : public Event
        {
        public:
            SceneUnloadedEvent(const std::string& sceneName) : sceneName(sceneName) {}

        public:
            std::string sceneName;
        };

        // Resource Events
        class ResourceLoadedEvent : public Event
        {
        public:
            ResourceLoadedEvent(const std::string& resourcePath, const std::string& resourceType) : resourcePath(resourcePath), resourceType(resourceType) {}

        public:
            std::string resourcePath;
            std::string resourceType;
        };

        class ResourceLoadFailedEvent : public Event
        {
        public:
            ResourceLoadFailedEvent(const std::string& resourcePath, const std::string& error) : resourcePath(resourcePath), error(error) {}

        public:
            std::string resourcePath;
            std::string error;
        };

        // Game State Events
        class GamePausedEvent : public Event
        {
        public:
            GamePausedEvent() = default;
        };

        class GameResumedEvent : public Event
        {
        public:
            GameResumedEvent() = default;
        };

        class GameOverEvent : public Event
        {
        public:
            GameOverEvent(bool victory, int score = 0) : victory(victory), score(score) {}

        public:
            bool victory;
            int score;
        };

        // Debug Events
        class DebugToggleEvent : public Event
        {
        public:
            DebugToggleEvent(const std::string& feature) : feature(feature) {}

        public:
            std::string feature;
        };
    }
}