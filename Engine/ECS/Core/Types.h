#pragma once

// this whole header file is just to declare the types we are using for ECS

#include <bitset>

namespace ECS
{
		//ECS Error code 
		enum class ECSErrorCode 
		{
				EC_None = 0,
				EC_EntityNotFound,
				EC_ComponentAlreadyExists,
				EC_ComponentNotFound,
				EC_ComponentLimitExceeded,
				EC_Unknown
		};
    
    // ECS
    using Entity = unsigned int;
    const Entity MAX_ENTITIES = 5000;
    using ComponentType = unsigned int;
    const ComponentType MAX_COMPONENTS = 32;
		
		// this is to track which components an entity has 
		using Signature = std::bitset<MAX_COMPONENTS>; 
}
