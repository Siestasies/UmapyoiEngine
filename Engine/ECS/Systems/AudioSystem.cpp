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

    pEventSystem->Subscribe<Uma_Engine::PlayEntitySoundEvent>(
        [this](const Uma_Engine::PlayEntitySoundEvent& e)
        {
            auto& audioArray = pCoordinator->GetComponentArray<AudioComponent>();

            // Auto-add AudioComponent if doesn't exist
            if (!audioArray.Has(e.entity)) {
                pCoordinator->AddComponent(e.entity, AudioComponent{});
            }

            auto& audio = audioArray.GetData(e.entity);

            // Stop existing sound with same name
            if (audio.HasSound(e.soundName)) {
                pSoundManager->StopChannel(audio.GetSound(e.soundName)->channel);
                audio.RemoveSound(e.soundName);
            }

            // Get entity position
            auto& tfArray = pCoordinator->GetComponentArray<Transform>();
            if (!tfArray.Has(e.entity)) return;

            auto& tf = tfArray.GetData(e.entity);
            FMOD_VECTOR pos = { tf.position.x, tf.position.y, 0.0f };

            bool is3D = audio.default3D;

            // Play sound
            FMOD_CHANNEL* channel = pSoundManager->PlaySoundInstance(e.soundName, e.loop, e.volume, pos, is3D);

            if (channel) {
                SoundInstance instance;
                instance.channel = channel;
                instance.soundName = e.soundName;
                instance.volume = e.volume;
                instance.isPlaying = true;
                instance.shouldLoop = e.loop;
                instance.is3D = is3D;

                audio.activeSounds[e.soundName] = instance;
            }
        });

    // Stop all entity sounds
    pEventSystem->Subscribe<Uma_Engine::StopEntitySoundEvent>(
        [this](const Uma_Engine::StopEntitySoundEvent& e)
        {
            auto& audioArray = pCoordinator->GetComponentArray<AudioComponent>();
            if (!audioArray.Has(e.entity)) return;

            auto& audio = audioArray.GetData(e.entity);

            for (auto& [name, sound] : audio.activeSounds) {
                pSoundManager->StopChannel(sound.channel);
            }

            audio.activeSounds.clear();
        });

    // Stop specific sound by name
    pEventSystem->Subscribe<Uma_Engine::StopEntitySoundByNameEvent>(
        [this](const Uma_Engine::StopEntitySoundByNameEvent& e)
        {
            auto& audioArray = pCoordinator->GetComponentArray<AudioComponent>();
            if (!audioArray.Has(e.entity)) return;

            auto& audio = audioArray.GetData(e.entity);

            if (audio.HasSound(e.soundName)) {
                pSoundManager->StopChannel(audio.GetSound(e.soundName)->channel);
                audio.RemoveSound(e.soundName);
            }
        });

    // Play one-shot at entity
    pEventSystem->Subscribe<Uma_Engine::PlayOneShotAtEntityEvent>(
        [this](const Uma_Engine::PlayOneShotAtEntityEvent& e)
        {
            auto& tfArray = pCoordinator->GetComponentArray<Transform>();
            if (!tfArray.Has(e.entity)) return;

            auto& tf = tfArray.GetData(e.entity);
            FMOD_VECTOR pos = { tf.position.x, tf.position.y, 0.0f };

            pSoundManager->PlayOneShotAt(e.soundName, pos, e.volume, e.is3D);
        });

    // Play one-shot at position
    pEventSystem->Subscribe<Uma_Engine::PlayOneShotAtPositionEvent>(
        [this](const Uma_Engine::PlayOneShotAtPositionEvent& e)
        {
            FMOD_VECTOR pos = { e.x, e.y, 0.0f };

            pSoundManager->PlayOneShotAt(e.soundName, pos, e.volume, e.is3D);
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
    auto& alArray = pCoordinator->GetComponentArray<AudioListener>();

    for (size_t i = 0; i < alArray.Size(); ++i)
    {
        Entity entity = alArray.GetEntity(i);

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

    for (auto const& entity : aEntities)
    {

        // Check if entity also has Transform component
        if (!tfArray.Has(entity))
        {
            continue;  // Skip if no transform
        }

        auto& tf = tfArray.GetData(entity);
        auto& rb = rbArray.GetData(entity);
        auto& ac = audioArray.GetData(entity);

        // Update audio component position from transform
        FMOD_VECTOR newPosition = { tf.position.x, tf.position.y, 0.0f };

        // Calculate velocity (change in position / delta time)
        FMOD_VECTOR velocity = { rb.velocity.x, rb.velocity.y, 0.0f};

        ac.position = newPosition;
        ac.velocity = velocity;

        for (auto it = ac.activeSounds.begin(); it != ac.activeSounds.end(); )
        {
            SoundInstance& sound = it->second;

            // Check if still playing
            FMOD_BOOL isPlaying = false;
            FMOD_RESULT result = FMOD_Channel_IsPlaying(sound.channel, &isPlaying);

            if (result != FMOD_OK || !isPlaying) {
                // Sound finished, remove it
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
