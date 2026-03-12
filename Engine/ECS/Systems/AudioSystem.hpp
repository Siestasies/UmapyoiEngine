#pragma once
/*!
\file   AudioSystem.hpp
\par    Project: GAM250
\par    Course: CSD2451
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
         * \brief Updates the listener position and all active audio systems in the sound manager.
         * \param dt Time step since the last update in seconds.
         */
        void Update(float dt);

        /*!
         * \brief Shuts down the sound manager and releases all resources.
         */
        void Shutdown();

        /*!
         * \brief Stops all audio currently playing from a specific entity.
         * \param entity Entity whose audio to stop.
         */
        void StopAllEntityAudio();

        /*!
         * \brief Cleans up audio resources when an entity is destroyed.
         * \param entity Entity that was destroyed.
         */
        void OnEntityDestroyed(Entity entity);

        /*!
         * \brief Starts playing a named sound attached to an entity.
         * \param entity Entity to attach the sound to.
         * \param soundName Name of the sound resource to play.
         */
        void PlayEntitySound(Entity entity, const std::string& soundName);

        /*!
         * \brief Stops all sounds currently playing on an entity.
         * \param entity Entity whose sounds to stop.
         */
        void StopEntitySound(Entity entity);

        /*!
         * \brief Stops a specific named sound on an entity.
         * \param entity Entity playing the sound.
         * \param soundName Name of the sound resource to stop.
         */
        void StopEntitySound(Entity entity, const std::string& soundName);

        /*!
         * \brief Plays a one-shot sound attached to an entity position.
         * \param entity Entity whose position to use for 3D audio.
         * \param soundName Name of the sound resource to play.
         */
        void PlayOneShotAtEntity(Entity entity, const std::string& soundName);

        /*!
         * \brief Plays a one-shot sound at a specific world position relative to an entity.
         * \param entity Reference entity for listener-relative positioning.
         * \param x X-coordinate of sound position.
         * \param y Y-coordinate of sound position.
         * \param soundName Name of the sound resource to play.
         * \param volume Playback volume (0.0�1.0).
         * \param is3D Whether to play as 3D spatialized sound.
         */
        void PlayOneShotAtPosition(Entity entity, float x, float y, const std::string& soundName, float volume, bool is3D);

        /*!
         * \brief Retrieves the SoundInfo for a specific sound on an entity.
         * \param entity Entity owning the sound.
         * \param soundName Name of the sound resource.
         * \return Pointer to SoundInfo, or nullptr if not found.
         */
        SoundInfo* GetSoundInfo(Entity entity, const std::string& soundName);

        /*!
         * \brief Plays an entity sound with a fade-in effect.
         * \param entity Entity to attach the sound to.
         * \param soundName Name of the sound resource to play.
         * \param fadeInTime Duration of fade-in in seconds.
         */
        void PlayEntitySoundFaded(Entity entity, const std::string& soundName, float fadeInTime = 1.0f);

        /*!
         * \brief Fades out a specific sound on an entity.
         * \param entity Entity playing the sound.
         * \param soundName Name of the sound resource to fade out.
         * \param fadeOutTime Duration of fade-out in seconds.
         */
        void FadeOutSound(Entity entity, const std::string& soundName, float fadeOutTime = 1.0f);

        /*!
         * \brief Fades out and stops all sounds on an entity.
         * \param entity Entity whose sounds to fade out.
         * \param fadeOutTime Duration of fade-out in seconds.
         */
        void FadeOutEntity(Entity entity, float fadeOutTime = 1.0f);

        /*!
         * \brief Toggles a low-pass filter on a specific sound instance owned by an entity.
         * \param entity    The entity holding this sound channel
         * \param soundName The name key identifying the sound within the entity's sound map.
         * \param dulled    If true, applies a muffled low-pass cutoff (1000 Hz);
         *                  if false, restores a near-full-range cutoff (22000 Hz).
         */
        void toggleLowpass(Entity entity, const std::string& soundName, bool dulled);

        /*!
         * \brief Toggles a low-pass filter on an entire sound group bus.
         * \param type   The bus to target: SoundType::SFX, SoundType::BGM, or SoundType::MASTER.
         * \param dulled If true, activates the low-pass DSP on the specified bus;
         *               if false, bypasses it.
         */
        void toggleLowpass(SoundType type, bool dulled);

    private:

        /*!
         * \brief Updates the audio listener position and orientation based on the listener entity's transform.
         * \param dt Delta time in seconds since last frame.
         */
        void UpdateListener(float dt);

        /*!
         * \brief Updates all audio emitter components, synchronizing positions and playback state with the sound manager.
         * \param dt Delta time in seconds since last frame.
         */
        void UpdateAudioEmitters(float dt);

        Coordinator* pCoordinator = nullptr;
        Uma_Engine::SoundManager* pSoundManager = nullptr;
        Uma_Engine::ResourcesManager* pResourcesManager = nullptr;
        Uma_Engine::EventSystem* pEventSystem = nullptr;

    };
}