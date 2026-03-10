/*!
\file   Cutscene.h
\brief
Defines the Cutscene component for sequencing camera movements and dialogues
triggered by collider interactions.

Each cutscene contains a list of CutsceneAction steps that execute in order:
- SetCameraToPosition: snap camera to a world position
- LerpCameraToPosition: smoothly move camera to a world position over duration
- PlayDialogue: play a dialogue sequence (references Dialogue component)
- Wait: pause for a duration before next action
- ReturnCameraToPlayer: re-enable camera follow on player

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "Math/Math.h"
#include "rapidjson/document.h"
#include <string>
#include <vector>

namespace Uma_ECS
{
    enum class CutsceneActionType : int
    {
        SetCameraToPosition = 0,
        LerpCameraToPosition,
        PlayDialogue,
        Wait,
        ReturnCameraToPlayer,
        ShakeCamera
    };

    struct CutsceneAction
    {
        CutsceneActionType type = CutsceneActionType::PlayDialogue;
        Vec2 targetPosition{ 0.0f, 0.0f };
        float duration = 1.0f;
        std::string dialogueSequenceId;
        float shakeIntensity = 1.0f;

        void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const
        {
            out.SetObject();
            out.AddMember("type", static_cast<int>(type), alloc);
            out.AddMember("targetX", targetPosition.x, alloc);
            out.AddMember("targetY", targetPosition.y, alloc);
            out.AddMember("duration", duration, alloc);
            out.AddMember("dialogueSequenceId",
                rapidjson::Value(dialogueSequenceId.c_str(), alloc), alloc);
            out.AddMember("shakeIntensity", shakeIntensity, alloc);
        }

        void Deserialize(const rapidjson::Value& in)
        {
            type = static_cast<CutsceneActionType>(
                in.HasMember("type") ? in["type"].GetInt() : 0);
            targetPosition.x = in.HasMember("targetX") ? in["targetX"].GetFloat() : 0.0f;
            targetPosition.y = in.HasMember("targetY") ? in["targetY"].GetFloat() : 0.0f;
            duration = in.HasMember("duration") ? in["duration"].GetFloat() : 1.0f;
            dialogueSequenceId = in.HasMember("dialogueSequenceId")
                ? in["dialogueSequenceId"].GetString() : "";
            shakeIntensity = in.HasMember("shakeIntensity")
                ? in["shakeIntensity"].GetFloat() : 1.0f;
        }
    };

    struct Cutscene
    {
        bool playOnce = true;
        bool hasPlayed = false;
        std::vector<CutsceneAction> actions;

        void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const
        {
            out.SetObject();
            out.AddMember("playOnce", playOnce, alloc);
            out.AddMember("hasPlayed", hasPlayed, alloc);

            rapidjson::Value arr(rapidjson::kArrayType);
            for (const auto& action : actions)
            {
                rapidjson::Value actionVal;
                action.Serialize(actionVal, alloc);
                arr.PushBack(actionVal, alloc);
            }
            out.AddMember("actions", arr, alloc);
        }

        void Deserialize(const rapidjson::Value& in)
        {
            playOnce = in.HasMember("playOnce") ? in["playOnce"].GetBool() : true;
            hasPlayed = in.HasMember("hasPlayed") ? in["hasPlayed"].GetBool() : false;
            actions.clear();
            if (in.HasMember("actions") && in["actions"].IsArray())
            {
                for (const auto& actionVal : in["actions"].GetArray())
                {
                    CutsceneAction action;
                    action.Deserialize(actionVal);
                    actions.push_back(action);
                }
            }
        }
    };
}
