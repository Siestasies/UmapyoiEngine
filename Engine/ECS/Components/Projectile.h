/*!
\file   Projectile.h // Changed from Player.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines Projectile component with damage, speed, and lifetime parameters. // Updated description

Marker component used by systems (e.g., MovementSystem, CollisionSystem) to process entities that behave as projectiles.
Properties include: damage on hit, movement speed, and time before automatic destruction.
Includes JSON serialization/deserialization for all properties. // Updated detail
Contains design notes for event-driven architecture... (Keep the design notes section if still relevant to the project structure)

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

namespace Uma_ECS
{
    struct Projectile
    {
        int mDamage = 10;
        float mSpeed = 70.f;
        bool mFadeOVerTime = false;
        float mLifeTime = 2.f;


        // currently empty, just to let coordinator to 
        // identify entity with this component to be the player

        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const //override
        {
            value.SetObject();

            value.AddMember("mDamage", mDamage, allocator);
            value.AddMember("mSpeed", mSpeed, allocator);
            value.AddMember("mFadeOVerTime", mFadeOVerTime, allocator);
            value.AddMember("mLifeTime", mLifeTime, allocator);
        }

        // Deserialize from JSON
        void Deserialize(const rapidjson::Value& value) //override
        {
            if (value.HasMember("mDamage"))
                mDamage = value["mDamage"].GetInt();

            if (value.HasMember("mSpeed"))
                mSpeed = value["mSpeed"].GetFloat();

            if (value.HasMember("mFadeOVerTime"))
                mFadeOVerTime = value["mFadeOVerTime"].GetBool();

            if (value.HasMember("mLifeTime"))
                mLifeTime = value["mLifeTime"].GetFloat();
        }
    };
}

// player continuous movement mouse input - states

// player skill (one time off) - event system handles -> check logic if ok -> send off event

// player hurt -> check validity -> sent off event

// ui skill icon -> subcribe to player skill -> catch the event and process -> (action updating cd visuals)

// player health bar -> subcribe to player health changes -> catch n update the health bar