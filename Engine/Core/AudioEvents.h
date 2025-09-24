#pragma once

#include "EventType.h"

namespace Uma_Engine
{
    class PlaySoundEvent : public Event
    {
    public:
        PlaySoundEvent(const std::string& soundName, float volume = 1.0f, bool loop = false) : soundName(soundName), volume(volume), loop(loop) { priority = Priority::Low; }

    public:
        std::string soundName;
        float volume;
        bool loop;
    };

    class StopSoundEvent : public Event
    {
    public:
        StopSoundEvent(const std::string& soundName) : soundName(soundName) { priority = Priority::Low; }

    public:
        std::string soundName;
    };

    class PlayMusicEvent : public Event
    {
    public:
        PlayMusicEvent(const std::string& musicName, float volume = 1.0f, bool loop = true) : musicName(musicName), volume(volume), loop(loop) { priority = Priority::Low; }

    public:
        std::string musicName;
        float volume;
        bool loop;
    };

    class StopMusicEvent : public Event
    {
    public:
        StopMusicEvent() { priority = Priority::Low; }
    };
}