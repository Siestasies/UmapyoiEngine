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
    struct Tileset
    {
        // Tileset reference
        std::shared_ptr<Uma_Engine::Texture> texture = nullptr;
        std::string textureName{}; // will be depriciated later on
        std::string texturePath{};

        // currently not in use
        int tilesetWidth = 0;
        int tilesetHeight = 0;

        int columns = 0;
        int rows = 0;

        void GetUVs(Vec2& uvOffset, Vec2& uvSize, Vec2 cell) const
        {
            if (!IsLoaded()) 
            {
                return;
            }

            // Calculate size of one cell in UV space
            uvSize.x = 1.0f / columns;
            uvSize.y = 1.0f / rows;

            // Calculate offset for the specific cell
            uvOffset.x = cell.x * uvSize.x;
            uvOffset.y = cell.y * uvSize.y;
        }

        void GetUVs(int tileIndex, float& u0, float& v0, float& u1, float& v1) const 
        {
            if (!IsLoaded() || tileIndex < 0) {
                u0 = v0 = u1 = v1 = 0.0f;
                return;
            }

            int col = tileIndex % columns;
            int row = tileIndex / columns;

            u0 = (float)col / columns;
            v0 = (float)row / rows;
            u1 = (float)(col + 1) / columns;
            v1 = (float)(row + 1) / rows;
        }

        bool IsLoaded() const
        {
            return texture != nullptr;
        }

        void Load(Uma_Engine::ResourcesManager* pResourcesManager, const std::string& texture_path = "")
        {
            // use the path from the parameter if its not empty
            texturePath = (!texture_path.empty()) ? texture_path : texturePath;
            
            // texture name is depriciated need to chane this later on
            // WIP
            texture = pResourcesManager->GetTexture(textureName);

            // Verify texture is valid before using it
            if (!texture || texture->tex_id == 0)
            {
                std::stringstream log;
                log << "tileset is failing to get texture from path:(" << texturePath << ")";
                Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning, log.str());
            }
        }

        int GetTileCount()
        {
            return columns * rows;
        }
    };

    struct TileLayer
    {
        std::vector<int> tiles;
        std::string name;
        unsigned int width;
        unsigned int height;

        int renderOrder = 0;
        LayerMask renderLayer = RL_NONE;

        Vec3 tintColor = Vec3(1.0f, 1.0f, 1.0f);    // RGB multiplier
        float alpha = 1.0f;                         // Opacity

        bool locked = false;
    };

    struct Tilemap
    {
        std::vector<TileLayer> layers;
        int mapWidth = 0;
        int mapHeight = 0;
        int tileSize = 16;

        // Tileset reference
        Tileset tileset;

        // Runtime settings (visible in inspector)
        std::vector<bool> layerVisibility;  // For each layer
        std::vector<std::string> layerNames;

        // Editor state
        bool isInEditMode = false;
        int activeLayerIndex = 0;

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
            if (!tileset.textureName.empty())
            {
                rapidjson::Value tilesetObj(rapidjson::kObjectType);

                rapidjson::Value tilesetName(tileset.textureName.c_str(), allocator);
                rapidjson::Value tilesetPath(tileset.texturePath.c_str(), allocator);

                tilesetObj.AddMember("textureName", tilesetName, allocator);
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

                if (tilesetObj.HasMember("textureName") && tilesetObj["textureName"].IsString())
                    tileset.textureName = tilesetObj["textureName"].GetString();

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