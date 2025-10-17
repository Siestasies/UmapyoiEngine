/*!
\file   BaseSerializer.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines ISerializer interface for polymorphic serialization of engine subsystems to JSON format.

Requires derived classes to provide section name identifier and implement serialize/deserialize methods using RapidJSON.
Enables GameSerializer to orchestrate multi-system persistence by treating all subsystems uniformly through this interface.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "RapidJSON/document.h"

#include <string>

namespace Uma_Engine
{
    using Entity = unsigned int;

    class ISerializer {
    public:
        virtual const char* GetSectionName() const = 0;  // e.g. "entities", "resources"
        virtual std::string GetSerializerName() const = 0;  // e.g. "entities", "resources"

        virtual void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) = 0;
        virtual void Deserialize(const rapidjson::Value& in) = 0;

        virtual void SerializePrefab(Entity entity, rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) = 0;
        virtual void DeserializePrefab(const rapidjson::Value& in) = 0;

        virtual ~ISerializer() = default;
    };
}