/*!
\file   ResourcesManager.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines resource management system that provides name-based caching and lookup for textures and sounds.

Implements both ISystem for engine lifecycle integration and ISerializer for scene persistence.
Maintains unordered maps for O(1) resource lookups by string identifiers with separate storage for textures and audio.
Provides load/unload/query operations for both resource types with file path and type parameters.
Returns raw pointers for textures and references for sounds to avoid ownership transfer.
Serializes resource manifests (not binary data) for scene reloading on deserialization.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once
#include "../Core/SystemType.h"
#include "Math/Math.h"
#include "ResourcesTypes.hpp"

#include "Core/BaseSerializer.h"

#include <string>
#include <unordered_map>
#include <optional>

namespace Uma_Engine
{
    class Graphics;

    class Sound;

    class ResourcesManager : public ISystem, public ISerializer
    {
    public:
        // ISystem virtual functions
        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;
        
        // Textures
        bool LoadTexture(const std::string& textureName, const std::string& filePath);
        void UnloadTexture(const std::string& textureName);
        Texture* GetTexture(const std::string& textureName);
        bool HasTexture(const std::string& textureName) const;
        void PrintLoadedTextureNames() const; // Print all loaded texture names (for debug)
        void UnloadAllTextures();
        
        // Audio
        bool LoadSound(const std::string& name, const std::string& filePath, SoundType type);
        void UnloadSound(const std::string& name);
        bool HasSound(const std::string& name);
        SoundInfo& GetSound(const std::string& name);
        void UnloadAllSound();
        
        // serializer
        const char* GetSectionName() const override { return "resources"; };  // e.g. "entities", "resources"
        std::string GetSerializerName() const override { return "resources_manager"; };  // e.g. "resources_manager", "coordinator"
        void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) override;
        void Deserialize(const rapidjson::Value& in) override;
        void SerializePrefab(Entity entity, rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) override;
        void DeserializePrefab(const rapidjson::Value& in) override;
        
    private:
        std::unordered_map<std::string, Texture> mTextures{};
        Graphics* mGraphics = nullptr;

        std::unordered_map<std::string, SoundInfo> mSoundList{};
        Sound* mSound = nullptr;
    };
}