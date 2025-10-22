/*!
\file   SceneManager.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Shahir Rasid (100%)
\par    E-mail: b.muhammadshahir@digipen.edu
\par    DigiPen login: b.muhammadshahir

\brief
This file implements the definition for a Scene manager which stores
and controls the life-cycle of a scene.
Also contains helper functions to add and set active scene.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/
#pragma once
#include "SceneType.h"
#include "Core/SystemType.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>

//#include "WIP_Scripts/EditorScene.h"
//#include "WIP_Scripts/TestScene2.h"

namespace Uma_Engine
{
    class SceneManager : public ISystem
    {
    public:
        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;

        std::shared_ptr<Scene> CreateScene(const std::string& name);
        std::shared_ptr<Scene> LoadScene(const std::string& filepath, bool additive);
        void LoadSceneAsync(const std::string& filepath, bool additive);
        void UnloadScene(const std::string& name);
        void UnloadAllScenes();

        void SetActiveScene(const std::string& name);
        std::shared_ptr<Scene> GetScene(const std::string& name);
        bool IsSceneLoaded(const std::string& name) const;
        std::vector<std::string> GetAvailableScenes() const;
        void UpdateLoadingScenes();
        void RemoveUnloadedScenes();

    private:
        std::unordered_map<std::string, std::shared_ptr<Scene>> m_Scenes;
        std::unordered_map<std::string, std::shared_ptr<Scene>> m_LoadedScenes;
        std::shared_ptr<Scene> m_ActiveScene;
    };
}