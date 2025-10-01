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