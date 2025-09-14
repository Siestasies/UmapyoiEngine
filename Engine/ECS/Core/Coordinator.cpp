#include "Coordinator.h"

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

    template<typename T>
    void Coordinator::RegisterComponent()
    {
        aComponentManager->RegisterComponent<T>();
    }

    template<typename T>
    void Coordinator::AddComponent(Entity entity, const T& component)
    {
        // add component
        aComponentManager->AddComponent<T>(entity, component);

        // get curr signature of the entity 
        // set the bitset of the component to true
        // update the entity manager and system manager
        auto signature = aEntityManager->GetSignature(entity);
        signature.set(aComponentManager->GetComponentType<T>(), true);

        aEntityManager->SetSignature(entity, signature);

        aSystemManager->EntitySignatureChanged(entity, signature);
    }

    template<typename T>
    void Coordinator::RemoveComponent(Entity entity, const T& component)
    {
        // remove component
        aComponentManager->RemoveComponent<T>(entity, component);

        // get curr signature of the entity 
        // set the bitset of the component to false
        // update the entity manager and system manager
        auto signature = aEntityManager->GetSignature(entity);
        signature.set(aComponentManager->GetComponentType<T>(), false);

        aEntityManager->SetSignature(entity, signature);

        aSystemManager->EntitySignatureChanged(entity, signature);
    }

    template<typename T>
    T& Coordinator::GetComponent(Entity entity)
    {
        return aComponentManager->GetComponent<T>(entity);
    }

    template<typename T>
    ComponentType Coordinator::GetComponentType()
    {
        return aComponentManager->GetComponentType<T>();
    }

    // System functions

    template<typename T>
    std::shared_ptr<T> Coordinator::RegisterSystem()
    {
        aSystemManager->RegisterSystem<T>();
    }

    template<typename T>
    void Coordinator::SetSystemSignature(Signature signature)
    {
        aSystemManager->SetSignature<T>(signature);
    }
}
