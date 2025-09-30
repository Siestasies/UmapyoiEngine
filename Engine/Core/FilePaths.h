#pragma once
#include <string>

namespace Uma_FilePath 
{

    inline const std::string ASSET_ROOT =

#if defined(_DEBUG)
        "../../../../Assets/"; // Dev mode — raw asset folder
#else
        "Assets/"; // Build mode — copied to build folder
#endif

    // Assets
    inline const std::string TEXTURES_DIR = ASSET_ROOT + "Textures/";
    inline const std::string AUDIO_DIR = ASSET_ROOT + "Audios/";

    // Scenes
    inline const std::string SCENES_DIR = ASSET_ROOT + "Scenes/";
    inline const std::string PREFAB_DIR = ASSET_ROOT + "Prefabs/";

    // Engine Settings

}