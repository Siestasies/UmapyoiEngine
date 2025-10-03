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
#include "IMGUIEvents.h"
#include "DebugEvents.h"
#include "ECSEvents.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Core/FilePaths.h"
#include <iostream>
#include <random>

struct GLFWwindow;

namespace Uma_Engine
{
    class ImguiManager : public ISystem, public IWindowSystem
    {   
        public:
            ImguiManager()
                : m_initialized(false)
                , m_window(nullptr)
                , m_showEngineDebug(true)
                , m_showEventDebug(true)
                , m_showDemoWindow(false)
                , m_showPerformanceWindow(true)
                , m_showSystemsWindow(true)
                , m_historyOffset(0)
                , pEventSystem(nullptr)
                , mEntityCount(0)
                , windowWidth(1920)
                , windowHeight(1080)
            {
                // init array
                for (int i = 0; i < 120; ++i)
                {
                    m_fpsHistory[i] = 0.0f;
                    m_dtHistory[i] = 0.0f;
                }
            }

            // isystem stuff
            void Init() override
            {
                if (m_initialized)
                {
                    return;
                }

                if (!m_window)
                {
                    return;
                }

                IMGUI_CHECKVERSION();
                ImGui::CreateContext();
                ImGuiIO& io = ImGui::GetIO(); (void)io;

                io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
                io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
                io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

                ImGui::StyleColorsDark();

                // set font and font size
                float fontSize = 20.0f;
                io.Fonts->AddFontDefault();

                std::string path = Uma_FilePath::ASSET_ROOT + "Roboto-Medium.ttf";

                io.FontDefault = io.Fonts->AddFontFromFileTTF(path.c_str(), fontSize);

                // set up backend stuff
                ImGui_ImplGlfw_InitForOpenGL(m_window, true);
                const char* glsl_version = "#version 130";
                ImGui_ImplOpenGL3_Init(glsl_version);

                // event listeners
                pEventSystem = pSystemManager->GetSystem<EventSystem>();

                pEventSystem->Subscribe<DebugLogEvent>([this](const DebugLogEvent& e) { AddConsoleLog(e.message); });

                pEventSystem->Subscribe<EntityCreatedEvent>([this](const EntityCreatedEvent& e) { mEntityCount = e.entityCnt; });
                pEventSystem->Subscribe<EntityDestroyedEvent>([this](const EntityDestroyedEvent& e) { mEntityCount = e.entityCnt; });

                m_initialized = true;
            }

            void Update(float deltaTime) override
            {
                if (!m_initialized)
                {
                    return;
                }

                static float fpsAccumulator = 0.0f;
                static int frameCount = 0;
                static float lastFpsUpdate = 0.0f;

                fpsAccumulator += deltaTime;
                frameCount++;

                // update every 0.1 seconds
                if (fpsAccumulator >= 0.1f)
                {
                    float fps = frameCount / fpsAccumulator;
                    m_fpsHistory[m_historyOffset] = fps;
                    m_dtHistory[m_historyOffset] = (deltaTime * 1000.0f);
                    m_historyOffset = (m_historyOffset + 1) % 120;

                    fpsAccumulator = 0.0f;
                    frameCount = 0;
                }

                StartFrame();

                // call for windows to be shown
                float currentFps = deltaTime > 0.0f ? (1.0f / deltaTime) : 0.0f;
                CreateDebugWindows(currentFps, deltaTime);

                Render();
            }

            void Shutdown() override
            {
                if (!m_initialized)
                    return;

                ImGui_ImplOpenGL3_Shutdown();
                ImGui_ImplGlfw_Shutdown();
                ImGui::DestroyContext();

                m_initialized = false;
                std::cout << "imgui SHUTDOWN" << std::endl;
            }

            void SetWindow(GLFWwindow* window) override
            {
                m_window = window;
                if (!m_initialized && m_window)
                    Init();
            }

            // ImGui-specific methods
            void StartFrame()
            {
                if (!m_initialized)
                    return;

                // start imgui fram
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();
            }

            void Render()
            {
                if (!m_initialized)
                    return;
                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            }

