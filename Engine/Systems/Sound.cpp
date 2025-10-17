/*!
\file   Sound.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Koh Kai Yang (100%)
\par    E-mail: k.kaiyang@digipen.edu
\par    DigiPen login: k.kaiyang

\brief
This implements wrapper functions for the use of Fmod

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "Sound.hpp"
#include <iostream>
#include <filesystem>

#define DEBUG

namespace Uma_Engine {

    Sound::Sound() : pFmodSystem(nullptr) 
    {

    }

    Sound::~Sound()
    {
        //Shutdown();
    }

    void Sound::Init()
    {
        FMOD_RESULT result = FMOD_System_Create(&pFmodSystem, FMOD_VERSION);
        if (result != FMOD_OK) {
            std::cerr << "Failed to create FMOD system: " << FMOD_ErrorString(result) << std::endl;
            return;
        }
        // Initialize FMOD system
        result = FMOD_System_Init(pFmodSystem, 32, FMOD_INIT_NORMAL, nullptr);
        if (result != FMOD_OK) {
            std::cerr << "Failed to initialize FMOD system: " << FMOD_ErrorString(result) << std::endl;
            return;
        }
        std::cout << "AudioManager initialized successfully" << std::endl;

        result = FMOD_System_CreateChannelGroup(pFmodSystem, "SFX", &SFX);
        if (result != FMOD_OK) {
            std::cerr << "Failed to create channel sfx: " << FMOD_ErrorString(result) << std::endl;
            return;
        }
        result = FMOD_System_CreateChannelGroup(pFmodSystem, "BGM", &BGM);
        if (result != FMOD_OK) {
            std::cerr << "Failed to create channel bgm: " << FMOD_ErrorString(result) << std::endl;
            return;
        }
        result = FMOD_System_GetMasterChannelGroup(pFmodSystem, &Master);
        if (result != FMOD_OK) {
            std::cerr << "Failed to create channel master: " << FMOD_ErrorString(result) << std::endl;
            return;
        }

        // Add BGM to Master
        result = FMOD_ChannelGroup_AddGroup(Master, BGM, true, nullptr);
        if (result != FMOD_OK) {
            printf("Error adding BGM to Master: %d\n", result);
        }

        // Add SFX to Master
        result = FMOD_ChannelGroup_AddGroup(Master, SFX, true, nullptr);
        if (result != FMOD_OK) {
            printf("Error adding SFX to Master: %d\n", result);
        }

        FMOD_System_CreateSoundGroup(pFmodSystem, "SFX_SG", &SFX_SG);

        FMOD_SoundGroup_SetMaxAudible(SFX_SG, 1);
        FMOD_SoundGroup_SetMaxAudibleBehavior(SFX_SG, FMOD_SOUNDGROUP_BEHAVIOR_MUTE);

        return;
    }

    void Sound::Shutdown()
    {
#ifdef DEBUG
        std::cout << "sound shutdown\n";
#endif // DEBUG
    }

    void Sound::Update(float dt)
    {
        (void)dt;

        if (pFmodSystem) {
            FMOD_System_Update(pFmodSystem);
        }
    }

    SoundInfo Sound::loadSound(const std::string& filePath, SoundType type)
    {
        SoundInfo info;
        info.type = type;
        info.filePath = filePath;

        if (!pFmodSystem) {
            std::cout << "system not init\n";
            return info;
        }

        FMOD_MODE mode = FMOD_LOOP_NORMAL;

        if (type == SoundType::SFX) {
            FMOD_RESULT result = FMOD_System_CreateSound(pFmodSystem, filePath.c_str(), mode, nullptr, &info.sound);
            if (result != FMOD_OK) {
                std::cout << FMOD_ErrorString(result) << "sound not loaded\n";
                return info;
            }
            FMOD_Sound_SetSoundGroup(info.sound, SFX_SG);
        }
        else if (type == SoundType::BGM) {
            mode = FMOD_LOOP_NORMAL | FMOD_CREATESTREAM;
            FMOD_RESULT result = FMOD_System_CreateSound(pFmodSystem, filePath.c_str(), mode, nullptr, &info.sound);
            if (result != FMOD_OK) {
                std::cout << FMOD_ErrorString(result) << "sound not loaded\n";
                return info;
            }
        }

        return info;
    }

    void Sound::unloadSound(FMOD_SOUND* sound)
    {
        //goes thru the map and looks for the sound file if it is found release it and removes it from the map
        if (pFmodSystem) {
            FMOD_Sound_Release(sound);
        }
    }

	void Sound::release() 
    {
		if (!pFmodSystem) return;
		stopAllSounds();
		if (SFX) {
			FMOD_ChannelGroup_Release(SFX);
			SFX = nullptr;
		}
		if (BGM) {
			FMOD_ChannelGroup_Release(BGM);
			BGM = nullptr;
		}
		FMOD_System_Close(pFmodSystem);
		FMOD_System_Release(pFmodSystem);
		pFmodSystem = nullptr;
	}

    void Sound::unloadAllSounds(std::unordered_map<std::string, SoundInfo>& mSoundList)
    {
        if (!pFmodSystem) return;
        stopAllSounds();
        //goes thru the map and releases each sound file then clears the map
        for (auto& it : mSoundList) {
            if (it.second.sound) {
                FMOD_RESULT result = FMOD_Sound_Release(it.second.sound);
                if (result != FMOD_OK) {
                    std::cerr << "FMOD_Sound_Release failed: "
                        << FMOD_ErrorString(result) << std::endl;
                }
                it.second.sound = nullptr;
            }
        }
        mSoundList.clear();
    }

    void Sound::playSound(SoundInfo& info, int loopCount, float volume, float pitch)
    {
        if (!pFmodSystem) { //check if fmod has been init
            return;
        }

        //create channel holder
        FMOD_CHANNEL* channel = nullptr;
        FMOD_RESULT result;

        if (loopCount >= 0) {
            FMOD_Sound_SetLoopCount(info.sound, loopCount);
        }
        //play in whichever channel group that it was set to
        if (info.type == SoundType::SFX) {
            result = FMOD_System_PlaySound(pFmodSystem, info.sound, SFX, false, &channel);
        }
        else if (info.type == SoundType::BGM) {
            result = FMOD_System_PlaySound(pFmodSystem, info.sound, BGM, false, &channel);
        }
        else {
            result = FMOD_System_PlaySound(pFmodSystem, info.sound, nullptr, false, &channel);
        }
        if (result != FMOD_OK) {
            return;
        }

        // Set volume and pitch
        FMOD_Channel_SetVolume(channel, volume);
        FMOD_Channel_SetPitch(channel, pitch);
        // Store channel for later control
        info.channel = channel;
        FMOD_Sound_SetLoopCount(info.sound, loopCount);

        //add the channel to its respective group channel
        if (info.type == SoundType::SFX) {
            FMOD_Channel_SetChannelGroup(channel, SFX);
        }
        else if (info.type == SoundType::BGM) {
            FMOD_Channel_SetChannelGroup(channel, BGM);
        }
        return;
    }

    void Sound::stopSound(SoundInfo& info)
    {
        FMOD_RESULT result = FMOD_Channel_Stop(info.channel);
        if (result != FMOD_OK) {
            return;
        }
    }

    void Sound::stopAllSounds()
    {
        FMOD_RESULT result = FMOD_ChannelGroup_Stop(Master);
        if (result != FMOD_OK) {
            return;
        }
    }

    void Sound::pauseSound(SoundInfo& info, bool pause)
    {
        FMOD_Channel_SetPaused(info.channel, pause);
    }

    void Sound::pauseAllSounds(bool pause)
    {
        FMOD_ChannelGroup_SetPaused(Master, pause);
    }

    void Sound::setSoundVolume(SoundInfo& info, float volume)
    {
        FMOD_Channel_SetVolume(info.channel, volume);
    }

    void Sound::setSoundPitch(SoundInfo& info, float pitch)
    {
        FMOD_Channel_SetPitch(info.channel, pitch);
    }

    void Sound::setChannelGroupVolume(float volume, SoundType type = SoundType::END) {
        if (type == SoundType::SFX) {
            FMOD_ChannelGroup_SetVolume(SFX, volume);
        }
        else if (type == SoundType::BGM) {
            FMOD_ChannelGroup_SetVolume(BGM, volume);
        }
        else {
            FMOD_ChannelGroup_SetVolume(Master, volume);
        }
    }
}
