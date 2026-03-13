/*!
\file   ResourcesManager.cpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (Everything else)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\co-author Javier Chua Dong Qing (Auto loading assets refactor)
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
#include <filesystem>
#include <fstream>
#include <sstream>
#include <glad/glad.h>

namespace Uma_Engine
{
    void ResourcesManager::Init()
    {
        mGraphics = pSystemManager->GetSystem<Graphics>();
        mSound = pSystemManager->GetSystem<SoundManager>();

        assert(mGraphics != nullptr && "Error: Graphics system failed to initialize");
        assert(mSound != nullptr && "Error: Sound system failed to initialize");

        std::cout << "ResourcesManager initialized" << std::endl;

        LoadAllEffectShaders();
    }

    void ResourcesManager::SetCoordinator(Uma_ECS::Coordinator* coordinator)
    {
        mCoordinator = coordinator;
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
        UnloadAllShaders();
        UnloadAllPrefabs();

        mSound->release();
    }

    bool ResourcesManager::LoadTexture(const std::string& filePath)
    {
        assert(mGraphics != nullptr && "Graphics system not initialized");

        if (filePath.empty())
        {
            std::cout << "Warning: Empty file path" << std::endl;
            return false;
        }

        std::string normalizedPath = NormalizePath(filePath);

        // Check if already loaded
        if (mTextures.find(normalizedPath) != mTextures.end())
        {
            std::cout << "Texture already loaded: " << normalizedPath << std::endl;
            return true;
        }

        // Load texture
        std::shared_ptr<Texture> texture = std::make_shared<Texture>(
            mGraphics->LoadTextureFromFile(filePath)
        );

        if (texture->tex_id == 0)
        {
            std::cerr << "Failed to load texture: " << filePath << std::endl;
            return false;
        }

        // Store with path as key
        mTextures[normalizedPath] = texture;

        return true;
    }

    std::shared_ptr<Texture> ResourcesManager::GetTexture(const std::string& filePath)
    {
        if (filePath.empty())
            return nullptr;

        std::string normalizedPath = NormalizePath(filePath);

        auto it = mTextures.find(normalizedPath);
        if (it != mTextures.end())
        {
            return it->second;
        }

        // Not found
        if (LoadTexture(filePath))
        {
            return mTextures[normalizedPath];
        }

        return nullptr;
    }

    bool ResourcesManager::HasTexture(const std::string& filePath) const
    {
        if (filePath.empty())
            return false;

        std::string normalizedPath = NormalizePath(filePath);
        return mTextures.find(normalizedPath) != mTextures.end();
    }

    void ResourcesManager::PrintLoadedTextureNames() const
    {
        std::cout << "Loaded textures (" << mTextures.size() << "):" << std::endl;
        for (const auto& pair : mTextures)
        {
            std::cout << "  - " << pair.first << " (ID: " << pair.second->tex_id << ")" << std::endl;
        }
    }

    void ResourcesManager::UnloadTexture(const std::string& filePath)
    {
        std::string normalizedPath = NormalizePath(filePath);

        auto it = mTextures.find(normalizedPath);
        if (it != mTextures.end())
        {
            mGraphics->UnloadTexture(it->second->tex_id);
            it->second->tex_id = 0;
            mTextures.erase(it);
            std::cout << "Texture unloaded: " << normalizedPath << std::endl;
        }
        else
        {
            std::cout << "Warning: Texture not found: " << normalizedPath << std::endl;
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
        for (const auto& [path, texture] : mTextures)
        {
            rapidjson::Value textureObj(rapidjson::kObjectType);

            // Save path
            rapidjson::Value pathVal;
            pathVal.SetString(path.c_str(),
                static_cast<rapidjson::SizeType>(path.size()), allocator);
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

        // Shaders
        rapidjson::Value shadersArr(rapidjson::kArrayType);
        for (const auto& pair : mShaders)
        {
            rapidjson::Value obj(rapidjson::kObjectType);

            rapidjson::Value nameVal;
            nameVal.SetString(pair.first.c_str(), allocator);
            obj.AddMember("name", nameVal, allocator);

            rapidjson::Value vPath;
            vPath.SetString(pair.second->vertexPath.c_str(), allocator);
            obj.AddMember("vertexPath", vPath, allocator);

            rapidjson::Value fPath;
            fPath.SetString(pair.second->fragmentPath.c_str(), allocator);
            obj.AddMember("fragmentPath", fPath, allocator);

            shadersArr.PushBack(obj, allocator);
        }
        out.AddMember("shaders", shadersArr, allocator);
    }

    void ResourcesManager::Deserialize(const rapidjson::Value& in)
    {
        assert(in.IsObject());

        if (in.HasMember("textures") && in["textures"].IsArray())
        {
            for (const auto& texVal : in["textures"].GetArray())
            {
                if (texVal.HasMember("path"))
                {
                    std::string path = texVal["path"].GetString();
                    LoadTexture(path);
                }
                // Backward compatibility
                else if (texVal.HasMember("name") && texVal.HasMember("path"))
                {
                    std::string path = texVal["path"].GetString();
                    LoadTexture(path);
                }
            }
        }

        if (in.HasMember("fonts") && in["fonts"].IsArray())
        {
            for (const auto& fontVal : in["fonts"].GetArray())
            {
                if (fontVal.HasMember("name") && fontVal.HasMember("path") && fontVal.HasMember("size"))
                {
                    std::string path = fontVal["path"].GetString();
                    unsigned int size = fontVal["size"].GetUint();

                    LoadFont(path, size);
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

                    //LoadSound(name, path, type);
                }
            }
        }

        if (in.HasMember("shaders") && in["shaders"].IsArray())
        {
            for (const auto& val : in["shaders"].GetArray())
            {
                if (val.HasMember("name") && val.HasMember("vertexPath") && val.HasMember("fragmentPath"))
                {
                    LoadShader(
                        val["name"].GetString(),
                        val["vertexPath"].GetString(),
                        val["fragmentPath"].GetString()
                    );
                }
            }
        }
    }

    bool ResourcesManager::LoadSound(const std::string& name,const std::string& path,SoundType type,bool is3D) 
    {
        if (!HasSound(name)) {
            SoundInfo temp = mSound->loadSound(path, type, is3D);
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
            mSound->unloadSound(mSoundList.find(name)->second);
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

    SoundInfo* ResourcesManager::GetSound(const std::string& name, const std::string& path)
    {
        auto it = mSoundList.find(name);

        // already loaded
        if (it != mSoundList.end())
            return &it->second;

        return nullptr;
    }

    bool ResourcesManager::LoadShader(const std::string& shaderName, const std::string& vertexPath, const std::string& fragmentPath)
    {
        if (HasShader(shaderName)) {
            std::cout << "Warning: Shader '" << shaderName << "' already loaded." << std::endl;
            return true;
        }

        Shader shaderData = mGraphics->LoadShaderFromFile(vertexPath, fragmentPath);
        if (shaderData.shaderProgramID == 0) return false;

        mShaders[shaderName] = std::make_shared<Shader>(shaderData);
        return true;
    }

    void ResourcesManager::UnloadShader(const std::string& shaderName)
    {
        auto it = mShaders.find(shaderName);
        if (it != mShaders.end()) {
            mGraphics->UnloadShader(it->second->shaderProgramID);
            mShaders.erase(it);
        }
    }

    std::shared_ptr<Shader> ResourcesManager::GetShader(const std::string& shaderName)
    {
        auto it = mShaders.find(shaderName);
        return (it != mShaders.end()) ? it->second : nullptr;
    }

    bool ResourcesManager::HasShader(const std::string& shaderName) const
    {
        return mShaders.find(shaderName) != mShaders.end();
    }

    void ResourcesManager::UnloadAllShaders()
    {
        for (auto& pair : mShaders) {
            mGraphics->UnloadShader(pair.second->shaderProgramID);
        }
        mShaders.clear();
    }

    const std::unordered_map<std::string, std::shared_ptr<Shader>>& ResourcesManager::GetLoadedShaders() const
    {
        return mShaders;
    }

    void ResourcesManager::SerializePrefab(Entity entity, rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator)
    {
        // Note: This is called by GameSerializer, which should handle resource collection
        // For now, this is a placeholder - actual implementation is in SerializeSpecificResources
        (void)entity;
        (void)out;
        (void)allocator;
    }

    Entity ResourcesManager::DeserializePrefab(const rapidjson::Value& in)
    {
        // Deserialize resources used by prefab (same as regular Deserialize)
        Deserialize(in);
        return static_cast<Entity>(-1); // ResourcesManager doesn't create entities
    }

    void ResourcesManager::SerializeSpecificResources(
        const std::unordered_set<std::string>& texturePaths,
        const std::unordered_set<std::string>& soundNames,
        const std::unordered_set<std::string>& fontNames,
        rapidjson::Value& out,
        rapidjson::Document::AllocatorType& allocator)
    {
        out.SetObject();

        // Serialize textures
        rapidjson::Value texturesArr(rapidjson::kArrayType);
        for (const std::string& texturePath : texturePaths)
        {
            std::string normalized = NormalizePath(texturePath);
            auto it = mTextures.find(normalized);
            if (it != mTextures.end())
            {
                rapidjson::Value textureObj(rapidjson::kObjectType);

                rapidjson::Value pathVal;
                pathVal.SetString(it->first.c_str(),
                    static_cast<rapidjson::SizeType>(it->first.size()), allocator);
                textureObj.AddMember("path", pathVal, allocator);

                texturesArr.PushBack(textureObj, allocator);
            }
        }
        out.AddMember("textures", texturesArr, allocator);

        // Serialize fonts
        rapidjson::Value fontsArr(rapidjson::kArrayType);
        for (const std::string& fontName : fontNames)
        {
            auto it = mFonts.find(fontName);
            if (it != mFonts.end())
            {
                rapidjson::Value fontObj(rapidjson::kObjectType);

                // Name
                rapidjson::Value nameVal;
                nameVal.SetString(fontName.c_str(),
                    static_cast<rapidjson::SizeType>(fontName.size()), allocator);
                fontObj.AddMember("name", nameVal, allocator);

                // Path
                rapidjson::Value pathVal;
                pathVal.SetString(it->second.filePath.c_str(),
                    static_cast<rapidjson::SizeType>(it->second.filePath.size()),
                    allocator);
                fontObj.AddMember("path", pathVal, allocator);

                // Size
                fontObj.AddMember("size", it->second.fontSize, allocator);

                fontsArr.PushBack(fontObj, allocator);
            }
        }
        out.AddMember("fonts", fontsArr, allocator);

        // Serialize sounds
        rapidjson::Value audioArr(rapidjson::kArrayType);
        for (const std::string& soundName : soundNames)
        {
            auto it = mSoundList.find(soundName);
            if (it != mSoundList.end())
            {
                rapidjson::Value soundObj(rapidjson::kObjectType);

                // Name
                rapidjson::Value nameVal;
                nameVal.SetString(soundName.c_str(),
                    static_cast<rapidjson::SizeType>(soundName.size()), allocator);
                soundObj.AddMember("name", nameVal, allocator);

                // Path
                rapidjson::Value pathVal;
                pathVal.SetString(it->second.filePath.c_str(),
                    static_cast<rapidjson::SizeType>(it->second.filePath.size()),
                    allocator);
                soundObj.AddMember("path", pathVal, allocator);

                // Type
                soundObj.AddMember("type", static_cast<int>(it->second.type), allocator);

                audioArr.PushBack(soundObj, allocator);
            }
        }
        out.AddMember("sounds", audioArr, allocator);
    }

    bool ResourcesManager::LoadFont(const std::string& filePath, unsigned int fontSize)
    {
        assert(mGraphics != nullptr && "Error: Graphics system is not initialized.");

        if (filePath.empty())
        {
            std::cout << "Warning: Empty font path" << std::endl;
            return false;
        }

        std::string normalizedPath = NormalizePath(filePath);

        // Check if already loaded
        if (mFonts.find(normalizedPath) != mFonts.end())
        {
            std::cout << "Font already loaded: " << normalizedPath << std::endl;
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

        // Store in map with path as key
        mFonts[normalizedPath] = fontData;
        std::cout << "Font '" << normalizedPath << "' loaded and managed." << std::endl;
        return true;
    }

    FontData* ResourcesManager::GetFont(const std::string& filePath)
    {
        if (filePath.empty())
            return nullptr;

        std::string normalizedPath = NormalizePath(filePath);

        auto it = mFonts.find(normalizedPath);
        if (it != mFonts.end())
        {
            return &it->second;
        }

        // Not found - try to load it
        if (LoadFont(filePath))
        {
            return &mFonts[normalizedPath];
        }

        return nullptr;
    }

    bool ResourcesManager::HasFont(const std::string& filePath) const
    {
        if (filePath.empty())
            return false;

        std::string normalizedPath = NormalizePath(filePath);
        return mFonts.find(normalizedPath) != mFonts.end();
    }

    void ResourcesManager::PrintLoadedFontNames() const
    {
        std::cout << "Loaded fonts (" << mFonts.size() << "):" << std::endl;
        for (const auto& pair : mFonts)
        {
            std::cout << "  - " << pair.first << " (Size: " << pair.second.fontSize << ")" << std::endl;
        }
    }

    void ResourcesManager::UnloadFont(const std::string& filePath)
    {
        std::string normalizedPath = NormalizePath(filePath);

        auto it = mFonts.find(normalizedPath);
        if (it != mFonts.end())
        {
            // Unload font data
            mGraphics->UnloadFontData(it->second);

            // Remove from map
            mFonts.erase(it);
            std::cout << "Font '" << normalizedPath << "' unloaded" << std::endl;
        }
        else
        {
            std::cout << "Warning: Font does not exist: '" << normalizedPath << "'" << std::endl;
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
        return mFonts;
    }

    std::string ResourcesManager::NormalizePath(const std::string& path)
    {
        std::string normalized = path;
        std::replace(normalized.begin(), normalized.end(), '\\', '/');
        // Convert to lowercase for case-insensitive comparison
        std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return normalized;
    }

    bool ResourcesManager::LoadPrefab(const std::string& filePath)
    {
        if (filePath.empty()) return false;
        std::string normalized = NormalizePath(filePath);

        // Check cache
        if (mPrefabs.find(normalized) != mPrefabs.end()) return true;

        // Load file
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            std::cerr << "Error: Failed to open prefab file: " << filePath << std::endl;
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();
        file.close();

        // Parse JSON
        std::shared_ptr<rapidjson::Document> doc = std::make_shared<rapidjson::Document>();
        if (doc->Parse(content.c_str()).HasParseError())
        {
            std::cerr << "Error: Failed to parse prefab JSON: " << filePath << std::endl;
            return false;
        }

        // Store in Cache
        mPrefabs[normalized] = doc;
        std::cout << "Prefab cached: " << normalized << std::endl;
        return true;
    }

    std::shared_ptr<rapidjson::Document> ResourcesManager::GetPrefab(const std::string& filePath)
    {
        std::string normalized = NormalizePath(filePath);

        // If not in cache, try to load it now
        if (mPrefabs.find(normalized) == mPrefabs.end())
        {
            if (!LoadPrefab(filePath)) return nullptr;
        }

        return mPrefabs[normalized];
    }

    void ResourcesManager::UnloadPrefab(const std::string& filePath)
    {
        std::string normalized = NormalizePath(filePath);
        mPrefabs.erase(normalized);
    }

    bool ResourcesManager::HasPrefab(const std::string& filePath) const
    {
        return mPrefabs.find(NormalizePath(filePath)) != mPrefabs.end();
    }

    void ResourcesManager::UnloadAllPrefabs()
    {
        // shared_ptr handles the deletion automatically
        mPrefabs.clear();
        std::cout << "All prefabs unloaded" << std::endl;
    }

    const std::unordered_map<std::string, std::shared_ptr<rapidjson::Document>>& ResourcesManager::GetLoadedPrefabs() const
    {
        return mPrefabs;
    }

    void ResourcesManager::ReflectUniforms(ShaderEffect& effect)
    {
        effect.uniforms.clear();
        GLuint program = effect.shaderProgramID;
        if (program == 0) return;

        GLint count = 0;
        glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);

        // Skip engine-managed uniforms
        static const std::unordered_set<std::string> skip = {
            "image", "projection", "uTime"
        };

        for (GLint i = 0; i < count; i++)
        {
            char name[256];
            GLsizei length;
            GLint size;
            GLenum glType;
            glGetActiveUniform(program, i, sizeof(name), &length, &size, &glType, name);

            std::string uName(name);
            if (skip.count(uName)) continue;

            UniformInfo info;
            info.name = uName;
            info.location = glGetUniformLocation(program, name);

            switch (glType)
            {
            case GL_FLOAT:      info.type = UniformType::Float; break;
            case GL_FLOAT_VEC2: info.type = UniformType::Vec2;  break;
            case GL_FLOAT_VEC3: info.type = UniformType::Vec3;  break;
            case GL_FLOAT_VEC4: info.type = UniformType::Vec4;  break;
            case GL_INT:        info.type = UniformType::Int;    break;
            default: continue;  // skip unsupported types
            }

            effect.uniforms.push_back(info);
        }
    }

    void ResourcesManager::LoadAllEffectShaders()
    {
        const std::string effectDir = "Assets/Shaders/Effects";

        if (!std::filesystem::exists(effectDir) ||
            !std::filesystem::is_directory(effectDir))
        {
            std::cout << "No Effects shader directory found at: " << effectDir << std::endl;
            return;
        }

        for (const auto& entry : std::filesystem::directory_iterator(effectDir))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".frag")
            {
                std::string effectName = entry.path().stem().string();
                std::string fragPath = entry.path().string();
                // Normalize to forward slashes
                std::replace(fragPath.begin(), fragPath.end(), '\\', '/');

                LoadEffectShader(effectName, fragPath);
            }
        }
    }

    bool ResourcesManager::LoadEffectShader(const std::string& effectName, const std::string& fragPath)
    {
        // Reuse the instanced vertex shader, effect shaders only override the fragment stage
        const std::string vertPath = "Assets/Shaders/instanced.vert";

        Shader shaderData = mGraphics->LoadShaderFromFile(vertPath, fragPath);

        ShaderEffect effect;
        effect.name = effectName;
        effect.fragPath = fragPath;
        effect.shaderProgramID = shaderData.shaderProgramID;

        if (effect.shaderProgramID != 0)
        {
            ReflectUniforms(effect);
            std::cout << "Effect shader loaded: " << effectName
                << " (" << effect.uniforms.size() << " uniforms)" << std::endl;
        }
        else
        {
            std::cerr << "Effect shader FAILED to compile: " << effectName << std::endl;
        }

        // Store even if compilation failed (shaderProgramID == 0) so the UI can show the error state
        mEffects[effectName] = std::move(effect);
        return effect.shaderProgramID != 0;
    }

    void ResourcesManager::RefreshEffectShaders()
    {
        // Unload existing effect shader programs
        for (auto& [name, effect] : mEffects)
        {
            if (effect.shaderProgramID != 0)
            {
                mGraphics->UnloadShader(effect.shaderProgramID);
            }
        }
        mEffects.clear();

        // Reload all from disk
        LoadAllEffectShaders();
    }

    bool ResourcesManager::CreateEffectShaderFile(const std::string& effectName)
    {
        const std::string effectDir = "Assets/Shaders/Effects";
        std::filesystem::create_directories(effectDir);

        std::string filePath = effectDir + "/" + effectName + ".frag";

        if (std::filesystem::exists(filePath))
        {
            std::cerr << "Effect shader file already exists: " << filePath << std::endl;
            return false;
        }

        // Write a starter template that matches the instanced vertex shader outputs
        std::ofstream file(filePath);
        if (!file.is_open())
        {
            std::cerr << "Failed to create effect shader file: " << filePath << std::endl;
            return false;
        }

        file << "#version 450 core\n"
            << "in vec2 TexCoords;\n"
            << "in vec4 Tint;\n"
            << "out vec4 color;\n"
            << "\n"
            << "uniform sampler2D image;\n"
            << "uniform float uTime;\n"
            << "\n"
            << "// Add your custom uniforms here, e.g.:\n"
            << "// uniform float intensity;\n"
            << "\n"
            << "void main()\n"
            << "{\n"
            << "    vec4 texColor = texture(image, TexCoords);\n"
            << "    color = vec4(texColor.rgb * Tint.rgb, texColor.a * Tint.a);\n"
            << "}\n";

        file.close();

        // Immediately compile so it shows up in the effect list
        std::string normalizedPath = filePath;
        std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');
        LoadEffectShader(effectName, normalizedPath);

        return true;
    }

    std::vector<std::string> ResourcesManager::GetEffectShaderNames() const
    {
        std::vector<std::string> names;
        names.reserve(mEffects.size());
        for (const auto& [name, effect] : mEffects)
        {
            names.push_back(name);
        }
        return names;
    }

    bool ResourcesManager::EffectShaderFileExists(const std::string& effectName) const
    {
        std::string filePath = "Assets/Shaders/Effects/" + effectName + ".frag";
        return std::filesystem::exists(filePath);
    }

    const ShaderEffect* ResourcesManager::GetEffect(const std::string& effectName) const
    {
        auto it = mEffects.find(effectName);
        return (it != mEffects.end()) ? &it->second : nullptr;
    }
}