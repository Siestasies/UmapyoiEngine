#include "ProjectileSystem.hpp"

namespace Uma_ECS
{
    void ProjectileSystem::Update(float dt)
    {
        auto& rbArray = pCoordinator->GetComponentArray<RigidBody>();
        auto& tfArray = pCoordinator->GetComponentArray<Transform>();
        auto& pArray = pCoordinator->GetComponentArray<Projectile>();

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