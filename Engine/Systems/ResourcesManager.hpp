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
        
        // Audio
        bool LoadSound(const std::string& name, const std::string& filePath, SoundType type);
        void UnloadSound(const std::string& name);
        bool HasSound(const std::string& name);
        SoundInfo& GetSound(const std::string& name);
        
        // serializer
        const char* GetSectionName() const override { return "resources"; };  // e.g. "entities", "resources"
        void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) override;
        void Deserialize(const rapidjson::Value& in) override;
        
    private:
        std::unordered_map<std::string, Texture> mTextures;
        Graphics* mGraphics;

        std::unordered_map<std::string, SoundInfo> mSoundList;
        Sound* mSound;

        // Clear all textures
        void UnloadAllTextures();

        void UnloadAllSound(std::unordered_map<std::string, SoundInfo>& mSoundList);
    };
}