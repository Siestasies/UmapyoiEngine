/*!
\file   Camera.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (Everything else)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\co-author Javier Chua Dong Qing (Screenshake implementation)
\par       E-mail: javierdongqing.chua@digipen.edu
\par       DigiPen login: javierdongqing.chua

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
        float mZoom{10.f}; // default to 10.0f
        bool followPlayer{};
        //float viewportWidth;
        //float viewportHeight;
        Vec2 mShakeOffset{ 0.0f, 0.0f };
        float mShakeTimer{ 0.0f };
        float mShakeIntensity{ 0.0f };

        /*!
        \brief Serializes the camera configuration to a JSON object.
        \note Screen shake variables are runtime-only and are not saved.
        \param value The JSON object to write to.
        \param allocator The JSON allocator.
        */
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const //override
        {
            value.SetObject();

            value.AddMember("zoom", mZoom, allocator);
            value.AddMember("followPlayer", followPlayer, allocator);
        }

        /*!
        \brief Deserializes the camera configuration from a JSON object.
        \param value The JSON object to read from.
        */
        void Deserialize(const rapidjson::Value& value) //override
        {
            mZoom = value["zoom"].GetFloat();
            followPlayer = value["followPlayer"].GetBool();
        }
    };
}