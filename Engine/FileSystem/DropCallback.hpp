/*!
\file   FileDropHandler.hpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author     Lai Jun Siang (100%)
\par        E-mail: lai.j@digipen.edu
\par        DigiPen login: lai.j

\brief
Declares the FileDropHandler class, which processes file drag-and-drop events
received from the GLFW window system. This handler captures dropped file paths
and stores them for retrieval by the FileBrowser, allowing user-initiated
imports such as assets, scenes, or external resources.

The class exposes a static callback compatible with GLFW�s drop event system
and maintains an internal list of received file paths. Integration with the
FileBrowser enables editor features such as direct dropping of assets into the
workspace or project directory.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/
#pragma once
#include <vector>
#include <string>
#include <GLFW/glfw3.h>

namespace Uma_Engine
{
    class FileBrowser;

    class FileDropHandler
    {
    public:
        /*!
        \brief GLFW drop callback that captures file paths dragged onto the window.
        \param window The GLFW window that received the drop event.
        \param count Number of file paths dropped.
        \param paths Array of C-string file paths.
        */
        static void DropCallback(GLFWwindow* window, int count, const char** paths);

    private:
        static std::vector<std::string> aDroppedFiles;

        friend class FileBrowser;
    };

} // namespace Uma_Engine
