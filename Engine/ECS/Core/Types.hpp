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
    
    // Uma_ECS
    using Entity = unsigned int;
    const Entity MAX_ENTITIES = 11000;
    using ComponentType = unsigned int;
    const ComponentType MAX_COMPONENTS = 32;
		
		// this is to track which components an entity has 
		using Signature = std::bitset<MAX_COMPONENTS>;
}
