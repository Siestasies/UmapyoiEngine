#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

#include <fmod.h>
#include <fmod_errors.h>

#include "SystemType.h"

// Forward declarations for FMOD
struct FMOD_SYSTEM;
struct FMOD_SOUND;
struct FMOD_CHANNEL;

namespace Uma_Sound {

	enum class SoundType {
		SFX = 0,
		BGM,
		END
	};

	struct SoundInfo {
		FMOD_SOUND* sound;
		FMOD_CHANNEL* channel;
		SoundType type;
	};

	class Sound : public Uma_Engine::ISystem {
	public:
		Sound();
		~Sound();

		// Initialize and cleanup
		void Init() override;
		void Shutdown() override;
		void Update(float dt) override; // Call this every frame

		// Sound loading and management
		SoundInfo loadSound(const std::string& name, const std::string& filePath, SoundType type);
		void unloadSound(const std::string& name);
		void unloadAllSounds();

		// Sound playback
		FMOD_CHANNEL* playSound(SoundInfo& info, float volume = 1.0f, float pitch = 1.0f);
		void stopSound(SoundInfo& info);
		void stopAllSounds();
		void pauseSound(SoundInfo& info, bool pause = true);
		void pauseAllSounds(bool pause = true);

		// Volume and pitch control
		void setMasterVolume(float volume);
		void setSoundVolume(SoundInfo& info, float volume);
		void setSoundPitch(SoundInfo& info, float pitch);

	private:
		FMOD_SYSTEM* pFmodSystem = nullptr;
		std::unordered_map<std::string, SoundInfo> aSoundListMap;

		FMOD_CHANNELGROUP* SFX = nullptr;
		FMOD_CHANNELGROUP* BGM = nullptr;
		FMOD_CHANNELGROUP* Master = nullptr;

		// Helper functions
		//std::string getFullPath(const std::string& fileName) const;
		//void checkFMODError(int result, const std::string& operation) const;
	};
}