            void CreateDebugWindows(float fps, float deltaTime)
            {
                if (!m_initialized)
                    return;

                CreateEngineDebugWindow(fps, deltaTime);
                CreatePerformanceWindow();

                if (m_showSystemsWindow)
                {
                    CreateSystemsWindow();
                    CreateEntityDebugWindow();
                    CreateConsoleWindow();
                    CreateSerializationDebugWindow();
                    CreateEntityPropertyWindow();
                }

                if (m_showDemoWindow)
                {
                    ImGui::ShowDemoWindow(&m_showDemoWindow);
                }
            }

            bool IsInitialized() const { return m_initialized; }

            // for window controls
            void ShowEngineDebug(bool show) { m_showEngineDebug = show; }
            void ShowEventDebug(bool show) { m_showEventDebug = show; }
            void ShowDemoWindow(bool show) { m_showDemoWindow = show; }
            void ShowPerformanceWindow(bool show) { m_showPerformanceWindow = show; }
            void ShowSystemsWindow(bool show) { m_showSystemsWindow = show; }

        private:
            // helper functions
            void CreateEngineDebugWindow(float fps, float deltaTime)
            {
                if (!m_showEngineDebug)
                {
                    return;
                }

                ImGui::SetNextWindowPos(ImVec2(windowWidth * 0.4f, windowHeight * 0.75f), ImGuiCond_Once);
                ImGui::SetNextWindowSize(ImVec2(windowWidth * 0.3f, windowHeight * 0.25f), ImGuiCond_Once);
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

            void CreatePerformanceWindow()
            {
                if (!m_showPerformanceWindow)
                {
                    return;
                }

                ImGui::SetNextWindowPos(ImVec2(0, windowHeight * 0.3f), ImGuiCond_Once);
                ImGui::SetNextWindowSize(ImVec2(windowWidth * 0.265f, windowHeight * 0.25f), ImGuiCond_Once);
                ImGui::Begin("Performance Monitor", &m_showPerformanceWindow);

                // FPS graph
                ImGui::PlotLines("FPS", m_fpsHistory, 120, m_historyOffset, nullptr, 0.0f, 200.0f, ImVec2(0, 80));

                // Frame time graph
                ImGui::PlotLines("Frame Time (ms)", m_dtHistory, 120, m_historyOffset, nullptr, 0.0f, 50.0f, ImVec2(0, 80));
                
                ImGui::End();
            }

            void CreateSystemsWindow()
            {
                if (!m_showSystemsWindow)
                {
                    return;
                }
                ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
                ImGui::SetNextWindowSize(ImVec2(windowWidth * 0.265f, windowHeight * 0.3f), ImGuiCond_Once);
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

            void CreateEntityDebugWindow()
            {
                bool b = true;
                ImGui::SetNextWindowPos(ImVec2(0, windowHeight * 0.75f), ImGuiCond_Once);
                ImGui::SetNextWindowSize(ImVec2(windowWidth * 0.1f, windowHeight * 0.25f), ImGuiCond_Once);
                ImGui::Begin("Entity Debug", &b);

                // get entity count here
                ImGui::Text("Entity Count: %i", mEntityCount);

                ImGui::Separator();

                if (ImGui::Button("dup or create Entity", { 160, 50 }))
                {
                    // do spawning here
                    //std::cout << "Entity spawned" << std::endl;
                    QueryActiveEntitiesEvent query;
                    pEventSystem->Dispatch(query);

                    //std::default_random_engine generator;
                    //std::uniform_int_distribution<Uma_ECS::Entity> distribution(1, query.mActiveEntityCnt);
                    //Uma_ECS::Entity rand = distribution(generator);

                    pEventSystem->Emit<CloneEntityRequestEvent>(1);
                }
                if (ImGui::Button("load Entity prefab", { 160, 50 }))
                {
                    // do spawning here
                    //std::cout << "Entity spawned" << std::endl;
                    QueryActiveEntitiesEvent query;
                    pEventSystem->Dispatch(query);

                    //std::default_random_engine generator;
                    //std::uniform_int_distribution<Uma_ECS::Entity> distribution(1, query.mActiveEntityCnt);
                    //Uma_ECS::Entity rand = distribution(generator);

                    pEventSystem->Emit<LoadPrefabRequestEvent>();
                }
                if (ImGui::Button("Destroy Rand Entity", { 160, 50 }))
                {
                    // do spawning here
                    //std::cout << "Entity destroyed" << std::endl;
                    QueryActiveEntitiesEvent query;
                    pEventSystem->Dispatch(query);

                    /*std::default_random_engine generator;
                    std::uniform_int_distribution<Uma_ECS::Entity> distribution(1, query.mActiveEntityCnt);
                    Uma_ECS::Entity rand = distribution(generator);*/

                    pEventSystem->Emit<DestroyEntityRequestEvent>(1);
                }
                if (ImGui::Button("Stress Test 10k GO", { 160, 50 }))
                {
                    // do spawning here
                    //std::cout << "Entity destroyed" << std::endl;
                    QueryActiveEntitiesEvent query;
                    pEventSystem->Dispatch(query);

                    /*std::default_random_engine generator;
                    std::uniform_int_distribution<Uma_ECS::Entity> distribution(1, query.mActiveEntityCnt);
                    Uma_ECS::Entity rand = distribution(generator);*/

                    pEventSystem->Emit<StressTestRequestEvent>();
                }
                if (ImGui::Button("Spawn 2.5k in VP", { 160, 50 }))
                {
                    // do spawning here
                    //std::cout << "Entity destroyed" << std::endl;
                    QueryActiveEntitiesEvent query;
                    pEventSystem->Dispatch(query);

                    /*std::default_random_engine generator;
                    std::uniform_int_distribution<Uma_ECS::Entity> distribution(1, query.mActiveEntityCnt);
                    Uma_ECS::Entity rand = distribution(generator);*/

                    pEventSystem->Emit<ShowEntityInVPRequestEvent>();
                }
                
                ImGui::End();
            }

            void CreateEntityPropertyWindow()
            {
                bool b = true;
                ImGui::Begin("Entity Run Time Property", &b);

                ImGui::Separator();

                static float rot = 0.f;
                static float scale = 1.f;
                static bool showBBox = false;

                if (ImGui::SliderFloat("enemy scale", &scale, 0.1f, 2.0f))
                {
                    QueryActiveEntitiesEvent query;
                    pEventSystem->Dispatch(query);
                    pEventSystem->Emit<ChangeEnemyScaleRequestEvent>(scale);
                }

                if (ImGui::SliderFloat("enemy rot", &rot, -1.0f, 1.0f))
                {
                }
                QueryActiveEntitiesEvent q;
                pEventSystem->Dispatch(q);
                pEventSystem->Emit<ChangeEnemyRotRequestEvent>(rot);

                if (ImGui::Button("Show BBox", { 160, 50 }))
                {
                    showBBox = !showBBox;
                    QueryActiveEntitiesEvent query;
                    pEventSystem->Dispatch(query);
                    pEventSystem->Emit<ShowBBoxRequestEvent>(showBBox);
                }

                ImGui::End();
            }

            void CreateSerializationDebugWindow()
            {
                bool b = true;
                ImGui::SetNextWindowPos(ImVec2(windowWidth * 0.1f, windowHeight * 0.75f), ImGuiCond_Once);
                ImGui::SetNextWindowSize(ImVec2(windowWidth * 0.125f, windowHeight * 0.25f), ImGuiCond_Once);
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

            void CreateConsoleWindow()
            {
                ImGui::SetNextWindowPos(ImVec2(windowWidth * 0.7f, windowHeight * 0.75f), ImGuiCond_Once);
                ImGui::SetNextWindowSize(ImVec2(windowWidth * 0.3f, windowHeight * 0.25f), ImGuiCond_Once);
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

            void AddConsoleLog(const std::string& message)
            {
                logsVec.push_back(message);

                // dont go beyond 100 messgaes shown
                if (logsVec.size() > 100)
                    logsVec.erase(logsVec.begin());
            }

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