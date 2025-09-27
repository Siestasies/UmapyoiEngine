#pragma once

#include "RapidJSON/document.h"

namespace Uma_ECS
{
    class SerializationBase
    {
        virtual void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const = 0;

        // Deserialize from JSON
        virtual  void Deserialize(const rapidjson::Value& value) = 0;
    };
}