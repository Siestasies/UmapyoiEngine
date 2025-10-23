/*!
\file   Coordinator.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Central facade class that unifies EntityManager, ComponentManager, and SystemManager into a single interface.

Provides the primary API for ECS operations: entity creation/destruction, component registration/manipulation,
and system registration with signature-based filtering.
Template methods handle component and system operations with automatic signature updates and system membership
recalculation. Implements ISerializer for JSON-based scene serialization with RapidJSON. Integrates with
Uma_Engine::EventSystem to emit entity lifecycle events for external observers.


All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include <memory>

#include "Types.hpp"
#include "ComponentManager.hpp"
#include "EntityManager.hpp"
#include "SystemManager.hpp"

#include "Core/BaseSerializer.h"

// Event system
#include "Core/EventSystem.h"
#include "Events/ECSEvents.h"

namespace Uma_ECS
{
    // this whole Corrdinator context is about combining:
    // Entity Manager, System Manager and Entity Manager 
    // into a single coordinator that can handles everything 
    // within this class
    class Coordinator : public Uma_Engine::ISerializer
    {
    public:
        void Init(Uma_Engine::EventSystem* eventSystem);

        // Entity functions

        Entity CreateEntity();

        void DestroyEntity(Entity entity);

        bool HasActiveEntity(Entity entity) const;

        Signature GetEntitySignature(Entity entity);

        int GetEntityCount() const;

        void DestroyAllEntities();

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

        //Serialization

        //void SerializeAllEntities(const std::string& filename);
        //void DeserializeAllEntities(const std::string& filename);

        const char* GetSectionName() const override { return "entities"; };  // e.g. "entities", "resources"
        std::string GetSerializerName() const override { return "coordinator"; };  // e.g. "coordinator", "resources_manager"
        void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) override;
        void Deserialize(const rapidjson::Value& in) override;
        void SerializePrefab(Entity entity, rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) override;
        void DeserializePrefab(const rapidjson::Value& in) override;

    private:
        std::unique_ptr<ComponentManager> aComponentManager;
        std::unique_ptr<EntityManager> aEntityManager;

        // we keep track of all entity systems here
        // so that we can update all systems when 
        // thr are changes for any entities
        std::unique_ptr<SystemManager> aSystemManager;

        Uma_Engine::EventSystem* pEventSystem = nullptr;
    };
}





