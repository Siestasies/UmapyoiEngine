/*!
\file   GameSceneScript.h
\par    Project: GAM250

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
        /*!
        \brief Constructs the GameSceneScript with the name "GameBehaviour".
        */
        GameSceneScript() : SceneScript("GameBehaviour") {}

        /*!
        \brief Attaches this script to a scene and records the scene file path.
        \param scene Pointer to the scene being attached to.
        */
        void OnAttach(Scene* scene) override
        {
            SceneScript::OnAttach(scene);
            m_CurrentSceneName = scene->GetFilePath();
            std::cout << "GameSceneScript attached" << std::endl;
        }

        /*!
        \brief Called when the attached scene is loaded.
        */
        void OnLoad() override
        {
            std::cout << "GameSceneScript: OnLoad" << std::endl;
        }

        /*!
        \brief Called when the attached scene is unloaded.
        */
        void OnUnload() override
        {
            std::cout << "GameSceneScript: OnUnload" << std::endl;
        }

        /*!
        \brief Called every frame while the attached scene is running.
        \param dt Delta time in seconds since the last frame.
        */
        void OnUpdate(float dt) override
        {
            UNREFERENCED_PARAMETER(dt);
        }

    private:
        std::string m_CurrentSceneName;
    };
}