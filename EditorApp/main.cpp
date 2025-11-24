/*!
\file   main.cpp
\brief  Entry point for the UmapyoiEditor application

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "EditorApplication.h"
#include <iostream>

int main()
{
    Uma_Engine::EditorApplication app;

    if (!app.Init())
    {
        std::cerr << "Failed to initialize editor application!" << std::endl;
        return -1;
    }

    app.Run();
    app.Shutdown();

    std::cout << "Editor closed" << std::endl;
    return 0;
}
