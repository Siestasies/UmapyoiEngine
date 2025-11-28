#pragma once
/*!
\file   ProjectileSystem.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief



All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"

namespace Uma_ECS
{
    class ProjectileSystem : public ECSSystem
    {
    public:

        inline void Init(Coordinator* c) { pCoordinator = c; }

        void Update(float dt);

    private:

        Coordinator* pCoordinator = nullptr;
    };
}