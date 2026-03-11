/*!
\file   Tilemap.h
\par    Project: GAM250
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Tilemap component and supporting structures for 2D tile-based rendering

 This file defines the core tilemap system used in UmapyoiEngine. It provides
 a complete implementation for grid-based tile rendering with support for:
 - Multi-layer tilemaps with independent visual properties
 - Tileset management with UV coordinate calculation
 - Serialization/deserialization for scene persistence
 - Runtime visibility and rendering controls


All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "RapidJSON/document.h"
#include "core/Types.hpp"
#include "../Systems/ResourcesTypes.hpp"
#include "../Systems/ResourcesManager.hpp"

#include "../Debugging/Debugger.hpp"

#include "../../Math/Math.h"
#include <string>
#include <vector>

namespace Uma_ECS
{
    /**
     * @brief Represents a tileset - a texture atlas containing multiple tiles
     *
     * A tileset is a grid-based texture that contains all the tiles used in a tilemap.
     * It stores the texture reference, dimensions, and provides UV calculation utilities.
     */
    struct Tileset
    {
        // Tileset reference
        std::shared_ptr<Uma_Engine::Texture> texture = nullptr;
        std::string texturePath{};

        // currently not in use
        int tilesetWidth = 0;
        int tilesetHeight = 0;

        int columns = 0;
        int rows = 0;

        /**
         * @brief Calculate UV coordinates for a tile at a given cell position
         *
         * Converts grid coordinates (column, row) to normalized UV coordinates (0-1 range)
         * for texture sampling.
         *
         * @param uvOffset Output: Bottom-left corner UV coordinates
         * @param uvSize Output: Width and height of UV region
         * @param cell Input: Grid cell position (x=column, y=row)
         */
        void GetUVs(Vec2& uvOffset, Vec2& uvSize, Vec2 cell) const
        {
            if (!IsLoaded())
            {
                return;
            }

            // Calculate size of one cell in UV space
            float cellW = 1.0f / columns;
            float cellH = 1.0f / rows;

            // Half-texel inset to prevent bleeding from adjacent tiles
            float insetX = 0.5f / texture->tex_size.x;
            float insetY = 0.5f / texture->tex_size.y;

            uvOffset.x = cell.x * cellW + insetX;
            uvOffset.y = cell.y * cellH + insetY;
            uvSize.x = cellW - 2.0f * insetX;
            uvSize.y = cellH - 2.0f * insetY;
        }

        /**
         * @brief Calculate UV coordinates for a tile using linear index
         *
         * Uses pixel insets to prevent texture bleeding between adjacent tiles.
         * This is crucial for avoiding visual artifacts at tile boundaries.
         *
         * @param tileIndex Linear tile index (0 = top-left, increases left-to-right, top-to-bottom)
         * @param u0 Output: Left UV coordinate
         * @param v0 Output: Bottom UV coordinate
         * @param u1 Output: Right UV coordinate
         * @param v1 Output: Top UV coordinate
         */
        void GetUVs(int tileIndex, float& u0, float& v0, float& u1, float& v1) const
        {
            if (!IsLoaded() || tileIndex < 0) {
                u0 = v0 = u1 = v1 = 0.0f;
                return;
            }

            int col = tileIndex % columns;
            int row = tileIndex / columns;

            float texelWidth = 1.0f / texture->tex_size.x;
            float texelHeight = 1.0f / texture->tex_size.y;
            float insetX = texelWidth * 0.5f;
            float insetY = texelHeight * 0.5f;

            u0 = (float)col / columns + insetX;
            v0 = (float)row / rows + insetY;
            u1 = (float)(col + 1) / columns - insetX;
            v1 = (float)(row + 1) / rows - insetY;
        }

        /**
         * @brief Check if the tileset texture is loaded and ready
         * @return true if texture is valid, false otherwise
         */
        bool IsLoaded() const
        {
            return texture != nullptr;
        }

        /**
         * @brief Load the tileset texture from file path
         *
         * NOTE: This currently only validates the texture but doesn't actually load it.
         * The actual texture loading happens through ResourcesManager.
         *
         * @param texture_path Optional path override. If empty, uses stored texturePath
         */
        void Load(const std::string& texture_path = "")
        {
            // use the path from the parameter if its not empty
            texturePath = (!texture_path.empty()) ? texture_path : texturePath;

            // Verify texture is valid before using it
            if (!texture || texture->tex_id == 0)
            {
                std::stringstream log;
                log << "tileset is failing to get texture from path:(" << texturePath << ")";
                Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning, log.str());
            }
        }

        /**
         * @brief Get total number of tiles in the tileset
         * @return Total tile count (columns * rows)
         */
        int GetTileCount()
        {
            return columns * rows;
        }
    };

    /**
     * @brief Represents a single layer in a tilemap
     *
     * Each layer is an independent grid of tile indices that can be rendered
     * with different visual properties (tint, alpha, render order).
     */
    struct TileLayer
    {
        std::vector<int> tiles;
        std::string name;
        unsigned int width = 10;
        unsigned int height = 10;

        int renderOrder = 0;
        LayerMask renderLayer = RL_NONE;

        Vec3 tintColor = Vec3(1.0f, 1.0f, 1.0f);    // RGB multiplier
        float alpha = 1.0f;                         // Opacity

        bool locked = false;
    };

    /**
     * @brief Complete tilemap component containing all layers and tileset data
     *
     * This is the main component attached to entities that represent tilemaps.
     * It manages multiple layers, each with their own tile data and visual properties.
     */
    struct Tilemap
    {
        std::vector<TileLayer> layers;
        int mapWidth = 10;
        int mapHeight = 10;
        int tileSize = 16;

        // Tileset reference
        Tileset tileset;

        // Runtime settings (visible in inspector)
        std::vector<bool> layerVisibility;  // For each layer
        std::vector<std::string> layerNames;

        // Editor state
        bool isInEditMode = false;
        int activeLayerIndex = 0;

        /**
         * @brief Create a new tile layer and add it to the tilemap
         *
         * @param name Display name for the layer
         * @param width Grid width in tiles
         * @param height Grid height in tiles
         * @param sortingOrder Render order (lower renders first)
         */
        void CreateLayer(std::string name, unsigned int width, unsigned int height, int sortingOrder) 
        {
            TileLayer layer;

            layer.name = name;
            layer.width = width;
            layer.height = height;
            layer.tiles.resize(size_t(width * height), -1);
            layer.renderOrder = sortingOrder;

            layers.push_back(layer);
            layerVisibility.push_back(true);
            layerNames.push_back(name);
        }

        /**
         * @brief Remove a layer at the specified index
         *
         * Also removes corresponding entries from visibility and names arrays.
         *
         * @param index Layer index to remove
         */
        void RemoveLayer(int index)
        {
            if (index >= 0 && index < layers.size()) 
            {
                layers.erase(layers.begin() + index);
                layerVisibility.erase(layerVisibility.begin() + index);
                layerNames.erase(layerNames.begin() + index);
            }
        }

        /**
         * @brief Serialize tilemap data to JSON format
         *
         * Converts all tilemap data (dimensions, tileset, layers, settings) into
         * a RapidJSON value for saving to file.
         *
         * @param value Output JSON value to populate
         * @param allocator RapidJSON allocator for creating new values
         */
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
        {
            value.SetObject();

            // Serialize map dimensions
            value.AddMember("mapWidth", mapWidth, allocator);
            value.AddMember("mapHeight", mapHeight, allocator);
            value.AddMember("tileSize", tileSize, allocator);

            // Serialize tileset info
            if (!tileset.texturePath.empty())
            {
                rapidjson::Value tilesetObj(rapidjson::kObjectType);

                rapidjson::Value tilesetPath(tileset.texturePath.c_str(), allocator);

                tilesetObj.AddMember("texturePath", tilesetPath, allocator);

                tilesetObj.AddMember("tilesetWidth", tileset.tilesetWidth, allocator);
                tilesetObj.AddMember("tilesetHeight", tileset.tilesetHeight, allocator);

                tilesetObj.AddMember("columns", tileset.columns, allocator);  
                tilesetObj.AddMember("rows", tileset.rows, allocator);      

                value.AddMember("tileset", tilesetObj, allocator);
            }

            // Serialize layers array
            rapidjson::Value layersArray(rapidjson::kArrayType);
            for (const auto& layer : layers)
            {
                rapidjson::Value layerObj(rapidjson::kObjectType);

                // Layer properties
                rapidjson::Value layerName(layer.name.c_str(), allocator);
                layerObj.AddMember("name", layerName, allocator);
                layerObj.AddMember("width", layer.width, allocator);
                layerObj.AddMember("height", layer.height, allocator);
                layerObj.AddMember("renderLayer", layer.renderLayer, allocator);
                layerObj.AddMember("renderOrder", layer.renderOrder, allocator);

                rapidjson::Value tintArray(rapidjson::kArrayType);
                tintArray.PushBack(layer.tintColor.x, allocator);
                tintArray.PushBack(layer.tintColor.y, allocator);
                tintArray.PushBack(layer.tintColor.z, allocator);
                value.AddMember("tintColor", tintArray, allocator);
                value.AddMember("alpha", layer.alpha, allocator);

                // Serialize tiles array
                rapidjson::Value tilesArray(rapidjson::kArrayType);
                for (int tile : layer.tiles)
                {
                    tilesArray.PushBack(tile, allocator);
                }
                layerObj.AddMember("tiles", tilesArray, allocator);

                layersArray.PushBack(layerObj, allocator);
            }
            value.AddMember("layers", layersArray, allocator);

            // Serialize layer visibility
            rapidjson::Value visibilityArray(rapidjson::kArrayType);
            for (bool visible : layerVisibility)
            {
                visibilityArray.PushBack(visible, allocator);
            }
            value.AddMember("layerVisibility", visibilityArray, allocator);

            // Serialize layer names (redundant with layer.name, but keeping for inspector)
            rapidjson::Value namesArray(rapidjson::kArrayType);
            for (const auto& name : layerNames)
            {
                rapidjson::Value nameVal(name.c_str(), allocator);
                namesArray.PushBack(nameVal, allocator);
            }
            value.AddMember("layerNames", namesArray, allocator);
        }

        /**
         * @brief Deserialize tilemap data from JSON format
         *
         * Loads all tilemap data from a RapidJSON value, typically read from a file.
         * Clears existing data before loading.
         *
         * @param value JSON value containing serialized tilemap data
         */
        void Deserialize(const rapidjson::Value& value)
        {
            // Clear existing data
            layers.clear();
            layerVisibility.clear();
            layerNames.clear();

            // Deserialize map dimensions
            if (value.HasMember("mapWidth") && value["mapWidth"].IsInt())
                mapWidth = value["mapWidth"].GetInt();

            if (value.HasMember("mapHeight") && value["mapHeight"].IsInt())
                mapHeight = value["mapHeight"].GetInt();

            if (value.HasMember("tileSize") && value["tileSize"].IsInt())
                tileSize = value["tileSize"].GetInt();

            // Deserialize tileset info
            if (value.HasMember("tileset") && value["tileset"].IsObject())  // Fixed: check for tileset object
            {
                const rapidjson::Value& tilesetObj = value["tileset"];

                if (tilesetObj.HasMember("texturePath") && tilesetObj["texturePath"].IsString())
                {
                    tileset.texturePath = tilesetObj["texturePath"].GetString();
                }

                if (tilesetObj.HasMember("tilesetWidth") && tilesetObj["tilesetWidth"].IsInt())
                    tileset.tilesetWidth = tilesetObj["tilesetWidth"].GetInt();

                if (tilesetObj.HasMember("tilesetHeight") && tilesetObj["tilesetHeight"].IsInt())
                    tileset.tilesetHeight = tilesetObj["tilesetHeight"].GetInt();

                if (tilesetObj.HasMember("columns") && tilesetObj["columns"].IsInt())
                    tileset.columns = tilesetObj["columns"].GetInt();

                if (tilesetObj.HasMember("rows") && tilesetObj["rows"].IsInt())
                    tileset.rows = tilesetObj["rows"].GetInt();
            }

            // Deserialize layers
            if (value.HasMember("layers") && value["layers"].IsArray())
            {
                const rapidjson::Value& layersArray = value["layers"];

                for (rapidjson::SizeType i = 0; i < layersArray.Size(); ++i)
                {
                    const rapidjson::Value& layerObj = layersArray[i];

                    TileLayer layer;

                    if (layerObj.HasMember("name") && layerObj["name"].IsString())
                        layer.name = layerObj["name"].GetString();

                    if (layerObj.HasMember("width") && layerObj["width"].IsUint())
                        layer.width = layerObj["width"].GetUint();

                    if (layerObj.HasMember("height") && layerObj["height"].IsUint())
                        layer.height = layerObj["height"].GetUint();

                    if (layerObj.HasMember("renderLayer") && layerObj["renderLayer"].IsUint())
                        layer.renderLayer = static_cast<LayerMask>(layerObj["renderLayer"].GetUint());  // Added cast

                    if (layerObj.HasMember("renderOrder") && layerObj["renderOrder"].IsInt())
                        layer.renderOrder = layerObj["renderOrder"].GetInt();

                    if (value.HasMember("tintColor") && value["tintColor"].IsArray())
                    {
                        const auto& tintArray = value["tintColor"].GetArray();
                        if (tintArray.Size() >= 3)
                        {
                            layer.tintColor.x = tintArray[0].GetFloat();
                            layer.tintColor.y = tintArray[1].GetFloat();
                            layer.tintColor.z = tintArray[2].GetFloat();
                        }
                    }

                    // Deserialize tiles array
                    if (layerObj.HasMember("tiles") && layerObj["tiles"].IsArray())
                    {
                        const rapidjson::Value& tilesArray = layerObj["tiles"];
                        layer.tiles.reserve(tilesArray.Size());

                        for (rapidjson::SizeType j = 0; j < tilesArray.Size(); ++j)
                        {
                            if (tilesArray[j].IsInt())
                                layer.tiles.push_back(tilesArray[j].GetInt());
                        }
                    }

                    layers.push_back(layer);
                }
            }

            // Deserialize layer visibility
            if (value.HasMember("layerVisibility") && value["layerVisibility"].IsArray())
            {
                const rapidjson::Value& visArray = value["layerVisibility"];

                for (rapidjson::SizeType i = 0; i < visArray.Size(); ++i)
                {
                    if (visArray[i].IsBool())
                        layerVisibility.push_back(visArray[i].GetBool());
                }
            }

            // Deserialize layer names
            if (value.HasMember("layerNames") && value["layerNames"].IsArray())
            {
                const rapidjson::Value& namesArray = value["layerNames"];

                for (rapidjson::SizeType i = 0; i < namesArray.Size(); ++i)
                {
                    if (namesArray[i].IsString())
                        layerNames.push_back(namesArray[i].GetString());
                }
            }

            // Ensure arrays are synchronized (in case of corrupted data)
            while (layerVisibility.size() < layers.size())
                layerVisibility.push_back(true);

            while (layerNames.size() < layers.size())
                layerNames.push_back("Layer " + std::to_string(layerNames.size()));
        }
    };
} // namespace Uma_ECS