/*!
\file   AudioEvents.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines audio-related events for the Uma Engine event system.

This file introduces event classes used to trigger and control audio playback,
including sounds and music. These events are dispatched through the event system
with configurable parameters such as volume and loop state.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

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