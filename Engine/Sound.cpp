#include "Sound.hpp"
#include <iostream>
#include <filesystem>

// Include FMOD headers
#include "fmod.h"
#include "fmod_errors.h"

namespace Uma_Sound {

	Sound::Sound() : pFmodSystem(nullptr) {

	}

	Sound::~Sound()
	{
		Shutdown();
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

		FMOD_System_CreateChannelGroup(pFmodSystem, "SFX", SFX);
		FMOD_System_CreateChannelGroup(pFmodSystem, "BGM", BGM);

		return;
	}

	void Sound::Shutdown()
	{
		if (pFmodSystem) {
			//stop all the sound
			stopAllSounds();
			//release all the sound
			unloadAllSounds();
			
			//release the system
			FMOD_System_Close(pFmodSystem);
			FMOD_System_Release(pFmodSystem);
			pFmodSystem = nullptr;
		}
	}

	void Sound::Update(float dt)
	{
		if (pFmodSystem) {
			FMOD_System_Update(pFmodSystem);
		}
	}

	bool Sound::loadSound(const std::string& name, const std::string& filePath, bool loop)
	{
		if (pFmodSystem) {
			return false;
		}

		if (aSoundListMap.find(name) != aSoundListMap.end()) {
			std::cout << name << " is loaded\n";
			return true;
		}


		FMOD_SOUND* sound = nullptr;
		FMOD_MODE mode = FMOD_LOOP_NORMAL;

		FMOD_RESULT result = FMOD_System_CreateSound(pFmodSystem, filePath.c_str(), mode, nullptr, &sound);
		if (result != FMOD_OK) {
			return false;
		}

		aSoundListMap[name] = sound;
		return true;
	}

	void Sound::unloadSound(const std::string& name)
	{
		if (pFmodSystem) {
			FMOD_SOUND* temp = aSoundListMap.find(name)->second;
			if (temp) {
				FMOD_Sound_Release(temp);
				aSoundListMap.erase(name);
			}
		}
	}

	void Sound::unloadAllSounds()
	{
		for (auto it : aSoundListMap) {
			if (it.second) {
				FMOD_Sound_Release(it.second);
			}
		}
		aSoundListMap.clear();
	}

	bool Sound::playSound(const std::string& name, float volume, float pitch)
	{
		if (!pFmodSystem) { //check if fmod has been init
			return false;
		}
		auto it = aSoundListMap.find(name);
		if (it == aSoundListMap.end()) { //check if the sound is loaded
			return false;
		}

		FMOD_CHANNEL* channel = nullptr;
		FMOD_RESULT result = FMOD_System_PlaySound(pFmodSystem, it->second, nullptr, false, &channel);
		if (result != FMOD_OK) {
			return false;
		}
		// Set volume and pitch
		FMOD_Channel_SetVolume(channel, volume);
		FMOD_Channel_SetPitch(channel, pitch);
		// Store channel for later control
		m_channels[name] = channel;
		return true;
	}



}
