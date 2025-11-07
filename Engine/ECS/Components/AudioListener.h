#pragma once

namespace Uma_ECS
{
	struct AudioListener {
		//empty on purpose this is js to tag the listener

        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const //override
        {
            (void)value;
            (void)allocator;
        }

        // Deserialize from JSON
        void Deserialize(const rapidjson::Value& value) //override
        {
            (void)value;
        }
	};
}