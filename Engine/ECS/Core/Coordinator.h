#pragma once

#include <memory>

#include "Types.h"
#include "ComponentManager.h"
#include "EntityManager.h"
#include "SystemManager.h"

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

        Entity CreateEntity();

        void DestroyEntity(Entity entity);

        // Components functions
        template<typename T>
        void RegisterComponent();

        template<typename T>
        void AddComponent(Entity entity, const T& component);

        template<typename T>
        void RemoveComponent(Entity entity, const T& component);

        template<typename T>
        T& GetComponent(Entity entity);

        template<typename T>
        ComponentType GetComponentType();

        // System functions

        template<typename T>
        std::shared_ptr<T> RegisterSystem();

        template<typename T>
        void SetSystemSignature(Signature signature);


    private:
        std::unique_ptr<ComponentManager> aComponentManager;
        std::unique_ptr<EntityManager> aEntityManager;
        std::unique_ptr<SystemManager> aSystemManager;
    };
}





