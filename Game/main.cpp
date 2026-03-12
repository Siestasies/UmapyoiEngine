/*!
\file   main.cpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Entry point for the standalone game build (no editor).
Initializes GameApplication, runs the game loop, and shuts down the engine
after execution.

This file represents the final packaged game runtime, without any editor
interfaces or development tools.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "GameApplication.h"
//#include "../Engine/Core/SystemManager.h"
//#include "../Engine/Systems/SceneManager.h"
#include "FilePaths.h"
#include <iostream>

int main()
{
    Uma_Engine::GameApplication app;

    //app.GetSystemManager()->GetSystem<Uma_Engine::SceneManager>()->LoadScene(Uma_FilePath::SCENES_DIR + "test_default.scn");

    if (!app.Init())
    {
        std::cerr << "Failed to initialize game application!" << std::endl;
        return -1;
    }

    app.Run();
    app.Shutdown();

    std::cout << "Game closed" << std::endl;
    return 0;
}