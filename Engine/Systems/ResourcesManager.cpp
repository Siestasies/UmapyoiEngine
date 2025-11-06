/*!
\file   ResourcesManager.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (Everything else)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\co-author Javier Chua Dong Qing (Font)
\par       E-mail: javierdongqing.chua@digipen.edu
\par       DigiPen login: javierdongqing.chua

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
#include "../Systems/SoundManager.hpp"
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
        mSound = pSystemManager->GetSystem<SoundManager>();

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
        UnloadAllFonts();
        UnloadAllSound();

        mSound->release();
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
        std::shared_ptr<Texture> texture = std::make_shared<Texture>(mGraphics->LoadTextureFromFile(filePath));

        // Store in map
        mTextures[textureName] = texture;
        return true;
    }

    std::shared_ptr<Texture> ResourcesManager::GetTexture(const std::string& textureName)
    {
        auto it = mTextures.find(textureName);
        return (it != mTextures.end()) ? it->second : nullptr;
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
            std::cout << "  - " << pair.first << " (ID: " << pair.second->tex_id << ")" << std::endl;
        }
    }

    void ResourcesManager::UnloadTexture(const std::string& textureName)
    {
        auto it = mTextures.find(textureName);
        if (it != mTextures.end())
        {
            // Unload texture
            mGraphics->UnloadTexture((*it->second).tex_id);

            // tex_id become invalid so set to 0
            (*it->second).tex_id = 0;

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
            mGraphics->UnloadTexture(pair.second->tex_id);
        }
        mTextures.clear();
        std::cout << "All textures unloaded" << std::endl;
    }

    const std::unordered_map<std::string, std::shared_ptr<Texture>>& ResourcesManager::GetLoadedTextures() const
    {
        // TODO: insert return statement here
        return mTextures;
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
            pathVal.SetString(tex.second->filePath.c_str(),
                static_cast<rapidjson::SizeType>(tex.second->filePath.size()),
                allocator);
            textureObj.AddMember("path", pathVal, allocator);

            texturesArr.PushBack(textureObj, allocator);
        }
        out.AddMember("textures", texturesArr, allocator);

        rapidjson::Value fontsArr(rapidjson::kArrayType);
        for (const auto& font : mFonts)
        {
            rapidjson::Value fontObj(rapidjson::kObjectType);

            // Name
            rapidjson::Value nameVal;
            nameVal.SetString(font.first.c_str(), static_cast<rapidjson::SizeType>(font.first.size()), allocator);
            fontObj.AddMember("name", nameVal, allocator);

            // Path
            rapidjson::Value pathVal;
            pathVal.SetString(font.second.filePath.c_str(),
                static_cast<rapidjson::SizeType>(font.second.filePath.size()),
                allocator);
            fontObj.AddMember("path", pathVal, allocator);

            // Size
            rapidjson::Value sizeVal;
            sizeVal.SetUint(font.second.fontSize);
            fontObj.AddMember("size", sizeVal, allocator);

            fontsArr.PushBack(fontObj, allocator);
        }
        out.AddMember("fonts", fontsArr, allocator);

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

        if (in.HasMember("fonts") && in["fonts"].IsArray())
        {
            for (const auto& fontVal : in["fonts"].GetArray())
            {
                if (fontVal.HasMember("name") && fontVal.HasMember("path") && fontVal.HasMember("size"))
                {
                    std::string name = fontVal["name"].GetString();
                    std::string path = fontVal["path"].GetString();
                    unsigned int size = fontVal["size"].GetUint();

                    LoadFont(name, path, size);
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
        FMOD_SOUND* sound = mSoundList.find(name)->second.sound;

        if (sound)
        {
            mSound->unloadSound(mSoundList.find(name)->second.sound);
            mSoundList.erase(name);
        }
    }

    void ResourcesManager::UnloadAllSound() 
    {
        mSound->unloadAllSounds(mSoundList);
       /* mSound->release();*/
    }

    const std::unordered_map<std::string, SoundInfo>& ResourcesManager::GetLoadedSounds() const
    {
        // TODO: insert return statement here
        return mSoundList;
    }

    bool ResourcesManager::HasSound(const std::string& name) 
    {
        if (mSoundList.find(name) != mSoundList.end())
            return true;
        return false;
    }

    SoundInfo* ResourcesManager::GetSound(const std::string& name) 
    {
        auto it = mSoundList.find(name);
        return (it != mSoundList.end()) ? &it->second : nullptr;
    }

    void ResourcesManager::SerializePrefab(Entity entity, rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator)
    {
        // we are not using this function in resources manager 
        (void)entity;
        (void)out;
        (void)allocator;
    }

    Entity ResourcesManager::DeserializePrefab(const rapidjson::Value& in)
    {
        // we are not using this function in resources manager 
        (void)in;
        return static_cast<Entity>(-1);
    }

    bool ResourcesManager::LoadFont(const std::string& fontName, const std::string& filePath, unsigned int fontSize)
    {
        assert(mGraphics != nullptr && "Error: Graphics system is not initialized.");

        // Check if font is already loaded
        if (HasFont(fontName))
        {
            std::cout << "Warning: Font '" << fontName << "' is already loaded!" << std::endl;
            return true;
        }

        // Load font data
        FontData fontData = mGraphics->LoadFontFromFile(filePath, fontSize);

        // Check for loading failure
        if (fontData.VAO == 0)
        {
            std::cerr << "Error: Failed to load font file: " << filePath << std::endl;
            return false;
        }

        // Store in map
        mFonts[fontName] = fontData;
        std::cout << "Font '" << fontName << "' loaded and managed." << std::endl;
        return true;
    }

    FontData* ResourcesManager::GetFont(const std::string& fontName)
    {
        auto it = mFonts.find(fontName);
        return (it != mFonts.end()) ? &it->second : nullptr;
    }

    bool ResourcesManager::HasFont(const std::string& fontName) const
    {
        return mFonts.find(fontName) != mFonts.end();
    }

    void ResourcesManager::PrintLoadedFontNames() const
    {
        std::cout << "Loaded fonts (" << mFonts.size() << "):" << std::endl;
        for (const auto& pair : mFonts)
        {
            std::cout << "  - " << pair.first << " (Size: " << pair.second.fontSize << ")" << std::endl;
        }
    }

    void ResourcesManager::UnloadFont(const std::string& fontName)
    {
        auto it = mFonts.find(fontName);
        if (it != mFonts.end())
        {
            // Unload font data
            mGraphics->UnloadFontData(it->second);

            // Remove from map
            mFonts.erase(it);
            std::cout << "Font '" << fontName << "' unloaded" << std::endl;
        }
        else
        {
            std::cout << "Warning: Font does not exist: '" << fontName << "'" << std::endl;
        }
    }

    void ResourcesManager::UnloadAllFonts()
    {
        for (auto& pair : mFonts)
        {
            mGraphics->UnloadFontData(pair.second);
        }
        mFonts.clear();
        std::cout << "All fonts unloaded" << std::endl;
    }
    const std::unordered_map<std::string, FontData>& ResourcesManager::GetLoadedFonts() const
    {
        // TODO: insert return statement here
        return mFonts;
    }
}