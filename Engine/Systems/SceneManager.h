#pragma once
#include "SceneType.h"
#include "Core/SystemType.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>

#include "TestScene.h"
#include "TestScene2.h"

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

                // Create a TestScene and store it as a unique_ptr<Scene>
                std::unique_ptr<TestScene> testScene1 = std::make_unique<TestScene>();
                std::unique_ptr<TestScene2> testScene2 = std::make_unique<TestScene2>();

                // Add TestScene as a unique_ptr<Scene> to the scenes map
                AddScene("testScene1", std::move(testScene1));
                AddScene("testScene2", std::move(testScene2));

                // Optionally, set it as the active scene
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

                static float fulltime = 0.f;
                fulltime += dt;

                if (fulltime > 5.f)
                {
                    SetActiveScene("testScene2");
                    fulltime = 0.f;
                }
            }
            void Shutdown() override
            {
                std::cout << "Scene Manager SHUTDOWN" << std::endl;
            }

        public:
            void AddScene(const std::string& name, std::unique_ptr<Scene> scene)
            {
                scenes[name] = std::move(scene);
            }

            void SetActiveScene(const std::string& name)
            {
                if (activeScene) activeScene->OnUnload();
                activeScene = scenes[name].get();
                activeScene->OnLoad();
            }
    };
}