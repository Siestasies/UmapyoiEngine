/*!
\file   ProjectileSystem.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Projectile processing system responsible for updating projectile lifetime,
handling fade-over-time behavior, and destroying expired projectile entities.

This system iterates over all entities containing a Projectile component,
decrements their lifetime when fade-over-time is active, and marks them for
destruction when their lifetime reaches zero. Destroyed projectiles are removed
along with any children entities using the ECS coordinator.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/


#include "ProjectileSystem.hpp"

#include "Events/CollisionEvent.h"

namespace Uma_ECS
{

    void ProjectileSystem::Init(Coordinator* c, Uma_Engine::EventSystem* es)
    {
        pCoordinator = c;
        pEventSystem = es;

        pEventSystem->Subscribe<Uma_Engine::OnTriggerEnterEvent, ProjectileSystem>([this](const Uma_Engine::OnTriggerEnterEvent& e)
            {
                if (!pCoordinator || aEntities.empty()) return;

                if (!pCoordinator->HasComponent<Projectile>(e.entityA) && !pCoordinator->HasComponent<Projectile>(e.entityB)) return;

                Entity self = (pCoordinator->HasComponent<Projectile>(e.entityB)) ? e.entityB : e.entityA;
                Entity trigger = (self == e.entityA) ? e.entityB : e.entityA;

                HandleCollision(self, trigger);
            });
        pEventSystem->Subscribe<Uma_Engine::OnTriggerEvent, ProjectileSystem>([this](const Uma_Engine::OnTriggerEvent& e)
            {
                (void)e;
                // do nth yet
            });
        pEventSystem->Subscribe<Uma_Engine::OnTriggerExitEvent, ProjectileSystem>([this](const Uma_Engine::OnTriggerExitEvent& e)
            {
                (void)e;
                // do nth yet
            });
    }

    void ProjectileSystem::HandleCollision(Entity self, Entity trigger)
    {
        auto& colliderB = pCoordinator->GetComponent<Collider>(trigger);

        if (colliderB.GetPrimaryShape().layer == CL_WALL)
        {
            pCoordinator->DestroyEntityAndChildren(self);
        }
    }

    void ProjectileSystem::Shutdown()
    {
        pEventSystem->UnsubscribeSystem<ProjectileSystem>();
    }

    void ProjectileSystem::Update(float dt)
    {
        std::vector<Entity> entityToDestroy;

        for (auto const& entity : aEntities)
        {
            auto& projectile = pCoordinator->GetComponent<Projectile>(entity);

            if (projectile.mStats.fadeOVerTime)
            {
                projectile.mStats.lifeTime -= dt;

                if (projectile.mStats.lifeTime <= 0)
                {
                    entityToDestroy.push_back(entity);
                }
            }
        }

        if (!entityToDestroy.empty())
        {
            std::for_each(std::begin(entityToDestroy), std::end(entityToDestroy), [&](const Entity& e)
                {
                    pCoordinator->DestroyEntityAndChildren(e);
                });
        }
    }
}