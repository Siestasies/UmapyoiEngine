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
Defines the PhysicsSystem responsible for rigid-body physics updates.
This includes velocity integration, position updates, force application,
and interpolation support during fixed-timestep simulation.

The system inherits from ECSSystem and operates on entities that contain
both Transform and RigidBody components. It provides initialization using
a Coordinator reference, per-frame Update for physics calculations, and
debug logging support through PrintLog().

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

        // this have to be done before updating in the fixed timestamp
        void SavePrevPos();
        void ApplyVelocity(float dt); // Apply velocities to positions

        void PrintLog();

        void AddForce(Entity entity, Vec2 pos, Vec2 dir, float force, float rotation);

    private:

        Coordinator* gCoordinator = nullptr;
    };
}