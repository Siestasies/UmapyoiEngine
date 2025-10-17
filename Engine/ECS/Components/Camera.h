/*!
\file   Camera.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines Camera component containing viewport zoom level and player following behavior flag.

Provides JSON serialization/deserialization for zoom and followPlayer properties via RapidJSON.
Used by CameraSystem to control camera positioning and tracking behavior.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "Math/Math.h"

namespace Uma_ECS
{
    struct Camera
    {
        float mZoom{}; // default to 1.0f
        bool followPlayer{};
        //float viewportWidth;
        //float viewportHeight;

        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const //override
        {
            value.SetObject();

            value.AddMember("zoom", mZoom, allocator);
            value.AddMember("followPlayer", followPlayer, allocator);
        }

        // Deserialize from JSON
        void Deserialize(const rapidjson::Value& value) //override
        {
            mZoom = value["zoom"].GetFloat();
            followPlayer = value["followPlayer"].GetBool();
        }
    };
}