#include "Coordinator.hpp"

namespace Uma_ECS
{
    void Coordinator::Init()
    {
        aComponentManager = std::make_unique<ComponentManager>();
        aEntityManager = std::make_unique<EntityManager>();
        aSystemManager = std::make_unique<SystemManager>();
    }

    Entity Coordinator::CreateEntity()
    {
        return aEntityManager->CreateEntity();
    }

    void Coordinator::DestroyEntity(Entity entity)
    {
        aEntityManager->DestroyEntity(entity);
        aComponentManager->EntityDestroyed(entity);
        aSystemManager->EntityDestroyed(entity);
    }

    Signature Coordinator::GetEntitySignature(Entity entity)
    {
        return aEntityManager->GetSignature(entity);
    }

}
