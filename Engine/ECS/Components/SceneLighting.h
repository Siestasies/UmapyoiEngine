/*!
\file   SceneLighting.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A

\author Javier Chua Dong Qing (100%)
\par    E-mail: javierdongqing.chua@digipen.edu
\par    DigiPen login: javierdongqing.chua

\brief
Global scene lighting settings component. Attach to one entity per scene
to control ambient darkness and other scene-wide lighting properties.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../../Math/Math.h"
#include <rapidjson/document.h>

namespace Uma_ECS
{
    struct SceneLighting
    {
        Vec3  ambientColor    = Vec3(0.4f, 0.4f, 0.45f);
        float ambientStrength = 1.0f;  // 0 = full darkness, 1 = ambient color as-is, >1 = brighter

        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
        {
            value.SetObject();

            rapidjson::Value col(rapidjson::kArrayType);
            col.PushBack(ambientColor.x, allocator);
            col.PushBack(ambientColor.y, allocator);
            col.PushBack(ambientColor.z, allocator);
            value.AddMember("ambientColor", col, allocator);
            value.AddMember("ambientStrength", ambientStrength, allocator);
        }

        void Deserialize(const rapidjson::Value& value)
        {
            if (value.HasMember("ambientColor") && value["ambientColor"].IsArray())
            {
                const auto& arr = value["ambientColor"].GetArray();
                if (arr.Size() >= 3)
                {
                    ambientColor.x = arr[0].GetFloat();
                    ambientColor.y = arr[1].GetFloat();
                    ambientColor.z = arr[2].GetFloat();
                }
            }
            if (value.HasMember("ambientStrength"))
                ambientStrength = value["ambientStrength"].GetFloat();
        }
    };
}
