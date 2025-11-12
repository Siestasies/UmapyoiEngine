#pragma once
/*!
\file   AudioComponent.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Koh Kai Yang (100%)
\par    E-mail: k.kaiyang@digipen.edu
\par    DigiPen login: k.kaiyang

\brief
Adds the component tag to the entity so the system can use this entity to update the audio listener

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

namespace Uma_ECS
{
    struct SoundInstance
    {
        FMOD_CHANNEL* channel = nullptr;
        std::string soundName;
        float volume = 1.0f;
        bool isPlaying = false;
        bool shouldLoop = false;
        bool is3D = true;

        // Optional: per-sound overrides
        float pitch = 1.0f;
        float minDistance = 100.0f;
        float maxDistance = 1000.0f;
    };

	struct AudioComponent {
        FMOD_VECTOR position = { 0.0f, 0.0f, 0.0f };
        FMOD_VECTOR velocity = { 0.0f, 0.0f, 0.0f };

        float defaultVolume = 1.0f;
        bool default3D = true;

        //All active sounds playing on this entity
        std::unordered_map<std::string, SoundInstance> activeSounds;

        //runtime
        std::string loopingSoundName;

        /*!
        * \brief this is to serialize the info from json
        * \param value, allocator 
        * \return nothing
        */
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const //override
        {
            (void)value;
            (void)allocator;
        }

        /*!
        * \brief this is to deserialize the info into json
        * \param value
        * \return nothing
        */
        void Deserialize(const rapidjson::Value& value) //override
        {
            (void)value;
        }

        // Helper methods
        bool HasSound(const std::string& soundName) const
        {
            return activeSounds.find(soundName) != activeSounds.end();
        }

        SoundInstance* GetSound(const std::string& soundName)
        {
            auto it = activeSounds.find(soundName);
            return (it != activeSounds.end()) ? &it->second : nullptr;
        }

        void RemoveSound(const std::string& soundName)
        {
            activeSounds.erase(soundName);
        }

        size_t GetActiveSoundCount() const
        {
            return activeSounds.size();
        }
	};
}