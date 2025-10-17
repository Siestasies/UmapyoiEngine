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

#include "WIP_Scripts/EditorScene.h"
#include "WIP_Scripts/TestScene2.h"

namespace Uma_Engine
{
    class SceneManager : public ISystem
    {
        private:
            std::unordered_map<std::string, std::unique_ptr<Scene>> scenes;
            Scene* activeScene = nullptr;

            // inherit from isystem
            void Init() override
            {
                std::cout << "Scene Manager INIT" << std::endl;

                // create a TestScene and store it as a unique_ptr<Scene>
                std::unique_ptr<EditorScene> testScene1 = std::make_unique<EditorScene>(pSystemManager);

                AddScene("testScene1", std::move(testScene1));

                SetActiveScene("testScene1");
            }
            void Update(float dt) override
            {
                if (activeScene)
                {
                    activeScene->Update(dt);
                    activeScene->Render();
                }
                if (scenes.empty())
                {
                    std::cout << "Scene Manager MAP is EMPTY" << std::endl;
                }
            }
            void Shutdown() override
            {
                std::cout << "Scene Manager SHUTDOWN" << std::endl;
                
                // Unload the active scene
                // other scenes should already be unloaded
                if (activeScene)
                {
                    activeScene->OnUnload();
                    activeScene = nullptr;
                }
                // clear scenes map
                scenes.clear();
            }

        public:
            void AddScene(const std::string& name, std::unique_ptr<Scene> scene)
            {
                scenes[name] = std::move(scene);
            }

            void SetActiveScene(const std::string& name)
            {
                if (activeScene)
                    activeScene->OnUnload();
                activeScene = scenes[name].get();
                activeScene->OnLoad();
            }
    };
}