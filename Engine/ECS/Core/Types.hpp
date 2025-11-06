/*!
\file   Types.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines core type aliases and constants for the Uma_ECS namespace.

Establishes Entity as unsigned int with MAX_ENTITIES capacity of 11,000.
ComponentType as unsigned int with MAX_COMPONENTS limit of 32 types.
Signature as std::bitset<MAX_COMPONENTS> for efficient component presence tracking.
Includes ECSErrorCode enum for error handling in debug and release builds.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

// this whole header file is just to declare the types we are using for Uma_ECS

#include <bitset>

namespace Uma_ECS
{
		//Uma_ECS Error code 
		enum class ECSErrorCode 
		{
				EC_None = 0,
				EC_EntityNotFound,
				EC_ComponentAlreadyExists,
				EC_ComponentNotFound,
				EC_ComponentLimitExceeded,
				EC_Unknown
		};

		// for component that requires layer
		// eg collider and renderer
		using LayerMask = unsigned int;

		enum RenderLayer : LayerMask
		{
				RL_NONE = 1 << 0,
				RL_WALL_TOP = 1 << 1,
				RL_FLOOR = 1 << 2,
				RL_ENV = 1 << 3,
				RL_ENEMY = 1 << 4,
				RL_PLAYER = 1 << 5,
				RL_WALL_BTM = 1 << 6,
				RL_UI = 1 << 7
		};

		enum CollisionLayer : LayerMask
		{
				CL_NONE = 0,
				CL_DEFAULT = 1 << 0,
				CL_PLAYER = 1 << 1,
				CL_ENEMY = 1 << 2,
				CL_WALL = 1 << 3,
				CL_PROJECTILE = 1 << 4,
				CL_PICKUP = 1 << 5,
				CL_ALL = 1 << 6
		};

		enum class ColliderPurpose
		{
				Physics = 0,      // Entity-to-entity (damage, interaction)
				Environment = 1,  // Entity-to-wall (movement blocking)
				Trigger = 2       // Non-blocking detection zones
		};
    
    // Uma_ECS
    using Entity = unsigned int;
    const Entity MAX_ENTITIES = 11000;
    using ComponentType = unsigned int;
    const ComponentType MAX_COMPONENTS = 32;


		
		// this is to track which components an entity has 
		using Signature = std::bitset<MAX_COMPONENTS>;
}
