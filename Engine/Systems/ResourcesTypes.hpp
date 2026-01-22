/*!
\file   ResourcesType.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (Everything else)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\co-author Javier Chua Dong Qing (Character, FontData)
\par       E-mail: javierdongqing.chua@digipen.edu
\par       DigiPen login: javierdongqing.chua

\brief
Defines resource handle structures for textures, audio and font assets used throughout the engine.

Texture struct contains OpenGL texture ID, dimensions, and source file path for resource tracking.
SoundInfo struct wraps FMOD pointers (sound, channel) with type classification (SFX/BGM) and file path.
Forward declares FMOD types to avoid header dependency propagation. SoundType enum distinguishes
sound effects from background music for separate audio channel management and volume control.
Character and FontData structs store FreeType glyph metrics and atlas information for text rendering.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "Math/Math.h"
#include "fmod/inc/fmod.h"

#include <string>
#include <map>

// forward declare
struct FMOD_SYSTEM;
struct FMOD_SOUND;
struct FMOD_CHANNEL;
struct FMOD_VECTOR;

namespace Uma_Engine
{
    struct Texture
    {
        unsigned int tex_id;
		std::string filePath;
        Vec2 tex_size;
		float pixelsPerUnit = 100.f; // by default to 100

		Vec2 GetNativeSize() const
		{
			return tex_size / pixelsPerUnit;
		}
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

		FMOD_VECTOR pos{};
		FMOD_VECTOR vel{};
	};

	struct Character
	{
		unsigned int textureID;
		Vec2   size;    // Size of glyph
		Vec2   bearing; // Offset from baseline to left/top of glyph
		float  advance; // Offset to advance to next glyph
	};

	struct FontData
	{
		std::map<char, Character> characters;
		unsigned int VAO = 0;
		unsigned int VBO = 0;
		unsigned int fontSize = 0;
		std::string filePath;
	};
}