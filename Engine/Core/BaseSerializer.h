#pragma once

#include "RapidJSON/document.h"

namespace Uma_Engine
{
    using Entity = unsigned int;

    class ISerializer {
    public:
        virtual const char* GetSectionName() const = 0;  // e.g. "entities", "resources"
        virtual const char* GetSerializerName() const = 0;  // e.g. "entities", "resources"

        virtual void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) = 0;
        virtual void Deserialize(const rapidjson::Value& in) = 0;

        virtual void SerializePrefab(Entity entity, rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) = 0;
        virtual void DeserializePrefab(const rapidjson::Value& in) = 0;

        virtual ~ISerializer() = default;
    };
}