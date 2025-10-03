/*!
\file   EngineConfigSerializer.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

/*!
\brief
Serializer for engine configuration settings such as screen resolution, window mode,
VSync, and window title.

Provides JSON-based save/load functionality, enabling configuration to persist across
engine runs. Uses RapidJSON for structured serialization and deserialization of
settings. Currently focused on loading engine configuration from external files to
initialize runtime parameters.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include <vector>
#include <string>
#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>   // pretty JSON output

namespace Uma_Engine
{
    class EngineConfigSerializer
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
            Debugger::Log(WarningLevel::eCritical, ss.str());
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

            Debugger::Log(WarningLevel::eCritical, ss.str());
        }
    };
}