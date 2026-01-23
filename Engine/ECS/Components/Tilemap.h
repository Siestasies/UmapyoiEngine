#pragma once

#include "core/Types.hpp"
#include "../Systems/ResourcesTypes.hpp"
#include <string>
#include <vector>

namespace Uma_ECS
{
    struct TileLayer
    {
        std::vector<int> tiles;
        std::string name;
        unsigned int width;
        unsigned int height;
        int renderOrder = 0;
        LayerMask renderLayer = RL_NONE;
    };

    struct Tilemap
    {
        std::vector<TileLayer> layers;
        int mapWidth = 0;
        int mapHeight = 0;
        int tileSize = 16;

        // Tileset reference
        Uma_Engine::Texture tilesetTexture;
        int tilesetColumns = 0;
        int tilesetRows = 0;

        // Runtime settings (visible in inspector)
        std::vector<bool> layerVisibility;  // For each layer
        std::vector<std::string> layerNames;

        // Editor state
        bool isInEditMode = false;
        //int activeLayerIndex = 0;

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

        void RemoveLayer(int index)
        {
            if (index >= 0 && index < layers.size()) 
            {
                layers.erase(layers.begin() + index);
                layerVisibility.erase(layerVisibility.begin() + index);
                layerNames.erase(layerNames.begin() + index);
            }
        }

        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
        {
            value.SetObject();

            // Serialize map dimensions
            value.AddMember("mapWidth", mapWidth, allocator);
            value.AddMember("mapHeight", mapHeight, allocator);
            value.AddMember("tileSize", tileSize, allocator);

            // Serialize tileset info
            rapidjson::Value tilesetPath(tilesetTexture.filePath.c_str(), allocator);
            value.AddMember("tilesetPath", tilesetPath, allocator);
            value.AddMember("tilesetColumns", tilesetColumns, allocator);
            value.AddMember("tilesetRows", tilesetRows, allocator);

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
            if (value.HasMember("tilesetPath") && value["tilesetPath"].IsString())
            {
                std::string path = value["tilesetPath"].GetString();
                // You'll need to load the texture through your resource manager
                // tilesetTexture = ResourceManager::LoadTexture(path);
                tilesetTexture.filePath = path;
            }

            if (value.HasMember("tilesetColumns") && value["tilesetColumns"].IsInt())
                tilesetColumns = value["tilesetColumns"].GetInt();

            if (value.HasMember("tilesetRows") && value["tilesetRows"].IsInt())
                tilesetRows = value["tilesetRows"].GetInt();

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
                        layer.renderLayer = layerObj["renderLayer"].GetUint();

                    if (layerObj.HasMember("renderOrder") && layerObj["renderOrder"].IsInt())
                        layer.renderOrder = layerObj["renderOrder"].GetInt();

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