/*!
\file   EngineConfig.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

/*!
\brief
A container to store configs

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
    /*!
     * \class EngineConfig
     * \brief Stores engine configuration settings and implements JSON serialization.
     */
    class EngineConfig : public ISerializer
    {
    public:
        // Screen settings
        int screenWidth = 1600;
        int screenHeight = 900;
        bool fullscreen = true;
        bool vsync = true;
        int fps = 60;

        // Window settings
        std::string windowTitle = "My Game Engine";

        /*!
         * \brief Returns the window mode derived from the fullscreen setting.
         * \return WindowMode::Fullscreen or WindowMode::Windowed.
         */
        WindowMode GetWindowMode() const
        {
            return fullscreen ? WindowMode::Fullscreen : WindowMode::Windowed;
        }

        // Performance / timing
        // Physics timing
        float fixedTimeStep = 1.0f / 60.0f;  // 60 FPS physics
        float maxFrameTime = 0.25f;           // Cap to prevent spiral of death
        int maxPhysicsSteps = 5;              // Max physics updates per frame

        // Debug / development options
        //bool enableDebugOverlay = false;

        /*! \brief Returns the JSON section name for this serializer. \return Section name string. */
        const char* GetSectionName() const override { return "engine_config"; };
        /*! \brief Returns a human-readable name for this serializer. \return Serializer name string. */
        std::string GetSerializerName() const override { return "EngineConfig"; };

        /*!
         * \brief Serializes engine configuration to a JSON value.
         * \param out Output JSON value to populate.
         * \param allocator RapidJSON allocator for memory management.
         */
        void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) override
        {
            out.SetObject();

            out.AddMember("screenWidth", screenWidth, allocator);
            out.AddMember("screenHeight", screenHeight, allocator);
            out.AddMember("fps", fps, allocator);
            out.AddMember("fullscreen", fullscreen, allocator);
            out.AddMember("vsync", vsync, allocator);

            rapidjson::Value title;
            title.SetString(windowTitle.c_str(), static_cast<rapidjson::SizeType>(windowTitle.length()), allocator);
            out.AddMember("windowTitle", title, allocator);
        }
        /*!
         * \brief Deserializes engine configuration from a JSON value.
         * \param in Input JSON value to read from.
         */
        void Deserialize(const rapidjson::Value& in) override
        {
            if (in.HasMember("screenWidth") && in["screenWidth"].IsInt())
                screenWidth = in["screenWidth"].GetInt();

            if (in.HasMember("screenHeight") && in["screenHeight"].IsInt())
                screenHeight = in["screenHeight"].GetInt();

            if (in.HasMember("fps"))
                fps = in["fps"].GetInt();

            if (in.HasMember("fullscreen") && in["fullscreen"].IsBool())
                fullscreen = in["fullscreen"].GetBool();

            if (in.HasMember("vsync") && in["vsync"].IsBool())
                vsync = in["vsync"].GetBool();

            if (in.HasMember("windowTitle") && in["windowTitle"].IsString())
                windowTitle = in["windowTitle"].GetString();
        }

        /*!
         * \brief Prefab serialization stub (not used for engine config).
         * \param entity Unused entity parameter.
         * \param out Unused output parameter.
         * \param allocator Unused allocator parameter.
         */
        void SerializePrefab(Entity entity, rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) override
        {
            // we are not using this function in Engine Config
            (void)entity;
            (void)out;
            (void)allocator;
        }

        /*!
         * \brief Prefab deserialization stub (not used for engine config).
         * \param in Unused input parameter.
         * \return Invalid entity ID.
         */
        Entity DeserializePrefab(const rapidjson::Value& in) override
        {
            // we are not using this function in Engine Config 
            (void)in;
            return static_cast<Entity>(-1);
        }
    };
}