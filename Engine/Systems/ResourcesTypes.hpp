#pragma once

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

	struct FMOD_SYSTEM;
	struct FMOD_SOUND;
	struct FMOD_CHANNEL;

	struct SoundInfo {
		FMOD_SOUND* sound;
		FMOD_CHANNEL* channel;
		SoundType type;
	};
}