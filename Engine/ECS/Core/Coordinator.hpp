#pragma once

#include <memory>

#include "Types.hpp"
#include "ComponentManager.hpp"
#include "EntityManager.hpp"
#include "SystemManager.hpp"

namespace Uma_ECS
{
    // this whole Corrdinator context is about combining:
    // Entity Manager, System Manager and Entity Manager 
    // into a single coordinator that can handles everything 
    // within this class
    class Coordinator
    {
    public:
        void Init();

        // Entity functions

        Entity CreateEntity();

        void DestroyEntity(Entity entity);

        Signature GetEntitySignature(Entity entity);

        int GetEntityCount() const;

        // Components functions

        template<typename T>
        void RegisterComponent() 
        {
            aComponentManager->RegisterComponent<T>();
        }

        template<typename T>
        void AddComponent(Entity entity, const T& component) 
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
        void RemoveComponent(Entity entity, const T& component)
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
        T& GetComponent(Entity entity)
        {
            return aComponentManager->GetComponent<T>(entity);
        }

        template<typename T>
        ComponentArray<T>& GetComponentArray()
        {
            return aComponentManager->GetComponentArray<T>();
        }

        template<typename T>
        ComponentType GetComponentType()
        {
            return aComponentManager->GetComponentType<T>();
        }

        // System functions

        template<typename T>
        std::shared_ptr<T> RegisterSystem()
        {
            return aSystemManager->RegisterSystem<T>();
        }

        template<typename T>
        void SetSystemSignature(Signature signature)
        {
            aSystemManager->SetSignature<T>(signature);
        }

        Entity DuplicateEntity(Entity src);

    private:
        std::unique_ptr<ComponentManager> aComponentManager;
        std::unique_ptr<EntityManager> aEntityManager;

        // we keep track of all entity systems here
        // so that we can update all systems when 
        // thr are changes for any entities
        std::unique_ptr<SystemManager> aSystemManager;
    };
}





