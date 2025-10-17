/*!
\file   System.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines the base ECSSystem class that serves as the parent for all game logic systems.

Contains a vector of Entity IDs representing entities that match the system's component signature.
Systems automatically receive entity additions/removals based on signature matching performed by SystemManager.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

//#include "../Core/Coordinator.h"
#include "../Core/Types.hpp"
#include <vector>

namespace Uma_ECS
{
    class ECSSystem
    {
    public:

        std::vector<Entity> aEntities;

    };
}
