#pragma once
/*!
\file   AudioSystem.hpp
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
        /*!
        * \brief passes reference of the sound manager and coordinator to this component update
        * \param sound manager and coordinator pointer
        * \return nothing
        */
        void Init(Uma_Engine::SoundManager* sm, Coordinator* c, Uma_Engine::EventSystem* es, Uma_Engine::ResourcesManager* rm);

        /*!
        * \brief updates the listner position in sound manager
        * \return nothing
        */
        void Update(float dt);

        void Shutdown();

        void StopAllEntityAudio();

        void OnEntityDestroyed(Entity entity);

        void PlayEntitySound(Entity entity, const std::string& soundName);
        void StopEntitySound(Entity entity);
        void StopEntitySound(Entity entity, const std::string& soundName);
        void PlayOneShotAtEntity(Entity entity, const std::string& soundName);
        void PlayOneShotAtPosition(Entity entity, float x, float y, const std::string& soundName, float volume, bool is3D);
        SoundInfo* GetSoundInfo(Entity entity, const std::string& soundName);

    private:

        void UpdateListener(float dt);
        void UpdateAudioEmitters(float dt);

        Coordinator* pCoordinator = nullptr;
        Uma_Engine::SoundManager* pSoundManager = nullptr;
        Uma_Engine::ResourcesManager* pResourcesManager = nullptr;
        Uma_Engine::EventSystem* pEventSystem = nullptr;

    };
}