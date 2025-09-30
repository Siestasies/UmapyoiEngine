#include "Coordinator.hpp"
#include "Core/IMGUIEvents.h"

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
            pEventSystem->Emit<Uma_Engine::EntityCreatedEvent>(en);
        }

        //std::string log;
        //std::stringstream ss(log);
        //ss << "Created Entity : " << en;
        //Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo, ss.str());

        return en;
    }

    void Coordinator::DestroyEntity(Entity entity)
    {
        pEventSystem->Emit<Uma_Engine::EntityDestroyedEvent>(entity);
        aEntityManager->DestroyEntity(entity);
        aComponentManager->EntityDestroyed(entity);
        aSystemManager->EntityDestroyed(entity);

        //std::string log;
        //std::stringstream ss(log);
        //ss << "Destroyed Entity : " << entity;
        //Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo, ss.str());
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
}
