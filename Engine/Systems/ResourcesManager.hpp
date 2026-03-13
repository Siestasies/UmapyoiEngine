/*!
\file   ResourcesManager.hpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (Everything else)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\co-author Javier Chua Dong Qing (Auto loading assets refactor, Effect shader)
\par       E-mail: javierdongqing.chua@digipen.edu
\par       DigiPen login: javierdongqing.chua

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
#include <unordered_set>
#include <optional>
#include <vector>

namespace Uma_ECS
{
    class Coordinator;
}

namespace Uma_Engine
{
    class Graphics;

    class SoundManager;

    class ResourcesManager : public ISystem, public ISerializer
    {
    public:
        // ISystem virtual functions
        /*!
        \brief Initializes the ResourcesManager
        */
        void Init() override;

        /*!
        \brief Updates the system. Currently unused for ResourcesManager
        \param dt Delta time for the current frame
        */
        void Update(float dt) override;

        /*!
        \brief Shuts down the system, unloading all managed resources
        */
        void Shutdown() override;

        /*!
        \brief Sets the ECS Coordinator pointer for the ResourcesManager.
        \param coordinator Pointer to the ECS Coordinator instance.
        */
        void SetCoordinator(Uma_ECS::Coordinator* coordinator);
        
        // Textures
        /*!
        \brief Loads a texture from a file and caches it under a unique name
        \param textureName The unique identifier to assign to this texture
        \param filePath The relative path to the image file
        \return True if loaded successfully or if the texture already exists; false otherwise
        */
        bool LoadTexture(const std::string& filePath);

        /*!
        \brief Unloads a specific texture from memory
        \param textureName The unique identifier of the texture to unload
        */
        void UnloadTexture(const std::string& filePath);

        /*!
        \brief Retrieves a shared pointer to a loaded texture
        \param textureName The unique identifier of the texture
        \return A shared pointer to the Texture, or nullptr if not found
        */
        std::shared_ptr<Texture> GetTexture(const std::string& filePath);

        /*!
        \brief Checks if a specific texture is currently loaded
        \param textureName The unique identifier to check
        \return True if the texture exists in the cache, false otherwise
        */
        bool HasTexture(const std::string& filePath) const;

        /*!
        \brief Prints the names and IDs of all currently loaded textures to the console
        */
        void PrintLoadedTextureNames() const; // For debug

        /*!
        \brief Unloads all currently managed textures from the Graphics system
        */
        void UnloadAllTextures();

        /*!
        \brief Retrieves the entire map of loaded textures
        \return A constant reference to the underlying unordered map of textures
        */
        const std::unordered_map<std::string, std::shared_ptr<Texture>>& GetLoadedTextures() const;
        
        // Audio
        /*!
        \brief Loads a sound file and caches it.
        \param name The unique identifier to assign to this sound.
        \param filePath The relative path to the audio file.
        \param type The type of sound (e.g., BGM, SFX).
        \return True if loaded successfully, false if failed or already exists.
        */
        bool LoadSound(const std::string& name, const std::string& filePath, SoundType type, bool is3D);

        /*!
        \brief Unloads a specific sound resource.
        \param name The unique identifier of the sound to unload.
        */
        void UnloadSound(const std::string& name);

        /*!
        \brief Checks if a specific sound is currently loaded.
        \param name The unique identifier to check.
        \return True if the sound exists in the cache, false otherwise.
        */
        bool HasSound(const std::string& name);

        /*!
        \brief Retrieves a pointer to the SoundInfo structure.
        \param name The unique identifier of the sound.
        \return A pointer to the SoundInfo, or nullptr if not found.
        */
        SoundInfo* GetSound(const std::string& name, const std::string& path = "");


        /*!
        \brief Unloads all managed sounds and releases audio resources.
        */
        void UnloadAllSound();

        /*!
        \brief Retrieves the entire map of loaded sounds.
        \return A constant reference to the underlying unordered map of SoundInfo.
        */
        const std::unordered_map<std::string, SoundInfo>& GetLoadedSounds() const;

        // Font
        /*!
        \brief Loads a font from a file with a specific size.
        \param fontName The unique identifier to assign to this font.
        \param filePath The relative path to the font file.
        \param fontSize The size of the font to generate (default is 48).
        \return True if loaded successfully, false on failure.
        */
        bool LoadFont(const std::string& filePath, unsigned int fontSize = 48);

        /*!
        \brief Unloads a specific font and frees its graphics resources.
        \param fontName The unique identifier of the font to unload.
        */
        void UnloadFont(const std::string& filePath);

        /*!
        \brief Retrieves a pointer to the FontData structure.
        \param fontName The unique identifier of the font.
        \return A pointer to FontData, or nullptr if not found.
        */
        FontData* GetFont(const std::string& filePath);

        /*!
        \brief Checks if a specific font is currently loaded.
        \param fontName The unique identifier to check.
        \return True if the font exists in the cache, false otherwise.
        */
        bool HasFont(const std::string& filePath) const;

        /*!
        \brief Prints the names and sizes of all currently loaded fonts to the console for debugging.
        */
        void PrintLoadedFontNames() const; // For debug

        /*!
        \brief Unloads all currently managed fonts.
        */
        void UnloadAllFonts();

        /*!
        \brief Retrieves the entire map of loaded fonts.
        \return A constant reference to the underlying unordered map of FontData.
        */
        const std::unordered_map<std::string, FontData>& GetLoadedFonts() const;

        // Default Shaders
        /*!
        \brief Loads a shader program from vertex and fragment shader source files.
        \param shaderName The unique identifier to assign to this shader.
        \param vertexPath The file path to the vertex shader source.
        \param fragmentPath The file path to the fragment shader source.
        \return True if loaded successfully, false on failure or if the shader already exists.
        */
        bool LoadShader(const std::string& shaderName, const std::string& vertexPath, const std::string& fragmentPath);

        /*!
        \brief Unloads a specific shader program and frees its resources.
        \param shaderName The unique identifier of the shader to unload.
        */
        void UnloadShader(const std::string& shaderName);

        /*!
        \brief Retrieves a shared pointer to a loaded shader program.
        \param shaderName The unique identifier of the shader.
        \return A shared pointer to the Shader, or nullptr if not found.
        */
        std::shared_ptr<Shader> GetShader(const std::string& shaderName);

        /*!
        \brief Checks if a specific shader is currently loaded.
        \param shaderName The unique identifier to check.
        \return True if the shader exists in the cache, false otherwise.
        */
        bool HasShader(const std::string& shaderName) const;

        /*!
        \brief Unloads all currently managed shader programs.
        */
        void UnloadAllShaders();

        /*!
        \brief Retrieves the entire map of loaded shaders.
        \return A constant reference to the underlying unordered map of shaders.
        */
        const std::unordered_map<std::string, std::shared_ptr<Shader>>& GetLoadedShaders() const;

        // Effect Shaders
        /*!
        \brief Loads all effect shaders found in the effects shader directory.
        */
        void LoadAllEffectShaders();

        /*!
        \brief Loads a single effect shader from a fragment shader file.
        \param effectName The unique identifier to assign to this effect.
        \param fragPath The file path to the fragment shader source.
        \return True if loaded successfully, false on failure.
        */
        bool LoadEffectShader(const std::string& effectName, const std::string& fragPath);

        /*!
        \brief Reloads all effect shaders from disk, refreshing any changes.
        */
        void RefreshEffectShaders();

        /*!
        \brief Creates a new effect shader file from a default template.
        \param effectName The name for the new effect shader file.
        \return True if the file was created successfully, false if it already exists or on failure.
        */
        bool CreateEffectShaderFile(const std::string& effectName);

        /*!
        \brief Retrieves the names of all currently loaded effect shaders.
        \return A vector of effect shader name strings.
        */
        std::vector<std::string> GetEffectShaderNames() const;

        /*!
        \brief Checks if an effect shader file exists on disk.
        \param effectName The name of the effect shader to check.
        \return True if the file exists, false otherwise.
        */
        bool EffectShaderFileExists(const std::string& effectName) const;

        /*!
        \brief Retrieves a pointer to a loaded ShaderEffect.
        \param effectName The unique identifier of the effect.
        \return A pointer to the ShaderEffect, or nullptr if not found.
        */
        const ShaderEffect* GetEffect(const std::string& effectName) const;

        // Prefab
        /*!
        \brief Loads a prefab from a JSON file and caches it.
        \param filePath The file path to the prefab JSON file.
        \return True if loaded successfully, false on failure or if already loaded.
        */
        bool LoadPrefab(const std::string& filePath);

        /*!
        \brief Unloads a specific prefab from the cache.
        \param filePath The file path of the prefab to unload.
        */
        void UnloadPrefab(const std::string& filePath);

        /*!
        \brief Retrieves a shared pointer to a loaded prefab document.
        \param filePath The file path of the prefab.
        \return A shared pointer to the rapidjson::Document, or nullptr if not found.
        */
        std::shared_ptr<rapidjson::Document> GetPrefab(const std::string& filePath);

        /*!
        \brief Unloads all currently managed prefabs.
        */
        void UnloadAllPrefabs();

        /*!
        \brief Checks if a specific prefab is currently loaded.
        \param filePath The file path of the prefab to check.
        \return True if the prefab exists in the cache, false otherwise.
        */
        bool HasPrefab(const std::string& filePath) const;

        /*!
        \brief Retrieves the entire map of loaded prefabs.
        \return A constant reference to the underlying unordered map of prefab documents.
        */
        const std::unordered_map<std::string, std::shared_ptr<rapidjson::Document>>& GetLoadedPrefabs() const;
        
        // serializer
        /*!
        \brief Gets the section name for the JSON document ("resources").
        \return The section name string.
        */
        const char* GetSectionName() const override { return "resources"; };  // e.g. "entities", "resources"

        /*!
        \brief Gets the unique serializer name ("resources_manager").
        \return The serializer name string.
        */
        std::string GetSerializerName() const override { return "resources_manager"; };  // e.g. "resources_manager", "coordinator"

        /*!
        \brief Serializes all currently loaded resources (name, path, metadata) into a JSON object.
        \param out The JSON value object to write to.
        \param allocator The JSON document allocator.
        */
        void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) override;

        /*!
        \brief Deserializes a JSON object, reloading all listed textures, fonts, and sounds.
        \param in The JSON value object containing resource data.
        */
        void Deserialize(const rapidjson::Value& in) override;

        /*!
        \brief Placeholder for entity prefab serialization. ResourcesManager usually serializes global state, not per-entity.
        \param entity The entity ID (unused).
        \param out Output JSON value (unused).
        \param allocator JSON allocator (unused).
        */
        void SerializePrefab(Entity entity, rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) override;

        /*!
        \brief Deserializes resources from a prefab definition. Delegates to standard Deserialize().
        \param in The JSON value containing resource data.
        \return Returns -1 (static_cast<Entity>(-1)) as this manager does not create entities.
        */
        Entity DeserializePrefab(const rapidjson::Value& in) override;

        // Helper to serialize specific resources (for prefabs)
        /*!
        \brief Helper function to serialize only a specific subset of resources (used for saving Prefabs).
        \param textureNames Set of texture names to serialize.
        \param soundNames Set of sound names to serialize.
        \param fontNames Set of font names to serialize.
        \param out The output JSON value.
        \param allocator The JSON allocator.
        */
        void SerializeSpecificResources(
            const std::unordered_set<std::string>& texturePaths,
            const std::unordered_set<std::string>& soundNames,
            const std::unordered_set<std::string>& fontNames,
            rapidjson::Value& out,
            rapidjson::Document::AllocatorType& allocator);

    private:
        Uma_ECS::Coordinator* mCoordinator = nullptr;

        /*!
        \brief Normalizes a file path by converting backslashes to forward slashes and resolving inconsistencies.
        \param path The raw file path string to normalize.
        \return The normalized path string.
        */
        static std::string NormalizePath(const std::string& path);

        std::unordered_map<std::string, std::shared_ptr<Texture>> mTextures{};
        Graphics* mGraphics = nullptr;

        // key is the name
        std::unordered_map<std::string, SoundInfo> mSoundList{};
        SoundManager* mSound = nullptr;

        std::unordered_map<std::string, FontData> mFonts{};

        std::unordered_map<std::string, std::shared_ptr<Shader>> mShaders{};

        std::unordered_map<std::string, std::shared_ptr<rapidjson::Document>> mPrefabs;

        std::unordered_map<std::string, ShaderEffect> mEffects{};
        /*!
        \brief Reflects and populates the uniform metadata for a given shader effect.
        \param effect The ShaderEffect whose uniforms will be queried and stored.
        */
        void ReflectUniforms(ShaderEffect& effect);
    };
}