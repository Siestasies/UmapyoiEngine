#pragma once

#include "Core/BaseSerializer.h"
#include "Debugging/Debugger.hpp"

#include <vector>
#include <string>
#include <fstream>


//#if defined(_MSC_VER)
//#  pragma warning(push)
//#  pragma warning(disable : 26495)
//#endif

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>   // pretty JSON output

//// if you need internal stuff
//#include <rapidjson/internal/ieee754.h>
//
//#if defined(_MSC_VER)
//#  pragma warning(pop)
//#endif

namespace Uma_Engine
{
    class GameSerializer
    {
    private:
        std::vector<ISerializer*> serializers;

    public:

        void Register(ISerializer* s) 
        {
            serializers.push_back(s);
        }

        void save(const std::string& filename)
        {
            rapidjson::Document doc;
            doc.SetObject();
            auto& allocator = doc.GetAllocator();

            for (auto* s : serializers) 
            {
                rapidjson::Value section(rapidjson::kObjectType);
                s->Serialize(section, allocator);
                doc.AddMember(rapidjson::StringRef(s->GetSectionName()), section, allocator);
            }

            // write to file
            rapidjson::StringBuffer buffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
            //writer.SetIndent(' ', 4); // 4 spaces per indent
            writer.SetMaxDecimalPlaces(3);
            doc.Accept(writer);
            std::ofstream ofs(filename);
            ofs << buffer.GetString();
            ofs.close();

            std::string log;
            std::stringstream ss(log);
            ss << "Saved to file : " << filename;
            Debugger::Log(WarningLevel::eInfo, ss.str());
        }

        void load(const std::string& filename)
        {
            std::ifstream ifs(filename);
            rapidjson::IStreamWrapper isw(ifs);
            rapidjson::Document doc;
            doc.ParseStream(isw);
            ifs.close();

            for (auto* s : serializers)
            {
                if (doc.HasMember(s->GetSectionName())) 
                {
                    s->Deserialize(doc[s->GetSectionName()]);
                }
            }

            std::string log;
            std::stringstream ss(log);

            ss << "Loaded from file : " << filename;

            Debugger::Log(WarningLevel::eInfo, ss.str());
        }
    };

    //void Coordinator::SerializeAllEntities(const std::string& filename)
 //{
 //    rapidjson::Document doc;
 //    doc.SetObject();
 //    auto& allocator = doc.GetAllocator();
 //    rapidjson::Value entities(rapidjson::kArrayType);
 //    // loop thru all entities
 //    for (const Entity& en : aEntityManager->GetAllEntites())
 //    {
 //        if (!aEntityManager->IsEntityActive(en)) continue;
 //        rapidjson::Value entityObj(rapidjson::kObjectType);
 //        entityObj.AddMember("id", en, allocator);
 //        rapidjson::Value comps(rapidjson::kObjectType);
 //        aComponentManager->SerializeAll(en, comps, allocator);
 //        entityObj.AddMember("components", comps, allocator);
 //        entities.PushBack(entityObj, allocator);
 //    }
 //    doc.AddMember("entities", entities, allocator);
 //    // write to file
 //    rapidjson::StringBuffer buffer;
 //    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
 //    //writer.SetIndent(' ', 4); // 4 spaces per indent
 //    writer.SetMaxDecimalPlaces(3);
 //    doc.Accept(writer);
 //    std::ofstream ofs(filename);
 //    ofs << buffer.GetString();
 //    ofs.close();
 //}
 //void Coordinator::DeserializeAllEntities(const std::string& filename)
 //{
 //    std::ifstream ifs(filename);
 //    rapidjson::IStreamWrapper isw(ifs);
 //    rapidjson::Document doc;
 //    doc.ParseStream(isw);
 //    ifs.close();
 //    const auto& entities = doc["entities"];
 //    for (auto& entityVal : entities.GetArray())
 //    {
 //        Entity entity = aEntityManager->CreateEntity(); // new ID
 //        const auto& comps = entityVal["components"];
 //        Signature sign = aComponentManager->DeserializeAll(entity, comps);
 //        aEntityManager->SetSignature(entity, sign);
 //        aSystemManager->EntitySignatureChanged(entity, GetEntitySignature(entity));
 //    }
 //}
}