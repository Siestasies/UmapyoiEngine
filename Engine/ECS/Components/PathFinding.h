#pragma once
#include <vector>
#include "../pathfinding/NavMesh.hpp"
#include "../Math/Vector.h"

namespace Uma_ECS{
	using Vec2 = Uma_Math::Vector2D<float>;

	struct PathFinding {

		float pathUpdateTimer = 0.0f;
		float pathUpdateInterval = 0.5f;

		//run time info
		std::vector<Vec2> path;
		unsigned int pathIndex = 0;
		bool hasValidPath = false;
		bool reachedGoal = false;

		Vec2 goal{ 0.f,0.f };

		void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const //override
		{
			value.SetObject();

			value.AddMember("pathUpdateTimer", pathUpdateTimer, allocator);
			value.AddMember("pathUpdateInterval", pathUpdateInterval, allocator);
		}

		// Deserialize from JSON
		void Deserialize(const rapidjson::Value& value) //override
		{
			pathUpdateTimer = value["pathUpdateTimer"].GetFloat();
			pathUpdateInterval = value["pathUpdateInterval"].GetFloat();
		}
	};

	
}