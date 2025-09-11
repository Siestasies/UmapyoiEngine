#include "Sound.hpp"
#include <fmod_studio.hpp> // For FMOD Studio API
#include <fmod.hpp>        // For FMOD Core API
#include <stdio.h>
#include <stdlib.h>
#include <fmod_errors.h>

namespace UmapyoiEngine {
	void Sound::SoundInit()
	{
		FMOD_RESULT result;
		FMOD::System* system = NULL;

		result = FMOD::System_Create(&system);      // Create the main system object.
		if (result != FMOD_OK)
		{
			printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
			exit(-1);
		}

		result = system->init(512, FMOD_INIT_NORMAL, 0);    // Initialize FMOD.
		if (result != FMOD_OK)
		{
			printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
			exit(-1);
		}
	}
}


