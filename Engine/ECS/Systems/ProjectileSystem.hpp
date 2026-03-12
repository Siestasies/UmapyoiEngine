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
Declares the ProjectileSystem, responsible for managing projectile-related
behaviors such as updating projectile movement, handling lifetimes, and
processing interactions (e.g., collisions) within the ECS framework.

The system inherits from ECSSystem and operates on entities that contain
components relevant to projectile behavior. It also provides initialization
using a Coordinator reference and per-frame projectile updates.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"

namespace Uma_ECS
{
    /*!
    \class ProjectileSystem
    \brief Handles all projectile logic, including movement and lifetime updates.
    */
    class ProjectileSystem : public ECSSystem
    {
    public:

        /*!
        \brief Initializes the system with a Coordinator reference.
        \param c Pointer to the ECS Coordinator.
        */
        void Init(Coordinator* c, Uma_Engine::EventSystem* es);

        /*!
        \brief Updates all projectile entities each frame.
        \param dt Delta time for frame-based updates.
        */
        void Update(float dt);

        /*!
        \brief Shuts down the projectile system and releases resources.
        */
        void Shutdown();

    private:

        /*!
        \brief Handles collision response when a projectile hits a trigger entity.
        \param self The projectile entity.
        \param trigger The entity that triggered the collision.
        */
        void HandleCollision(Entity self, Entity trigger);

        Coordinator* pCoordinator = nullptr; //!< Pointer to ECS Coordinator.
        Uma_Engine::EventSystem* pEventSystem = nullptr;
    };
}
