#include "AudioSystem.hpp"

#include "../Components/AudioListener.h"
#include "../Components/AudioComponent.h"
#include "../Components/Transform.h"
#include "../Components/RigidBody.h"

#include "../Core/Coordinator.hpp"

#include "../Events/AudioEvents.h"

using PlayEntitySoundEvent = Uma_Engine::PlayEntitySoundEvent;
using StopEntitySoundEvent = Uma_Engine::StopEntitySoundEvent;
using PlayOneShotAtEntityEvent = Uma_Engine::PlayOneShotAtEntityEvent;
using UpdateEntityAudioComponentEvent = Uma_Engine::UpdateEntityAudioComponentEvent;

void Uma_ECS::AudioSystem::Init(Uma_Engine::SoundManager* sm, Coordinator* c, Uma_Engine::EventSystem* es)
{
    pCoordinator = c;
    pSoundManager = sm;
    pEventSystem = es;

    es->Subscribe<PlayEntitySoundEvent>(
        [this](const PlayEntitySoundEvent& e)
        {
            // Get entity position
            auto& tfArray = pCoordinator->GetComponentArray<Transform>();
            if (tfArray.Has(e.entity))
            {
                auto& tf = tfArray.GetData(e.entity);
                FMOD_VECTOR pos = { tf.position.x, tf.position.y, 0.0f };

                pSoundManager->PlayEntitySound(e.entity, e.soundName, pos, e.loop, e.volume);

                // Update AudioComponent if exists
                auto& audioArray = pCoordinator->GetComponentArray<AudioComponent>();
                if (audioArray.Has(e.entity))
                {
                    auto& audio = audioArray.GetData(e.entity);
                    audio.loopingSoundName = e.soundName;
                    audio.isPlaying = true;
                    audio.shouldLoop = e.loop;
                    audio.volume = e.volume;
                }
            }
        });

    es->Subscribe<StopEntitySoundEvent>(
        [this](const StopEntitySoundEvent& e)
        {
            pSoundManager->StopEntitySound(e.entity);

            // Update AudioComponent if exists
            auto& audioArray = pCoordinator->GetComponentArray<AudioComponent>();
            if (audioArray.Has(e.entity))
            {
                auto& audio = audioArray.GetData(e.entity);
                audio.isPlaying = false;
            }
        });

    es->Subscribe<PlayOneShotAtEntityEvent>(
        [this](const PlayOneShotAtEntityEvent& e)
        {
            auto& tfArray = pCoordinator->GetComponentArray<Transform>();
            if (tfArray.Has(e.entity))
            {
                auto& tf = tfArray.GetData(e.entity);
                FMOD_VECTOR pos = { tf.position.x, tf.position.y, 0.0f };
                pSoundManager->PlayOneShotAt(e.soundName, pos, e.volume);
            }
        });

    es->Subscribe<UpdateEntityAudioComponentEvent>(
        [this](const UpdateEntityAudioComponentEvent& e)
        {
            auto& audioArray = pCoordinator->GetComponentArray<AudioComponent>();
            if (audioArray.Has(e.entity))
            {
                auto& audio = audioArray.GetData(e.entity);
                audio.loopingSoundName = e.soundName;
                audio.isPlaying = e.isPlaying;
                audio.volume = e.volume;
            }
        });
}

void Uma_ECS::AudioSystem::Update(float dt)
{
    UpdateListener(dt);
    UpdateAudioEmitters(dt);
}

void Uma_ECS::AudioSystem::UpdateListener(float dt)
{
    (void)dt;

    auto& tfArray = pCoordinator->GetComponentArray<Transform>();
    auto& rbArray = pCoordinator->GetComponentArray<RigidBody>();

    for (auto const& entity : aEntities)
    {
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
    // Get entities with AudioComponent
    auto& tfArray = pCoordinator->GetComponentArray<Transform>();
    auto& rbArray = pCoordinator->GetComponentArray<RigidBody>();
    auto& audioArray = pCoordinator->GetComponentArray<AudioComponent>();

    for (size_t i = 0; i < audioArray.Size(); ++i)
    {
        Entity entity = audioArray.GetEntity(i);
        auto& audio = audioArray.GetComponentAt(i);

        // Check if entity also has Transform component
        if (!tfArray.Has(entity))
        {
            continue;  // Skip if no transform
        }

        auto& tf = tfArray.GetData(entity);

        // Update audio component position from transform
        FMOD_VECTOR newPosition = { tf.position.x, tf.position.y, 0.0f };

        // Calculate velocity (change in position / delta time)
        FMOD_VECTOR velocity = { rbArray.GetData(entity).velocity.x, rbArray.GetData(entity).velocity.y, 0.0f};

        // Update looping sound position in FMOD
        if (audio.isPlaying && !audio.loopingSoundName.empty())
        {
            pSoundManager->UpdateEntitySound(entity, newPosition, velocity);
        }
    }
}
