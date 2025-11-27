/*!
\file   ImguiManager.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Shahir Rasid (Everything else)
\par    E-mail: b.muhammadshahir@digipen.edu
\par    DigiPen login: b.muhammadshahir

\co-author Lai Jun Siang (Hierarchy/Inspector/Dockspace)
\par       E-mail: lai.j@digipen.edu
\par       DigiPen login: lai.j

\co-author Javier Chua Dong Qing (EditorCamera)
\par       E-mail: javierdongqing.chua@digipen.edu
\par       DigiPen login: javierdongqing.chua

\brief
Definition of functions for all IMGUI windows and their logics.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/
#include <glad/glad.h>
#include "Scripts/ImguiManager.h"
#include "Systems/SceneManager.h"
#include "Systems/Graphics.hpp"

#include <GLFW/glfw3.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui_internal.h"

#include "Core/FilePaths.h"
#include "Events/EditorEvents.h"

#include <unordered_map>
#include <algorithm>

// Commands
#include "Editor/Cmds/EntitySnapshotCmd.h"
#include "Editor/Cmds/EntityDeleteCmd.h"
#include "Editor/Cmds/EntityCreateCmd.h"
#include "Editor/Cmds/EntityDuplicateCmd.h"

namespace Uma_Engine
{
    ImguiManager::ImguiManager()
        : m_initialized(false)
        , ds_initialized(false)
        , m_window(nullptr)
        , mScriptName("Assets/Scripts/.lua")
        , m_showEngineDebug(true)
        , m_showEventDebug(true)
        , m_showPerformanceWindow(true)
        , m_showEditorCameraWindow(true)
        , m_showSystemsWindow(true)
        , m_historyOffset(0)
        , pEventSystem(nullptr)
        , pResourcesManager(nullptr)
        , mEntityCount(0)
        , windowWidth(1920)
        , windowHeight(1080)
        , m_selectedEntity(static_cast<Uma_ECS::Entity>(-1))
        , m_hideAll(false)
    {
        // init array
        for (int i = 0; i < 120; ++i)
        {
            m_fpsHistory[i] = 0.0f;
            m_dtHistory[i] = 0.0f;
        }
    }

    // LIFECYCLE STUFF
    void ImguiManager::Init()
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
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = "Configs/imgui.ini";

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // Text and background colors
        colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f); // Light text
        colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f); // Dark background (Unity's blackish background)
        colors[ImGuiCol_ChildBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f); // Light dark child window background
        colors[ImGuiCol_Border] = ImVec4(0.50f, 0.50f, 0.50f, 0.50f); // Dark gray border
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f); // No border shadow

        // Button colors
        colors[ImGuiCol_Button] = ImVec4(0.21f, 0.44f, 0.77f, 0.80f); // Light blue button
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.51f, 0.94f, 1.00f); // Light blue on hover
        colors[ImGuiCol_ButtonActive] = ImVec4(0.13f, 0.33f, 0.59f, 1.00f); // Darker blue when pressed

        // Header colors
        colors[ImGuiCol_Header] = ImVec4(0.14f, 0.35f, 0.58f, 0.60f); // Header background color (slightly faded blue)
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.19f, 0.42f, 0.73f, 0.80f); // Header hover color (brighter blue)
        colors[ImGuiCol_HeaderActive] = ImVec4(0.12f, 0.30f, 0.52f, 0.80f); // Header active (selected) color

        // Frame (input fields, etc.) colors
        colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f); // Dark gray frame background (input fields, combo boxes)
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f); // Frame background when hovered
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.34f, 0.34f, 0.34f, 1.00f); // Active frame background (when clicked)

        colors[ImGuiCol_TitleBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f); // Window title background
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.20f, 0.20f, 0.20f, 0.75f); // Collapsed window title background
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.23f, 0.23f, 0.23f, 1.00f); // Active window title background

        // Scrollbar and grab colors
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f); // Scrollbar background
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.22f, 0.48f, 0.79f, 1.00f); // Scrollbar grab (blue)
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.25f, 0.56f, 0.92f, 1.00f); // Scrollbar grab hover color
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.20f, 0.45f, 0.77f, 1.00f); // Scrollbar grab active color

        // Tab colors
        colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f); // Default tab background color
        colors[ImGuiCol_TabHovered] = ImVec4(0.22f, 0.48f, 0.79f, 1.00f); // Tab hovered color
        colors[ImGuiCol_TabActive] = ImVec4(0.14f, 0.35f, 0.59f, 1.00f); // Tab active color

        // Menu and menu bar colors
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f); // Menu bar background

        // Disabled item colors
        colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f); // Disabled text color
        colors[ImGuiCol_Separator] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f); // Separator line color
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f); // Hovered separator color
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f); // Active separator color

        // Tooltip and popup colors
        colors[ImGuiCol_PopupBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f); // Popup background color
        // Adjusting style values
        style.WindowRounding = 4.0f; // Rounded corners for windows
        style.ChildRounding = 4.0f; // Rounded corners for child windows
        style.FrameRounding = 4.0f; // Rounded corners for input frames
        style.GrabRounding = 4.0f; // Rounded corners for scrollbar handles and sliders
        style.ScrollbarRounding = 4.0f; // Rounded corners for scrollbar

        // Padding adjustments
        style.FramePadding = ImVec2(8.0f, 6.0f); // Padding inside input fields and buttons
        style.ItemSpacing = ImVec2(6.0f, 4.0f); // Space between items
        style.WindowPadding = ImVec2(8.0f, 8.0f); // Padding inside window borders

        // set font and font size
        float fontSize = 16.f;
        io.Fonts->AddFontDefault();

        std::string path = Uma_FilePath::ASSET_ROOT + "Roboto-Medium.ttf";

        io.FontDefault = io.Fonts->AddFontFromFileTTF(path.c_str(), fontSize);

        // set up backend stuff
        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        const char* glsl_version = "#version 130";
        ImGui_ImplOpenGL3_Init(glsl_version);

        // event listeners
        pEventSystem = pSystemManager->GetSystem<EventSystem>();
        pEventSystem->Subscribe<DebugLogEvent, ImguiManager>([this](const DebugLogEvent& e) { AddConsoleLog(e.message); });
        pEventSystem->Subscribe<EntityCreatedEvent, ImguiManager>([this](const EntityCreatedEvent& e) { mEntityCount = e.entityCnt; });
        pEventSystem->Subscribe<EntityDestroyedEvent, ImguiManager>([this](const EntityDestroyedEvent& e) { mEntityCount = e.entityCnt; });
        pEventSystem->Subscribe<SceneInfoRequest, ImguiManager>([this](const SceneInfoRequest& e)
            { sceneNames = e.sceneNames; scenePaths = e.scenePaths; activeSceneIndex = e.activeSceneIndex; });
        pEventSystem->Subscribe<IMGUIStopRequest, ImguiManager>([this](const IMGUIStopRequest& e)
             { 
                (void)e;
                m_playState = PlayState::Stopped; 
            });
        pEventSystem->Subscribe<ReturnDuplicatedRequestEvent, ImguiManager>([this](const ReturnDuplicatedRequestEvent& e)
            { 
                m_selectedEntity = e.entity;
                m_HierarchyScrollToBottom = true;
            });
        pEventSystem->Subscribe<ReturnSpawnedRequestEvent, ImguiManager>([this](const ReturnSpawnedRequestEvent& e)
            {
                m_selectedEntity = e.entity;
                m_HierarchyScrollToBottom = true;
            });
        pEventSystem->Subscribe<EntityPickedEvent, ImguiManager>([this](const EntityPickedEvent& e)
            { 
                (void)e;
                m_selectedEntity = e.entity;
            });
        pEventSystem->Subscribe<EntityDroppedEvent, ImguiManager>([this](const EntityDroppedEvent& e)
            {
                (void)e;
                m_selectedEntity = static_cast<Entity>(-1);
            });

        // resources manager
        pResourcesManager = pSystemManager->GetSystem<ResourcesManager>();

        resourcesWindow.SetResourcesManager(pResourcesManager);

        fileBrowser.setEventSystem(pEventSystem);

        m_initialized = true;
    }

    void ImguiManager::Update(float deltaTime)
    {
        if (!m_initialized)
        {
            return;
        }

        auto graphics = pSystemManager->GetSystem<Graphics>();

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

        if (graphics && graphics->GetRenderTarget() == Uma_Engine::RenderTarget::Framebuffer)
        {
            graphics->UnbindFramebuffer();
        }

        StartFrame();

        CreateDockspace();

        if (graphics && graphics->GetRenderTarget() == Uma_Engine::RenderTarget::Framebuffer)
        {
            CreateSceneViewWindow();
        }
        else
        {
            // Game mode (rendering to window) - set viewport to full window size
            auto inputSystem = pSystemManager->GetSystem<HybridInputSystem>();
            if (inputSystem && graphics->GetWindow())
            {
                int width, height;
                glfwGetWindowSize(graphics->GetWindow(), &width, &height);
                HybridInputSystem::SetSceneViewport(
                    0.0f,
                    0.0f,
                    static_cast<float>(width),
                    static_cast<float>(height),
                    false  // Not framebuffer mode
                );
            }
        }
        
        if (fileBrowser.isPrefabEdit())
        {
            CreatePrefabControlBar();
            CreatePrefabHierarchyWindow();
            CreateInspectorWindow();
            fileBrowser.Render();
            resourcesWindow.Render();
            CreateConsoleWindow();
        }
        else
        {
            // play stop bar
            CreateEditorControlBar();
            if (!m_hideAll)
            {
                SceneManagerWindow();

                // call for windows to be shown
                float currentFps = deltaTime > 0.0f ? (1.0f / deltaTime) : 0.0f;
                CreateDebugWindows(currentFps, deltaTime);

                fileBrowser.Render();

                resourcesWindow.Render();
            }
        }

        // Check mouse over UI
        mouseOverUI =
            ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
            ImGui::IsAnyItemHovered() ||
            ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);

        // Allow editor interactions when hovering Scene View
        bool effectiveMouseOverUI = m_sceneViewHovered ? false : mouseOverUI;

        if (prevMouseOverUI != effectiveMouseOverUI)
        {
            prevMouseOverUI = effectiveMouseOverUI;
            pEventSystem->Emit<UpdateMouseOverUIEvent>(prevMouseOverUI);
        }

        if (!m_hideAll)
        {
            HandleUndoRedoInput();
        }

        Render();
    }

    void ImguiManager::Shutdown()
    {
        if (!m_initialized)
            return;

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        m_initialized = false;
        std::cout << "imgui SHUTDOWN" << std::endl;

        pEventSystem->UnsubscribeSystem<ImguiManager>();
    }

    void ImguiManager::SetWindow(GLFWwindow* window)
    {
        m_window = window;
        if (!m_initialized && m_window)
            Init();
    }

    // IMGUI SPECIFIC METHODS
    void ImguiManager::StartFrame()
    {
        if (!m_initialized)
            return;

        // start imgui fram
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImguiManager::Render()
    {
        if (!m_initialized)
            return;
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    // ACTUAL EDITOR METHODS
    void ImguiManager::CreateEditorControlBar()
    {
        if (!m_showEditorControlBar)
            return;

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y));
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, 40));
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 4));

        if (ImGui::Begin("##EditorControlBar", nullptr, flags))
        {
            // Center the buttons
            float buttonWidth = 80.0f;
            float spacing = 8.0f;
            float totalWidth = (buttonWidth * 3) + (spacing * 2);
            float offset = (ImGui::GetWindowWidth() - totalWidth) * 0.5f;
            ImGui::SetCursorPosX(offset);

            // Play Button
            bool isPlaying = (m_playState == PlayState::Playing);
            if (isPlaying)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.6f, 0.15f, 1.0f));
            }

            if (ImGui::Button("Play", ImVec2(buttonWidth, 0)))
            {
                if (sceneNames.size() > 0)
                {
                    if (m_playState == PlayState::Stopped || m_playState == PlayState::Paused)
                    {
                        commandHistory.Clear();
                        pEventSystem->Emit<PlaySceneRequest>();
                        m_playState = PlayState::Playing;
                    }
                }
            }

            if (isPlaying)
            {
                ImGui::PopStyleColor(3);
            }

            ImGui::SameLine();

            // Pause Button
            bool isPaused = (m_playState == PlayState::Paused);
            if (isPaused)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.7f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.8f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.6f, 0.15f, 1.0f));
            }

            if (ImGui::Button("Pause", ImVec2(buttonWidth, 0)))
            {
                if (sceneNames.size() > 0)
                {
                    if (m_playState == PlayState::Playing)
                    {
                        pEventSystem->Emit<PauseSceneRequest>();
                        m_playState = PlayState::Paused;
                    }
                    else if (m_playState == PlayState::Paused)
                    {
                        pEventSystem->Emit<PlaySceneRequest>();
                        m_playState = PlayState::Playing;
                    }
                }
            }

            if (isPaused)
            {
                ImGui::PopStyleColor(3);
            }

            ImGui::SameLine();

            // Stop Button
            if (ImGui::Button("Stop", ImVec2(buttonWidth, 0)))
            {
                if (sceneNames.size() > 0)
                {
                    if (m_playState != PlayState::Stopped)
                    {
                        commandHistory.Clear();
                        pEventSystem->Emit<StopSceneRequest>();
                        m_playState = PlayState::Stopped;
                        //pEventSystem->Emit<ReLoadSceneRequestEvent>();
                    }
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Hide Windows", ImVec2(buttonWidth + 20, 0)))
            {
                m_hideAll = !m_hideAll;
            }

            // Show current state text
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.5f - 250);

            const char* stateText = "Stopped";
            ImVec4 stateColor = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);

            switch (m_playState)
            {
            case PlayState::Playing:
                stateText = "Game View";
                stateColor = ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
                break;
            case PlayState::Paused:
                stateText = "Game Paused";
                stateColor = ImVec4(1.0f, 1.0f, 0.2f, 1.0f);
                break;
            case PlayState::Stopped:
                stateText = "Scene View";
                stateColor = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
                break;
            }

            ImGui::TextColored(stateColor, "%s", stateText);
        }

        ImGui::End();
        ImGui::PopStyleVar(2);
    }

    void ImguiManager::CreatePrefabControlBar()
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y));
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, 40));
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 4));

        if (ImGui::Begin("##PrefabControlBar", nullptr, flags))
        {
            // Center the buttons
            float buttonWidth = 80.0f;
            float spacing = 8.0f;
            float totalWidth = (buttonWidth * 3) + (spacing * 2);
            float offset = (ImGui::GetWindowWidth() - totalWidth) * 0.5f;
            ImGui::SetCursorPosX(offset);

            // Save Button
            if (ImGui::Button("Save", ImVec2(buttonWidth, 0)))
            {
                pEventSystem->Emit<SavePrefabRequestEvent>(fileBrowser.getPrefabName(), m_prefabEntity);
            }

            ImGui::SameLine();

            // Quit Button
            if (ImGui::Button("Quit", ImVec2(buttonWidth, 0)))
            {
                //pEventSystem->Emit<StopSceneRequest>();
                pEventSystem->Emit<LoadSceneRequestEvent>(fileBrowser.getPrevSceneName());
                pEventSystem->Emit<DeleteCurrSceneRequest>(fileBrowser.getPrefabSceneName());
                fileBrowser.setIsPrefabEdit(false);
            }
        }

        ImGui::End();
        ImGui::PopStyleVar(2);
    }

    void ImguiManager::CreateDebugWindows(float fps, float deltaTime)
    {
        if (!m_initialized)
            return;

        CreateEngineDebugWindow(fps, deltaTime);
        CreatePerformanceWindow();

        CreateHierarchyWindow();
        CreateInspectorWindow();
        CreateEditorCameraWindow();
		CreateSystemsWindow();
		//CreateEntityDebugWindow();
		CreateConsoleWindow();
		//CreateEntityPropertyWindow();

        /*auto graphics = pSystemManager->GetSystem<Graphics>();
        if (graphics && graphics->GetRenderTarget() == Uma_Engine::RenderTarget::Framebuffer)
        {
            CreateSceneViewWindow();
        }*/
    }

    void ImguiManager::CreateSceneViewWindow()
    {
        ImGui::Begin("Scene View", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        if (viewportSize.x < 50) viewportSize.x = 50;
        if (viewportSize.y < 50) viewportSize.y = 50;

        auto graphics = pSystemManager->GetSystem<Graphics>();
        if (!graphics)
        {
            ImGui::Text("Graphics system not available");
            m_sceneViewHovered = false;
            m_isMouseInSceneView = false;
            ImGui::End();
            return;
        }

        // Resize framebuffer to match window size
        graphics->ResizeSceneFramebuffer(
            static_cast<int>(viewportSize.x),
            static_cast<int>(viewportSize.y)
        );

        // Get position
        ImVec2 imagePos = ImGui::GetCursorScreenPos();

        // Update HybridInputSystem with viewport bounds for mouse coordinate transformation
        auto inputSystem = pSystemManager->GetSystem<HybridInputSystem>();
        if (inputSystem)
        {
            bool isFramebuffer = (graphics->GetRenderTarget() == Uma_Engine::RenderTarget::Framebuffer);
            HybridInputSystem::SetSceneViewport(
                imagePos.x,
                imagePos.y,
                viewportSize.x,
                viewportSize.y,
                isFramebuffer
            );
        }

        // Display the scene texture
        GLuint texID = graphics->GetSceneTexture();
        ImGui::Image(
            reinterpret_cast<void*>(static_cast<intptr_t>(texID)),
            viewportSize,
            ImVec2(0, 1),
            ImVec2(1, 0)
        );

        // Check if mouse is hovering the Scene View window
        m_sceneViewHovered = ImGui::IsWindowHovered() && ImGui::IsItemHovered();

        if (m_sceneViewHovered)
        {
            // Get mouse position relative to the image
            ImVec2 mousePos = ImGui::GetMousePos();
            float localX = mousePos.x - imagePos.x;
            float localY = mousePos.y - imagePos.y;

            // Check if mouse is within bounds
            if (localX >= 0 && localY >= 0 && localX < viewportSize.x && localY < viewportSize.y)
            {
                m_sceneViewMousePos = Vec2(localX, localY);
                m_isMouseInSceneView = true;
            }
            else
            {
                m_isMouseInSceneView = false;
            }
        }
        else
        {
            m_isMouseInSceneView = false;
        }

        ImGui::End();
    }

    void ImguiManager::CreateSystemsWindow()
    {
        if (!m_showSystemsWindow)
        {
            return;
        }
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

    void ImguiManager::CreatePerformanceWindow()
    {
        if (!m_showPerformanceWindow)
        {
            return;
        }

        ImGui::Begin("Performance Monitor", &m_showPerformanceWindow);

        // FPS graph
        ImGui::PlotLines("FPS", m_fpsHistory, 120, m_historyOffset, nullptr, 0.0f, 200.0f, ImVec2(0, 80));

        // Frame time graph
        ImGui::PlotLines("Frame Time (ms)", m_dtHistory, 120, m_historyOffset, nullptr, 0.0f, 50.0f, ImVec2(0, 80));

        ImGui::End();
    }

    void ImguiManager::CreateEngineDebugWindow(float fps, float deltaTime)
    {
        if (!m_showEngineDebug)
        {
            return;
        }

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

    void ImguiManager::CreateConsoleWindow()
    {
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

        ImGui::Spacing();

        ImGui::End();
    }

    void ImguiManager::AddConsoleLog(const std::string& message)
    {
        logsVec.push_back(message);

        // dont go beyond 100 messgaes shown
        if (logsVec.size() > 100)
            logsVec.erase(logsVec.begin());
    }

    void ImguiManager::CreateDockspace()
    {
        // Create main dockspace window
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags window_flags = /*ImGuiWindowFlags_MenuBar |*/ ImGuiWindowFlags_NoDocking;
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
            std::ifstream file(Uma_FilePath::CONFIG_ROOT + "imgui.ini");
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
            if (!has_layout)
                InitDockspace(ds_id, viewport);
        }

        ImGui::End();
    }

    void ImguiManager::InitDockspace(ImGuiID dockspace_id, ImGuiViewport* viewport) {
        // Clear any existing layout
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

        // Split the dockspace into regions (Unity-style layout)
        ImGuiID dock_main_id = dockspace_id;
        ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.15f, nullptr, &dock_main_id);
        ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.15f, nullptr, &dock_main_id);
        ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.15f, nullptr, &dock_main_id);

        if (!WindowsInit())
        {
            // Dock windows to their initial positions
            ImGui::DockBuilderDockWindow("Scene View", dock_main_id);
            ImGui::DockBuilderDockWindow("Hierarchy", dock_id_left);
            ImGui::DockBuilderDockWindow("Inspector", dock_id_right);
            ImGui::DockBuilderDockWindow("Editor Camera", dock_id_right);
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

    bool ImguiManager::WindowsInit(std::string filename)
    {
        return std::ifstream(filename).good();
    }

    //HIERARCHY/INSPECTOR WINDOW IMPLEMENTATION

    void ImguiManager::CreateHierarchyWindow()
    {
        bool b = true;
        ImGui::Begin("Hierarchy", &b);

        // Header with entity count
        if (sceneNames.size() > 0)
            ImGui::Text("%s | Scene Entities: %d", sceneNames[activeSceneIndex].c_str(), mEntityCount);
        ImGui::Separator();

        // Scrollable region for entity list
        ImGui::BeginChild("EntityList", ImVec2(0, 0), true);

        // Get coordinator from scene manager
        auto sceneManager = pSystemManager->GetSystem<SceneManager>();
        if (!sceneManager)
        {
            ImGui::EndChild();
            ImGui::End();
            return;
        }

        auto activeScene = sceneManager->GetActiveScene();
        if (!activeScene)
        {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No active scene");
            ImGui::EndChild();
            ImGui::End();
            return;
        }
        if (fileBrowser.getPrevSceneName() != sceneManager->GetActiveSceneName())
            fileBrowser.setPrevSceneName(sceneManager->GetActiveSceneName());

        auto& coordinator = activeScene->GetCoordinator();
        auto& transformArray = coordinator.GetComponentArray<Uma_ECS::Transform>();

        // Build a list of root entities (entities with no parent)
        std::vector<Uma_ECS::Entity> rootEntities;
        for (size_t i = 0; i < transformArray.Size(); ++i)
        {
            Uma_ECS::Entity entity = transformArray.GetEntity(i);
            auto& transform = transformArray.GetData(entity);

            if (!transform.parent.has_value())
            {
                rootEntities.push_back(entity);
            }
        }

        // Render each root entity and its children recursively
        for (Uma_ECS::Entity rootEntity : rootEntities)
        {
            RenderEntityNode(rootEntity, coordinator, transformArray);
        }

        if (m_HierarchyScrollToBottomFrames > 0)
        {
            ImGui::SetScrollHereY(1.0f);
            m_HierarchyScrollToBottomFrames--;
        }

        ImGui::EndChild();
        ImGui::End();
    }

    void ImguiManager::CreatePrefabHierarchyWindow()
    {
        bool b = true;
        ImGui::Begin("Prefab Editor", &b);

        // Header with entity count
        if (sceneNames.size() > 0)
            ImGui::Text("%s | Scene Entities: %d", sceneNames[activeSceneIndex].c_str(), mEntityCount);
        ImGui::Separator();

        // Scrollable region for entity list
        ImGui::BeginChild("EntityList", ImVec2(0, 0), true);

        // Get coordinator from scene manager
        auto sceneManager = pSystemManager->GetSystem<SceneManager>();
        if (!sceneManager)
        {
            ImGui::EndChild();
            ImGui::End();
            return;
        }

        auto activeScene = sceneManager->GetActiveScene();
        if (!activeScene)
        {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No active scene");
            ImGui::EndChild();
            ImGui::End();
            return;
        }

        auto& coordinator = activeScene->GetCoordinator();
        auto& transformArray = coordinator.GetComponentArray<Uma_ECS::Transform>();

        // Build a list of root entities (entities with no parent)
        Uma_ECS::Entity rootEntity = static_cast<Uma_ECS::Entity>(-1);
        bool firstRoot = true;

        rootEntity = transformArray.GetEntity(0);

        /*for (size_t i = 0; i < transformArray.Size(); ++i)
        {
            Uma_ECS::Entity entity = transformArray.GetEntity(i);
            auto& transform = transformArray.GetData(entity);

            if (!transform.parent.has_value())
            {
                if (firstRoot)
                {
                    rootEntity = entity;
                    firstRoot = false;
                }
            }
        }*/

        // Render root entity and its children recursively
        if (rootEntity != static_cast<Uma_ECS::Entity>(-1))
        {
            RenderPrefabNode(rootEntity, coordinator, transformArray);
            m_prefabEntity = rootEntity;
        }

        if (m_HierarchyScrollToBottom)
        {
            ImGui::SetScrollHereY(1.0f);
            m_HierarchyScrollToBottom = false;
        }

        ImGui::EndChild();
        ImGui::End();
    }

    void ImguiManager::RenderEntityNode(Uma_ECS::Entity entity, Uma_ECS::Coordinator& coordinator,
        Uma_ECS::ComponentArray<Uma_ECS::Transform>& transformArray)
    {
        if (!coordinator.HasActiveEntity(entity))
            return;

        auto& transform = transformArray.GetData(entity);
        bool hasChildren = !transform.children.empty();

        // Generate entity name based on components
        std::string entityName = GetEntityDisplayName(entity, coordinator);

        // Setup tree node flags
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

        if (!hasChildren)
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

        if (m_selectedEntity == entity)
            flags |= ImGuiTreeNodeFlags_Selected;

        // Push unique ID for this entity
        ImGui::PushID(static_cast<int>(entity));

        // Render tree node
        bool nodeOpen = ImGui::TreeNodeEx(entityName.c_str(), flags);

        // Handle selection
        if (ImGui::IsItemClicked())
        {
            // If switching to a different entity, commit any unsaved edits
            if (m_selectedEntity != entity && m_hasUnsavedEdit)
            {
                // Force commit current edit before switching
                auto sceneManager = pSystemManager->GetSystem<SceneManager>();
                if (sceneManager && sceneManager->GetActiveScene())
                {
                    auto& coordinator = sceneManager->GetActiveScene()->GetCoordinator();
                    Uma_Editor::EntitySnapshot snapshotAfter = CaptureEntitySnapshot(m_editingEntity, coordinator);

                    auto cmd = std::make_unique<Uma_Editor::EntitySnapshotCmd>(
                        &coordinator,
                        std::move(m_snapshotBeforeEdit),
                        std::move(snapshotAfter),       
                        "Transform Edit"
                    );

                    commandHistory.ExecuteCommand(std::move(cmd));
                }

                m_hasUnsavedEdit = false;
            }

            m_selectedEntity = entity;
            m_editingEntity = static_cast<Uma_ECS::Entity>(-1);  // Reset tracking
            pEventSystem->Emit<EntityPickedEvent>(m_selectedEntity);
        }

        // Right-click context menu
        if (ImGui::BeginPopupContextItem())
        {
            m_selectedEntity = entity;

            if (ImGui::MenuItem("Create New"))
            {
                //pEventSystem->Emit<SpawnEntityRequestEvent>();

                auto cmd = std::make_unique<Uma_Editor::EntityCreateCmd>(
                    &coordinator,
                    std::nullopt,
                    "Create New Entity"
                );

                // Get raw pointer before moving
                Uma_Editor::EntityCreateCmd* rawCmd = cmd.get();

                // Execute command through history
                commandHistory.ExecuteCommand(std::move(cmd));

                // Access through raw pointer (still valid, owned by command history now)
                m_selectedEntity = rawCmd->GetCreatedEntity();
               
                m_HierarchyScrollToBottomFrames = 2;
            }

            if (ImGui::MenuItem("Create Child"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityCreateCmd>(
                    &coordinator,
                    entity,
                    "Create New Entity"
                );

                // Get raw pointer before moving
                Uma_Editor::EntityCreateCmd* rawCmd = cmd.get();

                // Execute command through history
                commandHistory.ExecuteCommand(std::move(cmd));

                // Access through raw pointer (still valid, owned by command history now)
                m_selectedEntity = rawCmd->GetCreatedEntity();

                m_HierarchyScrollToBottomFrames = 2;
            }

            if (ImGui::MenuItem("Duplicate"))
            {
                //pEventSystem->Emit<DuplicateEntityRequestEvent>(m_selectedEntity);
                
                auto cmd = std::make_unique<Uma_Editor::EntityDuplicateCmd>(
                    &coordinator,
                    m_selectedEntity,
                    "Duplicate Entity"
                );

                // Get raw pointer before moving
                Uma_Editor::EntityDuplicateCmd* rawCmd = cmd.get();

                // Execute command through history
                commandHistory.ExecuteCommand(std::move(cmd));

                // Access through raw pointer (still valid, owned by command history now)
                m_selectedEntity = rawCmd->GetCreatedEntity();

                m_HierarchyScrollToBottomFrames = 2;
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Delete"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityDeleteCmd>(
                    &coordinator,
                    entity,
                    false,
                    "Delete " + GetEntityDisplayName(entity, coordinator)
                );

                commandHistory.ExecuteCommand(std::move(cmd));

                //pEventSystem->Emit<DestroyEntityRequestEvent>(entity);
                if (m_selectedEntity == entity)
                {
                    m_selectedEntity = static_cast<Uma_ECS::Entity>(-1);
                }
                m_HierarchyScrollToBottom = true;
            }

            if (ImGui::MenuItem("Delete with Children"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityDeleteCmd>(
                    &coordinator,
                    entity,
                    true,
                    "Delete " + GetEntityDisplayName(entity, coordinator)
                );

                commandHistory.ExecuteCommand(std::move(cmd));

                //coordinator.DestroyEntityAndChildren(entity);
                if (m_selectedEntity == entity)
                {
                    m_selectedEntity = static_cast<Uma_ECS::Entity>(-1);
                }
                m_HierarchyScrollToBottom = true;
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Save as Prefab"))
            {
                pEventSystem->Emit<SavePrefabRequestEvent>(entityName, entity);
                pEventSystem->Emit<RefreshDirectoryRequest>();
            }

            ImGui::EndPopup();
        }

        // Drag and drop for reparenting
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::SetDragDropPayload("ENTITY_NODE", &entity, sizeof(Uma_ECS::Entity));
            ImGui::Text("Reparent: %s", entityName.c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_NODE"))
            {
                Uma_ECS::Entity droppedEntity = *(Uma_ECS::Entity*)payload->Data;

                // Don't allow setting parent to itself or to its own children
                if (droppedEntity != entity && !IsChildOf(droppedEntity, entity, transformArray))
                {
                    coordinator.SetParent(droppedEntity, entity);
                }
            }
            ImGui::EndDragDropTarget();
        }

        // Render children recursively
        if (nodeOpen && hasChildren)
        {
            for (Uma_ECS::Entity child : transform.children)
            {
                RenderEntityNode(child, coordinator, transformArray);
            }
            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    void ImguiManager::RenderPrefabNode(Uma_ECS::Entity entity, Uma_ECS::Coordinator& coordinator,
        Uma_ECS::ComponentArray<Uma_ECS::Transform>& transformArray)
    {
        if (!coordinator.HasActiveEntity(entity))
            return;

        auto& transform = transformArray.GetData(entity);
        bool hasChildren = !transform.children.empty();

        // Generate entity name based on components
        std::string entityName = GetEntityDisplayName(entity, coordinator);

        // Setup tree node flags
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

        if (!hasChildren)
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

        if (m_selectedEntity == entity)
            flags |= ImGuiTreeNodeFlags_Selected;

        // Push unique ID for this entity
        ImGui::PushID(static_cast<int>(entity));

        // Render tree node
        bool nodeOpen = ImGui::TreeNodeEx(entityName.c_str(), flags);

        // Handle selection
        if (ImGui::IsItemClicked())
        {
            m_selectedEntity = entity;
            pEventSystem->Emit<EntityPickedEvent>(m_selectedEntity);
        }

        // Right-click context menu
        if (ImGui::BeginPopupContextItem())
        {
            m_selectedEntity = entity;

            if (ImGui::MenuItem("Create Child"))
            {
                Uma_ECS::Entity child = coordinator.CreateEntity();
                coordinator.AddComponent(child, Uma_ECS::Transform{
                    .name = std::string("new enity"),
                    .position = Vec2(0, 0),
                    .rotation = Vec2(0, 0),
                    .scale = Vec2(1, 1)
                    });
                coordinator.SetParent(child, entity);

                m_HierarchyScrollToBottom = true;
            }

            if (coordinator.GetParent(m_selectedEntity) != std::nullopt && ImGui::MenuItem("Duplicate"))
            {
                Entity newEntity = coordinator.DuplicateEntity(entity);
                coordinator.SetParent(newEntity, coordinator.GetParent(entity).value());
                m_HierarchyScrollToBottom = true;
            }

            ImGui::Separator();

            if (coordinator.GetParent(m_selectedEntity) != std::nullopt && transformArray.Size() > 1 && ImGui::MenuItem("Delete"))
            {
                pEventSystem->Emit<DestroyEntityRequestEvent>(entity);
                if (m_selectedEntity == entity)
                {
                    m_selectedEntity = static_cast<Uma_ECS::Entity>(-1);
                }
                m_HierarchyScrollToBottom = true;
            }

            if (coordinator.GetParent(m_selectedEntity) != std::nullopt && transformArray.Size() > 1 && ImGui::MenuItem("Delete with Children"))
            {
                coordinator.DestroyEntityAndChildren(entity);
                if (m_selectedEntity == entity)
                {
                    m_selectedEntity = static_cast<Uma_ECS::Entity>(-1);
                }
                m_HierarchyScrollToBottom = true;
            }

            ImGui::EndPopup();
        }

        // Drag and drop for reparenting
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::SetDragDropPayload("ENTITY_NODE", &entity, sizeof(Uma_ECS::Entity));
            ImGui::Text("Reparent: %s", entityName.c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_NODE"))
            {
                Uma_ECS::Entity droppedEntity = *(Uma_ECS::Entity*)payload->Data;

                // Don't allow setting parent to itself or to its own children
                if (droppedEntity != entity && !IsChildOf(droppedEntity, entity, transformArray))
                {
                    coordinator.SetParent(droppedEntity, entity);
                }
            }
            ImGui::EndDragDropTarget();
        }

        // Render children recursively
        if (nodeOpen && hasChildren)
        {
            for (Uma_ECS::Entity child : transform.children)
            {
                RenderPrefabNode(child, coordinator, transformArray);
            }
            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    std::string ImguiManager::GetEntityDisplayName(Uma_ECS::Entity entity, Uma_ECS::Coordinator& coordinator)
    {

        // Add component indicators
        /*if (coordinator.GetComponentArray<Uma_ECS::Player>().Has(entity))
            name = "[Player] " + name;
        else if (coordinator.GetComponentArray<Uma_ECS::Enemy>().Has(entity))
            name = "[Enemy] " + name;
        else if (coordinator.GetComponentArray<Uma_ECS::Camera>().Has(entity))
            name = "[Camera] " + name;
        else if (coordinator.GetComponentArray<Uma_ECS::Sprite>().Has(entity))
        {
            auto& sprite = coordinator.GetComponent<Uma_ECS::Sprite>(entity);
            if (!sprite.textureName.empty())
                name = "[" + sprite.textureName + "] " + name;
        }*/

        auto& tfArray = coordinator.GetComponentArray<Uma_ECS::Transform>();

        std::string name = "Entity " + std::to_string(entity);
        if (tfArray.Has(entity))
        {
            const auto& tf = tfArray.GetData(entity);
            name = tf.name;
        }

        return name;
    }

    bool ImguiManager::IsChildOf(Uma_ECS::Entity potentialChild, Uma_ECS::Entity potentialParent,
        Uma_ECS::ComponentArray<Uma_ECS::Transform>& transformArray)
    {
        if (!transformArray.Has(potentialChild))
            return false;

        auto& transform = transformArray.GetData(potentialChild);

        if (!transform.parent.has_value())
            return false;

        Uma_ECS::Entity parent = transform.parent.value();

        if (parent == potentialParent)
            return true;

        return IsChildOf(parent, potentialParent, transformArray);
    }

    bool ImguiManager::DisplayComponent(Uma_ECS::Coordinator& coordinator, Uma_ECS::ComponentType type, Uma_ECS::Entity& entity)
    {
        if (type == coordinator.GetComponentType<Uma_ECS::Transform>())
        {
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto& transform = coordinator.GetComponent<Uma_ECS::Transform>(entity);
                ImGui::Indent();

                // begin tracking
                BeginComponentEdit(entity, coordinator);

                float position[2] = { transform.position.x, transform.position.y };
                if (ImGui::DragFloat2("Position", position, 0.1f))
                {
                    transform.position = Vec2(position[0], position[1]);
                    transform.isDirty = true;
                    m_hasUnsavedEdit = true;
                }

                float rotation = transform.rotation.x;
                if (ImGui::DragFloat("Rotation", &rotation, 1.0f))
                {
                    transform.rotation.x = rotation;
                    transform.isDirty = true;
                    m_hasUnsavedEdit = true;
                }

                float scale[2] = { transform.scale.x, transform.scale.y };
                if (ImGui::DragFloat2("Scale", scale, 0.01f))
                {
                    transform.scale = Vec2(scale[0], scale[1]);
                    transform.isDirty = true;
                    m_hasUnsavedEdit = true;
                }

                // end tracking
                EndComponentEdit(entity, coordinator, "Transform");

                ImGui::Separator();
                ImGui::Text("Hierarchy");

                if (transform.parent.has_value())
                {
                    ImGui::Text("Parent: Entity %d", transform.parent.value());
                    if (ImGui::Button("Remove Parent"))
                    {
                        coordinator.RemoveParent(entity);
                    }
                }
                else
                {
                    ImGui::TextDisabled("Parent: None");
                }

                if (!transform.children.empty())
                {
                    ImGui::Text("Children: %zu", transform.children.size());
                    for (auto child : transform.children)
                    {
                        ImGui::BulletText("Entity %d", child);
                    }
                }

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::RigidBody>())
        {
            if (ImGui::CollapsingHeader("RigidBody", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto& rb = coordinator.GetComponent<Uma_ECS::RigidBody>(entity);
                ImGui::Indent();

                // begin tracking
                BeginComponentEdit(entity, coordinator);

                float velocity[2] = { rb.velocity.x, rb.velocity.y };
                if (ImGui::DragFloat2("Velocity", velocity, 0.1f))
                {
                    rb.velocity = Vec2(velocity[0], velocity[1]);
                    m_hasUnsavedEdit = true;
                }

                float acceleration[2] = { rb.acceleration.x, rb.acceleration.y };
                if (ImGui::DragFloat2("Acceleration", acceleration, 0.1f))
                {
                    rb.acceleration = Vec2(acceleration[0], acceleration[1]);
                    m_hasUnsavedEdit = true;
                }

                if (ImGui::DragFloat("Accel Strength", &rb.accel_strength, 0.1f, 0.0f, 1000.0f))   m_hasUnsavedEdit = true;
                if (ImGui::DragFloat("Friction Coefficient", &rb.fric_coeff, 0.01f, 0.0f, 10.0f))  m_hasUnsavedEdit = true;

                // end tracking
                EndComponentEdit(entity, coordinator, "RigidBody");

                ImGui::Separator();
                ImGui::Text("Info");
                ImGui::Text("Speed: %.2f", std::sqrt(rb.velocity.x * rb.velocity.x + rb.velocity.y * rb.velocity.y));

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::Sprite>())
        {
            if (ImGui::CollapsingHeader("Sprite"))
            {
                auto& sprite = coordinator.GetComponent<Uma_ECS::Sprite>(entity);
                ImGui::Indent();

                // begin tracking
                BeginComponentEdit(entity, coordinator);

                // Texture name input
                ImGui::Text("Texture: %s", sprite.textureName.c_str());
                static char textureBuffer[256];
                strncpy(textureBuffer, sprite.textureName.c_str(), 255);
                textureBuffer[255] = '\0';
                if (ImGui::InputText("Texture Name", textureBuffer, 256))
                {
                    sprite.textureName = textureBuffer;
                    sprite.texture = nullptr; // Will reload
                    m_hasUnsavedEdit = true;
                }

                // Flip flags
                if (ImGui::Checkbox("Flip X", &sprite.flipX))  m_hasUnsavedEdit = true;
                if (ImGui::Checkbox("Flip Y", &sprite.flipY))  m_hasUnsavedEdit = true;
                if (ImGui::Checkbox("Use Native Size", &sprite.UseNativeSize)) m_hasUnsavedEdit = true;

                // Render layer dropdown
                const char* renderLayerNames[] = {
                    "RL_NONE",
                    "RL_WALL_TOP",
                    "RL_FLOOR",
                    "RL_ENV",
                    "RL_ENEMY",
                    "RL_PLAYER",
                    "RL_WALL_BTM",
                    "RL_UI"
                };
                int currentRenderLayer = 0;
                unsigned int rl = static_cast<unsigned int>(sprite.renderLayer);
                while (rl >>= 1) ++currentRenderLayer;
                if (ImGui::Combo("Render Layer", &currentRenderLayer, renderLayerNames, IM_ARRAYSIZE(renderLayerNames)))
                {
                    sprite.renderLayer = (1u << currentRenderLayer);
                    m_hasUnsavedEdit = true;
                }

                ImGui::Separator();
                ImGui::Text("Color & Alpha");

                // Tint color (RGB)
                float tintColorArray[3] = { sprite.tintColor.x, sprite.tintColor.y, sprite.tintColor.z };
                if (ImGui::ColorEdit3("Tint Color", tintColorArray))
                {
                    sprite.tintColor.x = tintColorArray[0];
                    sprite.tintColor.y = tintColorArray[1];
                    sprite.tintColor.z = tintColorArray[2];
                    m_hasUnsavedEdit = true;
                }

                // Alpha (opacity)
                if (ImGui::SliderFloat("Alpha", &sprite.alpha, 0.0f, 1.0f, "%.2f")) m_hasUnsavedEdit = true;

                ImGui::Separator();
                ImGui::Text("Sprite Sheet");

                // Sprite sheet grid (columns and rows)
                float gridArray[2] = { sprite.spriteSheetGrid.x, sprite.spriteSheetGrid.y };
                if (ImGui::DragFloat2("Grid (Cols x Rows)", gridArray, 1.0f, 1.0f, 100.0f, "%.0f"))
                {
                    sprite.spriteSheetGrid.x = gridArray[0];
                    sprite.spriteSheetGrid.y = gridArray[1];
                    m_hasUnsavedEdit = true;
                }

                // Sprite cell (which cell to render)
                float cellArray[2] = { sprite.spriteCell.x, sprite.spriteCell.y };
                if (ImGui::DragFloat2("Cell (Col, Row)", cellArray, 1.0f, 0.0f,
                    max(sprite.spriteSheetGrid.x - 1.0f, 0.0f), "%.0f"))
                {
                    sprite.spriteCell.x = cellArray[0];
                    sprite.spriteCell.y = cellArray[1];
                    m_hasUnsavedEdit = true;
                }

                // end tracking
                EndComponentEdit(entity, coordinator, "Sprite");

                ImGui::Separator();

                // Texture info (read-only)
                if (sprite.texture)
                {
                    ImGui::Text("Texture ID: %u", sprite.texture->tex_id);
                    ImGui::Text("Native Size: %.0f x %.0f", sprite.texture->GetNativeSize().x, sprite.texture->GetNativeSize().y);

                    // Show UV info
                    Vec2 uvOffset, uvSize;
                    sprite.GetUVs(uvOffset, uvSize);
                    ImGui::Text("UV Offset: (%.3f, %.3f)", uvOffset.x, uvOffset.y);
                    ImGui::Text("UV Size: (%.3f, %.3f)", uvSize.x, uvSize.y);
                }
                else
                {
                    ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Texture not loaded");
                }

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::Collider>())
        {
            if (ImGui::CollapsingHeader("Collider"))
            {
                auto& collider = coordinator.GetComponent<Uma_ECS::Collider>(entity);
                ImGui::Indent();

                // begin tracking
                BeginComponentEdit(entity, coordinator);

                if (ImGui::Checkbox("Show Bounding Box", &collider.showBBox)) m_hasUnsavedEdit = true;

                ImGui::Separator();
                ImGui::Text("Default Settings");

                // Collision layer names for dropdowns
                const char* collisionLayerNames[] = {
                    "CL_DEFAULT",
                    "CL_PLAYER",
                    "CL_ENEMY",
                    "CL_WALL",
                    "CL_PROJECTILE",
                    "CL_PICKUP",
                    "CL_ALL"
                };

                // Default Layer dropdown
                int defaultLayerIndex = 0;
                unsigned int dl = static_cast<unsigned int>(collider.defaultLayer);
                if (dl > 0)
                {
                    while (dl >>= 1) ++defaultLayerIndex;
                }

                if (ImGui::Combo("Default Layer", &defaultLayerIndex, collisionLayerNames, IM_ARRAYSIZE(collisionLayerNames)))
                {
                    collider.defaultLayer = (1u << defaultLayerIndex);
                    m_hasUnsavedEdit = true;
                }

                // Default Mask - using multi-select checkboxes for mask
                ImGui::Text("Default Mask:");
                ImGui::Indent();
                unsigned int tempMask = collider.defaultMask;
                for (int i = 0; i < IM_ARRAYSIZE(collisionLayerNames); ++i)
                {
                    bool isSet = (tempMask & (1u << i)) != 0;
                    if (ImGui::Checkbox(collisionLayerNames[i], &isSet))
                    {
                        if (isSet)
                            collider.defaultMask |= (1u << i);
                        else
                            collider.defaultMask &= ~(1u << i);

                        m_hasUnsavedEdit = true;
                    }
                }
                ImGui::Unindent();

                ImGui::Separator();
                ImGui::Text("Shapes: %zu", collider.shapes.size());

                for (size_t i = 0; i < collider.shapes.size(); ++i)
                {
                    ImGui::PushID(static_cast<int>(i));

                    auto& shape = collider.shapes[i];

                    if (ImGui::TreeNode("Shape", "Shape %zu %s", i, shape.isActive ? "" : "(Inactive)"))
                    {
                        if (ImGui::Checkbox("Active", &shape.isActive)) m_hasUnsavedEdit = true;
                        if (ImGui::Checkbox("Auto Fit to Sprite", &shape.autoFitToSprite)) m_hasUnsavedEdit = true;

                        float size[2] = { shape.size.x, shape.size.y };
                        if (ImGui::DragFloat2("Size", size, 0.1f, 0.0f, 100.0f))
                        {
                            shape.size = Vec2(size[0], size[1]);
                            m_hasUnsavedEdit = true;
                        }

                        float offset[2] = { shape.offset.x, shape.offset.y };
                        if (ImGui::DragFloat2("Offset", offset, 0.1f))
                        {
                            shape.offset = Vec2(offset[0], offset[1]);
                            m_hasUnsavedEdit = true;
                        }

                        // Purpose dropdown
                        const char* purposes[] = { "Physics", "Environment", "Trigger" };
                        int currentPurpose = static_cast<int>(shape.purpose);
                        if (ImGui::Combo("Purpose", &currentPurpose, purposes, 3))
                        {
                            shape.purpose = static_cast<Uma_ECS::ColliderPurpose>(currentPurpose);
                            m_hasUnsavedEdit = true;
                        }

                        // Layer dropdown
                        int layerIndex = 0;
                        unsigned int l = static_cast<unsigned int>(shape.layer);
                        if (l > 0)
                        {
                            while (l >>= 1) ++layerIndex;
                            m_hasUnsavedEdit = true;
                        }

                        if (ImGui::Combo("Collision Layer", &layerIndex, collisionLayerNames, IM_ARRAYSIZE(collisionLayerNames)))
                        {
                            shape.layer = (1u << layerIndex);
                            m_hasUnsavedEdit = true;
                        }

                        // Collider Mask - using multi-select checkboxes
                        ImGui::Text("Collider Mask:");
                        ImGui::Indent();
                        unsigned int tempShapeMask = shape.colliderMask;
                        for (int j = 0; j < IM_ARRAYSIZE(collisionLayerNames); ++j)
                        {
                            ImGui::PushID(j);
                            bool isSet = (tempShapeMask & (1u << j)) != 0;
                            if (ImGui::Checkbox(collisionLayerNames[j], &isSet))
                            {
                                if (isSet)
                                    shape.colliderMask |= (1u << j);
                                else
                                    shape.colliderMask &= ~(1u << j);

                                m_hasUnsavedEdit = true;
                            }
                            ImGui::PopID();
                        }
                        ImGui::Unindent();

                        if (ImGui::Button("Remove Shape"))
                        {
                            collider.shapes.erase(collider.shapes.begin() + i);
                            collider.bounds.resize(collider.shapes.size());
                            ImGui::TreePop();
                            ImGui::PopID();

                            m_hasUnsavedEdit = true;

                            break;
                        }

                        ImGui::TreePop();
                    }

                    ImGui::PopID();
                }

                if (ImGui::Button("Add Shape"))
                {
                    Uma_ECS::ColliderShape newShape;
                    newShape.size = Vec2(1.0f, 1.0f);
                    newShape.offset = Vec2(0.0f, 0.0f);
                    newShape.purpose = Uma_ECS::ColliderPurpose::Physics;
                    newShape.isActive = true;
                    collider.shapes.push_back(newShape);
                    collider.bounds.resize(collider.shapes.size());

                    m_hasUnsavedEdit = true;
                }

                // end tracking
                EndComponentEdit(entity, coordinator, "Collider");

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::Camera>())
        {
            if (ImGui::CollapsingHeader("Camera"))
            {
                auto& camera = coordinator.GetComponent<Uma_ECS::Camera>(entity);
                ImGui::Indent();

                // begin tracking
                BeginComponentEdit(entity, coordinator);

                if (ImGui::DragFloat("Zoom", &camera.mZoom, 0.1f, 0.1f, 10.0f)) m_hasUnsavedEdit = true;
                if (ImGui::Checkbox("Follow Player", &camera.followPlayer)) m_hasUnsavedEdit = true;

                ImGui::Separator();
                ImGui::Text("Camera Controls");
                if (ImGui::Button("Reset Zoom"))
                {
                    camera.mZoom = 1.0f;
                    m_hasUnsavedEdit = true;
                }

                EndComponentEdit(entity, coordinator, "Camera");

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::Player>())
        {
            if (ImGui::CollapsingHeader("Player"))
            {
                auto& player = coordinator.GetComponent<Uma_ECS::Player>(entity);
                ImGui::Indent();

                // begin tracking
                BeginComponentEdit(entity, coordinator);

                if (ImGui::DragInt("Health", &player.mHealth, 0.1f, 0, 300)) m_hasUnsavedEdit = true;
                if (ImGui::DragInt("Max Health", &player.mMaxHealth, 0, 0, 300)) m_hasUnsavedEdit = true;
                if (ImGui::DragFloat("Health Regen", &player.mHealthRegenRate, 0.1f, 0.0f, 10.0f)) m_hasUnsavedEdit = true;
                if (ImGui::DragFloat("Speed", &player.mSpeed, 0.1f, 0.0f, 20.f)) m_hasUnsavedEdit = true;
                if (ImGui::DragFloat("Dash Speed", &player.mDashSpeed, 0.1f, 0.0f, 100.0f)) m_hasUnsavedEdit = true;
                if (ImGui::DragFloat("Dash CD", &player.mDashCD, 0.1f, 0.0f, 100.0f)) m_hasUnsavedEdit = true;
                if (ImGui::DragInt("Atk Dmg", &player.mAttackDamage, 0.1f, 0, 100)) m_hasUnsavedEdit = true;
                if (ImGui::DragFloat("Atk Spd", &player.mAttackSpeed, 0.1f, 0.0f, 100.0f)) m_hasUnsavedEdit = true;
                if (ImGui::DragFloat("Atk Range", &player.mAttackRange, 0.1f, 0.0f, 100.0f)) m_hasUnsavedEdit = true;
                if (ImGui::DragInt("Def", &player.mDefense, 0.1f, 0, 100)) m_hasUnsavedEdit = true;
                if (ImGui::DragInt("Mana", &player.mMana, 0.1f, 0, 300)) m_hasUnsavedEdit = true;
                if (ImGui::DragInt("Max Mana", &player.mMaxMana, 0.1f, 0, 300)) m_hasUnsavedEdit = true;
                if (ImGui::DragFloat("Mana Regen", &player.mManaRegenRate, 0.1f, 0.0f, 100.0f)) m_hasUnsavedEdit = true;

                ImGui::Separator();
                ImGui::Text("Player Tag Component");
                ImGui::TextDisabled("This entity is marked as the player");

                // end tracking
                EndComponentEdit(entity, coordinator, "Player");

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::Enemy>())
        {
            if (ImGui::CollapsingHeader("Enemy"))
            {
                auto& enemy = coordinator.GetComponent<Uma_ECS::Enemy>(entity);
                ImGui::Indent();

                // begin tracking
                BeginComponentEdit(entity, coordinator);

                if (ImGui::DragInt("Health", &enemy.mHealth, 0.1f, 0, 300)) m_hasUnsavedEdit = true;
                if (ImGui::DragInt("Max Health", &enemy.mMaxHealth, 0, 0, 300)) m_hasUnsavedEdit = true;
                if (ImGui::DragFloat("Health Regen", &enemy.mHealthRegenRate, 0.1f, 0.0f, 10.0f)) m_hasUnsavedEdit = true;
                if (ImGui::DragFloat("Speed", &enemy.mSpeed, 0.1f, 0.0f, 20.f)) m_hasUnsavedEdit = true;
                if (ImGui::DragInt("Atk Dmg", &enemy.mAttackDamage, 0.1f, 0, 100)) m_hasUnsavedEdit = true;
                if (ImGui::DragFloat("Atk Spd", &enemy.mAttackSpeed, 0.1f, 0.0f, 100.0f)) m_hasUnsavedEdit = true;
                if (ImGui::DragFloat("Atk Range", &enemy.mAttackRange, 0.1f, 0.0f, 100.0f)) m_hasUnsavedEdit = true;
                if (ImGui::DragInt("Def", &enemy.mDefense, 0.1f, 0, 100)) m_hasUnsavedEdit = true;

                ImGui::Separator();
                ImGui::Text("Enemy Tag Component");
                ImGui::TextDisabled("This entity is marked as an enemy");

                // end tracking
                EndComponentEdit(entity, coordinator, "Enemy");

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::Animator>())
        {
            if (ImGui::CollapsingHeader("Animator"))
            {
                auto& animator = coordinator.GetComponent<Uma_ECS::Animator>(entity);
                ImGui::Indent();

                // begin tracking
                BeginComponentEdit(entity, coordinator);

                // Auto-play checkbox
                if (ImGui::Checkbox("Auto Play", &animator.autoPlay)) m_hasUnsavedEdit = true;

                // Initial clip input
                static char initialClipBuffer[256];
                strncpy(initialClipBuffer, animator.initialClip.c_str(), 255);
                initialClipBuffer[255] = '\0';
                if (ImGui::InputText("Initial Clip", initialClipBuffer, 256))
                {
                    animator.initialClip = initialClipBuffer;
                    m_hasUnsavedEdit = true;
                }

                ImGui::Separator();
                ImGui::Text("Current State");

                // Display current clip and playing status
                ImGui::Text("Current Clip: %s", animator.animator.GetCurrentClip().c_str());
                ImGui::Text("Is Playing: %s", animator.animator.IsPlaying() ? "Yes" : "No");

                // Current frame UVs (read-only)
                ImGui::Text("UV Offset: (%.3f, %.3f)", animator.uvOffset.x, animator.uvOffset.y);
                ImGui::Text("UV Size: (%.3f, %.3f)", animator.uvSize.x, animator.uvSize.y);

                ImGui::Separator();
                ImGui::Text("Animation Clips");

                // List all clips with play and delete buttons
                const auto& clips = animator.animator.GetClips();
                for (const auto& [name, clip] : clips)
                {
                    ImGui::PushID(name.c_str());

                    if (ImGui::TreeNode(name.c_str()))
                    {
                        ImGui::Text("Frames X: %d", clip.framesX);
                        ImGui::Text("Frames Y: %d", clip.framesY);
                        ImGui::Text("Start Frame: %d", clip.startFrame);
                        ImGui::Text("Frame Count: %d", clip.frameCount);
                        ImGui::Text("Speed: %.2f fps", clip.speed);
                        ImGui::Text("Loop: %s", clip.loop ? "Yes" : "No");

                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();

                        // Control buttons
                        if (ImGui::Button("Play", ImVec2(100, 0)))
                        {
                            animator.animator.Play(name);
                            m_hasUnsavedEdit = true;
                        }
                        ImGui::SameLine();

                        if (ImGui::Button("Play (Restart)", ImVec2(100, 0)))
                        {
                            animator.animator.Play(name, true);
                            m_hasUnsavedEdit = true;
                        }

                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();

                        // Delete button with confirmation color
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));

                        if (ImGui::Button("Delete Clip", ImVec2(-1, 0)))
                        {
                            // Store name before removing (since we're iterating over the map)
                            std::string clipToRemove = name;

                            if (animator.animator.RemoveClip(clipToRemove))
                            {
                                // If this was the initial clip, clear it
                                if (animator.initialClip == clipToRemove)
                                {
                                    animator.initialClip.clear();
                                }

                                m_hasUnsavedEdit = true;
                            }

                            ImGui::PopStyleColor(3);
                            ImGui::TreePop();
                            ImGui::PopID();
                            break;
                        }

                        ImGui::PopStyleColor(3);

                        ImGui::TreePop();
                    }

                    ImGui::PopID();
                }

                ImGui::Separator();
                ImGui::Text("Add New Clip");

                // Static variables to hold new clip data
                static char newClipName[256] = "";
                static int newFramesX = 1;
                static int newFramesY = 1;
                static int newStartFrame = 0;
                static int newFrameCount = 1;
                static float newSpeed = 10.0f;
                static bool newLoop = true;

                if (ImGui::InputText("Clip Name", newClipName, 256)) m_hasUnsavedEdit = true;
                if (ImGui::DragInt("Frames X", &newFramesX, 1.0f, 1, 100)) m_hasUnsavedEdit = true;
                if (ImGui::DragInt("Frames Y", &newFramesY, 1.0f, 1, 100)) m_hasUnsavedEdit = true;
                if (ImGui::DragInt("Start Frame", &newStartFrame, 1.0f, 0, 1000)) m_hasUnsavedEdit = true;
                if (ImGui::DragInt("Frame Count", &newFrameCount, 1.0f, 1, 1000)) m_hasUnsavedEdit = true;
                if (ImGui::DragFloat("Speed (fps)", &newSpeed, 0.1f, 0.1f, 60.0f)) m_hasUnsavedEdit = true;
                if (ImGui::Checkbox("Loop", &newLoop)) m_hasUnsavedEdit = true;

                if (ImGui::Button("Add Clip"))
                {
                    if (strlen(newClipName) > 0)
                    {
                        animator.animator.AddClip(
                            newClipName,
                            newFramesX,
                            newFramesY,
                            newStartFrame,
                            newFrameCount,
                            newSpeed,
                            newLoop
                        );

                        // Reset input fields
                        newClipName[0] = '\0';
                        newFramesX = 1;
                        newFramesY = 1;
                        newStartFrame = 0;
                        newFrameCount = 1;
                        newSpeed = 10.0f;
                        newLoop = true;
                    }
                    m_hasUnsavedEdit = true;
                }

                // Playback controls
                ImGui::Separator();
                ImGui::Text("Playback Controls");

                if (ImGui::Button("Reset"))
                {
                    animator.animator.Reset();
                    m_hasUnsavedEdit = true;
                }

                // end tracking
                EndComponentEdit(entity, coordinator, "RigidBody");

                ImGui::Unindent();
            }
            }
        else if (type == coordinator.GetComponentType<Uma_ECS::LuaScript>())
        {
            if (ImGui::CollapsingHeader("LuaScript"))
            {
                auto& luaScript = coordinator.GetComponent<Uma_ECS::LuaScript>(entity);
                ImGui::Indent();

                BeginComponentEdit(entity, coordinator);

                ImGui::Text("Scripts: %zu", luaScript.scripts.size());

                for (size_t i = 0; i < luaScript.scripts.size(); ++i)
                {
                    auto& script = luaScript.scripts[i];

                    ImGui::PushID(static_cast<int>(i));

                    std::string label = "Script " + std::to_string(i);
                    if (ImGui::TreeNode(label.c_str(), "%s %s",
                        script.scriptPath.c_str(),
                        script.isEnabled ? "" : "(Disabled)"))
                    {
                        if (ImGui::Checkbox("Enabled", &script.isEnabled))
                        {
                            m_hasUnsavedEdit = true;
                        }

                        ImGui::Separator();

                        if (script.hasError)
                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
                            ImGui::TextWrapped("Error: %s", script.errorMessage.c_str());
                            ImGui::PopStyleColor();
                        }
                        else if (script.isInitialized)
                        {
                            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Status: Running");
                        }
                        else
                        {
                            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Status: Not Initialized");
                        }

                        ImGui::Separator();

                        if (!script.exposedVariables.empty())
                        {
                            ImGui::Text("Exposed Variables:");

                            for (auto& var : script.exposedVariables)
                            {
                                ImGui::PushID(var.name.c_str());

                                switch (var.type)
                                {
                                case Uma_ECS::LuaVarType::T_FLOAT:
                                {
                                    float val = std::get<float>(var.value);
                                    if (var.isSlider)
                                    {
                                        if (ImGui::SliderFloat(var.name.c_str(), &val, var.min, var.max))
                                        {
                                            var.value = val;
                                            script.isVariableDirty = true;
                                            m_hasUnsavedEdit = true;
                                        }
                                    }
                                    else
                                    {
                                        if (ImGui::DragFloat(var.name.c_str(), &val, 0.1f))
                                        {
                                            var.value = val;
                                            script.isVariableDirty = true;
                                            m_hasUnsavedEdit = true;
                                        }
                                    }
                                    break;
                                }
                                case Uma_ECS::LuaVarType::T_INT:
                                {
                                    int val = std::get<int>(var.value);
                                    if (ImGui::DragInt(var.name.c_str(), &val))
                                    {
                                        var.value = val;
                                        script.isVariableDirty = true;
                                        m_hasUnsavedEdit = true;
                                    }
                                    break;
                                }
                                case Uma_ECS::LuaVarType::T_BOOL:
                                {
                                    bool val = std::get<bool>(var.value);
                                    if (ImGui::Checkbox(var.name.c_str(), &val))
                                    {
                                        var.value = val;
                                        script.isVariableDirty = true;
                                        m_hasUnsavedEdit = true;
                                    }
                                    break;
                                }
                                case Uma_ECS::LuaVarType::T_STRING:
                                {
                                    std::string val = std::get<std::string>(var.value);
                                    char buffer[256];
                                    strncpy(buffer, val.c_str(), 255);
                                    buffer[255] = '\0';
                                    if (ImGui::InputText(var.name.c_str(), buffer, 256))
                                    {
                                        var.value = std::string(buffer);
                                        script.isVariableDirty = true;
                                        m_hasUnsavedEdit = true;
                                    }
                                    break;
                                }
                                }

                                ImGui::PopID();
                            }
                        }
                        else
                        {
                            ImGui::TextDisabled("No exposed variables");
                        }

                        ImGui::Separator();

                        if (ImGui::Button("Remove Script", ImVec2(-1, 0)))
                        {
                            luaScript.RemoveScript(i);
                            ImGui::TreePop();
                            ImGui::PopID();

                            m_hasUnsavedEdit = true;

                            break;
                        }

                        ImGui::TreePop();
                    }

                    ImGui::PopID();
                }

                ImGui::Separator();

                if (ImGui::Button("Add Script", ImVec2(-1, 0)))
                {
                    ImGui::OpenPopup("Add Lua Script");
                }

                // Define popup EVERY frame
                if (ImGui::BeginPopupModal("Add Lua Script", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::Text("Script Name:");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(250);

                    static char scriptNameBuffer[256] = "";
                    if (ImGui::InputText("##scriptname", scriptNameBuffer, IM_ARRAYSIZE(scriptNameBuffer)))
                    {
                        // Auto-construct full path
                        mScriptName = "Assets/Scripts/";
                        mScriptName += scriptNameBuffer;
                        mScriptName += ".lua";
                    }

                    ImGui::Spacing();
                    ImGui::TextDisabled("Full Path: %s", mScriptName.c_str());
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    // Check if script file exists
                    bool fileExists = FileBrowser::fileExists(mScriptName);

                    // Check if script already added to component
                    bool scriptExists = false;
                    for (const auto& script : luaScript.scripts)
                    {
                        if (script.scriptPath == mScriptName)
                        {
                            scriptExists = true;
                            break;
                        }
                    }

                    // Show status message
                    if (strlen(scriptNameBuffer) == 0)
                    {
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Enter a script name...");
                    }
                    else if (scriptExists)
                    {
                        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Script already added to this component!");
                    }
                    else if (!fileExists)
                    {
                        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Script file not found!");
                    }
                    else
                    {
                        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Script found and ready to add!");
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    // Add button (only enabled if valid)
                    bool isValid = fileExists && !scriptExists && strlen(scriptNameBuffer) > 0;

                    if (!isValid)
                    {
                        ImGui::BeginDisabled();
                    }

                    if (ImGui::Button("Add Script", ImVec2(120, 0)))
                    {
                        luaScript.AddScript(mScriptName);
                        pEventSystem->Emit<CallLuaToInitScript>(entity);

                        // Clear buffer and close
                        scriptNameBuffer[0] = '\0';
                        mScriptName = "Assets/Scripts/.lua";
                        ImGui::CloseCurrentPopup();

                        m_hasUnsavedEdit = true;
                    }

                    if (!isValid)
                    {
                        ImGui::EndDisabled();
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("Cancel", ImVec2(120, 0)))
                    {
                        // Clear buffer and close
                        scriptNameBuffer[0] = '\0';
                        mScriptName = "Assets/Scripts/.lua";
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }

                // end tracking
                EndComponentEdit(entity, coordinator, "Lua Script");

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::AudioComponent>())
        {
            if (ImGui::CollapsingHeader("AudioComponent"))
            {
                auto& audio = coordinator.GetComponent<Uma_ECS::AudioComponent>(entity);
                ImGui::Indent();
            
                ImGui::Text("Position: (%.2f, %.2f, %.2f)",
                    audio.position.x, audio.position.y, audio.position.z);
            
                ImGui::Text("Velocity: (%.2f, %.2f, %.2f)",
                    audio.velocity.x, audio.velocity.y, audio.velocity.z);
            
                ImGui::Separator();
                ImGui::Text("Default Settings");
            
                ImGui::DragFloat("Default Volume", &audio.defaultVolume, 0.01f, 0.0f, 1.0f);
                ImGui::Checkbox("Default 3D", &audio.default3D);
            
                ImGui::Separator();
                ImGui::Text("Active Sounds: %zu", audio.activeSounds.size());
            
                if (!audio.activeSounds.empty())
                {
                    for (auto& [soundName, soundInstance] : audio.activeSounds)
                    {
                        ImGui::PushID(soundName.c_str());
            
                        if (ImGui::TreeNode(soundName.c_str()))
                        {
                            ImGui::Text("Sound: %s", soundName.c_str());
                            ImGui::Checkbox("Is Playing", &soundInstance.isPlaying);
                            ImGui::Checkbox("Should Loop", &soundInstance.shouldLoop);
                            ImGui::Checkbox("Is 3D", &soundInstance.is3D);
            
                            ImGui::Separator();
                            ImGui::Text("Sound Properties");
            
                            ImGui::DragFloat("Volume", &soundInstance.volume, 0.01f, 0.0f, 1.0f);
                            ImGui::DragFloat("Pitch", &soundInstance.pitch, 0.01f, 0.1f, 3.0f);
            
                            if (soundInstance.is3D)
                            {
                                ImGui::DragFloat("Min Distance", &soundInstance.minDistance, 1.0f, 0.0f, soundInstance.maxDistance);
                                ImGui::DragFloat("Max Distance", &soundInstance.maxDistance, 10.0f, soundInstance.minDistance, 10000.0f);
                            }
            
                            ImGui::TreePop();
                        }
            
                        ImGui::PopID();
                    }
                }
                else
                {
                    ImGui::TextDisabled("No active sounds");
                }
        
                ImGui::Separator();
                if (!audio.loopingSoundName.empty())
                {
                    ImGui::Text("Looping Sound: %s", audio.loopingSoundName.c_str());
                }
                else
                {
                    ImGui::TextDisabled("No looping sound");
                }
        
                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::AudioListener>())
        {
            if (ImGui::CollapsingHeader("AudioListener"))
            {
                ImGui::Indent();
                ImGui::Text("Audio Listener Component");
                ImGui::Separator();
                ImGui::TextDisabled("This entity is marked as the audio listener");
                ImGui::TextDisabled("(Used for 3D audio positioning)");
                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::PathFinding>())
        {
            if (ImGui::CollapsingHeader("PathFinding"))
            {
                auto& pathfinding = coordinator.GetComponent<Uma_ECS::PathFinding>(entity);
                ImGui::Indent();
            
                ImGui::Text("Path Update Settings");
                ImGui::DragFloat("Update Interval", &pathfinding.pathUpdateInterval, 0.01f, 0.01f, 5.0f);
            
                ImGui::Separator();
                ImGui::Text("Goal Position");
            
                float goalPos[2] = { pathfinding.goal.x, pathfinding.goal.y };
                if (ImGui::DragFloat2("Goal", goalPos, 0.1f))
                {
                    pathfinding.goal = Vec2(goalPos[0], goalPos[1]);
                }
            
                ImGui::Separator();
                ImGui::Text("Path Info");
            
                ImGui::Text("Path Length: %zu", pathfinding.path.size());
                ImGui::Text("Current Index: %u", pathfinding.pathIndex);
            
                if (!pathfinding.path.empty())
                {
                    if (ImGui::TreeNode("Path Points"))
                    {
                        for (size_t i = 0; i < pathfinding.path.size(); ++i)
                        {
                            const auto& point = pathfinding.path[i];
                            ImGui::Text("%zu: (%.2f, %.2f)", i, point.x, point.y);
                        }
                        ImGui::TreePop();
                    }
                }
                else
                {
                    ImGui::TextDisabled("No path points");
                }
            
                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::ParticleEmitter>())
        {
            if (ImGui::CollapsingHeader("ParticleEmitter", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto& emitterArray = coordinator.GetComponentArray<Uma_ECS::ParticleEmitter>();
                auto& component = emitterArray.GetData(entity);

                ImGui::Indent();

                // Add Emitter button
                if (ImGui::Button("Add Emitter"))
                {
                    component.AddEmitter("New Emitter");
                }

                ImGui::Separator();

                // Loop through all emitters
                for (int i = 0; i < component.GetEmitterCount(); ++i)
                {
                    ImGui::PushID(i);
                    auto* emitter = component.GetEmitter(i);
                    if (!emitter)
                    {
                        ImGui::PopID();
                        continue;
                    }

                    // Emitter header with name
                    std::string headerLabel = emitter->name + "###EmitterHeader";
                    bool emitterOpen = ImGui::CollapsingHeader(headerLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

                    // Right-click context menu for removing emitter
                    if (ImGui::BeginPopupContextItem())
                    {
                        if (ImGui::MenuItem("Remove Emitter"))
                        {
                            component.RemoveEmitter(i);
                            ImGui::EndPopup();
                            ImGui::PopID();
                            break; // Exit loop after removal
                        }
                        ImGui::EndPopup();
                    }

                    if (emitterOpen)
                    {
                        ImGui::Indent();

                        // Emitter Name
                        char nameBuffer[128];
                        strncpy(nameBuffer, emitter->name.c_str(), sizeof(nameBuffer) - 1);
                        nameBuffer[sizeof(nameBuffer) - 1] = '\0';
                        if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
                        {
                            emitter->name = nameBuffer;
                        }

                        // Control Buttons
                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));

                        if (ImGui::Button("Play", ImVec2(80, 0)))
                        {
                            emitter->Play();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Stop", ImVec2(80, 0)))
                        {
                            emitter->Stop();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Clear", ImVec2(80, 0)))
                        {
                            emitter->StopAndClear();
                        }

                        ImGui::PopStyleVar();

                        // Active Checkbox
                        ImGui::Checkbox("Active", &emitter->isActive);

                        // Emission Mode
                        const char* modes[] = { "Burst", "Continuous", "ScreenFill" };
                        int currentMode = static_cast<int>(emitter->mode);
                        if (ImGui::Combo("Emission Mode", &currentMode, modes, IM_ARRAYSIZE(modes)))
                        {
                            emitter->mode = static_cast<Uma_ECS::EmitterMode>(currentMode);
                        }

                        // Max Particles
                        ImGui::DragInt("Max Particles", &emitter->maxParticles, 1.0f, 1, 10000);

                        // Texture Name
                        char texBuffer[128];
                        strncpy(texBuffer, emitter->textureName.c_str(), sizeof(texBuffer) - 1);
                        texBuffer[sizeof(texBuffer) - 1] = '\0';
                        if (ImGui::InputText("Texture Name", texBuffer, sizeof(texBuffer)))
                        {
                            emitter->textureName = texBuffer;
                        }

                        ImGui::Separator();

                        // Appearance settings
                        if (ImGui::CollapsingHeader("Appearance", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ImGui::Indent();

                            // Scale Range
                            ImGui::DragFloat2("Scale Range", &emitter->appearance.scaleRange.x, 0.01f, 0.01f, 10.0f, "%.2f");

                            // Start Color
                            ImGui::ColorEdit3("Start Color", &emitter->appearance.startColor.x);

                            // End Color
                            ImGui::ColorEdit3("End Color", &emitter->appearance.endColor.x);

                            // Color Lerp
                            ImGui::Checkbox("Color Lerp", &emitter->appearance.colorLerp);

                            // Random Opacity
                            ImGui::Checkbox("Random Opacity", &emitter->appearance.randomOpacity);
                            if (emitter->appearance.randomOpacity)
                            {
                                ImGui::Indent();
                                ImGui::DragFloat2("Opacity Range", &emitter->appearance.opacityRange.x, 0.01f, 0.0f, 1.0f, "%.2f");
                                ImGui::Unindent();
                            }

                            // Rotate Particles
                            ImGui::Checkbox("Rotate Particles", &emitter->appearance.rotateParticles);
                            if (emitter->appearance.rotateParticles)
                            {
                                ImGui::Indent();
                                ImGui::DragFloat2("Rotation Speed Range", &emitter->appearance.rotationSpeedRange.x, 1.0f, -360.0f, 360.0f, "%.1f deg/s");
                                ImGui::Unindent();
                            }

                            ImGui::Unindent();
                        }

                        // Fade settings
                        if (ImGui::CollapsingHeader("Fade", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ImGui::Indent();

                            // Fade In
                            ImGui::Checkbox("Fade In", &emitter->fade.fadeIn);
                            if (emitter->fade.fadeIn)
                            {
                                ImGui::Indent();
                                ImGui::DragFloat("Fade In Duration", &emitter->fade.fadeInDuration, 0.01f, 0.01f, 5.0f, "%.2f sec");
                                ImGui::Unindent();
                            }

                            // Fade Out (only for Burst/Continuous)
                            if (emitter->mode != Uma_ECS::EmitterMode::ScreenFill)
                            {
                                ImGui::Checkbox("Fade Out", &emitter->fade.fadeOut);
                                if (emitter->fade.fadeOut)
                                {
                                    ImGui::Indent();
                                    ImGui::DragFloat("Fade Out Duration", &emitter->fade.fadeOutDuration, 0.01f, 0.01f, 5.0f, "%.2f sec");
                                    ImGui::Unindent();
                                }
                            }

                            // Fade At Edges (only for ScreenFill)
                            if (emitter->mode == Uma_ECS::EmitterMode::ScreenFill)
                            {
                                ImGui::Checkbox("Fade At Edges", &emitter->fade.fadeAtEdges);
                                if (emitter->fade.fadeAtEdges)
                                {
                                    ImGui::Indent();
                                    ImGui::DragFloat("Edge Fade Distance", &emitter->fade.edgeFadeDistance, 1.0f, 0.0f, 500.0f, "%.1f");
                                    ImGui::Unindent();
                                }
                            }

                            ImGui::Unindent();
                        }

                        // Physics settings
                        if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ImGui::Indent();

                            // Speed Range
                            ImGui::DragFloat2("Speed Range", &emitter->physics.speedRange.x, 0.1f, 0.0f, 1000.0f, "%.1f");

                            // Lifetime Range (not shown for ScreenFill)
                            if (emitter->mode != Uma_ECS::EmitterMode::ScreenFill)
                            {
                                ImGui::DragFloat2("Lifetime Range", &emitter->physics.lifetimeRange.x, 0.01f, 0.01f, 100.0f, "%.2f sec");
                            }

                            // Gravity
                            ImGui::DragFloat2("Gravity", &emitter->physics.gravity.x, 0.1f, -500.0f, 500.0f, "%.1f");

                            // Drag
                            ImGui::DragFloat("Drag", &emitter->physics.drag, 0.01f, 0.0f, 10.0f, "%.2f");

                            ImGui::Unindent();
                        }

                        // Spawn settings
                        if (ImGui::CollapsingHeader("Spawn", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ImGui::Indent();

                            // Spawn Offset (not shown for ScreenFill)
                            if (emitter->mode != Uma_ECS::EmitterMode::ScreenFill)
                            {
                                ImGui::DragFloat2("Spawn Offset", &emitter->spawn.spawnOffset.x, 1.0f, -1000.0f, 1000.0f, "%.1f");

                                // Spawn Radius
                                ImGui::DragFloat("Spawn Radius", &emitter->spawn.spawnRadius, 1.0f, 0.0f, 500.0f, "%.1f");

                                // Emission Cone
                                ImGui::Checkbox("Use Emission Cone", &emitter->spawn.useEmissionCone);
                                if (emitter->spawn.useEmissionCone)
                                {
                                    ImGui::Indent();
                                    ImGui::DragFloat("Emission Angle", &emitter->spawn.emissionAngle, 1.0f, 0.0f, 360.0f, "%.1f deg");
                                    ImGui::DragFloat("Emission Spread", &emitter->spawn.emissionSpread, 1.0f, 0.0f, 360.0f, "%.1f deg");
                                    ImGui::Unindent();
                                }
                            }

                            ImGui::Unindent();
                        }

                        // Emission settings
                        if (ImGui::CollapsingHeader("Emission", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ImGui::Indent();

                            // Continuous Mode Emission Rate
                            if (emitter->mode == Uma_ECS::EmitterMode::Continuous)
                            {
                                ImGui::DragFloat("Emission Rate", &emitter->emission.emissionRate, 1.0f, 1.0f, 1000.0f, "%.1f particles/sec");
                            }

                            // Burst Mode Loop Settings
                            if (emitter->mode == Uma_ECS::EmitterMode::Burst)
                            {
                                ImGui::Checkbox("Loop", &emitter->emission.loop);
                                if (emitter->emission.loop)
                                {
                                    ImGui::Indent();
                                    ImGui::DragFloat("Loop Delay", &emitter->emission.loopDelay, 0.1f, 0.0f, 60.0f, "%.1f sec");
                                    ImGui::Unindent();
                                }
                            }

                            ImGui::Unindent();
                        }

                        // Screen fill settings
                        if (emitter->mode == Uma_ECS::EmitterMode::ScreenFill)
                        {
                            if (ImGui::CollapsingHeader("ScreenFill", ImGuiTreeNodeFlags_DefaultOpen))
                            {
                                ImGui::Indent();

                                // Velocity Ranges
                                ImGui::DragFloat2("Velocity X Range", &emitter->screenFill.velocityXRange.x, 1.0f, -500.0f, 500.0f, "%.1f");
                                ImGui::DragFloat2("Velocity Y Range", &emitter->screenFill.velocityYRange.x, 1.0f, -500.0f, 500.0f, "%.1f");

                                // Spawn At Top
                                ImGui::Checkbox("Spawn At Top", &emitter->screenFill.spawnAtTop);

                                // Spawn Margin
                                ImGui::DragFloat("Spawn Margin", &emitter->screenFill.spawnMargin, 1.0f, 0.0f, 1000.0f, "%.1f");

                                ImGui::Unindent();
                            }
                        }

                        ImGui::Separator();

                        // Debug Info
                        if (ImGui::TreeNode("Debug Info"))
                        {
                            ImGui::Text("Active Particles: %d / %d", emitter->GetActiveParticleCount(), emitter->maxParticles);
                            ImGui::Text("Is Playing: %s", emitter->IsPlaying() ? "Yes" : "No");
                            ImGui::Text("Has Active Particles: %s", emitter->HasActiveParticles() ? "Yes" : "No");
                            ImGui::TreePop();
                        }

                        ImGui::Unindent();
                    }

                    ImGui::PopID();
                }
            }
        }
        else if (type == coordinator.GetComponentType<Uma_UI::RectTransform>())
        {
            if (ImGui::CollapsingHeader("RectTransform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto& rectTransform = coordinator.GetComponent<Uma_UI::RectTransform>(entity);
                ImGui::Indent();
            
                // Begin tracking
                BeginComponentEdit(entity, coordinator);
            
                ImGui::Text("Anchors");
                ImGui::Separator();
            
                float anchorMin[2] = { rectTransform.anchorMin.x, rectTransform.anchorMin.y };
                if (ImGui::DragFloat2("Anchor Min", anchorMin, 0.01f, 0.0f, 1.0f))
                {
                    rectTransform.anchorMin = Vec2(anchorMin[0], anchorMin[1]);
                    rectTransform.isDirty = true;
                    m_hasUnsavedEdit = true;
                }
            
                float anchorMax[2] = { rectTransform.anchorMax.x, rectTransform.anchorMax.y };
                if (ImGui::DragFloat2("Anchor Max", anchorMax, 0.01f, 0.0f, 1.0f))
                {
                    rectTransform.anchorMax = Vec2(anchorMax[0], anchorMax[1]);
                    rectTransform.isDirty = true;
                    m_hasUnsavedEdit = true;
                }
            
                ImGui::Spacing();
                ImGui::Text("Position & Size");
                ImGui::Separator();
            
                float pivot[2] = { rectTransform.pivot.x, rectTransform.pivot.y };
                if (ImGui::DragFloat2("Pivot", pivot, 0.01f, 0.0f, 1.0f))
                {
                    rectTransform.pivot = Vec2(pivot[0], pivot[1]);
                    rectTransform.isDirty = true;
                    m_hasUnsavedEdit = true;
                }
            
                float anchoredPos[2] = { rectTransform.anchoredPosition.x, rectTransform.anchoredPosition.y };
                if (ImGui::DragFloat2("Anchored Position", anchoredPos, 1.0f))
                {
                    rectTransform.anchoredPosition = Vec2(anchoredPos[0], anchoredPos[1]);
                    rectTransform.isDirty = true;
                    m_hasUnsavedEdit = true;
                }
            
                float sizeDelta[2] = { rectTransform.sizeDelta.x, rectTransform.sizeDelta.y };
                if (ImGui::DragFloat2("Size Delta", sizeDelta, 1.0f))
                {
                    rectTransform.sizeDelta = Vec2(sizeDelta[0], sizeDelta[1]);
                    rectTransform.isDirty = true;
                    m_hasUnsavedEdit = true;
                }
            
                ImGui::Spacing();
                ImGui::Text("Computed Rect (Read-Only)");
                ImGui::Separator();
                ImGui::Text("X: %.2f, Y: %.2f", rectTransform.computedRect.x, rectTransform.computedRect.y);
                ImGui::Text("Width: %.2f, Height: %.2f", rectTransform.computedRect.width, rectTransform.computedRect.height);
            
                // End tracking
                EndComponentEdit(entity, coordinator, "RectTransform");
            
                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_UI::Image>())
        {
           if (ImGui::CollapsingHeader("Image", ImGuiTreeNodeFlags_DefaultOpen))
           {
               auto& image = coordinator.GetComponent<Uma_UI::Image>(entity);
               ImGui::Indent();
           
               // Begin tracking
               BeginComponentEdit(entity, coordinator);
           
               ImGui::Text("Texture: %s", image.textureName.c_str());
               static char imageTextureBuffer[256];
               strncpy(imageTextureBuffer, image.textureName.c_str(), 255);
               imageTextureBuffer[255] = '\0';
               if (ImGui::InputText("Texture Name", imageTextureBuffer, 256))
               {
                   image.textureName = imageTextureBuffer;
                   m_hasUnsavedEdit = true;
               }
           
               ImGui::Separator();
               ImGui::Text("Color & Visibility");
           
               float imageColor[4] = { image.colour.r, image.colour.g, image.colour.b, image.colour.a };
               if (ImGui::ColorEdit4("Color", imageColor))
               {
                   image.colour.r = imageColor[0];
                   image.colour.g = imageColor[1];
                   image.colour.b = imageColor[2];
                   image.colour.a = imageColor[3];
                   m_hasUnsavedEdit = true;
               }
           
               if (ImGui::Checkbox("Visible", &image.visible))
               {
                   m_hasUnsavedEdit = true;
               }
           
               // End tracking
               EndComponentEdit(entity, coordinator, "Image");
           
               ImGui::Unindent();
           }
        }
        else if (type == coordinator.GetComponentType<Uma_UI::Button>())
        {
           if (ImGui::CollapsingHeader("Button", ImGuiTreeNodeFlags_DefaultOpen))
           {
               auto& button = coordinator.GetComponent<Uma_UI::Button>(entity);
               ImGui::Indent();
           
               // Begin tracking
               BeginComponentEdit(entity, coordinator);
           
               if (ImGui::Checkbox("Interactable", &button.interactable))
               {
                   m_hasUnsavedEdit = true;
               }
           
               ImGui::Spacing();
               ImGui::Text("Current State: %s",
                   button.currentState == Uma_UI::ButtonState::Normal ? "Normal" :
                   button.currentState == Uma_UI::ButtonState::Hovered ? "Hovered" :
                   button.currentState == Uma_UI::ButtonState::Pressed ? "Pressed" : "Disabled");
           
               ImGui::Separator();
               ImGui::Text("Button Colors");
           
               float normalColor[4] = { button.normalColour.r, button.normalColour.g, button.normalColour.b, button.normalColour.a };
               if (ImGui::ColorEdit4("Normal Color", normalColor))
               {
                   button.normalColour.r = normalColor[0];
                   button.normalColour.g = normalColor[1];
                   button.normalColour.b = normalColor[2];
                   button.normalColour.a = normalColor[3];
                   m_hasUnsavedEdit = true;
               }
           
               float hoverColor[4] = { button.hoverColour.r, button.hoverColour.g, button.hoverColour.b, button.hoverColour.a };
               if (ImGui::ColorEdit4("Hover Color", hoverColor))
               {
                   button.hoverColour.r = hoverColor[0];
                   button.hoverColour.g = hoverColor[1];
                   button.hoverColour.b = hoverColor[2];
                   button.hoverColour.a = hoverColor[3];
                   m_hasUnsavedEdit = true;
               }
           
               float pressedColor[4] = { button.pressedColour.r, button.pressedColour.g, button.pressedColour.b, button.pressedColour.a };
               if (ImGui::ColorEdit4("Pressed Color", pressedColor))
               {
                   button.pressedColour.r = pressedColor[0];
                   button.pressedColour.g = pressedColor[1];
                   button.pressedColour.b = pressedColor[2];
                   button.pressedColour.a = pressedColor[3];
                   m_hasUnsavedEdit = true;
               }
           
               float disabledColor[4] = { button.disabledColour.r, button.disabledColour.g, button.disabledColour.b, button.disabledColour.a };
               if (ImGui::ColorEdit4("Disabled Color", disabledColor))
               {
                   button.disabledColour.r = disabledColor[0];
                   button.disabledColour.g = disabledColor[1];
                   button.disabledColour.b = disabledColor[2];
                   button.disabledColour.a = disabledColor[3];
                   m_hasUnsavedEdit = true;
               }
           
               // End tracking
               EndComponentEdit(entity, coordinator, "Button");
           
               ImGui::Unindent();
           }
        }
        else if (type == coordinator.GetComponentType<Uma_UI::Canvas>())
        {
            if (ImGui::CollapsingHeader("Canvas", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto& canvas = coordinator.GetComponent<Uma_UI::Canvas>(entity);
                ImGui::Indent();
            
                // Begin tracking
                BeginComponentEdit(entity, coordinator);
            
                if (ImGui::DragInt("Sorting Order", &canvas.sortingOrder, 1.0f, -100, 100))
                {
                    m_hasUnsavedEdit = true;
                }
            
                ImGui::Separator();
                ImGui::Text("Reference Resolution");
            
                float refResolution[2] = { canvas.referenceResolution.x, canvas.referenceResolution.y };
                if (ImGui::DragFloat2("Resolution", refResolution, 1.0f, 1.0f, 10000.0f, "%.0f"))
                {
                    canvas.referenceResolution = Vec2(refResolution[0], refResolution[1]);
                    m_hasUnsavedEdit = true;
                }
            
                ImGui::Separator();
                ImGui::Text("Scale Mode");
            
                const char* scaleModes[] = { "Constant Pixel Size", "Scale With Screen Size", "Constant Physical Size" };
                int currentScaleMode = static_cast<int>(canvas.scaleMode);
                if (ImGui::Combo("Scale Mode", &currentScaleMode, scaleModes, IM_ARRAYSIZE(scaleModes)))
                {
                    canvas.scaleMode = static_cast<Uma_UI::CanvasScaleMode>(currentScaleMode);
                    m_hasUnsavedEdit = true;
                }
            
                if (canvas.scaleMode == Uma_UI::CanvasScaleMode::ScaleWithScreenSize)
                {
                    if (ImGui::SliderFloat("Match Width/Height", &canvas.matchWidthOrHeight, 0.0f, 1.0f, "%.2f"))
                    {
                        m_hasUnsavedEdit = true;
                    }
                    ImGui::TextDisabled("0 = Match Width, 1 = Match Height");
                }
            
                ImGui::Separator();
                ImGui::Text("Runtime Info");
                ImGui::Text("Scale Factor: %.3f", canvas.scaleFactor);
            
                // End tracking
                EndComponentEdit(entity, coordinator, "Canvas");
            
                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_UI::Text>())
        {
            if (ImGui::CollapsingHeader("Text", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto& text = coordinator.GetComponent<Uma_UI::Text>(entity);
                ImGui::Indent();
            
                // Begin tracking
                BeginComponentEdit(entity, coordinator);
            
                static char textContentBuffer[1024];
                strncpy(textContentBuffer, text.text.c_str(), 1023);
                textContentBuffer[1023] = '\0';
                if (ImGui::InputTextMultiline("Text Content", textContentBuffer, 1024, ImVec2(-1, 80)))
                {
                    text.text = textContentBuffer;
                    m_hasUnsavedEdit = true;
                }
            
                ImGui::Separator();
                ImGui::Text("Font Settings");
            
                static char fontNameBuffer[256];
                strncpy(fontNameBuffer, text.fontName.c_str(), 255);
                fontNameBuffer[255] = '\0';
                if (ImGui::InputText("Font Name", fontNameBuffer, 256))
                {
                    text.fontName = fontNameBuffer;
                    m_hasUnsavedEdit = true;
                }
            
                if (ImGui::DragFloat("Font Size", &text.fontSize, 1.0f, 1.0f, 200.0f, "%.1f"))
                {
                    m_hasUnsavedEdit = true;
                }
            
                ImGui::Separator();
                ImGui::Text("Appearance");
            
                float textColor[4] = { text.colour.r, text.colour.g, text.colour.b, text.colour.a };
                if (ImGui::ColorEdit4("Color", textColor))
                {
                    text.colour.r = textColor[0];
                    text.colour.g = textColor[1];
                    text.colour.b = textColor[2];
                    text.colour.a = textColor[3];
                    m_hasUnsavedEdit = true;
                }
            
                const char* alignments[] = { "Left", "Center", "Right" };
                int currentAlignment = static_cast<int>(text.alignment);
                if (ImGui::Combo("Alignment", &currentAlignment, alignments, IM_ARRAYSIZE(alignments)))
                {
                    text.alignment = static_cast<Uma_UI::TextAlignment>(currentAlignment);
                    m_hasUnsavedEdit = true;
                }
            
                if (ImGui::Checkbox("Visible", &text.visible))
                {
                    m_hasUnsavedEdit = true;
                }
            
                // End tracking
                EndComponentEdit(entity, coordinator, "Text");
            
                ImGui::Unindent();
            }
        }
        else
        {
            return false;
        }
        return true;
    }

    void ImguiManager::BeginComponentEdit(Uma_ECS::Entity entity, Uma_ECS::Coordinator& coordinator)
    {
        if (m_editingEntity != entity)
        {
            m_snapshotBeforeEdit = CaptureEntitySnapshot(entity, coordinator);
            m_editingEntity = entity;
            m_hasUnsavedEdit = false;
        }

        // Push an ID for this component section
        //ImGui::PushID("ComponentEdit");
    }

    void ImguiManager::EndComponentEdit(Uma_ECS::Entity entity, Uma_ECS::Coordinator& coordinator, const std::string& componentName, bool forceEdit)
    {
        //ImGui::PopID();

        // Check if user released mouse after editing
        if (m_hasUnsavedEdit && ImGui::IsMouseReleased(ImGuiMouseButton_Left) ||
            m_hasUnsavedEdit && forceEdit)
        {
            Uma_Editor::EntitySnapshot snapshotAfter = CaptureEntitySnapshot(entity, coordinator);

            auto cmd = std::make_unique<Uma_Editor::EntitySnapshotCmd>(
                &coordinator,
                std::move(m_snapshotBeforeEdit),
                std::move(snapshotAfter),
                componentName + " Edit"
            );

            commandHistory.ExecuteCommand(std::move(cmd));

            // Re-capture for next edit
            m_snapshotBeforeEdit = CaptureEntitySnapshot(entity, coordinator);
            m_hasUnsavedEdit = false;
        }
    }

    bool& ImguiManager::HasUnsavedChanges()
    {
        return m_hasUnsavedEdit;
    }

    Uma_Editor::EntitySnapshot ImguiManager::CaptureEntitySnapshot(Uma_ECS::Entity entity, Uma_ECS::Coordinator& coord)
    {
        Uma_ECS::Coordinator& coordinator = pSystemManager->GetSystem<SceneManager>()->GetActiveScene()->GetCoordinator();

        Uma_Editor::EntitySnapshot snapshot;
        snapshot.entityID = entity;

        // Serialize all components to JSON using Coordinator's wrapper
        snapshot.componentData.SetObject();
        auto& allocator = snapshot.componentData.GetAllocator();

        rapidjson::Value componentsObj(rapidjson::kObjectType);
        coordinator.SerializeEntity(entity, componentsObj, allocator);

        snapshot.componentData.AddMember("components", componentsObj, allocator);

        // Capture hierarchy info (optional, for quick access)
        auto& tfArray = coordinator.GetComponentArray<Uma_ECS::Transform>();
        if (tfArray.Has(entity))
        {
            auto& tf = tfArray.GetData(entity);
            snapshot.parentID = tf.parent;
            snapshot.childrenIDs = tf.children;
        }

        return snapshot;
    }

    void ImguiManager::HandleUndoRedoInput()
    {
        bool didUndoOrRedo = false;

        // Ctrl+Z = Undo
        if (InputSystem::KeyDown(GLFW_KEY_LEFT_CONTROL) && InputSystem::KeyPressed(GLFW_KEY_Z)) 
        {
            commandHistory.Undo();
            didUndoOrRedo = true;
        }

        // Ctrl+Y = Redo
        if (InputSystem::KeyDown(GLFW_KEY_LEFT_CONTROL) && InputSystem::KeyPressed(GLFW_KEY_Y)) 
        {
            commandHistory.Redo();
            didUndoOrRedo = true;
        }

        // Re-capture snapshot after undo/redo if we're editing an entity
        if (didUndoOrRedo && m_selectedEntity != static_cast<Uma_ECS::Entity>(-1))
        {
            auto sceneManager = pSystemManager->GetSystem<SceneManager>();
            if (sceneManager && sceneManager->GetActiveScene())
            {
                auto& coordinator = sceneManager->GetActiveScene()->GetCoordinator();

                if (coordinator.HasActiveEntity(m_selectedEntity))
                {
                    // Re-capture current state (after undo/redo)
                    m_snapshotBeforeEdit = CaptureEntitySnapshot(m_selectedEntity, coordinator);
                    m_editingEntity = m_selectedEntity;
                }
            }

            m_hasUnsavedEdit = false;
        }
    }

    void ImguiManager::CreateInspectorWindow()
    {
        bool b = true;
        ImGui::Begin("Inspector", &b);

        // Check if an entity is selected
        if (m_selectedEntity == static_cast<Uma_ECS::Entity>(-1))
        {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No entity selected");
            ImGui::End();
            return;
        }

        // Get coordinator from scene manager
        auto sceneManager = pSystemManager->GetSystem<SceneManager>();
        if (!sceneManager || !sceneManager->GetActiveScene())
        {
            ImGui::End();
            return;
        }

        auto& coordinator = sceneManager->GetActiveScene()->GetCoordinator();

        if (!coordinator.HasActiveEntity(m_selectedEntity))
        {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Selected entity is no longer valid");
            m_selectedEntity = static_cast<Uma_ECS::Entity>(-1);
            ImGui::End();
            return;
        }

        // Display entity ID
        ImGui::Text("Entity ID: %d", m_selectedEntity);
        ImGui::Separator();

        // Entity Name Field
        //std::string entityName = GetEntityDisplayName(m_selectedEntity, coordinator);
        //ImGui::Text("Name: %s", entityName.c_str());

        auto& tf = coordinator.GetComponent<Uma_ECS::Transform>(m_selectedEntity);

        static char textureBuffer[256];
        strncpy(textureBuffer, tf.name.c_str(), 255);
        textureBuffer[255] = '\0';
        if (ImGui::InputText("##name", textureBuffer, 256))
        {
            tf.name = textureBuffer;
        }

        ImGui::Separator();
        ImGui::Spacing();

        // ========== Transform Component ==========
        /*auto& transformArray = coordinator.GetComponentArray<Uma_ECS::Transform>();
        if (transformArray.Has(m_selectedEntity))
        {
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto& transform = transformArray.GetData(m_selectedEntity);

                ImGui::Indent();

                float position[2] = { transform.position.x, transform.position.y };
                if (ImGui::DragFloat2("Position", position, 0.1f))
                {
                    transform.position = Vec2(position[0], position[1]);
                }

                float rotation = transform.rotation.x;
                if (ImGui::DragFloat("Rotation", &rotation, 1.0f))
                {
                    transform.rotation.x = rotation;
                }

                float scale[2] = { transform.scale.x, transform.scale.y };
                if (ImGui::DragFloat2("Scale", scale, 0.1f))
                {
                    transform.scale = Vec2(scale[0], scale[1]);
                }

                ImGui::Unindent();
            }
        }*/

        coordinator.ForEachComponent(m_selectedEntity, [this, &coordinator](Uma_ECS::ComponentType type)
            {
                if (DisplayComponent(coordinator, type, m_selectedEntity))
                {
                    ImGui::Spacing();
                    ImGui::Separator();
                }
            });


        // ========== Add Component Button ==========

        if (ImGui::Button("Add Component", ImVec2(-1, 0)))
        {
            ImGui::OpenPopup("AddComponentPopup");
        }

        // Add Component Popup
        if (ImGui::BeginPopup("AddComponentPopup"))
        {
            ImGui::Text("Select Component Type:");
            ImGui::Separator();

            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::Sprite>()) && ImGui::MenuItem("Sprite"))
            {
                coordinator.AddComponent(m_selectedEntity, Uma_ECS::Sprite{});
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::RigidBody>()) && ImGui::MenuItem("RigidBody"))
            {
                coordinator.AddComponent(m_selectedEntity, Uma_ECS::RigidBody{});
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::Collider>()) && ImGui::MenuItem("Collider"))
            {
                coordinator.AddComponent(m_selectedEntity, Uma_ECS::Collider{});
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::Camera>()) && ImGui::MenuItem("Camera"))
            {
                coordinator.AddComponent(m_selectedEntity, Uma_ECS::Camera{});
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::LuaScript>()) && ImGui::MenuItem("LuaScript"))
            {
                coordinator.AddComponent(m_selectedEntity, Uma_ECS::LuaScript{});
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::Enemy>()) && ImGui::MenuItem("Enemy"))
            {
                coordinator.AddComponent(m_selectedEntity, Uma_ECS::Enemy{});
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::Animator>()) && ImGui::MenuItem("Animator"))
            {
                coordinator.AddComponent(m_selectedEntity, Uma_ECS::Animator{});
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::AudioComponent>()) && ImGui::MenuItem("AudioComponent"))
            {
                coordinator.AddComponent(m_selectedEntity, Uma_ECS::AudioComponent{});
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::AudioListener>()) && ImGui::MenuItem("AudioListener"))
            {
                coordinator.AddComponent(m_selectedEntity, Uma_ECS::AudioListener{});
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::PathFinding>()) && ImGui::MenuItem("PathFinding"))
            {
                coordinator.AddComponent(m_selectedEntity, Uma_ECS::PathFinding{});
                pEventSystem->Emit<CallPathFindToBake>();
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::ParticleEmitter>()) && ImGui::MenuItem("ParticleEmitter"))
            {
                coordinator.AddComponent(m_selectedEntity, Uma_ECS::ParticleEmitter{});
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_UI::RectTransform>()) && ImGui::MenuItem("RectTransform"))
            {
                coordinator.AddComponent(m_selectedEntity, Uma_UI::RectTransform{});
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_UI::Image>()) && ImGui::MenuItem("Image"))
            {
                coordinator.AddComponent(m_selectedEntity, Uma_UI::Image{});
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_UI::Button>()) && ImGui::MenuItem("Button"))
            {
                coordinator.AddComponent(m_selectedEntity, Uma_UI::Button{});
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_UI::Canvas>()) && ImGui::MenuItem("Canvas"))
            {
                coordinator.AddComponent(m_selectedEntity, Uma_UI::Canvas{});
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_UI::Text>()) && ImGui::MenuItem("Text"))
            {
                coordinator.AddComponent(m_selectedEntity, Uma_UI::Text{});
            }

            ImGui::EndPopup();
        }
        ImGui::End();
    }

    void ImguiManager::SceneManagerWindow()
    {
        ImGui::Begin("Scene Manager");

        // Button to create a new scene
        if (ImGui::Button("Create New Scene"))
        {
            pEventSystem->Emit<StopSceneRequest>();
            pEventSystem->Emit<CreateNewSceneRequest>();
        }

        // Button to remove current scene
        if (ImGui::Button("Remove Current Scene"))
        {
            pEventSystem->Emit<StopSceneRequest>();
            if (sceneNames.size() > 0)
            {
                pEventSystem->Emit<DeleteCurrSceneRequest>(sceneNames[activeSceneIndex]);
            }
        }

        if (ImGui::Button("Save Current Scene"))
        {
            if (sceneNames.size() > 0)
                pEventSystem->Emit<SaveSceneRequest>(sceneNames[activeSceneIndex]);
        }

        //list of loaded scenes
        ImGui::BeginChild("SceneList", ImVec2(0, 200), true);
        int selectedSceneIndex = -1;
        for (int i = 0; i < sceneNames.size(); i++)
        {
            bool isSelected = (selectedSceneIndex == i);
            bool isActive = (i == activeSceneIndex);

            std::string extra = (isActive ? " (Active)" : "");
            std::string sceneLabel = sceneNames[i] + extra;

            if (ImGui::Selectable(sceneLabel.c_str(), isSelected))
            {
                selectedSceneIndex = i;
            }

            // detect double click
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            {
                commandHistory.Clear();
                pEventSystem->Emit<StopSceneRequest>();
                pEventSystem->Emit<LoadSceneRequestEvent>(sceneNames[i]);
            }
        }
        ImGui::EndChild();

        ImGui::End();
    }

    void ImguiManager::CreateEditorCameraWindow()
    {
        if (!m_showEditorCameraWindow)
            return;

        auto sceneManager = pSystemManager->GetSystem<SceneManager>();
        if (!sceneManager)
            return;

        ImGui::Begin("Editor Camera", &m_showEditorCameraWindow);

        auto& editorCamera = sceneManager->GetEditorCamera();
        auto graphics = pSystemManager->GetSystem<Graphics>();

        bool isActive = false;

        if (graphics && graphics->GetRenderTarget() == Uma_Engine::RenderTarget::Framebuffer)
        {
            // In Scene View mode (active when mouse is in Scene View)
            isActive = editorCamera.IsActive() && m_isMouseInSceneView;
        }
        else
        {
            // In Window mode (active when mouse not over UI)
            isActive = editorCamera.IsActive() && !mouseOverUI;
        }

        // Display status
        if (isActive)
        {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Status: ACTIVE");
        }
        else
        {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Status: INACTIVE");
        }

        ImGui::Separator();

        // Position
        Vec2 pos = editorCamera.GetPosition();
        float position[2] = { pos.x, pos.y };
        if (ImGui::DragFloat2("Position", position, 0.1f))
        {
            editorCamera.SetPosition(Vec2(position[0], position[1]));
        }

        // Zoom
        float zoom = editorCamera.GetZoom();
        if (ImGui::DragFloat("Zoom", &zoom, 0.1f, 0.1f, 50.0f))
        {
            editorCamera.SetZoom(zoom);
        }

        ImGui::Separator();

        // Pan Speed
        static float panSpeed = 500.0f;
        if (ImGui::DragFloat("Pan Speed", &panSpeed, 10.0f, 10.0f, 2000.0f))
        {
            editorCamera.SetPanSpeed(panSpeed);
        }

        // Zoom Speed
        static float zoomSpeed = 1.0f;
        if (ImGui::DragFloat("Zoom Speed", &zoomSpeed, 0.1f, 0.1f, 10.0f))
        {
            editorCamera.SetZoomSpeed(zoomSpeed);
        }

        ImGui::Separator();

        // Zoom Limits
        static float minZoom = 0.1f;
        static float maxZoom = 20.0f;
        bool limitsChanged = false;

        limitsChanged |= ImGui::DragFloat("Min Zoom", &minZoom, 0.1f, 0.01f, maxZoom - 0.1f);
        limitsChanged |= ImGui::DragFloat("Max Zoom", &maxZoom, 0.1f, minZoom + 0.1f, 100.0f);

        if (limitsChanged)
        {
            editorCamera.SetZoomLimits(minZoom, maxZoom);
        }

        ImGui::Separator();

        // Reset button
        if (ImGui::Button("Reset Camera", ImVec2(-1, 0)))
        {
            editorCamera.Reset();
            panSpeed = 500.0f;
            zoomSpeed = 1.0f;
            minZoom = 0.1f;
            maxZoom = 20.0f;
        }

        ImGui::Spacing();

        // Controls info
        if (ImGui::CollapsingHeader("Controls"))
        {
            ImGui::BulletText("WASD - Pan");
            ImGui::BulletText("Q/E - Zoom");
            ImGui::BulletText("Middle Mouse - Drag to pan");
            ImGui::BulletText("Shift - Speed boost");
            ImGui::BulletText("R - Reset");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (graphics && graphics->GetRenderTarget() == Uma_Engine::RenderTarget::Framebuffer)
            {
                ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "Viewport Mode:");
                ImGui::Text("Camera only responds when");
                ImGui::Text("mouse is in Scene View window");
            }
            else
            {
                ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "Window Mode:");
                ImGui::Text("Camera responds when mouse");
                ImGui::Text("is not over UI windows");
            }
        }

        ImGui::End();
    }
}
