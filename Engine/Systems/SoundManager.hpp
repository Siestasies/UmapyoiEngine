/*!
\file   SoundManager.hpp
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

#include "Core/EventSystem.h"
#include "../Systems/ResourcesManager.hpp"

using SoundInfo = Uma_Engine::SoundInfo;
using SoundType = Uma_Engine::SoundType;

namespace Uma_Engine
{

		struct FadeChannel {
			FMOD_CHANNEL* channel;
			float targetVolume;
			float fadeDuration;
			float fadeStartTime;
			float startVolume;
			bool fadeOut;  // true = fade to 0, false = fade to targetVolume
		};

		class SoundManager : public ISystem 
		{
				public:
				SoundManager();
				~SoundManager();

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
				SoundInfo loadSound(const std::string& filePath, SoundType type, bool is3D = true);

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
				void release();

				// Sound playback
				/*!
				*\brief plays the sound file
				*\param info - sound info
				*\param loopCount - number of loops to play the sound
				*\param volume - volume of the sound
				*\param pitch - pitch of the sound
				*/
				void playSound(SoundInfo* info, int loopCount = 0, float volume = 1.0f, float pitch = 1.0f);

				/*!
				*\brief stops the sound from playing
				*\param info - sound info
				*/
				void stopSound(SoundInfo* info);

				/*!
				*\brief stops all sound from playing
				*/
				void stopAllSounds();

				/*!
				*\brief pauses sound specified
				*\param info - sound info
				*\param pause - true to pause
				*/
				void pauseSound(SoundInfo* info, bool pause = true);

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
				void setSoundVolume(SoundInfo* info, float volume);

				/*!
				*\brief set the pitch of a sound
				*\param info - the info for sound info
				*\param volume - pitch to set the sound to
				*/
				void setSoundPitch(SoundInfo* info, float pitch);

				//toggle volume groups i.e. SFX,BGM,MASTER
				//default value is master 
				/*!
				*\brief set the volume of the sound groups 
				*\param volume - volume of the sound group
				*\param type - the sound group
				*/
				void setChannelGroupVolume(float volume, SoundType type);

				//set the listener position for the update loop to update
				/*!
				*\brief sets the 3d positon,vel forward and up of the 3d listener
				*\param pos - position of the listener
				*\param vel - velocity of the listener
				*\param forward - direction of the listener
				*\param up - up vector of the listener
				*/
				void setListenerPosition(const FMOD_VECTOR& pos, const FMOD_VECTOR& vel, const FMOD_VECTOR& forward, const FMOD_VECTOR& up);

				void PlayOneShotAt(SoundInfo* info, const FMOD_VECTOR& pos, float volume = 1.0f, bool is3D = true);

				FMOD_CHANNEL* PlaySoundInstance(SoundInfo* info, bool loop, float volume, const FMOD_VECTOR& pos, bool is3D = true);

				void StopChannel(FMOD_CHANNEL* channel);

				void UpdateChannel3DPosition(FMOD_CHANNEL* channel, const FMOD_VECTOR& pos, const FMOD_VECTOR& vel);

				bool IsSoundPlaying(SoundInfo* info);

				void StartFade(FMOD_CHANNEL* channel, float targetVolume, float duration, bool fadeOut = false);
				void UpdateFades(float dt);

				FMOD_CHANNEL* PlaySoundInstanceFaded(SoundInfo* info, bool loop, float targetVolume, const FMOD_VECTOR& pos, bool is3D, float fadeInTime = 1.0f);
				void FadeOutChannel(FMOD_CHANNEL* channel, float fadeOutTime = 1.0f);

		private:
				FMOD_SYSTEM* pFmodSystem = nullptr;
				//std::unordered_map<std::string, SoundInfo> aSoundListMap;

				FMOD_CHANNELGROUP* SFX = nullptr;
				FMOD_CHANNELGROUP* BGM = nullptr;
				FMOD_CHANNELGROUP* Master = nullptr;

				FMOD_SOUNDGROUP* SFX_SG = nullptr;

				EventSystem* pEventSystem = nullptr;
				ResourcesManager* pResourcesManager = nullptr;

				// Helper functions
				//std::string getFullPath(const std::string& fileName) const;
				//void checkFMODError(int result, const std::string& operation) const;

				//for 3d sound
				FMOD_VECTOR listenerPos = { 0.0f, 0.0f, 0.0f };
				FMOD_VECTOR listenerVel = { 0.0f, 0.0f, 0.0f };
				FMOD_VECTOR listenerForward = { 0.0f, 0.0f, 1.0f };
				FMOD_VECTOR listenerUp = { 0.0f, 1.0f, 0.0f };

				std::vector<FadeChannel> fadingChannels;
				float currentTime = 0.0f;
		};
}

