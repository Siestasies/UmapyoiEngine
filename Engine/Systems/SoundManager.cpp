/*!
\file   SoundManager.cpp
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

#include "SoundManager.hpp"
#include <iostream>
#include <filesystem>

#include "Events/AudioEvents.h"
#include "Events/IMGUIEvents.h"

#include "Debugging/Debugger.hpp"

#define DEBUG

namespace Uma_Engine {

    SoundManager::SoundManager() : pFmodSystem(nullptr) 
    {

    }

    SoundManager::~SoundManager()
    {
        //Shutdown();
    }

    void SoundManager::Init()
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

        FMOD_SoundGroup_SetMaxAudible(SFX_SG, 120);
        FMOD_SoundGroup_SetMaxAudibleBehavior(SFX_SG, FMOD_SOUNDGROUP_BEHAVIOR_MUTE);

        // Set 3D settings (doppler scale, distance factor, rolloff scale)
        FMOD_System_Set3DSettings(pFmodSystem, 1.0f, 1.0f, 1.0f);
        pEventSystem = pSystemManager->GetSystem<EventSystem>();
        pResourcesManager = pSystemManager->GetSystem<ResourcesManager>();

        if (pEventSystem && pResourcesManager)
        {
            pEventSystem->Subscribe<Uma_Engine::PlaySoundEvent, SoundManager>(
                [this](const PlaySoundEvent& e)
                {
                    playSound(pResourcesManager->GetSound(e.soundName), e.loop, e.volume, 1.f);
                });

            pEventSystem->Subscribe<Uma_Engine::StopSoundEvent, SoundManager>(
                [this](const StopSoundEvent& e)
                {
                    stopSound(pResourcesManager->GetSound(e.soundName));
                });

            pEventSystem->Subscribe<Uma_Engine::PlayMusicEvent, SoundManager>(
                [this](const PlayMusicEvent& e)
                {
                    playSound(pResourcesManager->GetSound(e.musicName), e.loop, e.volume, 1.f);
                });

            pEventSystem->Subscribe<Uma_Engine::StopMusicEvent, SoundManager>(
                [this](const StopMusicEvent& e)
                {
                    stopSound(pResourcesManager->GetSound(e.musicName));
                });

            // subscribe to play maode changes events (TEMP SOLUTION)
            pEventSystem->Subscribe<Uma_Engine::PlaySceneRequest, SoundManager>(
                [this](const Uma_Engine::PlaySceneRequest& e)
                {
                    (void)e;
                    pauseAllSounds(false);
                });

            pEventSystem->Subscribe<Uma_Engine::PauseSceneRequest, SoundManager>(
                [this](const Uma_Engine::PauseSceneRequest& e)
                {
                    (void)e;
                    pauseAllSounds(true);
                });

            pEventSystem->Subscribe<Uma_Engine::StopSceneRequest, SoundManager>(
                [this](const Uma_Engine::StopSceneRequest& e)
                {
                    (void)e;
                    stopAllSounds();
                });
        }
        else
        {
            Debugger::Log(WarningLevel::eWarning, "Audio Manager did not subscribe to the audio events");
        }

        return;
    }

    void SoundManager::Shutdown()
    {
#ifdef DEBUG
        std::cout << "sound shutdown\n";
#endif // DEBUG
        pEventSystem->UnsubscribeSystem<SoundManager>();
    }

    void SoundManager::Update(float dt)
    {
        (void)dt;

        if (pFmodSystem) {
            FMOD_System_Set3DListenerAttributes(
                pFmodSystem, 0,
                &listenerPos, &listenerVel,
                &listenerForward, &listenerUp
            );

            FMOD_System_Update(pFmodSystem);
        }
    }

    SoundInfo SoundManager::loadSound(const std::string& filePath, SoundType type, bool is3D)
    {
        SoundInfo info;
        info.type = type;
        info.filePath = filePath;

        if (!pFmodSystem) {
            std::cout << "system not init\n";
            return info;
        }

        FMOD_MODE mode = FMOD_LOOP_NORMAL;
        if (is3D)
            mode |= FMOD_3D;
        else
            mode |= FMOD_2D;

        if (type == SoundType::SFX) {
            FMOD_RESULT result = FMOD_System_CreateSound(pFmodSystem, filePath.c_str(), mode, nullptr, &info.sound);
            if (result != FMOD_OK) {
                std::cout << FMOD_ErrorString(result) << "sound not loaded\n";
                return info;
            }
            FMOD_Sound_SetSoundGroup(info.sound, SFX_SG);
        }
        else if (type == SoundType::BGM) {
            mode |= FMOD_CREATESTREAM;
            FMOD_RESULT result = FMOD_System_CreateSound(pFmodSystem, filePath.c_str(), mode, nullptr, &info.sound);
            if (result != FMOD_OK) {
                std::cout << FMOD_ErrorString(result) << "sound not loaded\n";
                return info;
            }
        }

        return info;
    }

    void SoundManager::unloadSound(FMOD_SOUND* sound)
    {
        if (!sound)
        {
            Debugger::Log(WarningLevel::eWarning, "unload sound : sound doesnt exsists");
            return;
        }

        //goes thru the map and looks for the sound file if it is found release it and removes it from the map
        if (pFmodSystem) {
            FMOD_Sound_Release(sound);
        }
    }

    void SoundManager::release() 
    {
        if (!pFmodSystem) return;
        stopAllSounds();
        if (SFX) 
        {
            FMOD_ChannelGroup_Release(SFX);
            SFX = nullptr;
        }
        if (BGM) 
        {
            FMOD_ChannelGroup_Release(BGM);
            BGM = nullptr;
        }
        FMOD_System_Close(pFmodSystem);
        FMOD_System_Release(pFmodSystem);
        pFmodSystem = nullptr;
    }

    void SoundManager::unloadAllSounds(std::unordered_map<std::string, SoundInfo>& mSoundList)
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

    void SoundManager::playSound(SoundInfo* info, int loopCount, float volume, float pitch)
    {
        if (!pFmodSystem || !info) { //check if fmod has been init
            Debugger::Log(WarningLevel::eWarning, "play sound : sound doesnt exsists");
            return;
        }

        //create channel holder
        FMOD_RESULT result;

        if (loopCount >= 0) {
            FMOD_Sound_SetLoopCount(info->sound, loopCount);
        }
        //play in whichever channel group that it was set to
        if (info->type == SoundType::SFX) {
            result = FMOD_System_PlaySound(pFmodSystem, info->sound, SFX, false, &info->channel);
        }
        else if (info->type == SoundType::BGM) {
            result = FMOD_System_PlaySound(pFmodSystem, info->sound, BGM, false, &info->channel);
        }
        else {
            result = FMOD_System_PlaySound(pFmodSystem, info->sound, nullptr, false, &info->channel);
        }
        if (result != FMOD_OK) {
            return;
        }

        // Set volume and pitch
        FMOD_Channel_SetVolume(info->channel, volume);
        FMOD_Channel_SetPitch(info->channel, pitch);

        if (info->type == SoundType::BGM) {
            FMOD_Channel_SetMode(info->channel, FMOD_2D);
        }

        //add the channel to its respective group channel
        if (info->type == SoundType::SFX) {
            FMOD_Channel_SetChannelGroup(info->channel, SFX);
        }
        else if (info->type == SoundType::BGM) {
            FMOD_Channel_SetChannelGroup(info->channel, BGM);
        }
        return;
    }

    void SoundManager::stopSound(SoundInfo* info)
    {
        if (!info)
        {
            Debugger::Log(WarningLevel::eWarning, "stop sound : sound doesnt exsists");
            return;
        }

        FMOD_RESULT result = FMOD_Channel_Stop(info->channel);
        if (result != FMOD_OK) {
            return;
        }
    }

    void SoundManager::stopAllSounds()
    {
        FMOD_RESULT result = FMOD_ChannelGroup_Stop(Master);
        if (result != FMOD_OK) {
            return;
        }
    }

    void SoundManager::pauseSound(SoundInfo* info, bool pause)
    {
        FMOD_Channel_SetPaused(info->channel, pause);
    }

    void SoundManager::pauseAllSounds(bool pause)
    {
        FMOD_ChannelGroup_SetPaused(Master, pause);
    }

    void SoundManager::setSoundVolume(SoundInfo* info, float volume)
    {
        if (!info)
        {
            Debugger::Log(WarningLevel::eWarning, "set vol : sound doesnt exsists");
            return;
        }

        FMOD_Channel_SetVolume(info->channel, volume);
    }

    void SoundManager::setSoundPitch(SoundInfo* info, float pitch)
    {
        if (!info)
        {
            Debugger::Log(WarningLevel::eWarning, "set pitch : sound doesnt exsists");
            return;
        }

        FMOD_Channel_SetPitch(info->channel, pitch);
    }

    void SoundManager::setChannelGroupVolume(float volume, SoundType type = SoundType::END) {
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

    void SoundManager::setListenerPosition(const FMOD_VECTOR& pos, const FMOD_VECTOR& vel, const FMOD_VECTOR& forward, const FMOD_VECTOR& up) {
        listenerPos = pos;
        listenerVel = vel;
        listenerForward = forward;
        listenerUp = up;
    }

    void SoundManager::PlayOneShotAt(const std::string& soundName, const FMOD_VECTOR& pos, float volume, bool is3D)
    {
        if (!pFmodSystem || !pResourcesManager) {
            return;
        }

        // Get sound from resource manager
        SoundInfo* info = pResourcesManager->GetSound(soundName);
        if (!info || !info->sound) {
            return;
        }

        // Create temporary channel (no tracking needed - fire and forget)
        FMOD_CHANNEL* tempChannel = nullptr;
        FMOD_CHANNELGROUP* group = SFX;

        FMOD_Sound_SetLoopCount(info->sound, 0);

        // Play sound
        FMOD_RESULT result = FMOD_System_PlaySound(pFmodSystem, info->sound, group, false, &tempChannel);
        if (result != FMOD_OK || !tempChannel) {
            return;
        }

        // Set volume
        FMOD_Channel_SetVolume(tempChannel, volume);

        // Set 3D position if applicable
        if (is3D) {
            FMOD_VECTOR fmodPos = { pos.x, pos.y, pos.z };
            FMOD_VECTOR fmodVel = { 0.0f, 0.0f, 0.0f };
            FMOD_Channel_Set3DAttributes(tempChannel, &fmodPos, &fmodVel);
            FMOD_Channel_Set3DMinMaxDistance(tempChannel, 100.0f, 1000.0f);
        }
    }

    FMOD_CHANNEL* SoundManager::PlaySoundInstance(const std::string& soundName, bool loop,float volume, const FMOD_VECTOR& pos, bool is3D)
    {
        if (!pFmodSystem || !pResourcesManager) {
            return nullptr;
        }

        // Get sound from ResourcesManager
        SoundInfo* info = pResourcesManager->GetSound(soundName);
        if (!info || !info->sound) {
            std::cerr << "[SoundManager] Sound not found: " << soundName << std::endl;
            return nullptr;
        }

        FMOD_CHANNEL* channel = nullptr;
        FMOD_CHANNELGROUP* group = (info->type == SoundType::SFX) ? SFX : BGM;

        // Set loop mode
        FMOD_Sound_SetLoopCount(info->sound, loop ? -1 : 0);

        // Play sound
        FMOD_RESULT result = FMOD_System_PlaySound(pFmodSystem, info->sound, group, false, &channel);
        if (result != FMOD_OK || !channel) {
            std::cerr << "[SoundManager] Failed to play: " << FMOD_ErrorString(result) << std::endl;
            return nullptr;
        }

        // Set volume
        FMOD_Channel_SetVolume(channel, volume);

        // Set 3D attributes if requested
        if (is3D) {
            FMOD_Channel_SetMode(channel, FMOD_3D);
            FMOD_Channel_Set3DAttributes(channel, &pos, nullptr);
            FMOD_Channel_Set3DMinMaxDistance(channel, 100.0f, 1000.0f);
        }

        return channel;
    }

    void SoundManager::StopChannel(FMOD_CHANNEL* channel)
    {
        if (channel) {
            FMOD_Channel_Stop(channel);
        }
    }

    void SoundManager::UpdateChannel3DPosition(FMOD_CHANNEL* channel, const FMOD_VECTOR& pos, const FMOD_VECTOR& vel)
    {
        if (!channel) return;

        FMOD_BOOL isPlaying = false;
        FMOD_RESULT result = FMOD_Channel_IsPlaying(channel, &isPlaying);

        if (result != FMOD_OK || !isPlaying) {
            return;  // Channel is invalid or stopped
        }

        // Safe to update
        FMOD_Channel_Set3DAttributes(channel, &pos, &vel);
    }

    SoundInfo* SoundManager::GetSoundInfo(const std::string& soundName)
    {
        if (!pResourcesManager) {
            return nullptr;
        }
        return pResourcesManager->GetSound(soundName);
    }
}
