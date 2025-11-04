#pragma once
#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"
#include "../Core/EventSystem.h"

#include "../Systems/SoundManager.hpp"
#include "../Systems/ResourcesManager.hpp"

namespace Uma_ECS
{
    class AudioSystem : public ECSSystem
    {
    public:
        void Init(Uma_Engine::SoundManager* sm, Coordinator* c);
        void Update(float dt);

    private:

        Coordinator* pCoordinator = nullptr;
        Uma_Engine::SoundManager* pSoundManager = nullptr;
        Uma_Engine::ResourcesManager* pResourcesManager = nullptr;
        Uma_Engine::EventSystem* pEventSystem = nullptr;
    };
}