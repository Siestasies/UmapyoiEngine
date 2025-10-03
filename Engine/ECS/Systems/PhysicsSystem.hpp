/*!
\file   PhysicsSystem.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines rigid body physics system that handles velocity integration, acceleration, and friction.

Inherits from ECSSystem and operates on entities with both Transform and RigidBody components.
Provides initialization with Coordinator reference, per-frame Update for physics calculations, and PrintLog for debugging.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"

namespace Uma_ECS
{
    class PhysicsSystem : public ECSSystem
    {
    public:

        inline void Init(Coordinator* c) { gCoordinator = c; }

        void Update(float dt);

        void PrintLog();

    private:

        Coordinator* gCoordinator = nullptr;
    };
}