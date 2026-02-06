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

void Uma_ECS::AudioSystem::Init(Uma_Engine::SoundManager* sm, Coordinator* c, Uma_Engine::EventSystem* es, Uma_Engine::ResourcesManager* rm)
{
    pCoordinator = c;
    pSoundManager = sm;
    pResourcesManager = rm;
    pEventSystem = es;

    // Event subscriptions delegate to the public methods for backward compatibility
    /*pEventSystem->Subscribe<Uma_Engine::PlayEntitySoundEvent, AudioSystem>(
        [this](const Uma_Engine::PlayEntitySoundEvent& e)
        {
            PlayEntitySound(e.entity, e.soundName, e.loop, e.volume);
        });*/

    pEventSystem->Subscribe<Uma_Engine::StopEntitySoundEvent, AudioSystem>(
        [this](const Uma_Engine::StopEntitySoundEvent& e)
        {
            StopEntitySound(e.entity);
        });

    pEventSystem->Subscribe<Uma_Engine::StopEntitySoundByNameEvent, AudioSystem>(
        [this](const Uma_Engine::StopEntitySoundByNameEvent& e)
        {
            StopEntitySound(e.entity, e.soundName);
        });

    /*pEventSystem->Subscribe<Uma_Engine::PlayOneShotAtEntityEvent, AudioSystem>(
        [this](const Uma_Engine::PlayOneShotAtEntityEvent& e)
        {
            PlayOneShotAtEntity(e.entity, e.soundName, e.volume, e.is3D);
        });*/

    pEventSystem->Subscribe<Uma_Engine::PlayOneShotAtPositionEvent, AudioSystem>(
        [this](const Uma_Engine::PlayOneShotAtPositionEvent& e)
        {
            PlayOneShotAtPosition(e.entity, e.x, e.y, e.soundName, e.volume, e.is3D);
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

        if (!tfArray.Has(entity) || !rbArray.Has(entity))
            continue;

        auto& tf = tfArray.GetData(entity);
        auto& rb = rbArray.GetData(entity);

        FMOD_VECTOR newPosition = { tf.position.x, tf.position.y, 0.0f };
        FMOD_VECTOR velocity = { rb.velocity.x, rb.velocity.y, 0.0f };
        ac.position = newPosition;
        ac.velocity = velocity;

        for (auto groupIt = ac.activeSounds.begin(); groupIt != ac.activeSounds.end(); ) {
            auto& instances = groupIt->second;

            for (auto instIt = instances.begin(); instIt != instances.end(); ) {
                FMOD_BOOL isPlaying = false;
                FMOD_RESULT result = FMOD_Channel_IsPlaying(instIt->channel, &isPlaying);

                if (result != FMOD_OK || !isPlaying) {
                    instIt = instances.erase(instIt);
                    continue;
                }

                if (instIt->is3D && !instIt->isFading) {
                    pSoundManager->UpdateChannel3DPosition(instIt->channel, ac.position, ac.velocity);
                }


                instIt->isPlaying = true;

                ++instIt;
            }

            // Remove empty sound groups
            if (instances.empty()) {
                groupIt = ac.activeSounds.erase(groupIt);
            }
            else {
                ++groupIt;
            }
        }
    }
}


SoundInfo* Uma_ECS::AudioSystem::GetSoundInfo(Entity entity, const std::string& soundName) {
    if (!pResourcesManager) return nullptr;
    AudioComponent audio = pCoordinator->GetComponent<AudioComponent>(entity);
    if (!audio.HasLoadedSound(soundName)) {
        return nullptr;
    }
    //if dont have load it
    if (!pResourcesManager->HasSound(soundName)) {
        auto& instance = audio.loadedSounds[soundName];
        if(instance.is3D)
            pResourcesManager->LoadSound(soundName, audio.loadedSounds[soundName].path, SoundType::SFX);
        else
            pResourcesManager->LoadSound(soundName, audio.loadedSounds[soundName].path, SoundType::BGM);
    }
    return pResourcesManager->GetSound(soundName);
    
}

void Uma_ECS::AudioSystem::OnEntityDestroyed(Entity entity)
{
    auto& audioArray = pCoordinator->GetComponentArray<AudioComponent>();

    if (audioArray.Has(entity)) {
        StopEntitySound(entity);
    }
}

void Uma_ECS::AudioSystem::StopAllEntityAudio()
{
    auto& audioArray = pCoordinator->GetComponentArray<AudioComponent>();

    for (size_t i = 0; i < audioArray.Size(); ++i)
    {
        StopEntitySound(audioArray.GetEntity(i));
    }
}

void Uma_ECS::AudioSystem::PlayEntitySound(Entity entity, const std::string& soundName)
{
    auto& audioArray = pCoordinator->GetComponentArray<AudioComponent>();

    // Auto-add AudioComponent if doesn't exist
    if (!audioArray.Has(entity)) {
        pCoordinator->AddComponent(entity, AudioComponent{});
    }

    auto& audio = audioArray.GetData(entity);

    // Stop existing sound with same name
    SoundInfo* info = GetSoundInfo(entity, soundName);
    if (!info) return;

    // getting sound form resources here

    // Get entity position
    auto& tfArray = pCoordinator->GetComponentArray<Transform>();
    if (!tfArray.Has(entity)) return;

    auto& tf = tfArray.GetData(entity);
    FMOD_VECTOR pos = { tf.position.x, tf.position.y, 0.0f };

    bool is3D = audio.loadedSounds[soundName].is3D;

    // Play sound
    FMOD_CHANNEL* channel = pSoundManager->PlaySoundInstance(info, audio.loadedSounds[soundName].shouldLoop, audio.loadedSounds[soundName].volume, pos, is3D);

    if (channel) {
        audio.loadedSounds[soundName].channel = channel;
        audio.activeSounds[soundName].push_back(audio.loadedSounds[soundName]);
    }
}

void Uma_ECS::AudioSystem::StopEntitySound(Entity entity)
{
    auto& audioArray = pCoordinator->GetComponentArray<AudioComponent>();
    if (!audioArray.Has(entity)) return;

    auto& audio = audioArray.GetData(entity);

    for (auto& [name, instances] : audio.activeSounds) {
        for (auto& sound : instances) {
            if (sound.channel) {
                pSoundManager->StopChannel(sound.channel);
            }
        }
    }

    audio.activeSounds.clear();
}

void Uma_ECS::AudioSystem::StopEntitySound(Entity entity, const std::string& soundName)
{
    auto& audioArray = pCoordinator->GetComponentArray<AudioComponent>();
    if (!audioArray.Has(entity)) return;

    auto& audio = audioArray.GetData(entity);

    if (auto* instances = audio.GetSound(soundName)) {
        for (auto& sound : *instances) {
            if (sound.channel) {
                pSoundManager->StopChannel(sound.channel);
            }
        }
        instances->clear();
        if (instances->empty()) {
            audio.activeSounds.erase(soundName);
        }
    }
}

void Uma_ECS::AudioSystem::PlayOneShotAtEntity(Entity entity, const std::string& soundName)
{
    auto& tfArray = pCoordinator->GetComponentArray<Transform>();
    if (!tfArray.Has(entity)) return;

    auto& audioArray = pCoordinator->GetComponentArray<AudioComponent>();
    auto& audio = audioArray.GetData(entity);

    auto& tf = tfArray.GetData(entity);
    FMOD_VECTOR pos = { tf.position.x, tf.position.y, 0.0f };

    pSoundManager->PlayOneShotAt(GetSoundInfo(entity, soundName), pos, audio.loadedSounds[soundName].volume, audio.loadedSounds[soundName].is3D);
}

//the entity for this is just a container to hold all the audio files
void Uma_ECS::AudioSystem::PlayOneShotAtPosition(Entity entity, float x, float y, const std::string& soundName, float volume, bool is3D)
{
    FMOD_VECTOR pos = { x, y, 0.0f };

    pSoundManager->PlayOneShotAt(GetSoundInfo(entity, soundName), pos, volume, is3D);
}

void Uma_ECS::AudioSystem::PlayEntitySoundFaded(Entity entity, const std::string& soundName, float fadeInTime) {
    auto& audioArray = pCoordinator->GetComponentArray<AudioComponent>();
    if (!audioArray.Has(entity)) {
        pCoordinator->AddComponent(entity, AudioComponent{});
    }

    auto& audio = audioArray.GetData(entity);
    SoundInfo* info = GetSoundInfo(entity, soundName);
    if (!info) return;

    auto& tfArray = pCoordinator->GetComponentArray<Transform>();
    if (!tfArray.Has(entity)) return;

    auto& tf = tfArray.GetData(entity);
    FMOD_VECTOR pos = { tf.position.x, tf.position.y, 0.0f };
    bool is3D = audio.loadedSounds[soundName].is3D;
    bool shouldLoop = audio.loadedSounds[soundName].shouldLoop;
    float volume = audio.loadedSounds[soundName].volume;

    // Play SILENT, then fade in
    FMOD_CHANNEL* channel = pSoundManager->PlaySoundInstanceFaded(
        info, shouldLoop, volume, pos, is3D, fadeInTime
    );

    if (channel) {
        audio.activeSounds[soundName].emplace_back(SoundInstance{
            channel,                    // channel
            soundName,                  // soundName
            info->filePath,             // path
            volume,                     // volume
            false,                      // isPlaying
            shouldLoop,                 // shouldLoop
            is3D,                       // is3D
            true,                       // isFading
            channel                     // fadeHandle
            });
    }
}

void Uma_ECS::AudioSystem::FadeOutSound(Entity entity, const std::string& soundName, float fadeOutTime) {
    auto& audioArray = pCoordinator->GetComponentArray<AudioComponent>();
    if (!audioArray.Has(entity)) return;

    auto& audio = audioArray.GetData(entity);
    if (auto* instances = audio.GetSound(soundName)) {
        for (auto& instance : *instances) {
            if (instance.channel) {
                pSoundManager->FadeOutChannel(instance.channel, fadeOutTime);
                instance.isFading = true;
            }
        }
    }
}

void Uma_ECS::AudioSystem::FadeOutEntity(Entity entity, float fadeOutTime) {
    auto& audioArray = pCoordinator->GetComponentArray<AudioComponent>();
    if (!audioArray.Has(entity)) return;

    auto& audio = audioArray.GetData(entity);
    for (auto& [name, instances] : audio.activeSounds) {
        for (auto& instance : instances) {
            if (instance.channel) {
                pSoundManager->FadeOutChannel(instance.channel, fadeOutTime);
                instance.isFading = true;
            }
        }
    }
}


