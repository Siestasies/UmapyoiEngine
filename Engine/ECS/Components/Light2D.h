/*!
\file   Light2D.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A

\author Javier Chua Dong Qing (100%)
\par    E-mail: javierdongqing.chua@digipen.edu
\par    DigiPen login: javierdongqing.chua

\brief
Defines a 2D point light component with color, radius, intensity, and flicker properties.

Used by LightSystem to render a multiplicative light map over the scene.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../../Math/Math.h"
#include <rapidjson/document.h>

namespace Uma_ECS
{
    struct Light2D
    {
        Vec3  color         = Vec3(1.0f, 1.0f, 0.9f);
        float radius        = 5.0f;
        float intensity     = 1.0f;
        float innerRadius   = 0.3f;   // fraction of radius at full brightness (0-1)
        float flickerSpeed  = 0.0f;   // sin wave frequency (0 = no flicker)
        float flickerAmount = 0.0f;   // max intensity variation
        bool  enabled       = true;

        // Runtime only (not serialized)
        float flickerPhase  = 0.0f;

        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
        {
            value.SetObject();

            rapidjson::Value colorArr(rapidjson::kArrayType);
            colorArr.PushBack(color.x, allocator);
            colorArr.PushBack(color.y, allocator);
            colorArr.PushBack(color.z, allocator);
            value.AddMember("color", colorArr, allocator);

            value.AddMember("radius", radius, allocator);
            value.AddMember("intensity", intensity, allocator);
            value.AddMember("innerRadius", innerRadius, allocator);
            value.AddMember("flickerSpeed", flickerSpeed, allocator);
            value.AddMember("flickerAmount", flickerAmount, allocator);
            value.AddMember("enabled", enabled, allocator);
        }

        void Deserialize(const rapidjson::Value& value)
        {
            if (value.HasMember("color") && value["color"].IsArray())
            {
                const auto& arr = value["color"].GetArray();
                if (arr.Size() >= 3)
                {
                    color.x = arr[0].GetFloat();
                    color.y = arr[1].GetFloat();
                    color.z = arr[2].GetFloat();
                }
            }
            if (value.HasMember("radius"))        radius        = value["radius"].GetFloat();
            if (value.HasMember("intensity"))     intensity     = value["intensity"].GetFloat();
            if (value.HasMember("innerRadius"))   innerRadius   = value["innerRadius"].GetFloat();
            if (value.HasMember("flickerSpeed"))  flickerSpeed  = value["flickerSpeed"].GetFloat();
            if (value.HasMember("flickerAmount")) flickerAmount = value["flickerAmount"].GetFloat();
            if (value.HasMember("enabled"))       enabled       = value["enabled"].GetBool();
        }
    };
}
