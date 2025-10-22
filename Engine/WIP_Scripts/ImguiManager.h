/*!
\file   ImguiManager.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Shahir Rasid (100%)
\par    E-mail: b.muhammadshahir@digipen.edu
\par    DigiPen login: b.muhammadshahir

\brief
This file implements the definition for an IMGUI Manager as a system and
implements functions to create and show IMGUI windows.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/
#pragma once
#include "SystemType.h"
#include "Core/FilePaths.h"
#include "Window.hpp"
#include "EventSystem.h"

#include <vector>
#include <string>

struct GLFWwindow;

namespace Uma_Engine
{
    class ImguiManager : public ISystem, public IWindowSystem
    {   
        public:
            ImguiManager();

            // isystem stuff
            void Init() override;

            void Update(float deltaTime) override;

            void Shutdown() override;

            void SetWindow(GLFWwindow* window) override;

            // ImGui-specific methods
            void StartFrame();

            void Render();

            void CreateDebugWindows(float fps, float deltaTime);

            bool IsInitialized() const;

            // for window controls
            void ShowEngineDebug(bool show);
            void ShowEventDebug(bool show);
            void ShowDemoWindow(bool show);
            void ShowPerformanceWindow(bool show);
            void ShowSystemsWindow(bool show);

        private:
            // helper functions

            void CreateSystemsWindow();
            
            void CreatePerformanceWindow();

            void CreateEngineDebugWindow(float fps, float deltaTime);
            
            void CreateSerializationDebugWindow();

            void CreateEntityDebugWindow();

            void CreateEntityPropertyWindow();

            void CreateConsoleWindow();

            void AddConsoleLog(const std::string& message);

            bool m_initialized;
            GLFWwindow* m_window;

            // show or not
            bool m_showEngineDebug;
            bool m_showEventDebug;
            bool m_showDemoWindow;
            bool m_showPerformanceWindow;
            bool m_showSystemsWindow;

            // values that need to keep track
            int mEntityCount;
            int windowWidth, windowHeight;

            // performance window vars
            float m_fpsHistory[120];
            float m_dtHistory[120];
            int m_historyOffset;
            // debug var
            std::vector<std::string> logsVec;

            EventSystem* pEventSystem;
    };
}