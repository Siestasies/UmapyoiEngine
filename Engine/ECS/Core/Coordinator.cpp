#include "Coordinator.hpp"
#include "Core/IMGUIEvents.h"

#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>   // pretty JSON output

namespace Uma_ECS
{
    void Coordinator::Init(Uma_Engine::EventSystem* eventSystem)
    {
        aComponentManager = std::make_unique<ComponentManager>();
        aEntityManager = std::make_unique<EntityManager>();
        aSystemManager = std::make_unique<SystemManager>();

        pEventSystem = eventSystem;

        pEventSystem->Subscribe<Uma_Engine::QueryActiveEntitiesEvent>([this](const Uma_Engine::QueryActiveEntitiesEvent& e) { e.mActiveEntityCnt = GetEntityCount(); });
        pEventSystem->Subscribe<Uma_Engine::CloneEntityRequestEvent>([this](const Uma_Engine::CloneEntityRequestEvent& e) { if (HasActiveEntity(e.entityId)) DuplicateEntity(e.entityId); });
        pEventSystem->Subscribe<Uma_Engine::DestroyEntityRequestEvent>([this](const Uma_Engine::DestroyEntityRequestEvent& e) { if (HasActiveEntity(e.entityId)) DestroyEntity(e.entityId); });

        pEventSystem->Subscribe<Uma_Engine::SaveSceneRequestEvent>([this](const Uma_Engine::SaveSceneRequestEvent& e) { SerializeAllEntities(e.filepath); });
        pEventSystem->Subscribe<Uma_Engine::LoadSceneRequestEvent>([this](const Uma_Engine::LoadSceneRequestEvent& e) { DeserializeAllEntities(e.filepath); });
        pEventSystem->Subscribe<Uma_Engine::ClearSceneRequestEvent>([this](const Uma_Engine::ClearSceneRequestEvent& e) { DestroyAllEntities(); });
    }

    Entity Coordinator::CreateEntity()
    {
        Entity en = aEntityManager->CreateEntity();

        if (en >= 0) // entity is created
        {
            pEventSystem->Emit<Uma_Engine::EntityCreatedEvent>(en);
        }

        return en;
    }

    void Coordinator::DestroyEntity(Entity entity)
    {
        pEventSystem->Emit<Uma_Engine::EntityDestroyedEvent>(entity);
        aEntityManager->DestroyEntity(entity);
        aComponentManager->EntityDestroyed(entity);
        aSystemManager->EntityDestroyed(entity);
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

    void Coordinator::SerializeAllEntities(const std::string& filename)
    {
        rapidjson::Document doc;
        doc.SetObject();
        auto& allocator = doc.GetAllocator();

        rapidjson::Value entities(rapidjson::kArrayType);

        // loop thru all entities
        for (const Entity& en : aEntityManager->GetAllEntites())
        {
            if (!aEntityManager->IsEntityActive(en)) continue;

            rapidjson::Value entityObj(rapidjson::kObjectType);
            entityObj.AddMember("id", en, allocator);

            rapidjson::Value comps(rapidjson::kObjectType);
            aComponentManager->SerializeAll(en, comps, allocator);
            entityObj.AddMember("components", comps, allocator);

            entities.PushBack(entityObj, allocator);
        }

        doc.AddMember("entities", entities, allocator);

        // write to file
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        //writer.SetIndent(' ', 4); // 4 spaces per indent
        writer.SetMaxDecimalPlaces(3);
        doc.Accept(writer);

        std::ofstream ofs(filename);
        ofs << buffer.GetString();
        ofs.close();
    }
    void Coordinator::DeserializeAllEntities(const std::string& filename)
    {
        std::ifstream ifs(filename);
        rapidjson::IStreamWrapper isw(ifs);
        rapidjson::Document doc;
        doc.ParseStream(isw);
        ifs.close();

        const auto& entities = doc["entities"];
        for (auto& entityVal : entities.GetArray())
        {
            Entity entity = aEntityManager->CreateEntity(); // new ID

            const auto& comps = entityVal["components"];
            Signature sign = aComponentManager->DeserializeAll(entity, comps);

            aEntityManager->SetSignature(entity, sign);

            aSystemManager->EntitySignatureChanged(entity, GetEntitySignature(entity));
        }
    }

    void Coordinator::DestroyAllEntities()
    {
        std::vector<Entity> enList = aEntityManager->GetAllEntites();

        for (auto const& en : enList)
        {
            DestroyEntity(en);
        }
    }
}
