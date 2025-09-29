#pragma once

#include "RapidJSON/document.h"

namespace Uma_Engine
{
    class ISerializer {
    public:
        virtual const char* GetSectionName() const = 0;  // e.g. "entities", "resources"
        virtual void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) = 0;
        virtual void Deserialize(const rapidjson::Value& in) = 0;
        virtual ~ISerializer() = default;
    };
}