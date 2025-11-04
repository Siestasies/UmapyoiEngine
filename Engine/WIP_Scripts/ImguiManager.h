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
#include "Events/IMGUIEvents.h"
#include "Events/DebugEvents.h"
#include "Events/ECSEvents.h"
#include "FileSystem/FileSystem.hpp"

#include "Core/FilePaths.h"
#include <iostream>
#include <random>

struct GLFWwindow;

namespace Uma_Engine
{
    enum class PlayState
    {
        Stopped,
        Playing,
        Paused
    };

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
            bool IsInitialized() const { return m_initialized; }

            // for window controls
            bool IsPlaying() const { return m_playState == PlayState::Playing; }
            bool IsPaused() const { return m_playState == PlayState::Paused; }
            bool IsStopped() const { return m_playState == PlayState::Stopped; }
            void ShowEngineDebug(bool show) { m_showEngineDebug = show; }
            void ShowEventDebug(bool show) { m_showEventDebug = show; }
            void ShowDemoWindow(bool show) { m_showDemoWindow = show; }
            void ShowPerformanceWindow(bool show) { m_showPerformanceWindow = show; }
            void ShowSystemsWindow(bool show) { m_showSystemsWindow = show; }

            EventSystem* GetESHandler() { if (!pEventSystem) return pEventSystem; }
            SystemManager* GetSMHandler() { if (!pSystemManager) return pSystemManager; }

        private:
            // bigger space stuff
            bool windowsInit(const char* filename = "imgui.ini");
            void CreateDockspace();
            void InitDockspace(ImGuiID dockspace_id, ImGuiViewport* viewport);

            // helper functions
            void CreateHierarchyWindow();
            void CreateInspectorWindow();
            void SceneManagerWindow();
            
            void CreateEditorControlBar();
            void CreateSystemsWindow();
            void CreatePerformanceWindow();
            void CreateEngineDebugWindow(float fps, float deltaTime);
            void CreateSerializationDebugWindow();
            void CreateEntityDebugWindow();
            void CreateEntityPropertyWindow();
            void CreateConsoleWindow();
            void AddConsoleLog(const std::string& message);

            bool m_initialized;
            bool ds_initialized;
            GLFWwindow* m_window;
            std::vector<std::string> logsVec;
            std::vector<std::string> sceneNames;
            std::vector<std::string> scenePaths;
            int activeSceneIndex;
            EventSystem* pEventSystem;
            FileBrowser fileBrowser;

            // show or not
            bool m_showEngineDebug;
            bool m_showEventDebug;
            bool m_showDemoWindow;
            bool m_showPerformanceWindow;
            bool m_showSystemsWindow;
            bool m_showEditorControlBar;

            // values that need to keep track
            int mEntityCount;
            int windowWidth, windowHeight;
            PlayState m_playState = PlayState::Stopped;

            // performance window vars
            float m_fpsHistory[120];
            float m_dtHistory[120];
            int m_historyOffset;
    };
}

