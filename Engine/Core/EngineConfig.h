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

namespace Uma_Engine
{
    class EngineConfig : public ISerializer
    {
    public:
        // Screen settings
        int screenWidth = 1280;
        int screenHeight = 720;
        bool fullscreen = false;
        bool vsync = true;

        // Window settings
        std::string windowTitle = "My Game Engine";

        // Performance / timing
        //int targetFPS = 60;

        // Debug / development options
        //bool enableDebugOverlay = false;

        // serializer
        const char* GetSectionName() const override { return "engine_config"; };  // e.g. "entities", "resources"
        std::string GetSerializerName() const override { return "EngineConfig"; };  // e.g. "resources_manager", "coordinator"
        void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) override
        {
            out.SetObject();

            out.AddMember("screenWidth", screenWidth, allocator);
            out.AddMember("screenHeight", screenHeight, allocator);
            out.AddMember("fullscreen", fullscreen, allocator);
            out.AddMember("vsync", vsync, allocator);

            rapidjson::Value title;
            title.SetString(windowTitle.c_str(), static_cast<rapidjson::SizeType>(windowTitle.length()), allocator);
            out.AddMember("windowTitle", title, allocator);
        }
        void Deserialize(const rapidjson::Value& in) override
        {
            if (in.HasMember("screenWidth") && in["screenWidth"].IsInt())
                screenWidth = in["screenWidth"].GetInt();

            if (in.HasMember("screenHeight") && in["screenHeight"].IsInt())
                screenHeight = in["screenHeight"].GetInt();

            if (in.HasMember("fullscreen") && in["fullscreen"].IsBool())
                fullscreen = in["fullscreen"].GetBool();

            if (in.HasMember("vsync") && in["vsync"].IsBool())
                vsync = in["vsync"].GetBool();

            if (in.HasMember("windowTitle") && in["windowTitle"].IsString())
                windowTitle = in["windowTitle"].GetString();
        }

        void SerializePrefab(Entity entity, rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) override
        {
            // we are not using this function in Engine Config
            (void)entity;
            (void)out;
            (void)allocator;
        }

        void DeserializePrefab(const rapidjson::Value& in) override
        {
            // we are not using this function in Engine Config 
            (void)in;
        }
    };
}