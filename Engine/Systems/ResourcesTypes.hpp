/*!
\file   ResourcesType.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines resource handle structures for graphics and audio assets used throughout the engine.

Texture struct contains OpenGL texture ID, dimensions, and source file path for resource tracking.
SoundInfo struct wraps FMOD pointers (sound, channel) with type classification (SFX/BGM) and file path.
Forward declares FMOD types to avoid header dependency propagation. SoundType enum distinguishes
sound effects from background music for separate audio channel management and volume control.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "Math/Math.h"

#include <string>

// forward declare
struct FMOD_SYSTEM;
struct FMOD_SOUND;
struct FMOD_CHANNEL;

namespace Uma_Engine
{
    struct Texture
    {
        unsigned int tex_id;
        Vec2 tex_size;
				std::string filePath;
    };

	enum class SoundType {
		SFX = 0,
		BGM,
		END
	};

	struct SoundInfo {
		FMOD_SOUND* sound = nullptr;
		FMOD_CHANNEL* channel = nullptr;
		SoundType type = SoundType::END;
		std::string filePath;
	};
}