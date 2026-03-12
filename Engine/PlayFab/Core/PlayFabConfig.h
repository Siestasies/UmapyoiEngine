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

        /*!
        \brief Returns the JSON section name used for serialization.
        \return C-string section name "Playfab_config".
        */
        const char* GetSectionName() const override { return "Playfab_config"; };

        /*!
        \brief Returns the serializer identifier name.
        \return String serializer name "Playfab_config".
        */
        std::string GetSerializerName() const override { return "Playfab_config"; };

        /*!
        \brief Serializes PlayFab configuration fields to a JSON value.
        \param out JSON value to populate with config data.
        \param allocator RapidJSON allocator for memory management.
        */
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
        /*!
        \brief Deserializes PlayFab configuration fields from a JSON value.
        \param in JSON value containing the config data to read.
        */
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

        /*!
        \brief Serializes a prefab entity. Not used for engine config.
        \param entity Entity to serialize (unused).
        \param out JSON value output (unused).
        \param allocator RapidJSON allocator (unused).
        */
        void SerializePrefab(Entity entity, rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) override
        {
            // we are not using this function in Engine Config
            (void)entity;
            (void)out;
            (void)allocator;
        }

        /*!
        \brief Deserializes a prefab entity. Not used for engine config.
        \param in JSON value input (unused).
        \return Invalid entity sentinel value.
        */
        Entity DeserializePrefab(const rapidjson::Value& in) override
        {
            // we are not using this function in Engine Config 
            (void)in;
            return static_cast<Entity>(-1);
        }
    };
}