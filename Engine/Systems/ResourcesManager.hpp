#pragma once
#include "../Core/SystemType.h"
#include "Math/Math.h"
#include "ResourcesTypes.hpp"

#include <string>
#include <unordered_map>
#include <optional>

namespace Uma_Engine
{
    class Graphics;

    class Sound;

    class ResourcesManager : public ISystem
    {
    public:
        // ISystem virtual functions
        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;

        // Load a texture with a given name
        bool LoadTexture(const std::string& textureName, const std::string& filePath);

        // Unload a specific texture by name
        void UnloadTexture(const std::string& textureName);

        // Get texture by name
        Texture* GetTexture(const std::string& textureName);

        // Check if texture exists
        bool HasTexture(const std::string& textureName) const;

        // Print all loaded texture names (for debug)
        void PrintLoadedTextureNames() const;

        //sound
        bool LoadSound(const std::string& name, const std::string& filePath, SoundType type);

        void UnloadSound(const std::string& name);

        bool HasSound(const std::string& name);

        SoundInfo& GetSound(const std::string& name);

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