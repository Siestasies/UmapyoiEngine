/*!
\file   Coordinator.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Implements the Coordinator's entity lifecycle management, event emission, and serialization methods.

Coordinates entity creation/destruction across all three managers (Entity, Component, System) and emits corresponding events.
Handles entity duplication by cloning components and updating system membership.
Template methods handle component and system operations with automatic signature updates and system membership
recalculation. Implements ISerializer for JSON-based scene serialization with RapidJSON. Integrates with
Uma_Engine::EventSystem to emit entity lifecycle events for external observers.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "Coordinator.hpp"
#include "Events/IMGUIEvents.h"

#include "Debugging/Debugger.hpp"

#include <fstream>
#include <rapidjson/document.h>

namespace Uma_ECS
{
    void Coordinator::Init(Uma_Engine::EventSystem* eventSystem)
    {
        aComponentManager = std::make_unique<ComponentManager>();
        aEntityManager = std::make_unique<EntityManager>();
        aSystemManager = std::make_unique<SystemManager>();

        pEventSystem = eventSystem;
    }

    Entity Coordinator::CreateEntity()
    {
        Entity en = aEntityManager->CreateEntity();

        if (en >= 0) // entity is created
        {
            pEventSystem->Emit<Uma_Engine::EntityCreatedEvent>(en, GetEntityCount());
        }

        std::string log;
        std::stringstream ss(log);
        ss << "Created Entity : " << en;
        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo, ss.str());

        return en;
    }

    void Coordinator::DestroyEntity(Entity entity)
    {
        aEntityManager->DestroyEntity(entity);
        aComponentManager->EntityDestroyed(entity);
        aSystemManager->EntityDestroyed(entity);
        pEventSystem->Emit<Uma_Engine::EntityDestroyedEvent>(entity, GetEntityCount());

        std::string log;
        std::stringstream ss(log);
        ss << "Destroyed Entity : " << entity;
        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo, ss.str());
    }

    bool Coordinator::HasActiveEntity(Entity entity) const
    {
        return aEntityManager->HasActiveEntity(entity);
    }

    Signature Coordinator::GetEntitySignature(Entity entity)
    {
        return aEntityManager->GetSignature(entity);
    }

    int Coordinator::GetEntityCount() const
    {
        return aEntityManager->GetEntityCount();
    }

    Entity Coordinator::DuplicateEntity(Entity src)
    {
        Entity newEntity = CreateEntity();
        aComponentManager->CloneEntityComponents(src, newEntity);

        aSystemManager->EntitySignatureChanged(newEntity, GetEntitySignature(src));

        return newEntity;
    }

    void Coordinator::DestroyAllEntities()
    {
        std::vector<Entity> enList = aEntityManager->GetAllEntites();

        for (auto const& en : enList)
        {
            DestroyEntity(en);
        }

        std::string log;
        std::stringstream ss(log);
        ss << "Destroyed Entities : " << enList.size();
        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo, ss.str());
    }

    void Coordinator::Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator)
    {
        out.SetArray();

        // loop thru all entities
        for (const Entity& en : aEntityManager->GetAllEntites())
        {
            if (!aEntityManager->IsEntityActive(en)) continue;

            rapidjson::Value entityObj(rapidjson::kObjectType);
            entityObj.AddMember("id", en, allocator);

            rapidjson::Value comps(rapidjson::kObjectType);
            aComponentManager->SerializeAll(en, comps, allocator);
            entityObj.AddMember("components", comps, allocator);

            out.PushBack(entityObj, allocator);
        }
    }

    void Coordinator::Deserialize(const rapidjson::Value& in)
    {
        assert(in.IsArray());

        for (auto& entityVal : in.GetArray())
        {
            Entity entity = CreateEntity(); // new ID
            const auto& comps = entityVal["components"];
            Signature sign = aComponentManager->DeserializeAll(entity, comps);
            aEntityManager->SetSignature(entity, sign);
            aSystemManager->EntitySignatureChanged(entity, GetEntitySignature(entity));
        }

    }

    void Coordinator::SerializePrefab(Entity entity, rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator)
    {
        if (!aEntityManager->IsEntityActive(entity)) return;

        out.SetObject();
        
        out.AddMember("id", entity, allocator);

        rapidjson::Value comps(rapidjson::kObjectType);
        aComponentManager->SerializeAll(entity, comps, allocator);
        out.AddMember("components", comps, allocator);
    }

    void Coordinator::DeserializePrefab(const rapidjson::Value& in)
    {
        Entity entity = CreateEntity(); // new ID
        const auto& comps = in["components"];
        Signature sign = aComponentManager->DeserializeAll(entity, comps);
        aEntityManager->SetSignature(entity, sign);
        aSystemManager->EntitySignatureChanged(entity, GetEntitySignature(entity));
    }
}
