/*!
\file   Projectile.h
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
    enum ProjectileType
    {
        P_AOE,
        P_SINGLE
    };

    struct ProjectileStats
    {
        ProjectileType type;
        int damage = 10;
        float speed = 70.f;
        bool fadeOVerTime = false;
        float lifeTime = 2.f;

    };

    struct Projectile
    {
        ProjectileStats mStats;


        // currently empty, just to let coordinator to 
        // identify entity with this component to be the player

        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const //override
        {
            value.SetObject();

            rapidjson::Value stats(rapidjson::kObjectType);

            stats.AddMember("type", mStats.type, allocator);
            stats.AddMember("damage", mStats.damage, allocator);
            stats.AddMember("speed", mStats.speed, allocator);
            stats.AddMember("fadeOVerTime", mStats.fadeOVerTime, allocator);
            stats.AddMember("lifeTime", mStats.lifeTime, allocator);

            value.AddMember("mStats", stats, allocator);
        }

        // Deserialize from JSON
        void Deserialize(const rapidjson::Value& value) //override
        {
            if (value.HasMember("mStats") && value["mStats"].IsObject())
            {
                const rapidjson::Value& stats = value["mStats"];

                if (stats.HasMember("type") && stats["type"].IsInt())
                    mStats.type = static_cast<ProjectileType>(stats["type"].GetInt());

                if (stats.HasMember("damage") && stats["damage"].IsInt())
                    mStats.damage = stats["damage"].GetInt();

                if (stats.HasMember("speed") && stats["speed"].IsNumber())
                    mStats.speed = stats["speed"].GetFloat();

                if (stats.HasMember("fadeOVerTime") && stats["fadeOVerTime"].IsBool())
                    mStats.fadeOVerTime = stats["fadeOVerTime"].GetBool();

                if (stats.HasMember("lifeTime") && stats["lifeTime"].IsNumber())
                    mStats.lifeTime = stats["lifeTime"].GetFloat();
            }
        }
    };
}