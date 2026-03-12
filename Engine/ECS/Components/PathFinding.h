#pragma once
/*!
\file   PathFinding.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Koh Kai Yang (100%)
\par    E-mail: k.kaiyang@digipen.edu
\par    DigiPen login: k.kaiyang

\brief
Adds the component to the entity to allow for pathfinding

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include <vector>
//#include "../pathfinding/NavMesh.hpp"
#include "../Math/Vector.h"

namespace Uma_ECS{
	using Vec2 = Uma_Math::Vector2D<float>;

	struct PathFinding {

		float pathUpdateTimer = 0.0f;
		float pathUpdateInterval = 0.5f;

		float worldScale = 100.f;
		//run time info
		std::vector<Vec2> path;
		unsigned int pathIndex = 0;
		bool hasValidPath = false;
		bool reachedGoal = false;

		Vec2  lastGoal;
		bool  haveLastGoal = false;

		Vec2 goal{ 0.f,0.f };
		bool enabled = true;

		/*!
		\brief Serialize pathfinding data to JSON, including update timer and interval.
		\param value Output JSON value to populate.
		\param allocator RapidJSON allocator for creating new values.
		*/
		void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const //override
		{
			value.SetObject();

			value.AddMember("pathUpdateTimer", pathUpdateTimer, allocator);
			value.AddMember("pathUpdateInterval", pathUpdateInterval, allocator);
		}

		/*!
		\brief Deserialize pathfinding data from JSON, restoring update timer and interval.
		\param value JSON value containing serialized pathfinding data.
		*/
		void Deserialize(const rapidjson::Value& value) //override
		{
			pathUpdateTimer = value["pathUpdateTimer"].GetFloat();
			pathUpdateInterval = value["pathUpdateInterval"].GetFloat();
		}
	};

	
}