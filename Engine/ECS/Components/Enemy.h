/*!
\file   Enemy.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

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
        int mHealth = 100;
        int mMaxHealth = 100;
        float mHealthRegenRate = 1.f;

        float mSpeed = 10.f;

        int mAttackDamage = 10;
        float mAttackSpeed = 1.f;
        float mAttackRange = 20.f;
        int mDefense = 5;

        Vec2 dir = 180;     //what direction it is looking at
        float mFoV = 40;    //the field of view of enemy

        // currently empty, just to let coordinator to 
        // identify entity with this component to be the enemy

        /*!
        \brief Serialize enemy data to JSON, including health, speed, and combat stats.
        \param value Output JSON value to populate.
        \param allocator RapidJSON allocator for creating new values.
        */
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const //override
        {
            value.SetObject();

            value.AddMember("mHealth", mHealth, allocator);
            value.AddMember("mMaxHealth", mMaxHealth, allocator);
            value.AddMember("mHealthRegenRate", mHealthRegenRate, allocator);

            value.AddMember("mSpeed", mSpeed, allocator);

            value.AddMember("mAttackDamage", mAttackDamage, allocator);
            value.AddMember("mAttackSpeed", mAttackSpeed, allocator);
            value.AddMember("mAttackRange", mAttackRange, allocator);
            value.AddMember("mDefense", mDefense, allocator);
        }

        /*!
        \brief Deserialize enemy data from JSON, restoring health, speed, and combat stats.
        \param value JSON value containing serialized enemy data.
        */
        void Deserialize(const rapidjson::Value& value) //override
        {
            if (value.HasMember("mHealth"))
                mHealth = value["mHealth"].GetInt();

            if (value.HasMember("mMaxHealth"))
                mMaxHealth = value["mMaxHealth"].GetInt();

            if (value.HasMember("mHealthRegenRate"))
                mHealthRegenRate = value["mHealthRegenRate"].GetFloat();


            if (value.HasMember("mSpeed"))
                mSpeed = value["mSpeed"].GetFloat();


            if (value.HasMember("mAttackDamage"))
                mAttackDamage = value["mAttackDamage"].GetInt();

            if (value.HasMember("mAttackSpeed"))
                mAttackSpeed = value["mAttackSpeed"].GetFloat();

            if (value.HasMember("mAttackRange"))
                mAttackRange = value["mAttackRange"].GetFloat();

            if (value.HasMember("mDefense"))
                mDefense = value["mDefense"].GetInt();
        }
    };
}