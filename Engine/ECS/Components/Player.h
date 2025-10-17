/*!
\file   Player.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines Player tag component with speed parameter for entity identification in game systems.

Marker component enabling systems to distinguish player entity from other entities for input handling and camera tracking.
Includes JSON serialization/deserialization for mSpeed property. Contains design notes for event-driven architecture
using state-based continuous input and event-based skill/damage systems with UI subscriptions.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

namespace Uma_ECS
{
    struct Player
    {
        float mSpeed = 1.f;
        // currently empty, just to let coordinator to 
        // identify entity with this component to be the player

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

// player continuous movement mouse input - states

// player skill (one time off) - event system handles -> check logic if ok -> send off event

// player hurt -> check validity -> sent off event

// ui skill icon -> subcribe to player skill -> catch the event and process -> (action updating cd visuals)

// player health bar -> subcribe to player health changes -> catch n update the health bar