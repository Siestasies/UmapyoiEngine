/*!
\file   Enemy.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines Enemy tag component with speed parameter for entity identification and behavior configuration.

Lightweight marker component used by systems to identify and process enemy entities distinctly from other entity types.
Includes JSON serialization/deserialization for mSpeed property via RapidJSON.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

namespace Uma_ECS
{
    struct Enemy
    {
        float mSpeed = 1.f;
        // currently empty, just to let coordinator to 
        // identify entity with this component to be the enemy

        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const //override
        {
            value.SetObject();

            value.AddMember("mSpeed", mSpeed, allocator);
        }

        // Deserialize from JSON
        void Deserialize(const rapidjson::Value& value) //override
        {
            mSpeed = value["mSpeed"].GetFloat();
        }
    };
}