#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

#include "SystemType.h"

// Forward declarations for FMOD
struct FMOD_SYSTEM;
struct FMOD_SOUND;
struct FMOD_CHANNEL;

namespace Uma_Sound {

	class Sound : public Uma_Engine::ISystem {
	public:
		Sound();
		~Sound();

		// Initialize and cleanup
		void Init() override;
		void Shutdown() override;
		void Update(float dt) override; // Call this every frame

		// Sound loading and management
		bool loadSound(const std::string& name, const std::string& filePath, bool loop = false);
		void unloadSound(const std::string& name);
		void unloadAllSounds();

		// Sound playback
		bool playSound(const std::string& name, float volume = 1.0f, float pitch = 1.0f);
		bool playSound3D(const std::string& name, float x, float y, float z, float volume = 1.0f);
		void stopSound(const std::string& name);
		void stopAllSounds();
		void pauseSound(const std::string& name, bool pause = true);
		void pauseAllSounds(bool pause = true);

		// Volume and pitch control
		void setMasterVolume(float volume);
		void setSoundVolume(const std::string& name, float volume);
		void setSoundPitch(const std::string& name, float pitch);

		// Utility functions
		bool isSoundLoaded(const std::string& name) const;
		bool isSoundPlaying(const std::string& name) const;
		std::vector<std::string> getLoadedSounds() const;

	private:
		FMOD_SYSTEM* pFmodSystem;
		std::unordered_map<std::string, FMOD_SOUND*> aSoundListMap;
		std::unordered_map<std::string, FMOD_CHANNEL*> m_channels;

		FMOD_CHANNELGROUP* SFX = nullptr;
		FMOD_CHANNELGROUP* BGM = nullptr;

		// Helper functions
		//std::string getFullPath(const std::string& fileName) const;
		void checkFMODError(int result, const std::string& operation) const;
	};
}

