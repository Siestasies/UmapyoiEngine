#pragma once
/*!
\file   DropCallback.hpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author     Lai Jun Siang (100%)
\par        E-mail: lai.j@digipen.edu
\par        DigiPen login: lai.j

\brief
Implements the FileDropHandler class, which processes file drop events coming
from the operating system through GLFW's drop callback mechanism.

Collects and stores dropped file paths into an internal buffer for later
retrieval by editor systems such as the FileBrowser. This enables drag-and-drop
importing of assets, prefabs, and other project files directly into the engine’s
editor interface.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "DropCallback.hpp"

namespace Uma_Engine
{
    std::vector<std::string> FileDropHandler::aDroppedFiles;

    void FileDropHandler::DropCallback(GLFWwindow* window, int count, const char** paths)
    {
        (void)window;

        for (int i = 0; i < count; i++)
        {
            aDroppedFiles.push_back(paths[i]);
        }
    }
}
