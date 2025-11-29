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

namespace Uma_ECS
{
    void ProjectileSystem::Update(float dt)
    {
        std::vector<Entity> entityToDestroy;

        for (auto const& entity : aEntities)
        {
            auto& projectile = pCoordinator->GetComponent<Projectile>(entity);

            if (projectile.mFadeOVerTime)
            {
                projectile.mLifeTime -= dt;

                if (projectile.mLifeTime <= 0)
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