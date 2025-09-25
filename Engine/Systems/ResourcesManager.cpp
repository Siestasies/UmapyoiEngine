#include "ResourcesManager.hpp"
#include "../Systems/Graphics.hpp"
#include "../Systems/Sound.hpp"
#include "../../Core/SystemManager.h"

#include <cassert>
#include <iostream>
#include <vector>
#include <stdexcept>

namespace Uma_Engine
{
    void ResourcesManager::Init()
    {
        mGraphics = pSystemManager->GetSystem<Graphics>();
        mSound = pSystemManager->GetSystem<Sound>();

        assert(mGraphics != nullptr && "Error: Graphics system failed to initialize");
        assert(mSound != nullptr && "Error: Sound system failed to initialize");

        std::cout << "ResourcesManager initialized" << std::endl;
    }

    void ResourcesManager::Update(float dt)
    {
        // EMPTY (resource manager doesn't need to update anything)
    }

    void ResourcesManager::Shutdown()
    {
        std::cout << "ResourcesManager: Unloading all textures" << std::endl;
        UnloadAllTextures();

        UnloadAllSound(mSoundList);
    }

    bool ResourcesManager::LoadTexture(const std::string& textureName, const std::string& filePath)
    {
        assert(mGraphics != nullptr && "Error: Graphics system is not initialized properly.");

        // Check if texture is already loaded
        if (HasTexture(textureName))
        {
            std::cout << "Warning: Texture '" << textureName << "' is already loaded!" << std::endl;
            return true;
        }

        // Use Graphics class to load file texture
        Texture texture = mGraphics->LoadTextureFromFile(filePath);

        // Store in map
        mTextures[textureName] = texture;
        return true;
    }

    Texture* ResourcesManager::GetTexture(const std::string& textureName)
    {
        auto it = mTextures.find(textureName);
        return (it != mTextures.end()) ? &it->second : nullptr;
    }

    bool ResourcesManager::HasTexture(const std::string& textureName) const
    {
        return mTextures.find(textureName) != mTextures.end();
    }

    void ResourcesManager::PrintLoadedTextureNames() const
    {
        std::cout << "Loaded textures (" << mTextures.size() << "):" << std::endl;
        for (const auto& pair : mTextures)
        {
            std::cout << "  - " << pair.first << " (ID: " << pair.second.tex_id << ")" << std::endl;
        }
    }

    void ResourcesManager::UnloadTexture(const std::string& textureName)
    {
        auto it = mTextures.find(textureName);
        if (it != mTextures.end())
        {
            // Unload texture
            mGraphics->UnloadTexture(it->second.tex_id);

            // Remove from our map
            mTextures.erase(it);
            std::cout << "Texture '" << textureName << "' unloaded" << std::endl;
        }
        else
        {
            std::cout << "Warning: Texture does not exist: '" << textureName << "'" << std::endl;
        }
    }

    void ResourcesManager::UnloadAllTextures()
    {
        for (auto& pair : mTextures)
        {
            mGraphics->UnloadTexture(pair.second.tex_id);
        }
        mTextures.clear();
        std::cout << "All textures unloaded" << std::endl;
    }

    bool ResourcesManager::LoadSound(const std::string& name,const std::string& path,SoundType type) {
        if (!HasSound(name)) {
            mSoundList[name] = mSound->loadSound(path, type);
            return true;
        }
        return false;
    }

    void ResourcesManager::UnloadSound(const std::string& name) {
        mSound->unloadSound(mSoundList.find(name)->second.sound);
    }

    void ResourcesManager::UnloadAllSound(std::unordered_map<std::string, SoundInfo>& mSoundList) {
        mSound->unloadAllSounds(mSoundList);
    }

    bool ResourcesManager::HasSound(const std::string& name) {
        if (mSoundList.find(name) != mSoundList.end())
            return true;
        return false;
    }

    SoundInfo& ResourcesManager::GetSound(const std::string& name) {
        if (HasSound(name))
            return mSoundList.find(name)->second;
        else
            throw std::invalid_argument("Sound not found");
    }
}