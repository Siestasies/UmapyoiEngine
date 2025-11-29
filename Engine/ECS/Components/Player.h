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
    enum AnimatorState
    {
        PS_Idle,
        PS_Run,
        PS_Atk_1,
        PS_Atk_2,
        PS_Hurt,
        PS_Die
    };

    struct CombatState
    {
        bool attack_1_is_in_cd = false;
        bool attack_2_is_in_cd = false;

        float attack_1_cd;
        float attack_2_cd;

        float attack_1_cd_curr;
        float attack_2_cd_curr;
    };

    struct Player
    {
        int     mHealth = 100;
        int     mMaxHealth = 100;
        float   mHealthRegenRate = 1.f;

        float   mSpeed = 50.f;
        float   mDashSpeed = 10.f;
        float   mDashCD = 2.f;

        int     mAttackDamage = 10;
        float   mAttackSpeed = 1.f;
        float   mAttackRange = 20.f;
        int     mDefense = 5;

        int     mMana = 100;
        int     mMaxMana = 100;
        float   mManaRegenRate = 5.f;

        // runtime data
        CombatState combatState;
        AnimatorState animatorState;


        // currently empty, just to let coordinator to 
        // identify entity with this component to be the player

        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const //override
        {
            value.SetObject();

            value.AddMember("mHealth", mHealth, allocator);
            value.AddMember("mMaxHealth", mMaxHealth, allocator);
            value.AddMember("mHealthRegenRate", mHealthRegenRate, allocator);

            value.AddMember("mSpeed", mSpeed, allocator);
            value.AddMember("mDashSpeed", mDashSpeed, allocator);
            value.AddMember("mDashCD", mDashCD, allocator);

            value.AddMember("mAttackDamage", mAttackDamage, allocator);
            value.AddMember("mAttackSpeed", mAttackSpeed, allocator);
            value.AddMember("mAttackRange", mAttackRange, allocator);
            value.AddMember("mDefense", mDefense, allocator);

            value.AddMember("mMana", mMana, allocator);
            value.AddMember("mMaxMana", mMaxMana, allocator);
            value.AddMember("mManaRegenRate", mManaRegenRate, allocator);
        }

        // Deserialize from JSON
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

            if (value.HasMember("mDashSpeed"))
                mDashSpeed = value["mDashSpeed"].GetFloat();

            if (value.HasMember("mDashCD"))
                mDashCD = value["mDashCD"].GetFloat();


            if (value.HasMember("mAttackDamage"))
                mAttackDamage = value["mAttackDamage"].GetInt();

            if (value.HasMember("mAttackSpeed"))
                mAttackSpeed = value["mAttackSpeed"].GetFloat();

            if (value.HasMember("mAttackRange"))
                mAttackRange = value["mAttackRange"].GetFloat();

            if (value.HasMember("mDefense"))
                mDefense = value["mDefense"].GetInt();


            if (value.HasMember("mMana"))
                mMana = value["mMana"].GetInt();

            if (value.HasMember("mMaxMana"))
                mMaxMana = value["mMaxMana"].GetInt();

            if (value.HasMember("mManaRegenRate"))
                mManaRegenRate = value["mManaRegenRate"].GetFloat();

            combatState.attack_1_is_in_cd = false;
            combatState.attack_2_is_in_cd = false;

            combatState.attack_1_cd = 1.f / mAttackSpeed;
            combatState.attack_2_cd = 1.f / mAttackSpeed;

            combatState.attack_1_cd_curr = 0.f;
            combatState.attack_2_cd_curr = 0.f;

            animatorState = PS_Idle;
        }
    };
}

// player continuous movement mouse input - states

// player skill (one time off) - event system handles -> check logic if ok -> send off event

// player hurt -> check validity -> sent off event

// ui skill icon -> subcribe to player skill -> catch the event and process -> (action updating cd visuals)

// player health bar -> subcribe to player health changes -> catch n update the health bar