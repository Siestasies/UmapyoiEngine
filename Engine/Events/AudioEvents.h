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
#include "ECS/Core/Types.hpp"

namespace Uma_Engine
{
    class PlaySoundEvent : public Event
    {
    public:
        PlaySoundEvent(const std::string& soundName, float volume = 1.0f, int loop = 0) : soundName(soundName), volume(volume), loop(loop) { priority = Priority::Low; }

    public:
        std::string soundName;
        float volume;
        int loop;
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
        PlayMusicEvent(const std::string& musicName, float volume = 1.0f, int loop = 0) : musicName(musicName), volume(volume), loop(loop) { priority = Priority::Low; }

    public:
        std::string musicName;
        float volume;
        int loop;
    };

    class StopMusicEvent : public Event
    {
    public:
        StopMusicEvent(const std::string& musicName) : musicName(musicName) { priority = Priority::Low; }

        std::string musicName;
    };

    class PlaySound3DEvent : public Event
    {
    public:
        PlaySound3DEvent(const std::string& soundName, float x, float y, float volume = 1.0f, int loop = 0) : soundName(soundName), x(x), y(y), volume(volume), loop(loop) { priority = Priority::Low; }

    public:
        std::string soundName;
        float x, y, volume;
        int loop;
    };

    /*!
    \brief Event to play a sound attached to a specific entity.
           The sound will follow the entity's position automatically.
    */
    class PlayEntitySoundEvent : public Event
    {
    public:
        PlayEntitySoundEvent(Uma_ECS::Entity entity, const std::string& soundName, bool loop = false, float volume = 1.0f)
            : entity(entity), soundName(soundName), loop(loop), volume(volume) {
            priority = Priority::Low;
        }

    public:
        Uma_ECS::Entity entity;
        std::string soundName;
        bool loop;
        float volume;
    };

    /*!
    \brief Event to stop all looping sounds attached to a specific entity.
    */
    class StopEntitySoundEvent : public Event
    {
    public:
        StopEntitySoundEvent(Uma_ECS::Entity entity) : entity(entity) { priority = Priority::Low; }

    public:
        Uma_ECS::Entity entity;
    };

    /*!
    \brief Event to stop a specific sound by name from an entity.
    */
    class StopEntitySoundByNameEvent : public Event
    {
    public:
        StopEntitySoundByNameEvent(Uma_ECS::Entity entity, const std::string& soundName) 
            : entity(entity), soundName(soundName) {priority = Priority::Low;}
    public:
        Uma_ECS::Entity entity;
        std::string soundName;
    };

    class PlayOneShotAtEntityEvent : public Event
    {
    public:
        PlayOneShotAtEntityEvent(Uma_ECS::Entity entity, const std::string& soundName,
            float volume = 1.0f, bool is3D = true)
            : entity(entity), soundName(soundName), volume(volume), is3D(is3D) {
            priority = Priority::Low;
        }

    public:
        Uma_ECS::Entity entity;
        std::string soundName;
        float volume;
        bool is3D;
    };

    class PlayOneShotAtPositionEvent : public Event
    {
    public:
        PlayOneShotAtPositionEvent(float x, float y, const std::string& soundName,
            float volume = 1.0f, bool is3D = true)
            : x(x), y(y), soundName(soundName), volume(volume), is3D(is3D) {
            priority = Priority::Low;
        }

    public:
        float x, y;
        std::string soundName;
        float volume;
        bool is3D;
    };

}