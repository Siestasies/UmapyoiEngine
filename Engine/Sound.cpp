#include "Sound.hpp"
#include <iostream>
#include <filesystem>

// Include FMOD headers
#include <fmod.h>
#include <fmod_errors.h>

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
		result = FMOD_System_CreateChannelGroup(pFmodSystem, "MASTER", &Master);
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

	SoundInfo Sound::loadSound(const std::string& name, const std::string& filePath, SoundType type)
	{
		if (!pFmodSystem) {
			return;
		}

		if (aSoundListMap.find(name) != aSoundListMap.end()) {
			std::cout << name << " is loaded\n";
			return;
		}


		FMOD_SOUND* sound = nullptr;
		FMOD_MODE mode = FMOD_LOOP_NORMAL;

		FMOD_RESULT result = FMOD_System_CreateSound(pFmodSystem, filePath.c_str(), mode, nullptr, &sound);
		if (result != FMOD_OK) {
			return;
		}

		//adds the sound and type to the name
		/*aSoundListMap[name].sound = sound;
		aSoundListMap[name].type = type;*/

		SoundInfo info;
		info.channel = nullptr;
		info.sound = sound;
		info.type = type;
		return info;
	}

	void Sound::unloadSound(const std::string& name)
	{
		//goes thru the map and looks for the sound file if it is found release it and removes it from the map
		if (pFmodSystem) {
			FMOD_SOUND* temp = aSoundListMap.find(name)->second.sound;
			if (temp) {
				FMOD_Sound_Release(temp);
				aSoundListMap.erase(name);
			}
		}
	}

	void Sound::unloadAllSounds()
	{
		//goes thru the map and releases each sound file then clears the map
		for (auto it : aSoundListMap) {
			if (it.second.sound) {
				FMOD_Sound_Release(it.second.sound);
			}
		}
		aSoundListMap.clear();
	}

	FMOD_CHANNEL* Sound::playSound(SoundInfo& info, float volume, float pitch)
	{
		if (!pFmodSystem) { //check if fmod has been init
			return;
		}

		//create channel holder
		FMOD_CHANNEL* channel = nullptr;
		FMOD_RESULT result;
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
			return false;
		}

		// Set volume and pitch
		FMOD_Channel_SetVolume(channel, volume);
		FMOD_Channel_SetPitch(channel, pitch);
		// Store channel for later control
		info.channel = channel;

		//add the channel to its respective group channel
		if (info.type == SoundType::SFX) {
			FMOD_Channel_SetChannelGroup(channel, SFX);
		}
		else if (info.type == SoundType::BGM) {
			FMOD_Channel_SetChannelGroup(channel, BGM);
		}
		return channel;
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

	void Sound::setMasterVolume(float volume)
	{
		FMOD_ChannelGroup_SetVolume(Master, volume);
	}

	void Sound::setSoundVolume(SoundInfo& info, float volume)
	{
		FMOD_Channel_SetVolume(info.channel, volume);
	}

	void Sound::setSoundPitch(SoundInfo& info, float pitch)
	{
		FMOD_Channel_SetPitch(info.channel, pitch);
	}

}
