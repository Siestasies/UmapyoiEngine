#pragma once
#include "SystemType.h"
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
        private:
            SystemManager* m_systemManager;
        public:
            ImguiManager()
                : m_initialized(false)
                , m_window(nullptr)
                , m_showEngineDebug(true)
                , m_showEventDebug(true)
                , m_showDemoWindow(true)
                , m_showPerformanceWindow(true)
                , m_showSystemsWindow(true)
                , m_historyOffset(0)
                , m_systemManager(nullptr)
            {
                // Initialize history arrays
                for (int i = 0; i < 120; ++i)
                {
                    m_fpsHistory[i] = 0.0f;
                    m_frametimeHistory[i] = 0.0f;
                }
            }
            ~ImguiManager() override
            {
                // Shutdown is handled by the system manager calling Shutdown()
            }

            void SetSystemManager(SystemManager* manager)
            {
                m_systemManager = manager;
            }

            // ISystem interface
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
                std::cout << "ImGuiSystem: Successfully initialized!" << std::endl;
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

                // Start ImGui frame
                StartFrame();

                // Create debug windows
                float currentFps = deltaTime > 0.0f ? (1.0f / deltaTime) : 0.0f;
                CreateDebugWindows(currentFps, deltaTime);

                // Render ImGui
                Render();
            }

            void Shutdown() override
            {
                if (!m_initialized)
                {
                    return;
                }

                std::cout << "ImGuiSystem: Shutting down..." << std::endl;

                ImGui_ImplOpenGL3_Shutdown();
                ImGui_ImplGlfw_Shutdown();
                ImGui::DestroyContext();

                m_initialized = false;
                std::cout << "ImGuiSystem: Successfully shutdown!" << std::endl;
            }

            void SetWindow(GLFWwindow* window) override
            {
                m_window = window;

                // If we haven't initialized yet and now have a window, try to initialize
                if (!m_initialized && m_window)
                {
                    Init();
                }
            }

            // ImGui-specific methods
            void StartFrame()
            {
                if (!m_initialized)
                {
                    return;
                }

                // Start the Dear ImGui frame
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();
            }

            void Render()
            {
                if (!m_initialized)
                {
                    return;
                }

                // Rendering
                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                // Note: Multi-viewport support removed for compatibility
                // If you have a newer ImGui version with viewport support, uncomment these lines:
                /*
                ImGuiIO& io = ImGui::GetIO();
                if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
                {
                    GLFWwindow* backup_current_context = glfwGetCurrentContext();
                    ImGui::UpdatePlatformWindows();
                    ImGui::RenderPlatformWindowsDefault();
                    glfwMakeContextCurrent(backup_current_context);
                }
                */
            }

            void CreateDebugWindows(float fps, float deltaTime)
            {
                if (!m_initialized)
                {
                    return;
                }

                // Create various debug windows
                CreateEngineDebugWindow(fps, deltaTime);
                CreatePerformanceWindow(fps, deltaTime);

                if (m_showSystemsWindow)
                {
                    CreateSystemsWindow();
                }

                // ImGui Demo window (useful for learning ImGui features)
                if (m_showDemoWindow)
                {
                    ImGui::ShowDemoWindow(&m_showDemoWindow);
                }
            }

            // Check if ImGui is initialized
            bool IsInitialized() const { return m_initialized; }

            // Window visibility controls
            void ShowEngineDebug(bool show) { m_showEngineDebug = show; }
            void ShowEventDebug(bool show) { m_showEventDebug = show; }
            void ShowDemoWindow(bool show) { m_showDemoWindow = show; }
            void ShowPerformanceWindow(bool show) { m_showPerformanceWindow = show; }
            void ShowSystemsWindow(bool show) { m_showSystemsWindow = show; }

        private:
            bool m_initialized;
            GLFWwindow* m_window;

            // Internal helper methods
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

                ImGui::Spacing();

                // Engine status
                ImGui::Text("Engine Status");
                ImGui::Separator();
                ImGui::Text("Systems Manager: Active");
                ImGui::Text("ImGui System: Active");

                // System Manager info
                //if (m_systemManager)
                //{
                //    ImGui::Text("System Manager: %p", m_systemManager);
                //}

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
                            ImGui::Text("System %zu: %.1f ms (%.1f%%)", i, ms, percent);
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


            // Window visibility flags
            bool m_showEngineDebug;
            bool m_showEventDebug;
            bool m_showDemoWindow;
            bool m_showPerformanceWindow;
            bool m_showSystemsWindow;

            // Performance tracking
            float m_fpsHistory[120];
            float m_frametimeHistory[120];
            int m_historyOffset;
    };
}