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
        std::string path;
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
        std::unordered_map<std::string, SoundInstance> loadedSounds;

        //runtime
        std::string loopingSoundName;

        /*!
        * \brief this is to serialize the info from json
        * \param value, allocator 
        * \return nothing
        */
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const //override
        {
            value.SetObject();

            value.AddMember("defaultVolume", defaultVolume, allocator);
            value.AddMember("default3D", default3D, allocator);

            // Serialize loadedSounds manually - no SoundInstance::Serialize needed
            rapidjson::Value soundsObj(rapidjson::kObjectType);
            for (const auto& [name, instance] : loadedSounds) {
                rapidjson::Value soundVal(rapidjson::kObjectType);
                soundVal.AddMember("path", rapidjson::Value(instance.path.c_str(), allocator).Move(), allocator);
                soundVal.AddMember("isPlaying", instance.isPlaying, allocator);
                soundVal.AddMember("shouldLoop", instance.shouldLoop, allocator);
                soundVal.AddMember("is3D", instance.is3D, allocator);
                soundVal.AddMember("volume", instance.volume, allocator);
                soundVal.AddMember("pitch", instance.pitch, allocator);
                soundVal.AddMember("minDistance", instance.minDistance, allocator);
                soundVal.AddMember("maxDistance", instance.maxDistance, allocator);
                soundsObj.AddMember(rapidjson::Value(name.c_str(), allocator).Move(), soundVal.Move(), allocator);
            }
            value.AddMember("loadedSounds", soundsObj, allocator);
        }

        /*!
        * \brief this is to deserialize the info into json
        * \param value
        * \return nothing
        */
        void Deserialize(const rapidjson::Value& value) //override
        {
            defaultVolume = value["defaultVolume"].GetFloat();
            default3D = value["default3D"].GetBool();

            // Deserialize loadedSounds manually
            loadedSounds.clear();
            if (value.HasMember("loadedSounds") && value["loadedSounds"].IsObject()) {
                for (auto& member : value["loadedSounds"].GetObject()) {
                    std::string name(member.name.GetString(), member.name.GetStringLength());

                    SoundInstance instance;
                    const auto& soundVal = member.value;
                    instance.path = soundVal["path"].GetString();
                    instance.isPlaying = soundVal["isPlaying"].GetBool();
                    instance.shouldLoop = soundVal["shouldLoop"].GetBool();
                    instance.is3D = soundVal["is3D"].GetBool();
                    instance.volume = soundVal["volume"].GetFloat();
                    instance.pitch = soundVal["pitch"].GetFloat();
                    instance.minDistance = soundVal["minDistance"].GetFloat();
                    instance.maxDistance = soundVal["maxDistance"].GetFloat();

                    loadedSounds.emplace(std::move(name), std::move(instance));
                }
            }
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

        //for loaded sounds
        SoundInstance* GetLoadedSound(const std::string& soundName)
        {
            auto it = loadedSounds.find(soundName);
            return (it != loadedSounds.end()) ? &it->second : nullptr;
        }

        bool HasLoadedSound(const std::string& soundName) const
        {
            return loadedSounds.find(soundName) != loadedSounds.end();
        }

        void RemoveLoadedSound(const std::string& soundName)
        {
            loadedSounds.erase(soundName);
        }
	};
}