#include "AudioSystem.hpp"

#include "../Components/AudioListener.h"
#include "../Components/Transform.h"
#include "../Components/RigidBody.h"

#include "../Core/Coordinator.hpp"

#include "../Events/AudioEvents.h"

void Uma_ECS::AudioSystem::Init(Uma_Engine::SoundManager* sm, Coordinator* c)
{
    pCoordinator = c;
    pSoundManager = sm;
}

void Uma_ECS::AudioSystem::Update(float dt)
{
    auto& listenerArray = pCoordinator->GetComponentArray<AudioListener>();
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
