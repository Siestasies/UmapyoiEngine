#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

#include <../fmod/inc/fmod.h>
#include <../fmod/inc/fmod_errors.h>

#include "SystemType.h"
#include "ResourcesTypes.hpp"

using SoundInfo = Uma_Engine::SoundInfo;
using SoundType = Uma_Engine::SoundType;

struct FMOD_SYSTEM;
struct FMOD_SOUND;
struct FMOD_CHANNEL;

namespace Uma_Engine
{

		class Sound : public ISystem 
		{
				public:
				Sound();
				~Sound();

				// Initialize and cleanup
				void Init() override;
				void Shutdown() override;
				void Update(float dt) override; // Call this every frame

				// Sound loading and management
				SoundInfo loadSound(const std::string& filePath, SoundType type);
				void unloadSound(FMOD_SOUND* sound);
				void unloadAllSounds(std::unordered_map<std::string, SoundInfo>& mSoundList);

				// Sound playback
				void playSound(SoundInfo& info,int loopCount = 0, float volume = 1.0f, float pitch = 1.0f);
				void stopSound(SoundInfo& info);
				void stopAllSounds();
				void pauseSound(SoundInfo& info, bool pause = true);
				void pauseAllSounds(bool pause = true);

				// Volume and pitch control
				void setSoundVolume(SoundInfo& info, float volume);
				void setSoundPitch(SoundInfo& info, float pitch);
				//toggle volume groups i.e. SFX,BGM,MASTER
				//default value is master 
				void setChannelGroupVolume(float volume, SoundType type);

		private:
				FMOD_SYSTEM* pFmodSystem = nullptr;
				//std::unordered_map<std::string, SoundInfo> aSoundListMap;

				FMOD_CHANNELGROUP* SFX = nullptr;
				FMOD_CHANNELGROUP* BGM = nullptr;
				FMOD_CHANNELGROUP* Master = nullptr;

				FMOD_SOUNDGROUP* SFX_SG = nullptr;

				// Helper functions
				//std::string getFullPath(const std::string& fileName) const;
				//void checkFMODError(int result, const std::string& operation) const;
		};
}

