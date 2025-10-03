/*!
\file   FilePaths.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Implements centralized asset loading, caching, and lifecycle management for textures and audio resources.

Delegates low-level loading to Graphics and Sound systems while maintaining name-to-resource maps for efficient lookup.
Prevents duplicate loading through existence checks before allocation.
Serializes resource metadata (name, file path, type) to JSON for scene persistence, excluding runtime GPU/audio handles.
Deserializes by reloading assets from stored file paths. Provides debug utilities for listing loaded resources
and batch unload operations during shutdown. Initializes by retrieving Graphics and Sound system pointers from SystemManager.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once
#include <string>

namespace Uma_FilePath 
{

    inline const std::string ASSET_ROOT = "Assets/";
    inline const std::string LOGS_ROOT = "Logs/";
    inline const std::string CONFIG_ROOT = "Configs/";

    // Assets
    inline const std::string TEXTURES_DIR = ASSET_ROOT + "Textures/";
    inline const std::string AUDIO_DIR = ASSET_ROOT + "Audios/";

    // Scenes
    inline const std::string SCENES_DIR = ASSET_ROOT + "Scenes/";
    inline const std::string PREFAB_DIR = ASSET_ROOT + "Prefabs/";

    // Engine Settings

}