// shah fallback plan
/*
void CreateSystemsWindow()
            {
                if (!m_showSystemsWindow)
                {
                    return;
                }
                //ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
                //ImGui::SetNextWindowSize(ImVec2(windowWidth * 0.2f, windowHeight * 0.25f), ImGuiCond_Once);
                ImGui::Begin("Systems Monitor", &m_showSystemsWindow);

                if (pSystemManager)
                {
                    const auto& timings = pSystemManager->GetLastTimings();
                    double total = pSystemManager->GetLastTotalTime();

                    ImGui::Text("Registered Systems: %zu", timings.size());
                    ImGui::Separator();

                    if (total > 0.0)
                    {
                        for (size_t i = 0; i < timings.size(); ++i)
                        {
                            double ms = timings[i];
                            double percent = (ms / total) * 100.0;
                            ImGui::Text("%i. %s: %.1f ms (%.1f%%)", (i + 1), pSystemManager->GetSystemName(i).c_str(), ms, percent);
                        }

                        ImGui::Separator();
                        ImGui::Text("Total Update Time: %.3f ms", total);
                    }
                }
                ImGui::End();
            }
            
            void CreatePerformanceWindow()
            {
                if (!m_showPerformanceWindow)
                {
                    return;
                }

                //ImGui::SetNextWindowPos(ImVec2(0, windowHeight * 0.25f), ImGuiCond_Once);
                //ImGui::SetNextWindowSize(ImVec2(windowWidth * 0.2f, windowHeight * 0.2f), ImGuiCond_Once);
                ImGui::Begin("Performance Monitor", &m_showPerformanceWindow);

                // FPS graph
                ImGui::PlotLines("FPS", m_fpsHistory, 120, m_historyOffset, nullptr, 0.0f, 200.0f, ImVec2(0, 80));

                // Frame time graph
                ImGui::PlotLines("Frame Time (ms)", m_dtHistory, 120, m_historyOffset, nullptr, 0.0f, 50.0f, ImVec2(0, 80));
                
                ImGui::End();
            }

            void CreateEngineDebugWindow(float fps, float deltaTime)
            {
                if (!m_showEngineDebug)
                {
                    return;
                }

                //ImGui::SetNextWindowPos(ImVec2(0.f, windowHeight * 0.45f), ImGuiCond_Once);
                //ImGui::SetNextWindowSize(ImVec2(windowWidth * 0.2f, windowHeight * 0.225f), ImGuiCond_Once);
                ImGui::Begin("Engine Debug", &m_showEngineDebug);

                // some stats
                ImGui::Text("Performance Stats");
                ImGui::Separator();
                ImGui::Text("FPS: %.1f", fps);
                ImGui::Text("Frame Time: %.3f ms", deltaTime * 1000.0f);
                ImGui::Text("Delta Time: %.6f s", deltaTime);

                ImGui::Spacing();

                // onpengl info
                ImGui::Text("Graphics Information");
                ImGui::Separator();
                ImGui::Text("OpenGL Version: %s", glGetString(GL_VERSION));
                ImGui::Text("Renderer: %s", glGetString(GL_RENDERER));
                ImGui::Text("Vendor: %s", glGetString(GL_VENDOR));

                ImGui::End();
            }
            
            void CreateSerializationDebugWindow()
            {
                bool b = true;
                //ImGui::SetNextWindowPos(ImVec2(windowWidth * 0.82f, 0.f), ImGuiCond_Once);
                //ImGui::SetNextWindowSize(ImVec2(windowWidth * 0.08f, windowHeight * 0.315f), ImGuiCond_Once);
                ImGui::Begin("Serialization Debug", &b);

                // get entity count here
                ImGui::Separator();

                if (ImGui::Button("Load Scene", { 100, 50 }))
                {
                    // load scene from this file
                    pEventSystem->Emit<LoadSceneRequestEvent>("../../../../Assets/Scenes/NEW.json");
                }
                if (ImGui::Button("Save Scene", { 100, 50 }))
                {
                    // save scene into this file
                    pEventSystem->Emit<SaveSceneRequestEvent>("../../../../Assets/Scenes/NEW.json");
                }
                if (ImGui::Button("Destroy All", { 100, 50 }))
                {
                    // destroy entities within the scene lol
                    pEventSystem->Emit<ClearSceneRequestEvent>();
                }

                ImGui::End();
            }

            void CreateEntityDebugWindow()
            {
                bool b = true;
                //ImGui::SetNextWindowPos(ImVec2(windowWidth * 0.9f, 0.f), ImGuiCond_Once);
                //ImGui::SetNextWindowSize(ImVec2(windowWidth * 0.1f, windowHeight * 0.315f), ImGuiCond_Once);
                ImGui::Begin("Entity Debug", &b);

                // get entity count here
                ImGui::Text("Entity Count: %i", mEntityCount);

                ImGui::Separator();

                if (ImGui::Button("dup or create Entity", { 160, 50 }))
                {
                    pEventSystem->Emit<CloneEntityRequestEvent>(1);
                }
                if (ImGui::Button("load Entity prefab", { 160, 50 }))
                {
                    pEventSystem->Emit<LoadPrefabRequestEvent>();
                }
                if (ImGui::Button("Destroy Rand Entity", { 160, 50 }))
                {
                    pEventSystem->Emit<DestroyEntityRequestEvent>(1);
                }
                if (ImGui::Button("Stress Test 10k GO", { 160, 50 }))
                {
                    pEventSystem->Emit<StressTestRequestEvent>();
                }
                if (ImGui::Button("Spawn Default", { 160, 50 }))
                {
                    pEventSystem->Emit<ShowEntityInVPRequestEvent>();
                }
                
                ImGui::End();
            }

            void CreateEntityPropertyWindow()
            {
                bool b = true;
                //ImGui::SetNextWindowPos(ImVec2(windowWidth * 0.82f, windowHeight * 0.315f), ImGuiCond_Once);
                //ImGui::SetNextWindowSize(ImVec2(windowWidth * 0.18f, windowHeight * 0.21f), ImGuiCond_Once);
                ImGui::Begin("Entity Run Time Property", &b);

                ImGui::Separator();

                static float rot = 0.f;
                static float scale = 1.f;
                static float moveX = 0.f;
                static bool showBBox = false;

                if (ImGui::SliderFloat("enemy scale", &scale, 0.1f, 2.0f))
                {
                    pEventSystem->Emit<ChangeEnemyScaleRequestEvent>(scale);
                }

                if (ImGui::SliderFloat("enemy rot", &rot, -45.0f, 45.0f))
                {
                    pEventSystem->Emit<ChangeEnemyRotRequestEvent>(rot);
                }

                if (ImGui::SliderFloat("enemy move X", &moveX, -1.0f, 1.0f))
                {
                    pEventSystem->Emit<ChangeEnemyXposRequestEvent>(moveX);
                }

                if (ImGui::Button("Reset", { 160, 50 }))
                {
                    rot = 0.f;
                    scale = 1.f;
                    moveX = 0.f;
                    showBBox = false;

                    pEventSystem->Emit<ChangeEnemyScaleRequestEvent>(scale);
                    pEventSystem->Emit<ChangeEnemyRotRequestEvent>(rot);
                    pEventSystem->Emit<ChangeEnemyXposRequestEvent>(moveX);
                    pEventSystem->Emit<ShowBBoxRequestEvent>(showBBox);
                }

                if (ImGui::Button("Show BBox", { 160, 50 }))
                {
                    showBBox = !showBBox;
                    pEventSystem->Emit<ShowBBoxRequestEvent>(showBBox);
                }

                ImGui::End();
            }

            void CreateConsoleWindow()
            {
                //ImGui::SetNextWindowPos(ImVec2(windowWidth * 0.82f, windowHeight * 0.525f), ImGuiCond_Once);
                //ImGui::SetNextWindowSize(ImVec2(windowWidth * 0.18f, windowHeight * 0.25f), ImGuiCond_Once);
                bool b = true;
                ImGui::Begin("Console", &b);
                // to clear the console
                if (ImGui::Button("Clear"))
                    logsVec.clear();
                ImGui::SameLine();

                ImGui::Separator();

                ImGui::BeginChild("ConsoleLog", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
                for (const auto& entry : logsVec)
                {
                    ImVec4 color = ImVec4(1, 1, 1, 1); // whit color
                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                    ImGui::Text("%s", entry.c_str());
                    ImGui::PopStyleColor();
                }
                // keep scroll to bottom
                if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                    ImGui::SetScrollHereY(1.0f);
                ImGui::EndChild();

                ImGui::End();
            }

            void CreateDockspace()
            {
                // Create main dockspace window
                ImGuiViewport* viewport = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos(viewport->WorkPos);
                ImGui::SetNextWindowSize(viewport->WorkSize);
                ImGui::SetNextWindowViewport(viewport->ID);

                ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
                window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
                window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
                window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
                window_flags |= ImGuiWindowFlags_NoBackground;

                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

                ImGui::Begin("DockSpace", nullptr, window_flags);
                ImGui::PopStyleVar(3);
                ImGui::PopStyleColor(); // Don't forget to pop the color!

                ImGuiID ds_id = ImGui::GetID("DockSpace");
                ImGui::DockSpace(ds_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

                if (!ds_initialized)
                {
                    ds_initialized = true;

                    // Check for docking data
                    bool has_layout = false;
                    std::ifstream file("imgui.ini");
                    if (file.good())
                    {
                        std::string line;
                        while (std::getline(file, line))
                        {
                            if (line.find("[Docking][Data]") != std::string::npos)
                            {
                                has_layout = true;
                                break;
                            }
                        }
                    }
                    if (!has_layout);
                    InitDockspace(ds_id, viewport);
                }

                ImGui::End();
            }

            void AddConsoleLog(const std::string& message)
            {
                logsVec.push_back(message);

                // dont go beyond 100 messgaes shown
                if (logsVec.size() > 100)
                    logsVec.erase(logsVec.begin());
            }

            void InitDockspace(ImGuiID dockspace_id, ImGuiViewport* viewport) {
                 // Clear any existing layout
                 ImGui::DockBuilderRemoveNode(dockspace_id);
                 ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
                 ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

                 // Split the dockspace into regions (Unity-style layout)
                 ImGuiID dock_main_id = dockspace_id;
                 ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.15f, nullptr, &dock_main_id);
                 ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.15f, nullptr, &dock_main_id);
                 ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.15f, nullptr, &dock_main_id);

                 if (!windowsInit())
                 {
                     // Dock windows to their initial positions
                     ImGui::DockBuilderDockWindow("Console", dock_id_bottom);
                     ImGui::DockBuilderDockWindow("Engine Debug", dock_id_bottom);
                     ImGui::DockBuilderDockWindow("File Browser", dock_id_bottom);
                     ImGui::DockBuilderDockWindow("Systems Monitor", dock_id_left);
                     ImGui::DockBuilderDockWindow("Performance Monitor", dock_id_left);
                     ImGui::DockBuilderDockWindow("Entity Debug", dock_id_right);
                     ImGui::DockBuilderDockWindow("Entity Run Time Property", dock_id_right);
                     ImGui::DockBuilderDockWindow("Serialization Debug", dock_id_right);
                 }

                 ImGui::DockBuilderFinish(dockspace_id);
            };

            bool windowsInit(const char* filename = "imgui.ini")
            {
                return std::ifstream(filename).good();
            } 
            */