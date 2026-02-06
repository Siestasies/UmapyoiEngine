/*!
\file   IMGUIEvents.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines UI-related events for handling IMGUI interactions within the Uma Engine.

Includes event types for common UI requests such as spawning or destroying entities,
component modifications, scene management actions, and querying active entities.
Each event encapsulates relevant data and uses priority levels to control event processing
within the IMGUI event handling system.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include <string>
#include <vector>

#include "EventType.h"
#include "../ECS/Core/Types.hpp"

namespace Uma_Engine
{
    class SpawnEntityRequestEvent : public Event
    {
    public:
        SpawnEntityRequestEvent() { priority = Priority::Normal; }
    };

    class ReturnSpawnedRequestEvent : public Event
    {
    public:
        Uma_ECS::Entity entity;
        ReturnSpawnedRequestEvent(Uma_ECS::Entity en) : entity(en) { priority = Priority::Normal; }
    };

    class DuplicateEntityRequestEvent : public Event
    {
    public:
        Uma_ECS::Entity entity;
        DuplicateEntityRequestEvent(Uma_ECS::Entity en) : entity(en) { priority = Priority::Normal; }
    };

    class ReturnDuplicatedRequestEvent : public Event
    {
    public:
        Uma_ECS::Entity entity;
        ReturnDuplicatedRequestEvent(Uma_ECS::Entity en) : entity(en) { priority = Priority::Normal; }
    };

    class DestroyEntityRequestEvent : public Event
    {
    public:
        DestroyEntityRequestEvent(Uma_ECS::Entity entityId) : entityId(entityId) { priority = Priority::High; }
    public:
        Uma_ECS::Entity entityId;
    };

    class StressTestRequestEvent : public Event
    {
    public:
        StressTestRequestEvent() { priority = Priority::High; }
    };

    class ShowEntityInVPRequestEvent : public Event
    {
    public:
        ShowEntityInVPRequestEvent() { priority = Priority::High; }
    };

    class ChangeEnemyRotRequestEvent : public Event
    {
    public:
        float rot;
        ChangeEnemyRotRequestEvent(float rot) : rot(rot) { priority = Priority::High; }
    };

    class ChangeEnemyXposRequestEvent : public Event
    {
    public:
        float xpos;
        ChangeEnemyXposRequestEvent(float xpos) : xpos(xpos) { priority = Priority::High; }
    };

    class LoadPrefabRequestEvent : public Event
    {
    public:
        std::string prefab_name;
        bool script;
        LoadPrefabRequestEvent(std::string name = "bird", bool s = true) : prefab_name(name), script(s) { priority = Priority::High; }
    };

    class SavePrefabRequestEvent : public Event
    {
    public:
        std::string prefab_name;
        Uma_ECS::Entity entityId;
        SavePrefabRequestEvent(std::string name, Uma_ECS::Entity ent) : prefab_name(name), entityId(ent) { priority = Priority::High; }
    }; 

    class PrefabSceneRequestEvent : public Event
    {
    public:
        std::string scene_name;
        PrefabSceneRequestEvent(std::string name) : scene_name(name) { priority = Priority::High; }
    };

    class ChangeEnemyScaleRequestEvent : public Event
    {
    public:
        float scale;
        ChangeEnemyScaleRequestEvent(float scale) : scale(scale) { priority = Priority::High; }
    };

    class ShowBBoxRequestEvent : public Event
    {
    public:
        bool show;
        ShowBBoxRequestEvent(bool show) : show(show) { priority = Priority::High; }
    };

    class CloneEntityRequestEvent : public Event
    {
    public:
        CloneEntityRequestEvent(Uma_ECS::Entity entityId) : entityId(entityId) { priority = Priority::High; }
    public:
        Uma_ECS::Entity entityId;
    };

    class QueryActiveEntitiesEvent : public Event
    {
    public:
        QueryActiveEntitiesEvent()
        {
            priority = Priority::Critical;
        }

    public:
        mutable int mActiveEntityCnt = 0;
        //mutable std::vector<Uma_ECS::Entity> entities;
    };

    class AddComponentRequestEvent : public Event
    {
    public:
        AddComponentRequestEvent(Uma_ECS::Entity entityId, std::string componentType) : entityId(entityId), componentType(componentType) { priority = Priority::Normal; }

    public:
        Uma_ECS::Entity entityId;
        std::string componentType;
    };

    class RemoveComponentRequestEvent : public Event
    {
    public:
        RemoveComponentRequestEvent(Uma_ECS::Entity entityId, std::string componentType) : entityId(entityId), componentType(componentType) { priority = Priority::Normal; }

    public:
        Uma_ECS::Entity entityId;
        std::string componentType;
    };

    class ClearSceneRequestEvent : public Event
    {
    public:
        ClearSceneRequestEvent() { priority = Priority::High; }
    };

    class SaveSceneRequestEvent : public Event
    {
    public:
        SaveSceneRequestEvent() { priority = Priority::High; }
    };

    class LoadSceneRequestEvent : public Event
    {
    public:
        std::string name;
        bool load_n_play;
        LoadSceneRequestEvent(const std::string& name, bool load_n_play = false) : name(name), load_n_play(load_n_play){ priority = Priority::High; }
    };

    class UpdateImguiPlayModeEvent : public Event
    {
    public:
        bool isPlayMode;
        UpdateImguiPlayModeEvent(bool isPlay) : isPlayMode(isPlay) { priority = Priority::High; }
    };

    class ReLoadSceneRequestEvent : public Event
    {
    public:
        ReLoadSceneRequestEvent() { priority = Priority::High; }
    };

    class PlaySceneRequest : public Event
    {
    public:
        bool isPrevModePause;
        PlaySceneRequest(bool isPrevModePause = false) : isPrevModePause(isPrevModePause) { priority = Priority::High; }
    };

    class StopSceneRequest : public Event
    {
    public:
        StopSceneRequest() { priority = Priority::High; }
    };

    class PauseSceneRequest : public Event
    {
    public:
        PauseSceneRequest() { priority = Priority::High; }
    };

    class ResetSceneRequest : public Event
    {
    public:
        ResetSceneRequest(bool reset_n_play) : reset_n_play(reset_n_play) { priority = Priority::High; }

        bool reset_n_play;
    };

    class CreateNewSceneRequest : public Event
    {
    public:
        CreateNewSceneRequest() { priority = Priority::High; }
    };

    class DeleteCurrSceneRequest : public Event
    {
    public:
        std::string name;
        bool loadNewScene;
        DeleteCurrSceneRequest(std::string const& s, bool loadNewScene = true) : name(s), loadNewScene(loadNewScene) { priority = Priority::High; }
    };

    // save scene with the scene name
    class SaveSceneRequest : public Event
    {
    public:
        std::string name;
        SaveSceneRequest(std::string const& s) : name(s) { priority = Priority::High; }
    };


    // save current opened scene
    class SaveCurrSceneRequest : public Event
    {
    public:
        SaveCurrSceneRequest() { priority = Priority::High; }
    };

    class CallLuaToInitScript : public Event
    {
    public:
        Uma_ECS::Entity en;
        CallLuaToInitScript(Uma_ECS::Entity const& e) : en(e) { priority = Priority::High; }
    };

    class CallPathFindToBake : public Event
    {
    public:
        CallPathFindToBake() { priority = Priority::High; }
    };

    class SceneInfoRequest : public Event
    {
    public:
        SceneInfoRequest(const std::vector<std::string>& vec, const std::vector<std::string>& vec2, int x)
            : sceneNames(vec), scenePaths(vec2), activeSceneIndex(x) { priority = Priority::High; }
    public:
        std::vector<std::string> sceneNames;
        std::vector<std::string> scenePaths;
        int activeSceneIndex;
    };

    class RefreshDirectoryRequest : public Event
    {
    public:
        RefreshDirectoryRequest() { priority = Priority::High; }
    };
}