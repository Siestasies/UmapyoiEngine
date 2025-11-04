/*!
\file   GameSceneScript.h
\par    Project: GAM200

\brief
Editor behavior script that handles all editor-specific functionality.
This replaces the old EditorScene class inheritance approach.
*/
#pragma once
#include "SceneType.h"
#include "Core/GameSerializer.h"
#include "Core/FilePaths.h"
#include <random>

namespace Uma_Engine
{
    class GameSceneScript : public SceneScript
    {
    public:
        GameSceneScript() : SceneScript("GameBehaviour") {}

        void OnAttach(Scene* scene) override
        {
            SceneScript::OnAttach(scene);
            m_CurrentSceneName = scene->GetFilePath();
            std::cout << "GameSceneScript attached" << std::endl;
        }

        void OnLoad() override
        {
            std::cout << "GameSceneScript: OnLoad" << std::endl;
        }

        void OnUnload() override
        {
            std::cout << "GameSceneScript: OnUnload" << std::endl;
        }

        void OnUpdate(float dt) override
        {
            UNREFERENCED_PARAMETER(dt);
        }

    private:
        std::string m_CurrentSceneName;
    };
}