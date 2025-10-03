#pragma once

#include "Core/BaseSerializer.h"
#include "Debugging/Debugger.hpp"

#include <vector>
#include <string>
#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>   // pretty JSON output

namespace Uma_Engine
{
    using Entity = unsigned int;

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

        void savePrefab(Entity entity, const std::string& filename)
        {
            rapidjson::Document doc;
            doc.SetObject();
            auto& allocator = doc.GetAllocator();

            for (auto* s : serializers)
            {
                if (s->GetSerializerName() == "coordinator")
                {
                    rapidjson::Value section(rapidjson::kObjectType);
                    s->SerializePrefab(entity, section, allocator);
                    doc.AddMember(rapidjson::StringRef("Prefab"), section, allocator);
                    break;
                }
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

        void loadPrefab(const std::string& filename)
        {
            std::ifstream ifs(filename);
            rapidjson::IStreamWrapper isw(ifs);
            rapidjson::Document doc;
            doc.ParseStream(isw);
            ifs.close();

            for (auto* s : serializers)
            {
                if (s->GetSerializerName() == "coordinator" && doc.HasMember("Prefab"))
                {
                    s->DeserializePrefab(doc["Prefab"]);
                }
            }

            std::string log;
            std::stringstream ss(log);

            ss << "Loaded from file : " << filename;

            Debugger::Log(WarningLevel::eInfo, ss.str());
        }
    };
}