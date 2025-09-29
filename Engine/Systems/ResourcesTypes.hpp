#pragma once

#include "Math/Math.h"

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
	};
}