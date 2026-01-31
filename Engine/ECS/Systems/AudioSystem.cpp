/*!
\file   AudioSystem.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Koh Kai Yang (100%)
\par    E-mail: k.kaiyang@digipen.edu
\par    DigiPen login: k.kaiyang

\brief
Implementation of updating the audio listener position/audio component

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "AudioSystem.hpp"

#include "../Components/AudioListener.h"
#include "../Components/AudioComponent.h"
#include "../Components/Transform.h"
#include "../Components/RigidBody.h"

#include "../Core/Coordinator.hpp"

#include "../Events/AudioEvents.h"

void Uma_ECS::AudioSystem::Init(Uma_Engine::SoundManager* sm, Coordinator* c, Uma_Engine::EventSystem* es)
{
    pCoordinator = c;
    pSoundManager = sm;
    pEventSystem = es;

    // Event subscriptions delegate to the public methods for backward compatibility
    pEventSystem->Subscribe<Uma_Engine::PlayEntitySoundEvent, AudioSystem>(
        [this](const Uma_Engine::PlayEntitySoundEvent& e)
        {
            PlayEntitySound(e.entity, e.soundName, e.loop, e.volume);
        });

    pEventSystem->Subscribe<Uma_Engine::StopEntitySoundEvent, AudioSystem>(
        [this](const Uma_Engine::StopEntitySoundEvent& e)
        {
            StopEntitySound(e.entity);
        });

    pEventSystem->Subscribe<Uma_Engine::StopEntitySoundByNameEvent, AudioSystem>(
        [this](const Uma_Engine::StopEntitySoundByNameEvent& e)
        {
            StopEntitySoundByName(e.entity, e.soundName);
        });

    pEventSystem->Subscribe<Uma_Engine::PlayOneShotAtEntityEvent, AudioSystem>(
        [this](const Uma_Engine::PlayOneShotAtEntityEvent& e)
        {
            PlayOneShotAtEntity(e.entity, e.soundName, e.volume, e.is3D);
        });

    pEventSystem->Subscribe<Uma_Engine::PlayOneShotAtPositionEvent, AudioSystem>(
        [this](const Uma_Engine::PlayOneShotAtPositionEvent& e)
        {
            PlayOneShotAtPosition(e.x, e.y, e.soundName, e.volume, e.is3D);
        });
}

void Uma_ECS::AudioSystem::Update(float dt)
{
    UpdateListener(dt);
    UpdateAudioEmitters(dt);
}

void Uma_ECS::AudioSystem::Shutdown()
{
    pEventSystem->UnsubscribeSystem<AudioSystem>();
    StopAllEntityAudio();
}

void Uma_ECS::AudioSystem::UpdateListener(float dt)
{
    (void)dt;

    auto& tfArray = pCoordinator->GetComponentArray<Transform>();
    auto& rbArray = pCoordinator->GetComponentArray<RigidBody>();
    auto& alArray = pCoordinator->GetComponentArray<AudioListener>();

    for (size_t i = 0; i < alArray.Size(); ++i)
    {
        Entity entity = alArray.GetEntity(i);

        if (!pCoordinator->IsActiveInHierarchy(entity))
            continue;

        auto& tf = tfArray.GetData(entity);
        auto& rb = rbArray.GetData(entity);

        FMOD_VECTOR listenerPos = { tf.position.x, tf.position.y, 0.0f };
        FMOD_VECTOR listenerVel = { rb.velocity.x, rb.velocity.y, 0.0f };
        FMOD_VECTOR listenerForward = { 0.0f, 0.0f, 1.0f };
        FMOD_VECTOR listenerUp = { 0.0f, 1.0f, 0.0f };

        pSoundManager->setListenerPosition(listenerPos, listenerVel, listenerForward, listenerUp);
    }
}

void Uma_ECS::AudioSystem::UpdateAudioEmitters(float dt)
{
    (void)dt;

    auto& tfArray = pCoordinator->GetComponentArray<Transform>();
    auto& rbArray = pCoordinator->GetComponentArray<RigidBody>();
    auto& audioArray = pCoordinator->GetComponentArray<AudioComponent>();

    for (size_t i = 0; i < audioArray.Size(); ++i)
    {
        Entity entity = audioArray.GetEntity(i);
        auto& ac = audioArray.GetComponentAt(i);

        if (!pCoordinator->IsActiveInHierarchy(entity))
            continue;

        // Check if entity has required components
        if (!tfArray.Has(entity)) {
            continue;
        }

        if (!rbArray.Has(entity)) {
            continue;
        }

        auto& tf = tfArray.GetData(entity);
        auto& rb = rbArray.GetData(entity);

        // Update audio component position from transform
        FMOD_VECTOR newPosition = { tf.position.x, tf.position.y, 0.0f };
        FMOD_VECTOR velocity = { rb.velocity.x, rb.velocity.y, 0.0f };

        ac.position = newPosition;
        ac.velocity = velocity;

        for (auto it = ac.activeSounds.begin(); it != ac.activeSounds.end(); )
        {
            SoundInstance& sound = it->second;

            // Check if still playing
            FMOD_BOOL isPlaying = false;
            FMOD_RESULT result = FMOD_Channel_IsPlaying(sound.channel, &isPlaying);

            if (result != FMOD_OK || !isPlaying) {
                it = ac.activeSounds.erase(it);
                continue;
            }

            // Update 3D position
            if (sound.is3D) {
                pSoundManager->UpdateChannel3DPosition(sound.channel, ac.position, ac.velocity);
            }
            ++it;
        }
    }
}


void Uma_ECS::AudioSystem::StopAllEntityAudio()
{
    auto& audioArray = pCoordinator->GetComponentArray<AudioComponent>();

    for (size_t i = 0; i < audioArray.Size(); ++i)
    {
        auto& audio = audioArray.GetComponentAt(i);

        // Stop all channels
        for (auto& [name, sound] : audio.activeSounds) {
            pSoundManager->StopChannel(sound.channel);
        }

        audio.activeSounds.clear();
    }
}

void Uma_ECS::AudioSystem::OnEntityDestroyed(Entity entity)
{
    auto& audioArray = pCoordinator->GetComponentArray<AudioComponent>();

    if (audioArray.Has(entity)) {
        auto& audio = audioArray.GetData(entity);

        for (auto& [name, sound] : audio.activeSounds) {
            pSoundManager->StopChannel(sound.channel);
        }

        audio.activeSounds.clear();
    }
}

void Uma_ECS::AudioSystem::PlayEntitySound(Entity entity, const std::string& soundName, bool loop, float volume)
{
    auto& audioArray = pCoordinator->GetComponentArray<AudioComponent>();

    // Auto-add AudioComponent if doesn't exist
    if (!audioArray.Has(entity)) {
        pCoordinator->AddComponent(entity, AudioComponent{});
    }

    auto& audio = audioArray.GetData(entity);

    // Stop existing sound with same name
    if (audio.HasSound(soundName)) {
        pSoundManager->StopChannel(audio.GetSound(soundName)->channel);
        audio.RemoveSound(soundName);
    }

    // Get entity position
    auto& tfArray = pCoordinator->GetComponentArray<Transform>();
    if (!tfArray.Has(entity)) return;

    auto& tf = tfArray.GetData(entity);
    FMOD_VECTOR pos = { tf.position.x, tf.position.y, 0.0f };

    bool is3D = audio.default3D;

    // Play sound
    FMOD_CHANNEL* channel = pSoundManager->PlaySoundInstance(soundName, loop, volume, pos, is3D);

    if (channel) {
        SoundInstance instance;
        instance.channel = channel;
        instance.soundName = soundName;
        instance.volume = volume;
        instance.isPlaying = true;
        instance.shouldLoop = loop;
        instance.is3D = is3D;

        audio.activeSounds[soundName] = instance;
    }
}

void Uma_ECS::AudioSystem::StopEntitySound(Entity entity)
{
    auto& audioArray = pCoordinator->GetComponentArray<AudioComponent>();
    if (!audioArray.Has(entity)) return;

    auto& audio = audioArray.GetData(entity);

    for (auto& [name, sound] : audio.activeSounds) {
        pSoundManager->StopChannel(sound.channel);
    }

    audio.activeSounds.clear();
}

void Uma_ECS::AudioSystem::StopEntitySoundByName(Entity entity, const std::string& soundName)
{
    auto& audioArray = pCoordinator->GetComponentArray<AudioComponent>();
    if (!audioArray.Has(entity)) return;

    auto& audio = audioArray.GetData(entity);

    if (audio.HasSound(soundName)) {
        pSoundManager->StopChannel(audio.GetSound(soundName)->channel);
        audio.RemoveSound(soundName);
    }
}

void Uma_ECS::AudioSystem::PlayOneShotAtEntity(Entity entity, const std::string& soundName, float volume, bool is3D)
{
    auto& tfArray = pCoordinator->GetComponentArray<Transform>();
    if (!tfArray.Has(entity)) return;

    auto& tf = tfArray.GetData(entity);
    FMOD_VECTOR pos = { tf.position.x, tf.position.y, 0.0f };

    pSoundManager->PlayOneShotAt(soundName, pos, volume, is3D);
}

void Uma_ECS::AudioSystem::PlayOneShotAtPosition(float x, float y, const std::string& soundName, float volume, bool is3D)
{
    FMOD_VECTOR pos = { x, y, 0.0f };

    pSoundManager->PlayOneShotAt(soundName, pos, volume, is3D);
}
