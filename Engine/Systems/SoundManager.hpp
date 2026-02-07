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

				/*!
				 * \brief Plays a one-shot sound at a 3D position.
				 * \param info Sound resource and metadata to play.
				 * \param pos World-space position of the sound.
				 * \param volume Initial playback volume (0.0–1.0).
				 * \param is3D Whether to play the sound in 3D (true) or as 2D (false).
				 */
				void PlayOneShotAt(SoundInfo* info, const FMOD_VECTOR& pos, float volume = 1.0f, bool is3D = true);

				/*!
				 * \brief Starts a sound instance and returns its channel.
				 * \param info Sound resource and metadata to play.
				 * \param loop Whether the sound should loop.
				 * \param volume Initial playback volume (0.0–1.0).
				 * \param pos World-space position of the sound.
				 * \param is3D Whether to play the sound in 3D (true) or as 2D (false).
				 * \return The FMOD channel used to play this sound.
				 */
				FMOD_CHANNEL* PlaySoundInstance(SoundInfo* info, bool loop, float volume, const FMOD_VECTOR& pos, bool is3D = true);

				/*!
				 * \brief Stops playback on the given channel.
				 * \param channel Channel to stop.
				 */
				void StopChannel(FMOD_CHANNEL* channel);

				/*!
				 * \brief Updates the 3D position and velocity of a playing channel.
				 * \param channel Channel whose 3D attributes will be updated.
				 * \param pos New world-space position of the channel.
				 * \param vel New velocity of the channel in world space.
				 */
				void UpdateChannel3DPosition(FMOD_CHANNEL* channel, const FMOD_VECTOR& pos, const FMOD_VECTOR& vel);

				/*!
				 * \brief Checks if a sound is currently playing on any channel.
				 * \param info Sound resource to check.
				 * \return True if the sound is playing, false otherwise.
				 */
				bool IsSoundPlaying(SoundInfo* info);

				/*!
				 * \brief Starts a volume fade operation on a channel.
				 * \param channel Channel to fade.
				 * \param targetVolume Volume to reach at the end of the fade (0.0–1.0).
				 * \param duration Duration of the fade in seconds.
				 * \param fadeOut If true, fade from current volume down; if false, fade up to targetVolume.
				 */
				void StartFade(FMOD_CHANNEL* channel, float targetVolume, float duration, bool fadeOut = false);

				/*!
				 * \brief Updates all active volume fades.
				 * \param dt Time step since the last update in seconds.
				 */
				void UpdateFades(float dt);

				/*!
				 * \brief Plays a sound instance and fades it in to a target volume.
				 * \param info Sound resource and metadata to play.
				 * \param loop Whether the sound should loop.
				 * \param targetVolume Volume to reach at the end of the fade-in (0.0–1.0).
				 * \param pos World-space position of the sound.
				 * \param is3D Whether to play the sound in 3D (true) or as 2D (false).
				 * \param fadeInTime Duration of the fade-in in seconds.
				 * \return The FMOD channel used to play this sound.
				 */
				FMOD_CHANNEL* PlaySoundInstanceFaded(SoundInfo* info, bool loop, float targetVolume, const FMOD_VECTOR& pos, bool is3D, float fadeInTime = 1.0f);

				/*!
				 * \brief Starts a fade out on the given channel and optionally stops it when finished.
				 * \param channel Channel to fade out.
				 * \param fadeOutTime Duration of the fade-out in seconds.
				 */
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

