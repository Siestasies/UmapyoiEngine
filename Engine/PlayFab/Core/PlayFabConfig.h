/*!
\file   PlayFabConfig.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

/*!
\brief
A container to store playfab configs

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once
#include <string>

#include "BaseSerializer.h"
#include "FilePaths.h"
#include "Systems/Window.hpp"

namespace Uma_Engine
{
    class   PlayFabConfig : public ISerializer
    {
    public:
        std::string titleId;
        std::string secretKey;
        std::string customId;

        bool isAdmin;

        // Debug / development options
        //bool enableDebugOverlay = false;

        // serializer
        const char* GetSectionName() const override { return "Playfab_config"; };  // e.g. "entities", "resources"
        std::string GetSerializerName() const override { return "Playfab_config"; };  // e.g. "resources_manager", "coordinator"
        void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) override
        {
            out.SetObject();

            rapidjson::Value title;
            title.SetString(titleId.c_str(), static_cast<rapidjson::SizeType>(titleId.length()), allocator);
            out.AddMember("titleId", title, allocator);

            rapidjson::Value key;
            key.SetString(secretKey.c_str(), static_cast<rapidjson::SizeType>(secretKey.length()), allocator);
            out.AddMember("secretKey", key, allocator);

            rapidjson::Value custom;
            custom.SetString(customId.c_str(), static_cast<rapidjson::SizeType>(customId.length()), allocator);
            out.AddMember("customId", custom, allocator);
        }
        void Deserialize(const rapidjson::Value& in) override
        {
            if (in.HasMember("titleId") && in["titleId"].IsString())
                titleId = in["titleId"].GetString();

            if (in.HasMember("secretKey") && in["secretKey"].IsString())
                secretKey = in["secretKey"].GetString();

            if (in.HasMember("customId") && in["customId"].IsString())
                customId = in["customId"].GetString();

            isAdmin = secretKey.empty() == false;
        }

        void SerializePrefab(Entity entity, rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) override
        {
            // we are not using this function in Engine Config
            (void)entity;
            (void)out;
            (void)allocator;
        }

        Entity DeserializePrefab(const rapidjson::Value& in) override
        {
            // we are not using this function in Engine Config 
            (void)in;
            return static_cast<Entity>(-1);
        }
    };
}