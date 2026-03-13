#pragma once
/*!
\file   AudioComponent.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Koh Kai Yang (100%)
\par    E-mail: k.kaiyang@digipen.edu
\par    DigiPen login: k.kaiyang

\brief
Adds the component tag to the entity so the system can use this entity to update the audio listener

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

using SoundType = Uma_Engine::SoundType;

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
        SoundType type = SoundType::SFX;

        bool isFading = false;
        FMOD_CHANNEL* fadeHandle = nullptr;

        // Optional: per-sound overrides
        float pitch = 1.0f;
        float minDistance = 30.0f;
        float maxDistance = 80.0f;
    };

	struct AudioComponent {
        FMOD_VECTOR position = { 0.0f, 0.0f, 0.0f };
        FMOD_VECTOR velocity = { 0.0f, 0.0f, 0.0f };

        float defaultVolume = 1.0f;
        bool default3D = true;

        //All active sounds playing on this entity
        std::unordered_map<std::string, std::vector<SoundInstance>> activeSounds;
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

            rapidjson::Value soundsObj(rapidjson::kObjectType);
            for (const auto& [name, instance] : loadedSounds) {
                rapidjson::Value soundVal(rapidjson::kObjectType);
                soundVal.AddMember("path", rapidjson::Value(instance.path.c_str(), allocator).Move(), allocator);
                soundVal.AddMember("isPlaying", instance.isPlaying, allocator);
                soundVal.AddMember("shouldLoop", instance.shouldLoop, allocator);
                soundVal.AddMember("is3D", instance.is3D, allocator);
                soundVal.AddMember("type", static_cast<int>(instance.type), allocator);
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
        void Deserialize(const rapidjson::Value& value)
        {
            loadedSounds.clear();

            if (value.HasMember("loadedSounds") && value["loadedSounds"].IsObject()) {
                for (rapidjson::SizeType i = 0; i < value["loadedSounds"].MemberCount(); i++) {
                    auto& member = value["loadedSounds"].MemberBegin()[i];

                    if (member.value.IsObject() &&
                        member.value.HasMember("path") && member.value["path"].IsString() &&
                        member.value.HasMember("isPlaying") && member.value["isPlaying"].IsBool() &&
                        member.value.HasMember("shouldLoop") && member.value["shouldLoop"].IsBool() &&
                        member.value.HasMember("is3D") && member.value["is3D"].IsBool()) {

                        std::string name(member.name.GetString(), member.name.GetStringLength());

                        SoundInstance instance;
                        instance.path = member.value["path"].GetString();
                        instance.isPlaying = member.value["isPlaying"].GetBool();
                        instance.shouldLoop = member.value["shouldLoop"].GetBool();
                        instance.is3D = member.value["is3D"].GetBool();
                        instance.volume = member.value["volume"].GetFloat();
                        instance.pitch = member.value["pitch"].GetFloat();
                        instance.minDistance = member.value["minDistance"].GetFloat();
                        instance.maxDistance = member.value["maxDistance"].GetFloat();

                        instance.type = member.value.HasMember("type") ? static_cast<Uma_Engine::SoundType>(member.value["type"].GetInt()) : Uma_Engine::SoundType::SFX;

                        loadedSounds.emplace(std::move(name), std::move(instance));
                    }
                }
            }

            defaultVolume = value["defaultVolume"].GetFloat();
            default3D = value["default3D"].GetBool();
        }


        /*!
        \brief Check if a sound with the given name exists in active sounds.
        \param soundName Name of the sound to look up.
        \return True if the sound is found in active sounds, false otherwise.
        */
        bool HasSound(const std::string& soundName) const
        {
            return activeSounds.find(soundName) != activeSounds.end();
        }

        /*!
        \brief Get the list of active sound instances for the given sound name.
        \param soundName Name of the sound to retrieve.
        \return Pointer to the vector of SoundInstance, or nullptr if not found.
        */
        std::vector<SoundInstance>* GetSound(const std::string& soundName)
        {
            auto it = activeSounds.find(soundName);
            return (it != activeSounds.end()) ? &it->second : nullptr;
        }

        /*!
        \brief Remove a sound from the active sounds map by name.
        \param soundName Name of the sound to remove.
        */
        void RemoveSound(const std::string& soundName)
        {
            activeSounds.erase(soundName);
        }

        /*!
        \brief Get the number of distinct active sounds on this entity.
        \return Count of active sound entries.
        */
        size_t GetActiveSoundCount() const
        {
            return activeSounds.size();
        }

        /*!
        \brief Get a pointer to a loaded sound instance by name.
        \param soundName Name of the loaded sound to retrieve.
        \return Pointer to the SoundInstance, or nullptr if not found.
        */
        SoundInstance* GetLoadedSound(const std::string& soundName)
        {
            auto it = loadedSounds.find(soundName);
            return (it != loadedSounds.end()) ? &it->second : nullptr;
        }

        /*!
        \brief Check if a sound with the given name exists in loaded sounds.
        \param soundName Name of the sound to look up.
        \return True if the sound is found in loaded sounds, false otherwise.
        */
        bool HasLoadedSound(const std::string& soundName) const
        {
            return loadedSounds.find(soundName) != loadedSounds.end();
        }

        /*!
        \brief Remove a sound from the loaded sounds map by name.
        \param soundName Name of the loaded sound to remove.
        */
        void RemoveLoadedSound(const std::string& soundName)
        {
            loadedSounds.erase(soundName);
        }
	};
}