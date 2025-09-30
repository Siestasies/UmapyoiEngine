#pragma once
#include "SystemType.h"
#include "IMGUIEvents.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>

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
                , m_systemManager(nullptr)
            {
                // init array
                for (int i = 0; i < 120; ++i)
                {
                    m_fpsHistory[i] = 0.0f;
                    m_frametimeHistory[i] = 0.0f;
                }
            }
            ~ImguiManager() override
            {
                // nothing cus shutdown should handle destorying alr
            }

            void SetSystemManager(SystemManager* manager)
            {
                m_systemManager = manager;
            }

            // isystem stuff
            void Init() override
            {
                std::cout << "ImGuiSystem: Initializing..." << std::endl;

                if (m_initialized)
                {
                    std::cout << "ImGuiSystem: Already initialized!" << std::endl;
                    return;
                }

                if (!m_window)
                {
                    std::cout << "ImGuiSystem: Warning - No window set during Init(). Will initialize when window is set." << std::endl;
                    return;
                }

                // Setup Dear ImGui context
                IMGUI_CHECKVERSION();
                ImGui::CreateContext();
                ImGuiIO& io = ImGui::GetIO(); (void)io;
                io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
                io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

                // Note: Docking and Viewports might not be available in all ImGui versions
                // Uncomment these lines if your ImGui version supports them:
                 io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
                 //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

                // Setup Dear ImGui style
                ImGui::StyleColorsDark();
                // ImGui::StyleColorsLight();

                // Setup Platform/Renderer backends
                ImGui_ImplGlfw_InitForOpenGL(m_window, true);
                const char* glsl_version = "#version 130";
                ImGui_ImplOpenGL3_Init(glsl_version);

                m_initialized = true;
            }

            void Update(float deltaTime) override
            {
                if (!m_initialized)
                {
                    return;
                }

                // Update performance history
                static float fpsAccumulator = 0.0f;
                static int frameCount = 0;
                static float lastFpsUpdate = 0.0f;

                fpsAccumulator += deltaTime;
                frameCount++;

                // Update FPS every 0.1 seconds for smoother graphs
                if (fpsAccumulator >= 0.1f)
                {
                    float fps = frameCount / fpsAccumulator;
                    m_fpsHistory[m_historyOffset] = fps;
                    m_frametimeHistory[m_historyOffset] = (deltaTime * 1000.0f);
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
                CreatePerformanceWindow(fps, deltaTime);

                if (m_showSystemsWindow)
                {
                    CreateSystemsWindow();
                    CreateEntityDebugWindow();
                    CreateConsoleWindow();
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

                ImGui::Begin("Engine Debug", &m_showEngineDebug);

                // Menu bar for window options
                if (ImGui::BeginMenuBar())
                {
                    if (ImGui::BeginMenu("Windows"))
                    {
                        ImGui::MenuItem("Performance", nullptr, &m_showPerformanceWindow);
                        ImGui::MenuItem("Systems", nullptr, &m_showSystemsWindow);
                        ImGui::MenuItem("Event Debug", nullptr, &m_showEventDebug);
                        ImGui::MenuItem("Demo Window", nullptr, &m_showDemoWindow);
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Performance stats
                ImGui::Text("Performance Overview");
                ImGui::Separator();
                ImGui::Text("FPS: %.1f", fps);
                ImGui::Text("Frame Time: %.3f ms", deltaTime * 1000.0f);
                ImGui::Text("Delta Time: %.6f s", deltaTime);

                ImGui::Spacing();

                // OpenGL info
                ImGui::Text("Graphics Information");
                ImGui::Separator();
                ImGui::Text("OpenGL Version: %s", glGetString(GL_VERSION));
                ImGui::Text("Renderer: %s", glGetString(GL_RENDERER));
                ImGui::Text("Vendor: %s", glGetString(GL_VENDOR));

                ImGui::End();
            }

            void CreatePerformanceWindow(float fps, float deltaTime)
            {
                if (!m_showPerformanceWindow)
                {
                    return;
                }

                ImGui::Begin("Performance Monitor", &m_showPerformanceWindow);

                // FPS graph
                ImGui::PlotLines("FPS", m_fpsHistory, 120, m_historyOffset, nullptr, 0.0f, 200.0f, ImVec2(0, 80));

                // Frame time graph
                ImGui::PlotLines("Frame Time (ms)", m_frametimeHistory, 120, m_historyOffset, nullptr, 0.0f, 50.0f, ImVec2(0, 80));

                // Current stats
                ImGui::Separator();
                float frametime_ms = deltaTime * 1000.0f;
                ImGui::Text("Current: %.1f FPS (%.3f ms)", fps, frametime_ms);

                ImGui::End();
            }

            void CreateSystemsWindow()
            {
                if (!m_showSystemsWindow)
                {
                    return;
                }

                ImGui::Begin("Systems Monitor", &m_showSystemsWindow);

                if (m_systemManager)
                {
                    const auto& timings = m_systemManager->GetLastTimings();
                    double total = m_systemManager->GetLastTotalTime();

                    ImGui::Text("Registered Systems: %zu", timings.size());
                    ImGui::Separator();

                    if (total > 0.0)
                    {
                        for (size_t i = 0; i < timings.size(); ++i)
                        {
                            double ms = timings[i];
                            double percent = (ms / total) * 100.0;
                            ImGui::Text("%i. %s: %.1f ms (%.1f%%)", (i + 1), m_systemManager->GetSystemName(i).c_str(), ms, percent);
                        }

                        ImGui::Separator();
                        ImGui::Text("Total Update Time: %.3f ms", total);
                    }
                    else
                    {
                        ImGui::Text("No timing data available yet.");
                    }
                }
                else
                {
                    ImGui::Text("System Manager not attached!");
                }

                ImGui::End();
            }

            void CreateEntityDebugWindow()
            {
                bool b = true;
                ImGui::Begin("Entity Debug", &b);

                //Uma_ECS::Coordinator* coordRef = m_systemManager->GetSystem<Uma_ECS::Coordinator>();
                //std::cout << coordRef->GetEntityCount() << std::endl;
                // 
                // get entity count here
                //ImGui::Text("Entity Count: %i", coordRef->GetEntityCount());

                ImGui::Separator();

                if (ImGui::Button("Spawn Entity", { 100, 50 }))
                {
                    // do spawning here
                    std::cout << "Entity spawned" << std::endl;
                    pEventSystem->Emit<CloneEntityRequestEvent>(200);
                }
                //ImGui::SetCursorPos({5, 100});
                if (ImGui::Button("Destroy Entity", { 100, 50 }))
                {
                    // do spawning here
                    std::cout << "Entity destroyed" << std::endl;
                    pEventSystem->Emit<DestroyEntityRequestEvent>(200);
                }
                
                ImGui::End();
            }

            void CreateConsoleWindow()
            {
                bool b = true;
                ImGui::Begin("Console", &b);
                // to clear the console
                if (ImGui::Button("Clear"))
                    logsVec.clear();
                ImGui::SameLine();
                // test message 
                if (ImGui::Button("Test Message Button"))
                    AddConsoleLog("This is a test message");

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

            // refs to other classes
            SystemManager* m_systemManager;
            // show or not
            bool m_showEngineDebug;
            bool m_showEventDebug;
            bool m_showDemoWindow;
            bool m_showPerformanceWindow;
            bool m_showSystemsWindow;

            // performance window vars
            float m_fpsHistory[120];
            float m_frametimeHistory[120];
            int m_historyOffset;
            std::vector<std::string> logsVec;
    };
}