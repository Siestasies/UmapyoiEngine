/*!
\file   Sound.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Koh Kai Yang (100%)
\par    E-mail: k.kaiyang@digipen.edu
\par    DigiPen login: k.kaiyang

\brief
This declares wrapper functions for the use of Fmod

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

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
				/*!
				*\brief loads the sound file
				*\param filePath, type
				*\return struct SoundInfo that contains the sound, channel and type
				*/ 
				SoundInfo loadSound(const std::string& filePath, SoundType type);

				/*!
				*\brief unload the sound 
				*\param sound
				*/
				void unloadSound(FMOD_SOUND* sound);

				/*!
				*\brief unload and releases the sound
				*\param mSoundList
				*/
				void unloadAllSounds(std::unordered_map<std::string, SoundInfo>& mSoundList);

				/*!
				*\brief release all the systems
				*/
				void releaseSounds();

				// Sound playback
				/*!
				*\brief plays the sound file
				*\param info - sound info
				*\param loopCount - number of loops to play the sound
				*\param volume - volume of the sound
				*\param pitch - pitch of the sound
				*/
				void playSound(SoundInfo& info,int loopCount = 0, float volume = 1.0f, float pitch = 1.0f);

				/*!
				*\brief stops the sound from playing
				*\param info - sound info
				*/
				void stopSound(SoundInfo& info);

				/*!
				*\brief stops all sound from playing
				*/
				void stopAllSounds();

				/*!
				*\brief pauses sound specified
				*\param info - sound info
				*\param pause - true to pause
				*/
				void pauseSound(SoundInfo& info, bool pause = true);

				/*!
				*\brief pauses all sound
				*\param pause - true to pause
				*/
				void pauseAllSounds(bool pause = true);

				// Volume and pitch control
				/*!
				*\brief set the volume of a sound 
				*\param info - the info for sound info
				*\param volume - volume to set the sound to
				*/
				void setSoundVolume(SoundInfo& info, float volume);

				/*!
				*\brief set the pitch of a sound
				*\param info - the info for sound info
				*\param volume - pitch to set the sound to
				*/
				void setSoundPitch(SoundInfo& info, float pitch);

				//toggle volume groups i.e. SFX,BGM,MASTER
				//default value is master 
				/*!
				*\brief set the volume of the sound groups 
				*\param volume - volume of the sound group
				*\param type - the sound group
				*/
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

