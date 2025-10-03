/*!
\file   ResourcesManager.cpp
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
        (void)dt;
        // EMPTY (resource manager doesn't need to update anything)
    }

    void ResourcesManager::Shutdown()
    {
        std::cout << "ResourcesManager: Unloading all textures" << std::endl;
        UnloadAllTextures();
        UnloadAllSound();
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

    void ResourcesManager::Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator)
    {
        out.SetObject();

        rapidjson::Value texturesArr(rapidjson::kArrayType);
        for (const auto& tex : mTextures)
        {
            rapidjson::Value textureObj(rapidjson::kObjectType);

            // Name
            rapidjson::Value nameVal;
            nameVal.SetString(tex.first.c_str(), static_cast<rapidjson::SizeType>(tex.first.size()), allocator);
            textureObj.AddMember("name", nameVal, allocator);

            // Path
            rapidjson::Value pathVal;
            pathVal.SetString(tex.second.filePath.c_str(),
                static_cast<rapidjson::SizeType>(tex.second.filePath.size()),
                allocator);
            textureObj.AddMember("path", pathVal, allocator);

            texturesArr.PushBack(textureObj, allocator);
        }
        out.AddMember("textures", texturesArr, allocator);

        // sound
        rapidjson::Value audioArr(rapidjson::kArrayType);
        for (const auto& sound : mSoundList)
        {
            rapidjson::Value soundObj(rapidjson::kObjectType);

            // Name
            rapidjson::Value nameVal;
            nameVal.SetString(sound.first.c_str(), static_cast<rapidjson::SizeType>(sound.first.size()), allocator);
            soundObj.AddMember("name", nameVal, allocator);

            // Path
            rapidjson::Value pathVal;
            pathVal.SetString(sound.second.filePath.c_str(),
                static_cast<rapidjson::SizeType>(sound.second.filePath.size()),
                allocator);
            soundObj.AddMember("path", pathVal, allocator);

            // type
            rapidjson::Value typeVal;
            typeVal.SetInt(static_cast<int>(sound.second.type));
            soundObj.AddMember("type", typeVal, allocator);

            audioArr.PushBack(soundObj, allocator);
        }
        out.AddMember("sounds", audioArr, allocator);
    }

    void ResourcesManager::Deserialize(const rapidjson::Value& in)
    {
        assert(in.IsObject());

        if (in.HasMember("textures") && in["textures"].IsArray())
        {
            for (const auto& texVal : in["textures"].GetArray())
            {
                if (texVal.HasMember("name") && texVal.HasMember("path"))
                {
                    std::string name = texVal["name"].GetString();
                    std::string path = texVal["path"].GetString();

                    LoadTexture(name, path); // reuse your existing loader
                }
            }
        }

        if (in.HasMember("sounds") && in["sounds"].IsArray())
        {
            for (const auto& sndVal : in["sounds"].GetArray())
            {
                if (sndVal.HasMember("name") && sndVal.HasMember("path"))
                {
                    std::string name = sndVal["name"].GetString();
                    std::string path = sndVal["path"].GetString();
                    SoundType type = static_cast<SoundType>(sndVal["type"].GetInt());

                    LoadSound(name, path, type);
                }
            }
        }
    }

    bool ResourcesManager::LoadSound(const std::string& name,const std::string& path,SoundType type) 
    {
        if (!HasSound(name)) {
            SoundInfo temp = mSound->loadSound(path, type);
            if (temp.sound == nullptr) return false;
            mSoundList[name] = temp;
            return true;
        }
        return false;
    }

    void ResourcesManager::UnloadSound(const std::string& name) 
    {
        mSound->unloadSound(mSoundList.find(name)->second.sound);
    }

    void ResourcesManager::UnloadAllSound() 
    {
        mSound->unloadAllSounds(mSoundList);
        mSound->release();
    }

    bool ResourcesManager::HasSound(const std::string& name) 
    {
        if (mSoundList.find(name) != mSoundList.end())
            return true;
        return false;
    }

    SoundInfo& ResourcesManager::GetSound(const std::string& name) 
    {
        if (HasSound(name))
            return mSoundList.find(name)->second;
        else
            throw std::invalid_argument("Sound not found");
    }

    void ResourcesManager::SerializePrefab(Entity entity, rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator)
    {
        // we are not using this function in resources manager 
        (void)entity;
        (void)out;
        (void)allocator;
    }

    void ResourcesManager::DeserializePrefab(const rapidjson::Value& in)
    {
        // we are not using this function in resources manager 
        (void)in;
    }
}