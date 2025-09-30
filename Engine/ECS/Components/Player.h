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