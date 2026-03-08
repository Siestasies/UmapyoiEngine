#pragma once

#include "../Core/UITypes.h"

#include "rapidjson/document.h"
#include <string>
#include <vector>

namespace Uma_UI
{
    struct Dialogue
    {
        std::vector<DialogueSequence> sequences;

        const DialogueSequence* FindSequence(const std::string& seqId) const
        {
            for (const auto& seq : sequences)
                if (seq.id == seqId) return &seq;
            return nullptr;
        }

       void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const
        {
            out.SetObject();
            rapidjson::Value arr(rapidjson::kArrayType);
            for (const auto& seq : sequences)
            {
                rapidjson::Value seqVal;
                seq.Serialize(seqVal, alloc);
                arr.PushBack(seqVal, alloc);
            }
            out.AddMember("sequences", arr, alloc);
        }

        void Deserialize(const rapidjson::Value& in)
        {
            sequences.clear();
            if (in.HasMember("sequences") && in["sequences"].IsArray())
            {
                for (const auto& seqVal : in["sequences"].GetArray())
                {
                    DialogueSequence seq;
                    seq.Deserialize(seqVal);
                    sequences.push_back(seq);
                }
            }
        }
    };
}