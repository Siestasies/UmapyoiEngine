/*!
\file   Dialogue.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines the Dialogue UI component that holds a collection of dialogue sequences
for an entity, with lookup by sequence ID and JSON serialization support.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../Core/UITypes.h"

#include "rapidjson/document.h"
#include <string>
#include <vector>

namespace Uma_UI
{
    /*!
     * \struct Dialogue
     * \brief UI component holding a collection of dialogue sequences for an entity.
     */
    struct Dialogue
    {
        std::vector<DialogueSequence> sequences;

        /*!
         * \brief Finds a dialogue sequence by its ID.
         * \param seqId The ID of the sequence to find.
         * \return Pointer to the matching sequence, or nullptr if not found.
         */
        const DialogueSequence* FindSequence(const std::string& seqId) const
        {
            for (const auto& seq : sequences)
                if (seq.id == seqId) return &seq;
            return nullptr;
        }

       /*!
        * \brief Serializes all dialogue sequences to a JSON value.
        * \param out Output JSON value to populate.
        * \param alloc RapidJSON allocator for memory management.
        */
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

        /*!
         * \brief Deserializes all dialogue sequences from a JSON value.
         * \param in Input JSON value to read from.
         */
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