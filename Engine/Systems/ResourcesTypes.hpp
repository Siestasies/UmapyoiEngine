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
	/*!
	\brief Holds an OpenGL shader program ID and its source file paths.
	*/
	struct Shader
	{
		unsigned int shaderProgramID;  ///< Compiled OpenGL shader program ID.
		std::string vertexPath;        ///< File path to the vertex shader source.
		std::string fragmentPath;      ///< File path to the fragment shader source.
	};

	/*!
	\brief Holds an OpenGL texture ID, dimensions, and source file path.
	*/
    struct Texture
    {
        unsigned int tex_id;           ///< OpenGL texture ID.
		std::string filePath;          ///< Source file path of the texture image.
        Vec2 tex_size;                 ///< Texture dimensions in pixels.
		float pixelsPerUnit = 100.f;   ///< Pixels per world unit for size conversion.

		/*!
		\brief Calculates the native world-space size of the texture.
		\return Texture size divided by pixelsPerUnit.
		*/
		Vec2 GetNativeSize() const
		{
			return tex_size / pixelsPerUnit;
		}
    };

	/*!
	\brief Categorizes a sound as SFX, BGM, or MASTER for channel group routing.
	*/
	enum class SoundType {
		SFX = 0,
		BGM,
		MASTER,
		END
	};

	/*!
	\brief Wraps FMOD sound and channel pointers with type classification and 3D attributes.
	*/
	struct SoundInfo {
		FMOD_SOUND* sound = nullptr;
		FMOD_CHANNEL* channel = nullptr;
		SoundType type = SoundType::END;
		std::string filePath;

		FMOD_VECTOR pos{};
		FMOD_VECTOR vel{};

		//to toggle to low pass from normal audio
		FMOD_DSP* dspLowpass = nullptr;
	};

	/*!
	\brief Stores FreeType glyph metrics for a single character.
	*/
	struct Character
	{
		unsigned int textureID;
		Vec2   size;    // Size of glyph
		Vec2   bearing; // Offset from baseline to left/top of glyph
		float  advance; // Offset to advance to next glyph
	};

	/*!
	\brief Holds a character map and OpenGL buffers for rendering a loaded font.
	*/
	struct FontData
	{
		std::map<char, Character> characters;
		unsigned int VAO = 0;
		unsigned int VBO = 0;
		unsigned int fontSize = 0;
		std::string filePath;
	};

	/*!
	\brief Enumerates supported shader uniform data types.
	*/
	enum class UniformType
	{
		Float,
		Vec2,
		Vec3,
		Vec4,
		Int
	};

	/*!
	\brief Describes a single shader uniform variable with name, type, and location.
	*/
	struct UniformInfo
	{
		std::string name{};
		UniformType type = UniformType::Float;
		int location = -1;
	};

	/*!
	\brief Represents a post-process or visual effect shader with auto-reflected uniforms.
	*/
	struct ShaderEffect
	{
		std::string name{};           // e.g. "dissolve"
		std::string fragPath{};       // e.g. "Assets/Shaders/Effects/dissolve.frag"
		unsigned int shaderProgramID = 0;
		std::vector<UniformInfo> uniforms{};  // auto-reflected, excludes image/projection
	};
}