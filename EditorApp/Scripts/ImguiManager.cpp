/*!
\file   ImguiManager.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author     Shahir Rasid (Everything else)
\par        E-mail: b.muhammadshahir@digipen.edu
\par        DigiPen login: b.muhammadshahir

\co-author  Lai Jun Siang (Hierarchy/Inspector/Dockspace)
\par        E-mail: lai.j@digipen.edu
\par        DigiPen login: lai.j

\co-author  Javier Chua Dong Qing (EditorCamera, Drag and drop font,texture,
                                   UI Image, Particle Emitter and SoundComponent
                                    with duplicate/file check)
\par        E-mail: javierdongqing.chua@digipen.edu
\par        DigiPen login: javierdongqing.chua

\author     Leong Wai Men (Display Component modify, Bug fix for loading scene / play stop)
\par        E-mail: waimen.leong@digipen.edu
\par        DigiPen login: waimen.leong

\brief
Definition of functions for all IMGUI windows and their logics.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/
#include <glad/glad.h>
#include "Scripts/ImguiManager.h"
#include "Systems/SceneManager.h"
#include "Systems/Graphics.hpp"
#include "Systems/TilemapEditorManager.h"
#include "Systems/PlayFabEditorManager.h"
#include "ECS/Components/SpriteMaterial.h"
#include "ECS/Components/Cutscene.h"

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
#include "Editor/Cmds/EntitySetActiveCmd.h"
#include "Editor/Cmds/EntityAddComponentCmd.h"
#include "Editor/Cmds/EntityRemoveComponentCmd.h"
#include "Editor/Cmds/EntityReparentCmd.h"
#include "Editor/Cmds/EntityReorderCmd.h"

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
        , m_showEditorControlBar(true)
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

        // event listeners
        pEventSystem = pSystemManager->GetSystem<EventSystem>();
        pEventSystem->Subscribe<DebugLogEvent, ImguiManager>([this](const DebugLogEvent& e) { AddConsoleLog(e.message); });
        pEventSystem->Subscribe<EntityCreatedEvent, ImguiManager>([this](const EntityCreatedEvent& e) { mEntityCount = e.entityCnt; });
        pEventSystem->Subscribe<EntityDestroyedEvent, ImguiManager>([this](const EntityDestroyedEvent& e) { mEntityCount = e.entityCnt; });
        pEventSystem->Subscribe<SceneInfoRequest, ImguiManager>([this](const SceneInfoRequest& e)
            { sceneNames = e.sceneNames; scenePaths = e.scenePaths; activeSceneIndex = e.activeSceneIndex; });
        pEventSystem->Subscribe<UpdateImguiPlayModeEvent, ImguiManager>([this](const UpdateImguiPlayModeEvent& e)
            {
                m_playState = e.isPlayMode ? PlayState::Playing : PlayState::Stopped;
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
        pTilemapEditorManager = pSystemManager->GetSystem<TilemapEditorManager>();
        pPlayFabEditorManager = pSystemManager->GetSystem<PlayFabEditorManager>();

        resourcesWindow.SetResourcesManager(pResourcesManager);

        fileBrowser.setEventSystem(pEventSystem);
        auto pGraphics = pSystemManager->GetSystem<Graphics>();
        if (pGraphics)
        {
            fileBrowser.setGraphicsSystem(pGraphics);
        }

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

        // ── Main menu bar — rendered before dockspace so WorkPos is adjusted ──
        if (!fileBrowser.isPrefabEdit())
        {
            CreateMainMenuBar();
        }

        CreateDockspace();

        if (graphics && graphics->GetRenderTarget() == Uma_Engine::RenderTarget::Framebuffer)
        {
            CreateSceneViewWindow();
        }
        else
        {
            // Game mode (rendering to window) - set viewport to full window size
            auto inputSystem = pSystemManager->GetSystem<HybridInputSystem>();
            if (inputSystem && graphics && graphics->GetWindow())
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
            CreateEditorCameraWindow();
        }
        else
        {
            // play stop bar
            CreateEditorControlBar();
            if (!m_hideAll)
            {
                SceneManagerWindow();

                resourcesWindow.Render();
                // call for windows to be shown
                float currentFps = deltaTime > 0.0f ? (1.0f / deltaTime) : 0.0f;
                CreateDebugWindows(currentFps, deltaTime);

                fileBrowser.Render();
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

    // ACTUAL EDITOR METHODS
    void ImguiManager::CreateMainMenuBar()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Tools"))
            {
                if (ImGui::MenuItem("PlayFab Manager"))
                {
                    if (pPlayFabEditorManager)
                        pPlayFabEditorManager->ToggleWindow();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    void ImguiManager::CreateEditorControlBar()
    {
        if (!m_showEditorControlBar)
            return;

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y));
        // ── Toolbar (Play/Pause/Stop) below the main menu bar ─────────
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, 40));
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
                        pEventSystem->Emit<PlaySceneRequest>((m_playState == PlayState::Paused ? true : false));
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
                pEventSystem->Emit<DeleteCurrSceneRequest>(fileBrowser.getPrefabSceneName(), false);
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

        // Handle tilemap scene interaction
        if (pTilemapEditorManager && pTilemapEditorManager->IsEditing())
        {

            ImGui::Begin("Scene View");

            //Graphics* graphics = pSystemManager->GetSystem<Graphics>();

            // Get mouse position relative to the image
            ImVec2 mousePos = ImGui::GetMousePos();
            float localX = mousePos.x - imagePos.x;
            float localY = mousePos.y - imagePos.y;

            Vec2 worldPos = graphics->ScreenToWorld(Vec2(localX, localY));

            if (m_sceneViewHovered)
            {
                pTilemapEditorManager->HandlesSceneInput(ImVec2(worldPos.x, worldPos.y));
            }

            // render the grids
            // Render grid and highlight overlay
            ImDrawList* drawList = ImGui::GetWindowDrawList();

            auto& coordinator = pSystemManager->GetSystem<SceneManager>()->GetActiveScene()->GetCoordinator();

            if (coordinator.HasComponent<Uma_ECS::Tilemap>(m_selectedEntity))
            {
                auto& transform = coordinator.GetComponent<Uma_ECS::Transform>(m_selectedEntity);
                pTilemapEditorManager->RenderSceneOverlay(drawList, transform, imagePos);
            }

            ImGui::End();
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
        // Create main dockspace window, offset below the toolbar
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        float toolbarHeight = 40.f;
        ImVec2 dockPos = ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + toolbarHeight);
        ImVec2 dockSize = ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - toolbarHeight);
        ImGui::SetNextWindowPos(dockPos);
        ImGui::SetNextWindowSize(dockSize);
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

        const auto& hierarchyOrder = coordinator.GetHierarchyOrder();

        // Build a list of root entities (entities with no parent)
        std::vector<Uma_ECS::Entity> rootEntities;
        for (size_t i = 0; i < hierarchyOrder.size(); ++i)
        {
            Uma_ECS::Entity entity = hierarchyOrder[i];

            if (!transformArray.Has(entity)) continue;

            auto& transform = transformArray.GetData(entity);

            if (!transform.parent.has_value())
            {
                rootEntities.push_back(entity);
            }
        }

        // Render each root entity and its children recursively
        for (int i = 0; i < static_cast<int>(rootEntities.size()); i++)
        {
            // Get actual hierarchy index for this root entity
            int hierarchyIdx = coordinator.GetHierarchyIndex(rootEntities[i]);
            RenderHierarchyDropZone(hierarchyIdx, coordinator);
            RenderEntityNode(rootEntities[i], coordinator, transformArray);
        }
        // Final drop zone after last entity (end of hierarchy)
        RenderHierarchyDropZone(static_cast<int>(hierarchyOrder.size()), coordinator);

        if (m_HierarchyScrollToBottomFrames > 0)
        {
            ImGui::SetScrollHereY(1.0f);
            m_HierarchyScrollToBottomFrames--;
        }

        // Right-click on empty space in hierarchy window
        if (ImGui::BeginPopupContextWindow("HierarchyContextMenu", ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight))
        {
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
            ImGui::EndPopup();
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
        //bool firstRoot = true;

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

        // Add checkbox for enable/disable
        bool isActive = coordinator.IsActiveSelf(entity);

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0)); // Make checkbox smaller
        if (ImGui::Checkbox("##active", &isActive))
        {
            // Checkbox state changed - create command and execute
            auto cmd = std::make_unique<Uma_Editor::EntitySetActiveCmd>(
                &coordinator,
                entity,
                isActive,
                isActive ? "Enable " + GetEntityDisplayName(entity, coordinator)
                : "Disable " + GetEntityDisplayName(entity, coordinator)
            );
            commandHistory.ExecuteCommand(std::move(cmd));
        }
        ImGui::PopStyleVar();

        // Tooltip for checkbox
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip(isActive ? "Enabled (click to disable)" : "Disabled (click to enable)");
        }

        ImGui::SameLine();

        // Grey out the text if entity is disabled
        if (!isActive)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        }

        // Render tree node
        bool nodeOpen = ImGui::TreeNodeEx(entityName.c_str(), flags);

        if (!isActive)
        {
            ImGui::PopStyleColor();
        }

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
                    auto& newCoordinator = sceneManager->GetActiveScene()->GetCoordinator();
                    Uma_Editor::EntitySnapshot snapshotAfter = CaptureEntitySnapshot(m_editingEntity, newCoordinator);

                    auto cmd = std::make_unique<Uma_Editor::EntitySnapshotCmd>(
                        &newCoordinator,
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

            // Toggle enabled state menu item
            bool entityIsActive = coordinator.IsActiveSelf(entity);
            if (ImGui::MenuItem(entityIsActive ? "Disable" : "Enable"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntitySetActiveCmd>(
                    &coordinator,
                    entity,
                    !entityIsActive,
                    (!entityIsActive ? "Enable " : "Disable ") + GetEntityDisplayName(entity, coordinator)
                );
                commandHistory.ExecuteCommand(std::move(cmd));
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
            ImGui::Text("Dragging: %s", entityName.c_str()); // Preview while dragging
            ImGui::EndDragDropSource();
        }

        ImGui::PushStyleColor(ImGuiCol_DragDropTarget, ImVec4(100, 150, 255, 255));

        if (ImGui::BeginDragDropTarget())
        {
            const ImGuiPayload* payload = ImGui::GetDragDropPayload();

            if (payload != nullptr && payload->IsDataType("ENTITY_NODE") && payload->IsPreview()) // hovering
            {
                ImGui::SetTooltip("Reparent to: %s", entityName.c_str());
            }

            if (const ImGuiPayload* acceptedPayload = ImGui::AcceptDragDropPayload("ENTITY_NODE")) // accept
            {
                Uma_ECS::Entity droppedEntity = *(Uma_ECS::Entity*)acceptedPayload->Data;

                // Don't allow setting parent to itself or to its own children
                if (droppedEntity != entity && !IsChildOf(droppedEntity, entity, transformArray))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityReparentCmd>(
                        &coordinator, droppedEntity, entity, "Reparent Entity"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));
                }
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::PopStyleColor();

        // Render children recursively
        if (nodeOpen && hasChildren)
        {
            for (int i = 0; i < transform.children.size(); i++)
            {
                RenderHierarchyDropZone(i, coordinator, entity);
                RenderEntityNode(transform.children[i], coordinator, transformArray);
            }
            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    void ImguiManager::RenderHierarchyDropZone(int insertIndex, Uma_ECS::Coordinator& coordinator,
        std::optional<Uma_ECS::Entity> parentEntity)
    {
        ImGui::PushID(insertIndex);

        // Create a small invisible drop target area
        float availWidth = (ImGui::GetContentRegionAvail().x == 0.f) ? 1.f : ImGui::GetContentRegionAvail().x;
        ImVec2 cursorPos = ImGui::GetCursorScreenPos();

        // Invisible button to create the drop zone (thin horizontal area)
        ImGui::InvisibleButton("##dropzone", ImVec2(availWidth, 4.0f));
        ImGui::PushStyleColor(ImGuiCol_DragDropTarget, ImVec4(0, 0, 0, 0));

        // Draw visual feedback when dragging over
        if (ImGui::BeginDragDropTarget())
        {
            // Draw a highlight line to indicate drop position
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddLine(
                ImVec2(cursorPos.x, cursorPos.y + 2.0f),
                ImVec2(cursorPos.x + availWidth, cursorPos.y + 2.0f),
                IM_COL32(100, 150, 255, 255),
                2.0f
            );

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_NODE"))
            {
                Uma_ECS::Entity droppedEntity = *(Uma_ECS::Entity*)payload->Data;

                if (parentEntity.has_value())
                {
                    // Reordering within a parent's children list
                    auto& transformArray = coordinator.GetComponentArray<Uma_ECS::Transform>();
                    auto& droppedTransform = transformArray.GetData(droppedEntity);

                    // Only allow reordering if the dropped entity is a child of this parent
                    if (droppedTransform.parent.has_value() &&
                        droppedTransform.parent.value() == parentEntity.value())
                    {
                        auto& parentTransform = transformArray.GetData(parentEntity.value());

                        // Find current index in parent's children
                        int currentIndex = -1;
                        for (int i = 0; i < static_cast<int>(parentTransform.children.size()); i++)
                        {
                            if (parentTransform.children[i] == droppedEntity)
                            {
                                currentIndex = i;
                                break;
                            }
                        }

                        int targetIndex = insertIndex;
                        if (currentIndex >= 0 && currentIndex < targetIndex)
                        {
                            targetIndex--;
                        }

                        if (currentIndex != targetIndex)
                        {
                            auto cmd = std::make_unique<Uma_Editor::EntityReorderCmd>(
                                &coordinator, droppedEntity, parentEntity, currentIndex, targetIndex, "Reorder Child"
                            );
                            commandHistory.ExecuteCommand(std::move(cmd));
                        }
                    }
                }
                else
                {
                    // Reordering root entities in global hierarchy
                    auto& transformArray = coordinator.GetComponentArray<Uma_ECS::Transform>();
                    auto& droppedTransform = transformArray.GetData(droppedEntity);

                    // Only allow root entities to be reordered at root level
                    if (!droppedTransform.parent.has_value())
                    {
                        int currentIndex = coordinator.GetHierarchyIndex(droppedEntity);
                        int targetIndex = insertIndex;

                        // Adjust target index if moving down (since removal shifts indices)
                        if (currentIndex < targetIndex)
                        {
                            targetIndex--;
                        }

                        if (currentIndex != targetIndex)
                        {
                            auto cmd = std::make_unique<Uma_Editor::EntityReorderCmd>(
                                &coordinator, droppedEntity, std::nullopt, currentIndex, targetIndex, "Reorder Entity"
                            );
                            commandHistory.ExecuteCommand(std::move(cmd));
                        }
                    }
                    else // detaching from the parent
                    {
                        // Reparent command handles removing from old parent
                        auto reparentCmd = std::make_unique<Uma_Editor::EntityReparentCmd>(
                            &coordinator, droppedEntity, std::nullopt, "Detach from Parent"
                        );
                        commandHistory.ExecuteCommand(std::move(reparentCmd));

                        int currentIndex = coordinator.GetHierarchyIndex(droppedEntity);
                        int targetIndex = insertIndex;

                        // Adjust target index if moving down (since removal shifts indices)
                        if (currentIndex < targetIndex)
                        {
                            targetIndex--;
                        }

                        if (currentIndex != targetIndex)
                        {
                            auto reorderCmd = std::make_unique<Uma_Editor::EntityReorderCmd>(
                                &coordinator, droppedEntity, std::nullopt, currentIndex, targetIndex, "Reorder Entity"
                            );
                            commandHistory.ExecuteCommand(std::move(reorderCmd));
                        }
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::PopStyleColor();

        ImGui::PopID();
    }

    void ImguiManager::RenderPrefabNode(Uma_ECS::Entity entity, Uma_ECS::Coordinator& coordinator,
        Uma_ECS::ComponentArray<Uma_ECS::Transform>& transformArray)
    {
        if (!coordinator.HasActiveEntity(entity))
            return;

        auto& transform = transformArray.GetData(entity);
        bool hasChildren = !transform.children.empty();

        std::string entityName = GetEntityDisplayName(entity, coordinator);

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

        if (!hasChildren)
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

        if (m_selectedEntity == entity)
            flags |= ImGuiTreeNodeFlags_Selected;

        ImGui::PushID(static_cast<int>(entity));

        bool isActive = coordinator.IsActiveSelf(entity);

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        if (ImGui::Checkbox("##active", &isActive))
        {
            auto cmd = std::make_unique<Uma_Editor::EntitySetActiveCmd>(
                &coordinator,
                entity,
                isActive,
                isActive ? "Enable " + GetEntityDisplayName(entity, coordinator)
                : "Disable " + GetEntityDisplayName(entity, coordinator)
            );
            commandHistory.ExecuteCommand(std::move(cmd));
        }
        ImGui::PopStyleVar();

        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip(isActive ? "Enabled (click to disable)" : "Disabled (click to enable)");
        }

        ImGui::SameLine();

        if (!isActive)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

        bool nodeOpen = ImGui::TreeNodeEx(entityName.c_str(), flags);

        if (!isActive)
            ImGui::PopStyleColor();

        if (ImGui::IsItemClicked())
        {
            m_selectedEntity = entity;
            pEventSystem->Emit<EntityPickedEvent>(m_selectedEntity);
        }

        if (ImGui::BeginPopupContextItem())
        {
            m_selectedEntity = entity;

            if (ImGui::MenuItem("Create Child"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityCreateCmd>(
                    &coordinator, entity, "Create Child Entity"
                );
                Uma_Editor::EntityCreateCmd* rawCmd = cmd.get();
                commandHistory.ExecuteCommand(std::move(cmd));
                m_selectedEntity = rawCmd->GetCreatedEntity();
                m_HierarchyScrollToBottom = true;
            }

            if (coordinator.GetParent(m_selectedEntity) != std::nullopt && ImGui::MenuItem("Duplicate"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityDuplicateCmd>(
                    &coordinator, entity, "Duplicate Entity"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
                m_HierarchyScrollToBottom = true;
            }

            ImGui::Separator();

            bool entityIsActive = coordinator.IsActiveSelf(entity);
            if (ImGui::MenuItem(entityIsActive ? "Disable" : "Enable"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntitySetActiveCmd>(
                    &coordinator,
                    entity,
                    !entityIsActive,
                    (!entityIsActive ? "Enable " : "Disable ") + GetEntityDisplayName(entity, coordinator)
                );
                commandHistory.ExecuteCommand(std::move(cmd));
            }

            ImGui::Separator();

            if (coordinator.GetParent(m_selectedEntity) != std::nullopt && transformArray.Size() > 1 && ImGui::MenuItem("Delete"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityDeleteCmd>(
                    &coordinator, entity, false, "Delete Entity"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
                if (m_selectedEntity == entity)
                    m_selectedEntity = static_cast<Uma_ECS::Entity>(-1);
                m_HierarchyScrollToBottom = true;
            }

            if (coordinator.GetParent(m_selectedEntity) != std::nullopt && transformArray.Size() > 1 && ImGui::MenuItem("Delete with Children"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityDeleteCmd>(
                    &coordinator, entity, true, "Delete Entity and Children"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
                if (m_selectedEntity == entity)
                    m_selectedEntity = static_cast<Uma_ECS::Entity>(-1);
                m_HierarchyScrollToBottom = true;
            }

            ImGui::EndPopup();
        }

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
                if (droppedEntity != entity && !IsChildOf(droppedEntity, entity, transformArray))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityReparentCmd>(
                        &coordinator, droppedEntity, entity, "Reparent Entity"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));
                }
            }
            ImGui::EndDragDropTarget();
        }

        // KEY FIX: was (transform.children.size() - 1) for the trailing drop zone,
        // which duplicated index (size-1) that was already emitted in the last loop iteration.
        // Now it correctly uses children.size() as a unique "insert after last child" slot.
        if (nodeOpen && hasChildren)
        {
            for (size_t i = 0; i < transform.children.size(); i++)
            {
                Uma_ECS::Entity child = transform.children[i];
                RenderHierarchyDropZone(static_cast<int>(i), coordinator, entity);
                RenderPrefabNode(child, coordinator, transformArray);
            }
            RenderHierarchyDropZone(static_cast<int>(transform.children.size()), coordinator, entity);
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
                /*if (ImGui::Button("Remove Component"))
                {
                    coordinator.RemoveComponent<Uma_ECS::Transform>(entity);
                }*/

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
                        auto cmd = std::make_unique<Uma_Editor::EntityReparentCmd>(
                            &coordinator, entity, std::nullopt, "Remove Parent"
                        );
                        commandHistory.ExecuteCommand(std::move(cmd));
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
                if (ImGui::Button("Remove Component##RigidBody"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_ECS::RigidBody>>(
                        &coordinator,
                        entity,
                        "Remove RigidBody"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));

                    return true;
                }

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
                if (ImGui::Button("Remove Component##Sprite"))
                {
                    // Flush any pending material/sprite edits before removing
                    if (m_hasUnsavedEdit)
                    {
                        EndComponentEdit(entity, coordinator, "Sprite", true);
                    }

                    // Remove SpriteMaterial first if it exists (cascade)
                    if (coordinator.HasComponent<Uma_ECS::SpriteMaterial>(entity))
                    {
                        auto matCmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_ECS::SpriteMaterial>>(
                            &coordinator,
                            entity,
                            "Remove SpriteMaterial (cascade)"
                        );
                        commandHistory.ExecuteCommand(std::move(matCmd));
                    }

                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_ECS::Sprite>>(
                        &coordinator,
                        entity,
                        "Remove Sprite"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));

                    return true;
                }

                auto& sprite = coordinator.GetComponent<Uma_ECS::Sprite>(entity);
                ImGui::Indent();

                // begin tracking
                BeginComponentEdit(entity, coordinator);

                // In the Sprite component section, replace the current drag and drop code with this:

                ImGui::Text("Texture Path: %s", sprite.texturePath.c_str());

                // Create a visible drop zone with visual feedback
                ImVec2 dropZoneSize = ImVec2(ImGui::GetContentRegionAvail().x, 60.0f);
                ImVec2 cursorPos = ImGui::GetCursorScreenPos();

                // Draw a border box
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                ImU32 bgColor = IM_COL32(40, 40, 60, 100);

                // Background
                drawList->AddRectFilled(cursorPos,
                    ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                    bgColor, 4.0f);

                // Center text in the drop zone
                ImVec2 textSize = ImGui::CalcTextSize("Drag & Drop Texture Here");
                ImVec2 textPos = ImVec2(
                    cursorPos.x + (dropZoneSize.x - textSize.x) * 0.5f,
                    cursorPos.y + (dropZoneSize.y - textSize.y) * 0.5f - 10.0f
                );
                drawList->AddText(textPos, IM_COL32(150, 150, 150, 255), "Drag & Drop Texture Here");

                // Supported formats text
                ImVec2 formatTextSize = ImGui::CalcTextSize("(.png, .jpg, .jpeg, .bmp)");
                ImVec2 formatTextPos = ImVec2(
                    cursorPos.x + (dropZoneSize.x - formatTextSize.x) * 0.5f,
                    cursorPos.y + (dropZoneSize.y - formatTextSize.y) * 0.5f + 10.0f
                );
                drawList->AddText(formatTextPos, IM_COL32(100, 100, 100, 255), "(.png, .jpg, .jpeg, .bmp)");

                // Invisible button for the drop zone
                ImGui::SetCursorScreenPos(cursorPos);
                ImGui::InvisibleButton("##TextureDropZone", dropZoneSize);

                bool isHovered = ImGui::IsItemHovered();

                // Drag and Drop Target
                if (ImGui::BeginDragDropTarget())
                {
                    // Highlight the drop zone when dragging over
                    drawList->AddRect(cursorPos,
                        ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                        IM_COL32(100, 200, 255, 255), 4.0f, 0, 3.0f);

                    // Show glow effect
                    drawList->AddRectFilled(cursorPos,
                        ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                        IM_COL32(100, 150, 255, 50), 4.0f);

                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                    {
                        const auto* data = static_cast<const Uma_Engine::FilePayload*>(payload->Data);
                        std::string fullPath = data->filepath;
                        std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

                        std::filesystem::path p(fullPath);
                        std::string ext = p.extension().string();

                        // Convert to lowercase for comparison
                        std::transform(ext.begin(), ext.end(), ext.begin(),
                            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp")
                        {
                            std::string relativePath = fullPath;
                            size_t assetsPos = fullPath.find("Assets/");
                            if (assetsPos != std::string::npos)
                            {
                                relativePath = fullPath.substr(assetsPos);
                            }

                            sprite.texturePath = relativePath;
                            sprite.texture = nullptr;
                            m_hasUnsavedEdit = true;
                        }
                        else
                        {
                            m_popupErrorMessage = "Invalid file type for Sprite!\nExpected: .png, .jpg, .jpeg, .bmp";
                            ImGui::OpenPopup("Invalid File Format");
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
                else if (isHovered)
                {
                    // Subtle hover effect when not dragging
                    drawList->AddRect(cursorPos,
                        ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                        IM_COL32(100, 150, 200, 200), 4.0f, 0, 2.0f);
                }

                // Move cursor past the drop zone
                ImGui::SetCursorScreenPos(ImVec2(cursorPos.x, cursorPos.y + dropZoneSize.y + 5.0f));

                // Flip flags
                if (ImGui::Checkbox("Flip X", &sprite.flipX))  m_hasUnsavedEdit = true;
                if (ImGui::Checkbox("Flip Y", &sprite.flipY))  m_hasUnsavedEdit = true;
                if (ImGui::Checkbox("auto flip", &sprite.autoFlip))  m_hasUnsavedEdit = true;
                if (ImGui::Checkbox("Use Native Size", &sprite.UseNativeSize)) m_hasUnsavedEdit = true;

                // Sprite offset slider
                float offsetArray[2] = { sprite.spriteOffset.x, sprite.spriteOffset.y };
                if (ImGui::DragFloat2("Sprite Offset", offsetArray, 1.0f))
                {
                    sprite.spriteOffset.x = offsetArray[0];
                    sprite.spriteOffset.y = offsetArray[1];
                    m_hasUnsavedEdit = true;
                }

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
                ImGui::Text("Render Order");
                if (ImGui::InputInt("##Sprite Sorting Order", &sprite.renderOrder, 1, 0, 0)) m_hasUnsavedEdit = true;


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
                    std::max(sprite.spriteSheetGrid.x - 1.0f, 0.0f), "%.0f"))
                {
                    sprite.spriteCell.x = cellArray[0];
                    sprite.spriteCell.y = cellArray[1];
                    m_hasUnsavedEdit = true;
                }

                // end tracking for Sprite
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

                // SpriteMaterial (optional to Sprite)
                if (coordinator.HasComponent<Uma_ECS::SpriteMaterial>(entity))
                {
                    ImGui::Separator();
                    ImGui::Text("Material");
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Remove Material"))
                    {
                        // Flush any pending material edits before removing
                        if (m_hasUnsavedEdit)
                        {
                            EndComponentEdit(entity, coordinator, "SpriteMaterial", true);
                        }

                        auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_ECS::SpriteMaterial>>(
                            &coordinator,
                            entity,
                            "Remove SpriteMaterial"
                        );
                        commandHistory.ExecuteCommand(std::move(cmd));
                        return true;
                    }

                    auto& mat = coordinator.GetComponent<Uma_ECS::SpriteMaterial>(entity);

                    // Begin tracking for material edits (separate from Sprite)
                    BeginComponentEdit(entity, coordinator);

                    // Effect dropdown
                    auto effectNames = pResourcesManager->GetEffectShaderNames();
                    std::vector<const char*> items;
                    items.push_back("(None)");
                    int current = 0;
                    for (int i = 0; i < static_cast<int>(effectNames.size()); i++)
                    {
                        items.push_back(effectNames[i].c_str());
                        if (effectNames[i] == mat.effectName) current = i + 1;
                    }

                    if (ImGui::Combo("Effect##Material", &current, items.data(), static_cast<int>(items.size())))
                    {
                        mat.effectName = (current == 0) ? "" : effectNames[current - 1];
                        mat.properties.clear();  // reset when switching effects
                        m_hasUnsavedEdit = true;
                    }

                    // Auto-generated uniform editors from reflection
                    if (!mat.effectName.empty())
                    {
                        auto* effect = pResourcesManager->GetEffect(mat.effectName);
                        if (effect)
                        {
                            ImGui::Indent();
                            for (const auto& u : effect->uniforms)
                            {
                                // Ensure property exists with default
                                if (mat.properties.find(u.name) == mat.properties.end())
                                {
                                    switch (u.type)
                                    {
                                    case Uma_Engine::UniformType::Float: mat.properties[u.name] = 0.0f; break;
                                    case Uma_Engine::UniformType::Vec2:  mat.properties[u.name] = glm::vec2(0); break;
                                    case Uma_Engine::UniformType::Vec3:  mat.properties[u.name] = glm::vec3(0); break;
                                    case Uma_Engine::UniformType::Vec4:  mat.properties[u.name] = glm::vec4(0); break;
                                    case Uma_Engine::UniformType::Int:   mat.properties[u.name] = 0; break;
                                    }
                                }

                                // Render appropriate widget with type safety
                                auto& val = mat.properties[u.name];
                                switch (u.type)
                                {
                                case Uma_Engine::UniformType::Float:
                                    if (std::holds_alternative<float>(val))
                                    {
                                        if (ImGui::DragFloat(u.name.c_str(), &std::get<float>(val), 0.01f))
                                            m_hasUnsavedEdit = true;
                                    }
                                    break;
                                case Uma_Engine::UniformType::Vec2:
                                    if (std::holds_alternative<glm::vec2>(val))
                                    {
                                        auto& v = std::get<glm::vec2>(val);
                                        if (ImGui::DragFloat2(u.name.c_str(), &v.x, 0.01f))
                                            m_hasUnsavedEdit = true;
                                    }
                                    break;
                                case Uma_Engine::UniformType::Vec3:
                                    if (std::holds_alternative<glm::vec3>(val))
                                    {
                                        auto& v = std::get<glm::vec3>(val);
                                        if (ImGui::ColorEdit3(u.name.c_str(), &v.x))
                                            m_hasUnsavedEdit = true;
                                    }
                                    break;
                                case Uma_Engine::UniformType::Vec4:
                                    if (std::holds_alternative<glm::vec4>(val))
                                    {
                                        auto& v = std::get<glm::vec4>(val);
                                        if (ImGui::ColorEdit4(u.name.c_str(), &v.x))
                                            m_hasUnsavedEdit = true;
                                    }
                                    break;
                                case Uma_Engine::UniformType::Int:
                                    if (std::holds_alternative<int>(val))
                                    {
                                        if (ImGui::DragInt(u.name.c_str(), &std::get<int>(val)))
                                            m_hasUnsavedEdit = true;
                                    }
                                    break;
                                }
                            }
                            ImGui::Unindent();
                        }
                    }

                    // End tracking for material edits
                    EndComponentEdit(entity, coordinator, "SpriteMaterial");
                }
                else
                {
                    ImGui::Separator();
                    if (ImGui::SmallButton("Add Material"))
                    {
                        auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_ECS::SpriteMaterial>>(
                            &coordinator,
                            entity,
                            Uma_ECS::SpriteMaterial{},
                            "Add SpriteMaterial"
                        );
                        commandHistory.ExecuteCommand(std::move(cmd));
                    }
                }
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::Collider>())
        {
            if (ImGui::CollapsingHeader("Collider"))
            {
                if (ImGui::Button("Remove Component##Collider"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_ECS::Collider>>(
                        &coordinator,
                        entity,
                        "Remove Collider"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));

                    return true;
                }

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
                if (ImGui::Button("Remove Component##Camera"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_ECS::Camera>>(
                        &coordinator,
                        entity,
                        "Remove Camera"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));

                    return true;
                }

                auto& camera = coordinator.GetComponent<Uma_ECS::Camera>(entity);
                ImGui::Indent();

                // begin tracking
                BeginComponentEdit(entity, coordinator);

                if (ImGui::DragFloat("Zoom", &camera.mZoom, 0.1f, 0.1f, 20.0f)) m_hasUnsavedEdit = true;
                if (ImGui::Checkbox("Follow Player", &camera.followPlayer)) m_hasUnsavedEdit = true;

                ImGui::Separator();
                ImGui::Text("Camera Controls");
                if (ImGui::Button("Reset Zoom"))
                {
                    camera.mZoom = 10.0f;
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
                if (ImGui::Button("Remove Component##Player"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_ECS::Player>>(
                        &coordinator,
                        entity,
                        "Remove Player"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));

                    return true;
                }

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
                if (ImGui::DragFloat("Mana", &player.mMana, 0.1f, 0, 300)) m_hasUnsavedEdit = true;
                if (ImGui::DragFloat("Max Mana", &player.mMaxMana, 0.1f, 0, 300)) m_hasUnsavedEdit = true;
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
                if (ImGui::Button("Remove Component##Enemy"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_ECS::Enemy>>(
                        &coordinator,
                        entity,
                        "Remove Enemy"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));

                    return true;
                }
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
                if (ImGui::Button("Remove Component##Animator"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_ECS::Animator>>(
                        &coordinator,
                        entity,
                        "Remove Animator"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));

                    return true;
                }
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
                        // Create local copies to edit
                        int framesX = clip.framesX;
                        int framesY = clip.framesY;
                        int startFrame = clip.startFrame;
                        int frameCount = clip.frameCount;
                        float speed = clip.speed;
                        bool loop = clip.loop;

                        static char texturePathBuffer[512];
                        strncpy(texturePathBuffer, clip.texturePath.c_str(), 511);
                        texturePathBuffer[511] = '\0';

                        bool modified = false;
                        modified |= ImGui::DragInt("Frames X", &framesX, 1.0f, 1, 100);
                        modified |= ImGui::DragInt("Frames Y", &framesY, 1.0f, 1, 100);
                        modified |= ImGui::DragInt("Start Frame", &startFrame, 1.0f, 0, 1000);
                        modified |= ImGui::DragInt("Frame Count", &frameCount, 1.0f, 1, 1000);
                        modified |= ImGui::DragFloat("Speed (fps)", &speed, 0.1f, 0.1f, 60.0f);
                        modified |= ImGui::Checkbox("Loop", &loop);

                        ImGui::Text("Texture Path: %s", texturePathBuffer[0] ? texturePathBuffer : "(Use Sprite texture)");

                        // Create drag & drop zone
                        ImVec2 dropZoneSize = ImVec2(ImGui::GetContentRegionAvail().x, 60.0f);
                        ImVec2 cursorPos = ImGui::GetCursorScreenPos();

                        ImDrawList* drawList = ImGui::GetWindowDrawList();
                        ImU32 bgColor = IM_COL32(40, 40, 60, 100);

                        // Background
                        drawList->AddRectFilled(cursorPos,
                            ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                            bgColor, 4.0f);

                        // Center text in the drop zone
                        ImVec2 textSize = ImGui::CalcTextSize("Drag & Drop Texture Here");
                        ImVec2 textPos = ImVec2(
                            cursorPos.x + (dropZoneSize.x - textSize.x) * 0.5f,
                            cursorPos.y + (dropZoneSize.y - textSize.y) * 0.5f - 10.0f
                        );
                        drawList->AddText(textPos, IM_COL32(150, 150, 150, 255), "Drag & Drop Texture Here");

                        // Supported formats text
                        ImVec2 formatTextSize = ImGui::CalcTextSize("(.png, .jpg, .jpeg, .bmp)");
                        ImVec2 formatTextPos = ImVec2(
                            cursorPos.x + (dropZoneSize.x - formatTextSize.x) * 0.5f,
                            cursorPos.y + (dropZoneSize.y - formatTextSize.y) * 0.5f + 10.0f
                        );
                        drawList->AddText(formatTextPos, IM_COL32(100, 100, 100, 255), "(.png, .jpg, .jpeg, .bmp)");

                        // Invisible button for the drop zone
                        ImGui::SetCursorScreenPos(cursorPos);
                        std::string dropZoneID = "##ClipTextureDropZone_" + name;
                        ImGui::InvisibleButton(dropZoneID.c_str(), dropZoneSize);

                        bool isHovered = ImGui::IsItemHovered();

                        // Drag and Drop Target
                        if (ImGui::BeginDragDropTarget())
                        {
                            // Highlight the drop zone when dragging over
                            drawList->AddRect(cursorPos,
                                ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                                IM_COL32(100, 200, 255, 255), 4.0f, 0, 3.0f);

                            // Show glow effect
                            drawList->AddRectFilled(cursorPos,
                                ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                                IM_COL32(100, 150, 255, 50), 4.0f);

                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                            {
                                const auto* data = static_cast<const Uma_Engine::FilePayload*>(payload->Data);
                                std::string fullPath = data->filepath;
                                std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

                                std::filesystem::path p(fullPath);
                                std::string ext = p.extension().string();

                                // Convert to lowercase for comparison
                                std::transform(ext.begin(), ext.end(), ext.begin(),
                                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                                if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp")
                                {
                                    std::string relativePath = fullPath;
                                    size_t assetsPos = fullPath.find("Assets/");
                                    if (assetsPos != std::string::npos)
                                    {
                                        relativePath = fullPath.substr(assetsPos);
                                    }

                                    strncpy(texturePathBuffer, relativePath.c_str(), 511);
                                    texturePathBuffer[511] = '\0';
                                    modified = true;
                                }
                                else
                                {
                                    m_popupErrorMessage = "Invalid file type for Animation Clip Texture!\nExpected: .png, .jpg, .jpeg, .bmp";
                                    ImGui::OpenPopup("Invalid File Format");
                                }
                            }
                            ImGui::EndDragDropTarget();
                        }
                        else if (isHovered)
                        {
                            // Subtle hover effect when not dragging
                            drawList->AddRect(cursorPos,
                                ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                                IM_COL32(100, 150, 200, 200), 4.0f, 0, 2.0f);
                        }

                        // Move cursor past the drop zone
                        ImGui::SetCursorScreenPos(ImVec2(cursorPos.x, cursorPos.y + dropZoneSize.y + 5.0f));

                        // Clear button
                        if (ImGui::Button("Clear Texture Path", ImVec2(-1, 0)))
                        {
                            texturePathBuffer[0] = '\0';
                            modified = true;
                        }
                        ImGui::TextWrapped("Leave empty to use Sprite component texture");

                        // Update the clip if any value changed
                        // addclip function replaces key of same name
                        if (modified)
                        {
                            animator.animator.AddClip(name, framesX, framesY, startFrame,
                                frameCount, speed, loop, std::string(texturePathBuffer));
                            m_hasUnsavedEdit = true;
                        }

                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();

                        // Control buttons
                        if (ImGui::Button("Play", ImVec2(100, 0)))
                        {
                            animator.animator.Play(name);
                            animator.animator.GetUVs(animator.uvOffset, animator.uvSize);
                            //m_hasUnsavedEdit = true;
                        }
                        ImGui::SameLine();

                        if (ImGui::Button("Play (Restart)", ImVec2(100, 0)))
                        {
                            animator.animator.Play(name, true);
                            animator.animator.GetUVs(animator.uvOffset, animator.uvSize);
                            //m_hasUnsavedEdit = true;
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
                static char newTexturePath[512] = "";

                if (ImGui::InputText("Clip Name", newClipName, 256)) m_hasUnsavedEdit = true;
                if (ImGui::DragInt("Frames X", &newFramesX, 1.0f, 1, 100)) m_hasUnsavedEdit = true;
                if (ImGui::DragInt("Frames Y", &newFramesY, 1.0f, 1, 100)) m_hasUnsavedEdit = true;
                if (ImGui::DragInt("Start Frame", &newStartFrame, 1.0f, 0, 1000)) m_hasUnsavedEdit = true;
                if (ImGui::DragInt("Frame Count", &newFrameCount, 1.0f, 1, 1000)) m_hasUnsavedEdit = true;
                if (ImGui::DragFloat("Speed (fps)", &newSpeed, 0.1f, 0.1f, 60.0f)) m_hasUnsavedEdit = true;
                if (ImGui::Checkbox("Loop", &newLoop)) m_hasUnsavedEdit = true;

                ImGui::Separator();
                ImGui::Text("Texture Path: %s", newTexturePath[0] ? newTexturePath : "(Use Sprite texture)");

                // Create drag & drop zone for new clip
                ImVec2 newClipDropZoneSize = ImVec2(ImGui::GetContentRegionAvail().x, 60.0f);
                ImVec2 newClipCursorPos = ImGui::GetCursorScreenPos();

                ImDrawList* newClipDrawList = ImGui::GetWindowDrawList();
                ImU32 newClipBgColor = IM_COL32(40, 40, 60, 100);

                // Background
                newClipDrawList->AddRectFilled(newClipCursorPos,
                    ImVec2(newClipCursorPos.x + newClipDropZoneSize.x, newClipCursorPos.y + newClipDropZoneSize.y),
                    newClipBgColor, 4.0f);

                // Center text in the drop zone
                ImVec2 newClipTextSize = ImGui::CalcTextSize("Drag & Drop Texture Here");
                ImVec2 newClipTextPos = ImVec2(
                    newClipCursorPos.x + (newClipDropZoneSize.x - newClipTextSize.x) * 0.5f,
                    newClipCursorPos.y + (newClipDropZoneSize.y - newClipTextSize.y) * 0.5f - 10.0f
                );
                newClipDrawList->AddText(newClipTextPos, IM_COL32(150, 150, 150, 255), "Drag & Drop Texture Here");

                // Supported formats text
                ImVec2 newClipFormatTextSize = ImGui::CalcTextSize("(.png, .jpg, .jpeg, .bmp)");
                ImVec2 newClipFormatTextPos = ImVec2(
                    newClipCursorPos.x + (newClipDropZoneSize.x - newClipFormatTextSize.x) * 0.5f,
                    newClipCursorPos.y + (newClipDropZoneSize.y - newClipFormatTextSize.y) * 0.5f + 10.0f
                );
                newClipDrawList->AddText(newClipFormatTextPos, IM_COL32(100, 100, 100, 255), "(.png, .jpg, .jpeg, .bmp)");

                // Invisible button for the drop zone
                ImGui::SetCursorScreenPos(newClipCursorPos);
                ImGui::InvisibleButton("##NewClipTextureDropZone", newClipDropZoneSize);

                bool newClipIsHovered = ImGui::IsItemHovered();

                // Drag and Drop Target
                if (ImGui::BeginDragDropTarget())
                {
                    // Highlight the drop zone when dragging over
                    newClipDrawList->AddRect(newClipCursorPos,
                        ImVec2(newClipCursorPos.x + newClipDropZoneSize.x, newClipCursorPos.y + newClipDropZoneSize.y),
                        IM_COL32(100, 200, 255, 255), 4.0f, 0, 3.0f);

                    // Show glow effect
                    newClipDrawList->AddRectFilled(newClipCursorPos,
                        ImVec2(newClipCursorPos.x + newClipDropZoneSize.x, newClipCursorPos.y + newClipDropZoneSize.y),
                        IM_COL32(100, 150, 255, 50), 4.0f);

                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                    {
                        const auto* data = static_cast<const Uma_Engine::FilePayload*>(payload->Data);
                        std::string fullPath = data->filepath;
                        std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

                        std::filesystem::path p(fullPath);
                        std::string ext = p.extension().string();

                        // Convert to lowercase for comparison
                        std::transform(ext.begin(), ext.end(), ext.begin(),
                            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp")
                        {
                            std::string relativePath = fullPath;
                            size_t assetsPos = fullPath.find("Assets/");
                            if (assetsPos != std::string::npos)
                            {
                                relativePath = fullPath.substr(assetsPos);
                            }

                            strncpy(newTexturePath, relativePath.c_str(), 511);
                            newTexturePath[511] = '\0';
                            m_hasUnsavedEdit = true;
                        }
                        else
                        {
                            m_popupErrorMessage = "Invalid file type for Animation Clip Texture!\nExpected: .png, .jpg, .jpeg, .bmp";
                            ImGui::OpenPopup("Invalid File Format");
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
                else if (newClipIsHovered)
                {
                    // Subtle hover effect when not dragging
                    newClipDrawList->AddRect(newClipCursorPos,
                        ImVec2(newClipCursorPos.x + newClipDropZoneSize.x, newClipCursorPos.y + newClipDropZoneSize.y),
                        IM_COL32(100, 150, 200, 200), 4.0f, 0, 2.0f);
                }

                // Move cursor past the drop zone
                ImGui::SetCursorScreenPos(ImVec2(newClipCursorPos.x, newClipCursorPos.y + newClipDropZoneSize.y + 5.0f));

                // Clear button
                if (ImGui::Button("Clear Texture Path##NewClip", ImVec2(-1, 0)))
                {
                    newTexturePath[0] = '\0';
                    m_hasUnsavedEdit = true;
                }
                ImGui::TextWrapped("Leave empty to use Sprite component texture");

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
                            newLoop,
                            std::string(newTexturePath)
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
                EndComponentEdit(entity, coordinator, "Animator");

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::LuaScript>())
        {
            if (ImGui::CollapsingHeader("LuaScript"))
            {
                if (ImGui::Button("Remove Component##LuaScript"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_ECS::LuaScript>>(
                        &coordinator,
                        entity,
                        "Remove LuaScript"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));

                    return true;
                }
                auto& luaScript = coordinator.GetComponent<Uma_ECS::LuaScript>(entity);
                ImGui::Indent();

                BeginComponentEdit(entity, coordinator);

                ImGui::Text("Scripts: %zu", luaScript.scripts.size());

                for (size_t i = 0; i < luaScript.scripts.size(); ++i)
                {
                    auto& script = luaScript.scripts[i];

                    ImGui::PushID(static_cast<int>(i));

                    std::string label = "Script " + std::to_string(i);

                    bool isScriptEnabled = script.isEnabled;

                    if (ImGui::TreeNode(label.c_str(), "%s %s",
                        script.scriptName.c_str(),
                        isScriptEnabled ? "" : "(Disabled)"))
                    {
                        if (ImGui::Checkbox("Enabled", &isScriptEnabled))
                        {
                            // Inform the lua scripting system that a script has been disabled / enabled
                            if (IsPlaying())
                            {
                                pEventSystem->Emit<Uma_Engine::EntityScriptActiveStateChangedEvent>(entity, i, isScriptEnabled);
                            }

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
                std::vector<std::string> pendingRemoves;
                std::vector<std::string> removeLoaded;
                bool hasPendingEdit = false;

                if (ImGui::Button("Remove Component##AudioComponent"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_ECS::AudioComponent>>(
                        &coordinator,
                        entity,
                        "Remove AudioComponent"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));

                    return true;
                }

                auto& audio = coordinator.GetComponent<Uma_ECS::AudioComponent>(entity);
                ImGui::Indent();

                BeginComponentEdit(entity, coordinator);

                ImGui::Text("Position: (%.2f, %.2f, %.2f)",
                    audio.position.x, audio.position.y, audio.position.z);

                ImGui::Text("Velocity: (%.2f, %.2f, %.2f)",
                    audio.velocity.x, audio.velocity.y, audio.velocity.z);

                ImGui::Separator();
                ImGui::Text("Default Settings");

                if (ImGui::DragFloat("Default Volume", &audio.defaultVolume, 0.01f, 0.0f, 1.0f)) m_hasUnsavedEdit = true;
                if (ImGui::Checkbox("Default 3D", &audio.default3D)) m_hasUnsavedEdit = true;

                ImGui::Separator();
                ImGui::Text("Loaded Sounds: %zu", audio.loadedSounds.size());

                if (!audio.loadedSounds.empty())
                {
                    std::vector<std::string> soundNames;
                    for (const auto& [name, _] : audio.loadedSounds)
                        soundNames.push_back(name);

                    for (const std::string& soundName : soundNames)
                    {
                        auto it = audio.loadedSounds.find(soundName);
                        if (it == audio.loadedSounds.end()) continue;  // Removed during frame

                        auto& soundInstance = it->second;
                        ImGui::PushID(soundName.c_str());

                        if (ImGui::TreeNode(soundName.c_str()))
                        {
                            ImGui::Text("Sound: %s", soundName.c_str());
                            ImGui::Text("Path: %s", soundInstance.path.c_str());
                            if (ImGui::Checkbox("Is Playing", &soundInstance.isPlaying)) m_hasUnsavedEdit = true;
                            if (ImGui::Checkbox("Should Loop", &soundInstance.shouldLoop)) m_hasUnsavedEdit = true;
                            if (ImGui::Checkbox("Is 3D", &soundInstance.is3D)) m_hasUnsavedEdit = true;
                            const char* typeNames[] = { "SFX", "BGM"};
                            int currentType = static_cast<int>(soundInstance.type);
                            if (ImGui::Combo("Sound Type", &currentType, typeNames, IM_ARRAYSIZE(typeNames)))
                            {
                                soundInstance.type = static_cast<Uma_Engine::SoundType>(currentType);
                                m_hasUnsavedEdit = true;
                            }

                            ImGui::Separator();
                            ImGui::Text("Sound Properties");

                            if (ImGui::DragFloat("Volume", &soundInstance.volume, 0.01f, 0.0f, 1.0f)) m_hasUnsavedEdit = true;
                            if (ImGui::DragFloat("Pitch", &soundInstance.pitch, 0.01f, 0.1f, 3.0f)) m_hasUnsavedEdit = true;

                            if (soundInstance.is3D)
                            {
                                if (ImGui::DragFloat("Min Distance", &soundInstance.minDistance, 1.0f, 0.0f, soundInstance.maxDistance)) m_hasUnsavedEdit = true;
                                if (ImGui::DragFloat("Max Distance", &soundInstance.maxDistance, 10.0f, soundInstance.minDistance, 10000.0f)) m_hasUnsavedEdit = true;
                            }

                            ImGui::Separator();
                            if (ImGui::Button("Remove Sound"))
                            {
                                removeLoaded.push_back(soundName);
                                hasPendingEdit = true;
                            }
                            ImGui::TreePop();
                        }
                        ImGui::PopID();
                    }
                }
                else
                {
                    ImGui::TextDisabled("No loaded sounds");
                }


                ImGui::Separator();
                ImGui::Text("Active Sounds: %zu", audio.activeSounds.size());

                if (!audio.activeSounds.empty())
                {
                    std::vector<std::string> soundNames;
                    for (const auto& [name, instances] : audio.activeSounds)
                        if (!instances.empty())  // Only show non-empty groups
                            soundNames.push_back(name);

                    for (const std::string& soundName : soundNames)
                    {
                        auto it = audio.activeSounds.find(soundName);
                        if (it == audio.activeSounds.end()) continue;

                        auto& instances = it->second;  // Now vector<SoundInstance>

                        ImGui::PushID(soundName.c_str());

                        // Show sound group info + instance count
                        std::string treeLabel = std::format("{} ({})", soundName, instances.size());
                        if (ImGui::TreeNode(treeLabel.c_str()))
                        {
                            // Group-level controls (affects all instances)
                            ImGui::Text("Sound: %s", soundName.c_str());
                            ImGui::Text("Active Instances: %zu", instances.size());

                            // Show first instance's properties as representative
                            if (!instances.empty()) {
                                auto& reprInstance = instances[0];
                                ImGui::Text("Path: %s", reprInstance.path.c_str());
                                if (ImGui::Checkbox("Should Loop", &reprInstance.shouldLoop)) m_hasUnsavedEdit = true;
                                if (ImGui::Checkbox("Is 3D", &reprInstance.is3D)) m_hasUnsavedEdit = true;
                                const char* typeNames[] = { "SFX", "BGM" };
                                int currentType = static_cast<int>(reprInstance.type);
                                if (ImGui::Combo("Sound Type", &currentType, typeNames, IM_ARRAYSIZE(typeNames)))
                                {
                                    reprInstance.type = static_cast<Uma_Engine::SoundType>(currentType);
                                    m_hasUnsavedEdit = true;
                                }

                                ImGui::Separator();
                                ImGui::Text("Properties (first instance)");

                                if (ImGui::DragFloat("Volume", &reprInstance.volume, 0.01f, 0.0f, 1.0f)) m_hasUnsavedEdit = true;
                                if (ImGui::DragFloat("Pitch", &reprInstance.pitch, 0.01f, 0.1f, 3.0f)) m_hasUnsavedEdit = true;

                                if (reprInstance.is3D) {
                                    if (ImGui::DragFloat("Min Distance", &reprInstance.minDistance, 1.0f, 0.0f, reprInstance.maxDistance)) m_hasUnsavedEdit = true;
                                    if (ImGui::DragFloat("Max Distance", &reprInstance.maxDistance, 10.0f, reprInstance.minDistance, 10000.0f)) m_hasUnsavedEdit = true;
                                }
                            }

                            ImGui::Separator();
                            if (ImGui::Button("Remove All Instances"))
                            {
                                pendingRemoves.push_back(soundName);
                                hasPendingEdit = true;
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


                // Drag and Drop Zone for Adding Sounds
                ImGui::Separator();
                ImGui::Text("Add Sound");

                ImVec2 dropZoneSize = ImVec2(ImGui::GetContentRegionAvail().x, 60.0f);
                ImVec2 cursorPos = ImGui::GetCursorScreenPos();
                ImDrawList* drawList = ImGui::GetWindowDrawList();

                // Background box
                ImU32 bgColor = IM_COL32(40, 40, 60, 100);
                drawList->AddRectFilled(cursorPos,
                    ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                    bgColor, 4.0f);

                // Centered help text
                ImVec2 textSize = ImGui::CalcTextSize("Drag & Drop Audio File Here");
                ImVec2 textPos = ImVec2(
                    cursorPos.x + (dropZoneSize.x - textSize.x) * 0.5f,
                    cursorPos.y + (dropZoneSize.y - textSize.y) * 0.5f - 10.0f
                );
                drawList->AddText(textPos, IM_COL32(150, 150, 150, 255), "Drag & Drop Audio File Here");

                // Supported formats hint
                ImVec2 formatTextSize = ImGui::CalcTextSize("(.wav, .mp3, .ogg, .flac)");
                ImVec2 formatTextPos = ImVec2(
                    cursorPos.x + (dropZoneSize.x - formatTextSize.x) * 0.5f,
                    cursorPos.y + (dropZoneSize.y - formatTextSize.y) * 0.5f + 10.0f
                );
                drawList->AddText(formatTextPos, IM_COL32(100, 100, 100, 255), "(.wav, .mp3, .ogg, .flac)");

                // Invisible button for hit detection
                ImGui::SetCursorScreenPos(cursorPos);
                ImGui::InvisibleButton("##AudioDropZone", dropZoneSize);

                // Hover effect
                if (ImGui::IsItemHovered())
                {
                    drawList->AddRect(cursorPos,
                        ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                        IM_COL32(100, 150, 200, 200), 4.0f, 0, 2.0f);
                }

                // Accept drag and drop payload
                if (ImGui::BeginDragDropTarget())
                {
                    // Visual feedback when dragging over
                    drawList->AddRect(cursorPos,
                        ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                        IM_COL32(100, 200, 255, 255), 4.0f, 0, 3.0f);

                    // Glow effect
                    drawList->AddRect(
                        ImVec2(cursorPos.x - 3, cursorPos.y - 3),
                        ImVec2(cursorPos.x + dropZoneSize.x + 3, cursorPos.y + dropZoneSize.y + 3),
                        IM_COL32(100, 150, 255, 50), 4.0f, 0, 6.0f
                    );

                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                    {
                        const auto* data = static_cast<const Uma_Engine::FilePayload*>(payload->Data);
                        std::string fullPath = data->filepath;

                        // Normalize path
                        std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

                        // Extract extension
                        std::filesystem::path p(fullPath);
                        std::string ext = p.extension().string();
                        std::transform(ext.begin(), ext.end(), ext.begin(),
                            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                        // Validate audio file format
                        if (ext == ".wav" || ext == ".mp3" || ext == ".ogg" || ext == ".flac")
                        {
                            // Convert to relative path (Assets-relative)
                            std::string relativePath;
                            size_t assetsPos = fullPath.find("Assets/");
                            if (assetsPos != std::string::npos)
                            {
                                relativePath = fullPath.substr(assetsPos);
                            }
                            else
                            {
                                relativePath = fullPath; // Fallback to full path
                            }

                            // Extract sound name (filename without extension)
                            std::string soundName = p.stem().string();

                            // Check if sound with this name already exists
                            if (audio.HasSound(soundName))
                            {
                                m_popupErrorMessage = "Sound with name '" + soundName + "' already exists!\nPlease remove it first or rename the file.";
                                ImGui::OpenPopup("Sound Already Exists");
                            }
                            else
                            {
                                // Create new sound instance
                                Uma_ECS::SoundInstance newSound;
                                newSound.soundName = soundName;
                                newSound.path = relativePath;
                                newSound.volume = audio.defaultVolume;
                                newSound.is3D = audio.default3D;
                                newSound.type = Uma_Engine::SoundType::SFX;

                                // Add to active sounds map
                                audio.loadedSounds.emplace(soundName, newSound);
                                hasPendingEdit = true;
                            }
                        }
                        else
                        {
                            m_popupErrorMessage = "Invalid file type for AudioComponent!\nExpected: .wav, .mp3, .ogg, .flac";
                            ImGui::OpenPopup("Invalid File Format");
                        }
                    }
                    ImGui::EndDragDropTarget();
                }

                // Error popup for duplicate sounds
                if (ImGui::BeginPopupModal("Sound Already Exists", NULL, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::Text("%s", m_popupErrorMessage.c_str());
                    ImGui::Separator();
                    if (ImGui::Button("OK", ImVec2(120, 0)))
                    {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
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

                if (hasPendingEdit) {
                    for (const auto& name : pendingRemoves) {
                        audio.RemoveSound(name);
                    }
                    for (const auto& name : removeLoaded) {
                        audio.RemoveLoadedSound(name);
                    }
                    pendingRemoves.clear();
                    removeLoaded.clear();
                    m_hasUnsavedEdit = true;
                    ImGui::GetCurrentWindow()->Flags &= ~ImGuiWindowFlags_UnsavedDocument;
                    hasPendingEdit = false;
                }

                EndComponentEdit(entity, coordinator, "AudioComponent");

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::AudioListener>())
        {
            if (ImGui::CollapsingHeader("AudioListener"))
            {
                if (ImGui::Button("Remove Component##AudioListener"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_ECS::AudioListener>>(
                        &coordinator,
                        entity,
                        "Remove AudioListener"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));

                    return true;
                }

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
                if (ImGui::Button("Remove Component##PathFinding"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_ECS::PathFinding>>(
                        &coordinator,
                        entity,
                        "Remove PathFinding"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));

                    return true;
                }

                auto& pathfinding = coordinator.GetComponent<Uma_ECS::PathFinding>(entity);
                ImGui::Indent();

                BeginComponentEdit(entity, coordinator);

                ImGui::Text("Path Update Settings");
                if (ImGui::DragFloat("Update Interval", &pathfinding.pathUpdateInterval, 0.01f, 0.01f, 5.0f)) m_hasUnsavedEdit = true;

                ImGui::Separator();
                ImGui::Text("Goal Position");

                float goalPos[2] = { pathfinding.goal.x, pathfinding.goal.y };
                if (ImGui::DragFloat2("Goal", goalPos, 0.1f))
                {
                    pathfinding.goal = Vec2(goalPos[0], goalPos[1]);
                    m_hasUnsavedEdit = true;
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

                EndComponentEdit(entity, coordinator, "PathFinding");

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::ParticleEmitter>())
        {
            if (ImGui::CollapsingHeader("ParticleEmitter", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::Button("Remove Component##ParticleEmitter"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_ECS::ParticleEmitter>>(
                        &coordinator,
                        entity,
                        "Remove ParticleEmitter"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));

                    return true;
                }

                auto& emitterArray = coordinator.GetComponentArray<Uma_ECS::ParticleEmitter>();
                auto& component = emitterArray.GetData(entity);

                ImGui::Indent();

                BeginComponentEdit(entity, coordinator);

                // Add Emitter button
                if (ImGui::Button("Add Emitter"))
                {
                    component.AddEmitter("New Emitter");
                    m_hasUnsavedEdit = true;
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
                            m_hasUnsavedEdit = true;
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
                            m_hasUnsavedEdit = true;
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
                        if (ImGui::Checkbox("Active", &emitter->isActive)) m_hasUnsavedEdit = true;

                        // Emission Mode
                        const char* modes[] = { "Burst", "Continuous", "ScreenFill" };
                        int currentMode = static_cast<int>(emitter->mode);
                        if (ImGui::Combo("Emission Mode", &currentMode, modes, IM_ARRAYSIZE(modes)))
                        {
                            emitter->mode = static_cast<Uma_ECS::EmitterMode>(currentMode);
                            m_hasUnsavedEdit = true;
                        }

                        // Max Particles
                        if (ImGui::DragInt("Max Particles", &emitter->maxParticles, 1.0f, 1, 10000)) m_hasUnsavedEdit = true;

                        ImGui::Text("Texture Path: %s", emitter->texturePath.c_str());

                        // Create a visible drop zone with visual feedback
                        ImVec2 dropZoneSize = ImVec2(ImGui::GetContentRegionAvail().x, 60.0f);
                        ImVec2 cursorPos = ImGui::GetCursorScreenPos();

                        // Draw a border box
                        ImDrawList* drawList = ImGui::GetWindowDrawList();
                        ImU32 bgColor = IM_COL32(40, 40, 60, 100);

                        // Background
                        drawList->AddRectFilled(cursorPos,
                            ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                            bgColor, 4.0f);

                        // Center text in the drop zone
                        ImVec2 textSize = ImGui::CalcTextSize("Drag & Drop Texture Here");
                        ImVec2 textPos = ImVec2(
                            cursorPos.x + (dropZoneSize.x - textSize.x) * 0.5f,
                            cursorPos.y + (dropZoneSize.y - textSize.y) * 0.5f - 10.0f
                        );
                        drawList->AddText(textPos, IM_COL32(150, 150, 150, 255), "Drag & Drop Texture Here");

                        // Supported formats text
                        ImVec2 formatTextSize = ImGui::CalcTextSize("(.png, .jpg, .jpeg, .bmp)");
                        ImVec2 formatTextPos = ImVec2(
                            cursorPos.x + (dropZoneSize.x - formatTextSize.x) * 0.5f,
                            cursorPos.y + (dropZoneSize.y - formatTextSize.y) * 0.5f + 10.0f
                        );
                        drawList->AddText(formatTextPos, IM_COL32(100, 100, 100, 255), "(.png, .jpg, .jpeg, .bmp)");

                        // Invisible button for the drop zone
                        ImGui::SetCursorScreenPos(cursorPos);
                        ImGui::InvisibleButton("##ParticleTextureDropZone", dropZoneSize);

                        bool isHovered = ImGui::IsItemHovered();

                        // Drag and Drop Target
                        if (ImGui::BeginDragDropTarget())
                        {
                            // Highlight the drop zone when dragging over
                            drawList->AddRect(cursorPos,
                                ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                                IM_COL32(100, 200, 255, 255), 4.0f, 0, 3.0f);

                            // Show glow effect
                            drawList->AddRectFilled(cursorPos,
                                ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                                IM_COL32(100, 150, 255, 50), 4.0f);

                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                            {
                                const auto* data = static_cast<const Uma_Engine::FilePayload*>(payload->Data);
                                std::string fullPath = data->filepath;
                                std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

                                std::filesystem::path p(fullPath);
                                std::string ext = p.extension().string();

                                // Convert to lowercase for comparison
                                std::transform(ext.begin(), ext.end(), ext.begin(),
                                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                                if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp")
                                {
                                    std::string relativePath = fullPath;
                                    size_t assetsPos = fullPath.find("Assets/");
                                    if (assetsPos != std::string::npos)
                                    {
                                        relativePath = fullPath.substr(assetsPos);
                                    }

                                    emitter->texturePath = relativePath;
                                    m_hasUnsavedEdit = true;
                                }
                                else
                                {
                                    m_popupErrorMessage = "Invalid file type for Particle Texture!\nExpected: .png, .jpg, .jpeg, .bmp";
                                    ImGui::OpenPopup("Invalid File Format");
                                }
                            }
                            ImGui::EndDragDropTarget();
                        }
                        else if (isHovered)
                        {
                            // Subtle hover effect when not dragging
                            drawList->AddRect(cursorPos,
                                ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                                IM_COL32(100, 150, 200, 200), 4.0f, 0, 2.0f);
                        }

                        // Move cursor past the drop zone
                        ImGui::SetCursorScreenPos(ImVec2(cursorPos.x, cursorPos.y + dropZoneSize.y + 5.0f));

                        ImGui::Separator();

                        ImGui::Text("Rendering");

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
                        unsigned int rl = static_cast<unsigned int>(emitter->renderLayer);
                        while (rl >>= 1) ++currentRenderLayer;
                        if (ImGui::Combo("Render Layer", &currentRenderLayer, renderLayerNames, IM_ARRAYSIZE(renderLayerNames)))
                        {
                            emitter->renderLayer = (1u << currentRenderLayer);
                            m_hasUnsavedEdit = true;
                        }

                        ImGui::Text("Render Order");
                        if (ImGui::InputInt("##Particle Render Order", &emitter->renderOrder, 1, 0, 0)) m_hasUnsavedEdit = true;

                        ImGui::Separator();

                        // Appearance settings
                        if (ImGui::CollapsingHeader("Appearance", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ImGui::Indent();

                            // Scale Range
                            if (ImGui::DragFloat2("Scale Range", &emitter->appearance.scaleRange.x, 0.01f, 0.01f, 10.0f, "%.2f")) m_hasUnsavedEdit = true;

                            // Start Color
                            if (ImGui::ColorEdit3("Start Color", &emitter->appearance.startColor.x)) m_hasUnsavedEdit = true;

                            // End Color
                            if (ImGui::ColorEdit3("End Color", &emitter->appearance.endColor.x)) m_hasUnsavedEdit = true;

                            // Color Lerp
                            if (ImGui::Checkbox("Color Lerp", &emitter->appearance.colorLerp)) m_hasUnsavedEdit = true;

                            // Random Opacity
                            if (ImGui::Checkbox("Random Opacity", &emitter->appearance.randomOpacity)) m_hasUnsavedEdit = true;
                            if (emitter->appearance.randomOpacity)
                            {
                                ImGui::Indent();
                                if (ImGui::DragFloat2("Opacity Range", &emitter->appearance.opacityRange.x, 0.01f, 0.0f, 1.0f, "%.2f")) m_hasUnsavedEdit = true;
                                ImGui::Unindent();
                            }

                            // Rotate Particles
                            if (ImGui::Checkbox("Rotate Particles", &emitter->appearance.rotateParticles)) m_hasUnsavedEdit = true;
                            if (emitter->appearance.rotateParticles)
                            {
                                ImGui::Indent();
                                if (ImGui::DragFloat2("Rotation Speed Range", &emitter->appearance.rotationSpeedRange.x, 1.0f, -360.0f, 360.0f, "%.1f deg/s")) m_hasUnsavedEdit = true;
                                ImGui::Unindent();
                            }

                            ImGui::Unindent();
                        }

                        // Fade settings
                        if (ImGui::CollapsingHeader("Fade", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ImGui::Indent();

                            // Fade In
                            if (ImGui::Checkbox("Fade In", &emitter->fade.fadeIn)) m_hasUnsavedEdit = true;
                            if (emitter->fade.fadeIn)
                            {
                                ImGui::Indent();
                                if (ImGui::DragFloat("Fade In Duration", &emitter->fade.fadeInDuration, 0.01f, 0.01f, 5.0f, "%.2f sec")) m_hasUnsavedEdit = true;
                                ImGui::Unindent();
                            }

                            // Fade Out (only for Burst/Continuous)
                            if (emitter->mode != Uma_ECS::EmitterMode::ScreenFill)
                            {
                                if (ImGui::Checkbox("Fade Out", &emitter->fade.fadeOut)) m_hasUnsavedEdit = true;
                                if (emitter->fade.fadeOut)
                                {
                                    ImGui::Indent();
                                    if (ImGui::DragFloat("Fade Out Duration", &emitter->fade.fadeOutDuration, 0.01f, 0.01f, 5.0f, "%.2f sec")) m_hasUnsavedEdit = true;
                                    ImGui::Unindent();
                                }
                            }

                            // Fade At Edges (only for ScreenFill)
                            if (emitter->mode == Uma_ECS::EmitterMode::ScreenFill)
                            {
                                if (ImGui::Checkbox("Fade At Edges", &emitter->fade.fadeAtEdges)) m_hasUnsavedEdit = true;
                                if (emitter->fade.fadeAtEdges)
                                {
                                    ImGui::Indent();
                                    if (ImGui::DragFloat("Edge Fade Distance", &emitter->fade.edgeFadeDistance, 1.0f, 0.0f, 500.0f, "%.1f")) m_hasUnsavedEdit = true;
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
                            if (ImGui::DragFloat2("Speed Range", &emitter->physics.speedRange.x, 0.1f, 0.0f, 1000.0f, "%.1f")) m_hasUnsavedEdit = true;

                            // Lifetime Range (not shown for ScreenFill)
                            if (emitter->mode != Uma_ECS::EmitterMode::ScreenFill)
                            {
                                if (ImGui::DragFloat2("Lifetime Range", &emitter->physics.lifetimeRange.x, 0.01f, 0.01f, 100.0f, "%.2f sec")) m_hasUnsavedEdit = true;
                            }

                            // Gravity
                            if (ImGui::DragFloat2("Gravity", &emitter->physics.gravity.x, 0.1f, -500.0f, 500.0f, "%.1f")) m_hasUnsavedEdit = true;

                            // Drag
                            if (ImGui::DragFloat("Drag", &emitter->physics.drag, 0.01f, 0.0f, 10.0f, "%.2f")) m_hasUnsavedEdit = true;

                            ImGui::Unindent();
                        }

                        // Spawn settings
                        if (ImGui::CollapsingHeader("Spawn", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            ImGui::Indent();

                            // Spawn Offset (not shown for ScreenFill)
                            if (emitter->mode != Uma_ECS::EmitterMode::ScreenFill)
                            {
                                if (ImGui::DragFloat2("Spawn Offset", &emitter->spawn.spawnOffset.x, 1.0f, -1000.0f, 1000.0f, "%.1f")) m_hasUnsavedEdit = true;

                                // Spawn Radius
                                if (ImGui::DragFloat("Spawn Radius", &emitter->spawn.spawnRadius, 1.0f, 0.0f, 500.0f, "%.1f")) m_hasUnsavedEdit = true;

                                // Emission Cone
                                if (ImGui::Checkbox("Use Emission Cone", &emitter->spawn.useEmissionCone)) m_hasUnsavedEdit = true;
                                if (emitter->spawn.useEmissionCone)
                                {
                                    ImGui::Indent();
                                    if (ImGui::DragFloat("Emission Angle", &emitter->spawn.emissionAngle, 1.0f, 0.0f, 360.0f, "%.1f deg")) m_hasUnsavedEdit = true;
                                    if (ImGui::DragFloat("Emission Spread", &emitter->spawn.emissionSpread, 1.0f, 0.0f, 360.0f, "%.1f deg")) m_hasUnsavedEdit = true;
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
                                if (ImGui::DragFloat("Emission Rate", &emitter->emission.emissionRate, 1.0f, 1.0f, 1000.0f, "%.1f particles/sec")) m_hasUnsavedEdit = true;
                            }

                            // Burst Mode Loop Settings
                            if (emitter->mode == Uma_ECS::EmitterMode::Burst)
                            {
                                if (ImGui::Checkbox("Loop", &emitter->emission.loop)) m_hasUnsavedEdit = true;
                                if (emitter->emission.loop)
                                {
                                    ImGui::Indent();
                                    if (ImGui::DragFloat("Loop Delay", &emitter->emission.loopDelay, 0.1f, 0.0f, 60.0f, "%.1f sec")) m_hasUnsavedEdit = true;
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
                                if (ImGui::DragFloat2("Velocity X Range", &emitter->screenFill.velocityXRange.x, 1.0f, -500.0f, 500.0f, "%.1f")) m_hasUnsavedEdit = true;
                                if (ImGui::DragFloat2("Velocity Y Range", &emitter->screenFill.velocityYRange.x, 1.0f, -500.0f, 500.0f, "%.1f")) m_hasUnsavedEdit = true;

                                // Spawn At Top
                                if (ImGui::Checkbox("Spawn At Top", &emitter->screenFill.spawnAtTop)) m_hasUnsavedEdit = true;

                                // Spawn Margin
                                if (ImGui::DragFloat("Spawn Margin", &emitter->screenFill.spawnMargin, 1.0f, 0.0f, 1000.0f, "%.1f")) m_hasUnsavedEdit = true;

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

                EndComponentEdit(entity, coordinator, "ParticleEmitter");

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_UI::RectTransform>())
        {
            if (ImGui::CollapsingHeader("RectTransform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::Button("Remove Component##RectTransform"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_UI::RectTransform>>(
                        &coordinator,
                        entity,
                        "Remove RectTransform"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));

                    return true;
                }

                auto& rectTransform = coordinator.GetComponent<Uma_UI::RectTransform>(entity);
                ImGui::Indent();

                // Begin tracking
                BeginComponentEdit(entity, coordinator);

                ImGui::Text("Anchors");
                ImGui::Separator();

                // Anchor Preset Picker
                {
                    struct PresetEntry
                    {
                        const char* tooltip;
                        Uma_UI::AnchorPreset(*fn)();
                    };

                    static const PresetEntry presets[4][3] =
                    {
                        { { "Top Left",    Uma_UI::AnchorPreset::TopLeft    }, { "Top Center",    Uma_UI::AnchorPreset::TopCenter    }, { "Top Right",    Uma_UI::AnchorPreset::TopRight    } },
                        { { "Mid Left",    Uma_UI::AnchorPreset::MiddleLeft }, { "Mid Center",    Uma_UI::AnchorPreset::MiddleCenter }, { "Mid Right",    Uma_UI::AnchorPreset::MiddleRight } },
                        { { "Bot Left",    Uma_UI::AnchorPreset::BottomLeft }, { "Bot Center",    Uma_UI::AnchorPreset::BottomCenter }, { "Bot Right",    Uma_UI::AnchorPreset::BottomRight } },
                        { { "Stretch H",   Uma_UI::AnchorPreset::StretchHorizontal }, { "Stretch V", Uma_UI::AnchorPreset::StretchVertical }, { "Stretch All", Uma_UI::AnchorPreset::StretchAll } },
                    };

                    auto matchesPreset = [&](const Uma_UI::AnchorPreset& p) -> bool
                        {
                            return rectTransform.anchorMin.x == p.anchorMin.x
                                && rectTransform.anchorMin.y == p.anchorMin.y
                                && rectTransform.anchorMax.x == p.anchorMax.x
                                && rectTransform.anchorMax.y == p.anchorMax.y;
                        };

                    // Draws a miniature anchor diagram into a cell using ImDrawList.
                    // parentMin/parentMax: pixel bounds of the outer "parent" box.
                    // preset: the anchor config to visualise.
                    // col_dim: dim line colour, col_anchor: anchor marker colour.
                    auto drawAnchorDiagram = [&](
                        ImDrawList* dl,
                        ImVec2 parentMin, ImVec2 parentMax,
                        const Uma_UI::AnchorPreset& p,
                        ImU32 col_bg, ImU32 col_border,
                        ImU32 col_anchor, ImU32 col_child)
                        {
                            float pw = parentMax.x - parentMin.x;
                            float ph = parentMax.y - parentMin.y;

                            // Parent box
                            dl->AddRectFilled(parentMin, parentMax, col_bg);
                            dl->AddRect(parentMin, parentMax, col_border);

                            // Anchor point(s) in pixel space.
                            // Note: Y is flipped ? anchorMin.y=0 is bottom in UI space, but top in screen space.
                            float axMin = parentMin.x + p.anchorMin.x * pw;
                            float ayMin = parentMax.y - p.anchorMin.y * ph; // flip Y
                            float axMax = parentMin.x + p.anchorMax.x * pw;
                            float ayMax = parentMax.y - p.anchorMax.y * ph; // flip Y

                            bool stretchH = (p.anchorMin.x != p.anchorMax.x);
                            bool stretchV = (p.anchorMin.y != p.anchorMax.y);

                            if (stretchH || stretchV)
                            {
                                // Draw stretch lines across the parent
                                if (stretchH)
                                    dl->AddLine(ImVec2(axMin, parentMin.y + ph * 0.5f),
                                        ImVec2(axMax, parentMin.y + ph * 0.5f), col_anchor, 1.5f);
                                if (stretchV)
                                    dl->AddLine(ImVec2(parentMin.x + pw * 0.5f, ayMax),
                                        ImVec2(parentMin.x + pw * 0.5f, ayMin), col_anchor, 1.5f);

                                // Child rect fills anchor span
                                float childL = stretchH ? axMin : parentMin.x + pw * 0.15f;
                                float childR = stretchH ? axMax : parentMax.x - pw * 0.15f;
                                float childT = stretchV ? ayMax : parentMin.y + ph * 0.25f;
                                float childB = stretchV ? ayMin : parentMax.y - ph * 0.25f;
                                dl->AddRectFilled(ImVec2(childL, childT), ImVec2(childR, childB), col_child);

                                // Arrow heads on stretch lines
                                const float ar = 2.5f;
                                if (stretchH)
                                {
                                    float my = parentMin.y + ph * 0.5f;
                                    dl->AddTriangleFilled(ImVec2(axMin, my), ImVec2(axMin + ar, my - ar), ImVec2(axMin + ar, my + ar), col_anchor);
                                    dl->AddTriangleFilled(ImVec2(axMax, my), ImVec2(axMax - ar, my - ar), ImVec2(axMax - ar, my + ar), col_anchor);
                                }
                                if (stretchV)
                                {
                                    float mx = parentMin.x + pw * 0.5f;
                                    dl->AddTriangleFilled(ImVec2(mx, ayMin), ImVec2(mx - ar, ayMin - ar), ImVec2(mx + ar, ayMin - ar), col_anchor);
                                    dl->AddTriangleFilled(ImVec2(mx, ayMax), ImVec2(mx - ar, ayMax + ar), ImVec2(mx + ar, ayMax + ar), col_anchor);
                                }
                            }
                            else
                            {
                                // Point anchor ? draw crosshair at anchor point
                                float cx = axMin;
                                float cy = ayMin;
                                dl->AddLine(ImVec2(cx - 3, cy), ImVec2(cx + 3, cy), col_anchor, 1.5f);
                                dl->AddLine(ImVec2(cx, cy - 3), ImVec2(cx, cy + 3), col_anchor, 1.5f);

                                // Child rect offset from anchor via pivot
                                float cw = pw * 0.45f;
                                float ch = ph * 0.45f;
                                float childL = cx + (0.0f - p.pivot.x) * cw;
                                float childT = cy + (p.pivot.y - 1.0f) * ch;
                                dl->AddRectFilled(
                                    ImVec2(childL, childT),
                                    ImVec2(childL + cw, childT + ch),
                                    col_child);
                            }
                        };

                    // Preset button: "Anchor Presets" opens a popup
                    if (ImGui::Button("Anchor Presets##AnchorPresetBtn"))
                        ImGui::OpenPopup("AnchorPresetPopup");

                    if (ImGui::BeginPopup("AnchorPresetPopup"))
                    {
                        ImGui::TextDisabled("Anchor Presets");
                        ImGui::Separator();
                        ImGui::Spacing();

                        ImDrawList* dl = ImGui::GetWindowDrawList();
                        const float cell = 46.0f;  // cell size in pixels
                        const float pad = 4.0f;   // padding inside cell
                        const float diagSz = cell - pad * 2.0f;
                        const int   cols = 3;
                        const int   rows = 4;

                        ImU32 col_bg = IM_COL32(40, 40, 44, 255);
                        ImU32 col_border = IM_COL32(90, 90, 100, 255);
                        ImU32 col_anchor = IM_COL32(100, 200, 255, 255);
                        ImU32 col_child = IM_COL32(80, 140, 200, 120);
                        ImU32 col_active_bg = IM_COL32(60, 120, 200, 255);
                        ImU32 col_hover_bg = IM_COL32(55, 55, 65, 255);

                        for (int row = 0; row < rows; ++row)
                        {
                            if (row == 3)
                            {
                                ImGui::Spacing();
                                ImGui::Separator();
                                ImGui::Spacing();
                            }

                            for (int col = 0; col < cols; ++col)
                            {
                                if (col > 0) ImGui::SameLine(0.0f, 4.0f);

                                Uma_UI::AnchorPreset p = presets[row][col].fn();
                                bool active = matchesPreset(p);

                                // Invisible button to capture hover/click
                                std::string btnId = "##APCell_" + std::to_string(row) + "_" + std::to_string(col);
                                ImVec2 cellMin = ImGui::GetCursorScreenPos();
                                ImVec2 cellMax = ImVec2(cellMin.x + cell, cellMin.y + cell);

                                bool hovered = false;
                                bool clicked = ImGui::InvisibleButton(btnId.c_str(), ImVec2(cell, cell));
                                hovered = ImGui::IsItemHovered();

                                // Background
                                ImU32 bgCol = active ? col_active_bg : (hovered ? col_hover_bg : col_bg);
                                dl->AddRectFilled(cellMin, cellMax, bgCol, 4.0f);
                                dl->AddRect(cellMin, cellMax,
                                    active ? IM_COL32(120, 180, 255, 255) : IM_COL32(70, 70, 80, 255),
                                    4.0f, 0, active ? 1.5f : 1.0f);

                                // Anchor diagram inside cell
                                ImVec2 diagMin = ImVec2(cellMin.x + pad, cellMin.y + pad);
                                ImVec2 diagMax = ImVec2(diagMin.x + diagSz, diagMin.y + diagSz);
                                drawAnchorDiagram(dl, diagMin, diagMax, p,
                                    active ? IM_COL32(30, 70, 130, 255) : col_bg,
                                    active ? IM_COL32(140, 200, 255, 200) : col_border,
                                    active ? IM_COL32(255, 255, 255, 255) : col_anchor,
                                    active ? IM_COL32(180, 220, 255, 160) : col_child);

                                if (clicked)
                                {
                                    rectTransform.ApplyPreset(p);
                                    rectTransform.isDirty = true;
                                    m_hasUnsavedEdit = true;
                                    ImGui::CloseCurrentPopup();
                                }

                                if (hovered)
                                    ImGui::SetTooltip("%s\nanchorMin=(%.1f, %.1f)  anchorMax=(%.1f, %.1f)\npivot=(%.1f, %.1f)",
                                        presets[row][col].tooltip,
                                        p.anchorMin.x, p.anchorMin.y,
                                        p.anchorMax.x, p.anchorMax.y,
                                        p.pivot.x, p.pivot.y);
                            }
                        }

                        ImGui::Spacing();
                        ImGui::EndPopup();
                    }

                    // Small inline preview of the current anchor next to the button
                    ImGui::SameLine();
                    {
                        ImDrawList* dl = ImGui::GetWindowDrawList();
                        ImVec2 pos = ImGui::GetCursorScreenPos();
                        const float sz = 20.0f;
                        ImVec2 pMin = ImVec2(pos.x, pos.y + 1.0f);
                        ImVec2 pMax = ImVec2(pos.x + sz, pos.y + sz - 1.0f);
                        Uma_UI::AnchorPreset cur = { rectTransform.anchorMin, rectTransform.anchorMax, rectTransform.pivot };
                        ImGui::Dummy(ImVec2(sz + 2.0f, sz));
                        dl->AddRectFilled(pMin, pMax, IM_COL32(40, 40, 44, 255), 2.0f);
                        dl->AddRect(pMin, pMax, IM_COL32(90, 90, 100, 255), 2.0f);

                        float pw = pMax.x - pMin.x;
                        float ph = pMax.y - pMin.y;
                        float axMin = pMin.x + cur.anchorMin.x * pw;
                        float ayMin = pMax.y - cur.anchorMin.y * ph;
                        float axMax = pMin.x + cur.anchorMax.x * pw;
                        float ayMax = pMax.y - cur.anchorMax.y * ph;
                        bool strH = cur.anchorMin.x != cur.anchorMax.x;
                        bool strV = cur.anchorMin.y != cur.anchorMax.y;
                        ImU32 ac = IM_COL32(100, 200, 255, 255);
                        if (strH || strV)
                        {
                            if (strH) dl->AddLine(ImVec2(axMin, pMin.y + ph * 0.5f), ImVec2(axMax, pMin.y + ph * 0.5f), ac, 1.5f);
                            if (strV) dl->AddLine(ImVec2(pMin.x + pw * 0.5f, ayMax), ImVec2(pMin.x + pw * 0.5f, ayMin), ac, 1.5f);
                        }
                        else
                        {
                            dl->AddLine(ImVec2(axMin - 3, ayMin), ImVec2(axMin + 3, ayMin), ac, 1.5f);
                            dl->AddLine(ImVec2(axMin, ayMin - 3), ImVec2(axMin, ayMin + 3), ac, 1.5f);
                        }
                    }

                    ImGui::Spacing();
                }
                // End Anchor Preset Picker

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
                if (ImGui::Button("Remove Component##Image"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_UI::Image>>(
                        &coordinator,
                        entity,
                        "Remove Image"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));
                    return true;
                }

                auto& image = coordinator.GetComponent<Uma_UI::Image>(entity);
                ImGui::Indent();

                // Begin tracking
                BeginComponentEdit(entity, coordinator);

                ImGui::Text("Texture Path: %s", image.texturePath.c_str());

                // Create a visible drop zone with visual feedback
                ImVec2 dropZoneSize = ImVec2(ImGui::GetContentRegionAvail().x, 60.0f);
                ImVec2 cursorPos = ImGui::GetCursorScreenPos();

                // Draw a border box
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                ImU32 bgColor = IM_COL32(40, 40, 60, 100);

                // Background
                drawList->AddRectFilled(cursorPos,
                    ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                    bgColor, 4.0f);

                // Center text in the drop zone
                ImVec2 textSize = ImGui::CalcTextSize("Drag & Drop Texture Here");
                ImVec2 textPos = ImVec2(
                    cursorPos.x + (dropZoneSize.x - textSize.x) * 0.5f,
                    cursorPos.y + (dropZoneSize.y - textSize.y) * 0.5f - 10.0f
                );
                drawList->AddText(textPos, IM_COL32(150, 150, 150, 255), "Drag & Drop Texture Here");

                // Supported formats text
                ImVec2 formatTextSize = ImGui::CalcTextSize("(.png, .jpg, .jpeg, .bmp)");
                ImVec2 formatTextPos = ImVec2(
                    cursorPos.x + (dropZoneSize.x - formatTextSize.x) * 0.5f,
                    cursorPos.y + (dropZoneSize.y - formatTextSize.y) * 0.5f + 10.0f
                );
                drawList->AddText(formatTextPos, IM_COL32(100, 100, 100, 255), "(.png, .jpg, .jpeg, .bmp)");

                // Invisible button for the drop zone
                ImGui::SetCursorScreenPos(cursorPos);
                ImGui::InvisibleButton("##TextureDropZone", dropZoneSize);

                bool isHovered = ImGui::IsItemHovered();

                // Drag and Drop Target
                if (ImGui::BeginDragDropTarget())
                {
                    // Highlight the drop zone when dragging over
                    drawList->AddRect(cursorPos,
                        ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                        IM_COL32(100, 200, 255, 255), 4.0f, 0, 3.0f);

                    // Show glow effect
                    drawList->AddRectFilled(cursorPos,
                        ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                        IM_COL32(100, 150, 255, 50), 4.0f);

                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                    {
                        const auto* data = static_cast<const Uma_Engine::FilePayload*>(payload->Data);
                        std::string fullPath = data->filepath;
                        std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

                        std::filesystem::path p(fullPath);
                        std::string ext = p.extension().string();

                        // Convert to lowercase for comparison
                        std::transform(ext.begin(), ext.end(), ext.begin(),
                            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp")
                        {
                            std::string relativePath = fullPath;
                            size_t assetsPos = fullPath.find("Assets/");
                            if (assetsPos != std::string::npos)
                            {
                                relativePath = fullPath.substr(assetsPos);
                            }

                            image.texturePath = relativePath;
                            image.texture = nullptr;
                            m_hasUnsavedEdit = true;
                        }
                        else
                        {
                            m_popupErrorMessage = "Invalid file type for Image!\nExpected: .png, .jpg, .jpeg, .bmp";
                            ImGui::OpenPopup("Invalid File Format");
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
                else if (isHovered)
                {
                    // Subtle hover effect when not dragging
                    drawList->AddRect(cursorPos,
                        ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                        IM_COL32(100, 150, 200, 200), 4.0f, 0, 2.0f);
                }

                // Move cursor past the drop zone
                ImGui::SetCursorScreenPos(ImVec2(cursorPos.x, cursorPos.y + dropZoneSize.y + 5.0f));

                ImGui::Separator();
                ImGui::Text("Sorting Order");
                if (ImGui::InputInt("##Image Sorting Order", &image.sortingOrder, 1, 0, 0)) m_hasUnsavedEdit = true;

                ImGui::Separator();
                ImGui::Text("Color & Visibility");

                float imageColor[4] = { image.color.r, image.color.g, image.color.b, image.color.a };
                if (ImGui::ColorEdit4("Image Color", imageColor))
                {
                    image.color.r = imageColor[0];
                    image.color.g = imageColor[1];
                    image.color.b = imageColor[2];
                    image.color.a = imageColor[3];
                    m_hasUnsavedEdit = true;
                }

                if (ImGui::Checkbox("Image Visible", &image.visible))
                {
                    m_hasUnsavedEdit = true;
                }

                ImGui::Separator();
                ImGui::Text("Fill Settings");

                const char* fillDirectionItems[] = { "None", "Left to Right", "Right to Left", "Top to Bottom", "Bottom to Top" };
                int currentFillDirection = static_cast<int>(image.fillDirection);
                if (ImGui::Combo("Fill Direction", &currentFillDirection, fillDirectionItems, IM_ARRAYSIZE(fillDirectionItems)))
                {
                    image.fillDirection = static_cast<Uma_UI::FillDirection>(currentFillDirection);
                    m_hasUnsavedEdit = true;
                }

                if (image.fillDirection != Uma_UI::FillDirection::None)
                {
                    if (ImGui::SliderFloat("Fill Amount", &image.fillAmount, 0.0f, 1.0f, "%.2f"))
                    {
                        m_hasUnsavedEdit = true;
                    }
                }

                ImGui::Separator();
                ImGui::Text("Sprite Sheet");

                // Grid dimensions
                float gridArray[2] = { image.spriteSheetGrid.x, image.spriteSheetGrid.y };
                if (ImGui::DragFloat2("Grid (Cols x Rows)", gridArray, 1.0f, 1.0f, 100.0f, "%.0f"))
                {
                    image.spriteSheetGrid.x = gridArray[0];
                    image.spriteSheetGrid.y = gridArray[1];
                    // Clamp current cell to new grid bounds
                    image.spriteCell.x = std::min(image.spriteCell.x, image.spriteSheetGrid.x - 1.0f);
                    image.spriteCell.y = std::min(image.spriteCell.y, image.spriteSheetGrid.y - 1.0f);
                    m_hasUnsavedEdit = true;
                }

                // Active cell
                float cellArray[2] = { image.spriteCell.x, image.spriteCell.y };
                if (ImGui::DragFloat2("Cell (Col, Row)", cellArray, 1.0f, 0.0f,
                    std::max(image.spriteSheetGrid.x - 1.0f, 0.0f), "%.0f"))
                {
                    image.spriteCell.x = std::min(cellArray[0], image.spriteSheetGrid.x - 1.0f);
                    image.spriteCell.y = std::min(cellArray[1], image.spriteSheetGrid.y - 1.0f);
                    m_hasUnsavedEdit = true;
                }

                // Flat frame index (convenience alias)
                int frameIndex = image.GetFrame();
                int totalFrames = static_cast<int>(image.spriteSheetGrid.x * image.spriteSheetGrid.y);
                if (ImGui::DragInt("Frame Index", &frameIndex, 1.0f, 0, totalFrames - 1))
                {
                    image.SetFrame(frameIndex);
                    m_hasUnsavedEdit = true;
                }
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Row-major flat index. Synced with Cell (Col, Row) above.");
                }

                // UV offset (fine-tuning within a cell)
                float offsetArray[2] = { image.spriteOffset.x, image.spriteOffset.y };
                if (ImGui::DragFloat2("UV Offset", offsetArray, 0.001f, -1.0f, 1.0f, "%.4f"))
                {
                    image.spriteOffset.x = offsetArray[0];
                    image.spriteOffset.y = offsetArray[1];
                    m_hasUnsavedEdit = true;
                }
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Fine UV offset applied on top of the cell position.");
                }

                // Texture info
                ImGui::Separator();
                if (image.texture)
                {
                    ImGui::Text("Texture ID: %u", image.texture->tex_id);
                    ImGui::Text("Size: %.0f x %.0f",
                        image.texture->GetNativeSize().x,
                        image.texture->GetNativeSize().y);
                }
                else
                {
                    ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Texture not loaded");
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
                if (ImGui::Button("Remove Component##Button"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_UI::Button>>(
                        &coordinator,
                        entity,
                        "Remove Button"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));

                    return true;
                }

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


                static char imageTextureBuffer[256];
                strncpy(imageTextureBuffer, button.scriptName.c_str(), 255);
                imageTextureBuffer[255] = '\0';
                if (ImGui::InputText("Script Name", imageTextureBuffer, 256))
                {
                    button.scriptName = imageTextureBuffer;
                    m_hasUnsavedEdit = true;
                }
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
        else if (type == coordinator.GetComponentType<Uma_UI::Slider>())
        {
            if (ImGui::CollapsingHeader("Slider", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::Button("Remove Component##Slider"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_UI::Slider>>(
                        &coordinator,
                        entity,
                        "Remove Slider"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));
                    return true;
                }

                auto& slider = coordinator.GetComponent<Uma_UI::Slider>(entity);
                ImGui::Indent();

                // Begin tracking
                BeginComponentEdit(entity, coordinator);

                ImGui::SeparatorText("Settings");

                if (ImGui::Checkbox("Interactable", &slider.interactable))
                {
                    m_hasUnsavedEdit = true;
                }
                ImGui::SameLine();
                ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Can the user drag this slider?");
                }

                // Direction
                const char* directions[] = { "Left To Right", "Right To Left", "Bottom To Top", "Top To Bottom" };
                int direction = static_cast<int>(slider.direction);
                if (ImGui::Combo("Direction", &direction, directions, IM_ARRAYSIZE(directions)))
                {
                    slider.direction = static_cast<Uma_UI::SliderDirection>(direction);
                    m_hasUnsavedEdit = true;
                }

                ImGui::Spacing();

                ImGui::SeparatorText("Value");

                if (ImGui::DragFloat("Min Value", &slider.minValue, 1.0f))
                {
                    if (slider.minValue > slider.maxValue) slider.minValue = slider.maxValue;
                    Uma_Engine::Clamp(slider.value, slider.minValue, slider.maxValue);
                    m_hasUnsavedEdit = true;
                }

                if (ImGui::DragFloat("Max Value", &slider.maxValue, 1.0f))
                {
                    if (slider.maxValue < slider.minValue) slider.maxValue = slider.minValue;
                    Uma_Engine::Clamp(slider.value, slider.minValue, slider.maxValue);
                    m_hasUnsavedEdit = true;
                }

                if (ImGui::DragFloat("Current Value", &slider.value, 1.0f, slider.minValue, slider.maxValue))
                {
                    Uma_Engine::Clamp(slider.value, slider.minValue, slider.maxValue);
                    m_hasUnsavedEdit = true;
                }

                if (ImGui::Checkbox("Whole Numbers", &slider.wholeNumbers))
                {
                    Uma_Engine::Clamp(slider.value, slider.minValue, slider.maxValue);
                    if (slider.wholeNumbers)
                    {
                        slider.value = std::round(slider.value);
                    }
                    m_hasUnsavedEdit = true;
                }
                ImGui::SameLine();
                ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Round to integers (for discrete values)");
                }

                float normalizedValue = (slider.maxValue - slider.minValue == 0.f) ? 0.f : (slider.value - slider.minValue) / (slider.maxValue - slider.minValue);
                ImGui::Text("Normalized: %.2f (%.0f%%)", normalizedValue, normalizedValue * 100.0f);

                ImGui::Spacing();

                ImGui::SeparatorText("Visual References");

                ImGui::TextWrapped("These are the child entities that make up the slider visuals:");
                ImGui::Spacing();

                // Background Entity
                ImGui::Text("Background:");
                ImGui::SameLine(120);

                int bgEntityID = static_cast<int>(slider.background);
                ImGui::SetNextItemWidth(80);
                if (ImGui::InputInt("##Background Entity", &bgEntityID))
                {
                    if (bgEntityID < 0)
                        slider.background = static_cast<Uma_ECS::Entity>(-1);
                    else
                        slider.background = static_cast<Uma_ECS::Entity>(bgEntityID);
                    std::cout << "Set to: " << slider.background << std::endl;
                    m_hasUnsavedEdit = true;
                }

                ImGui::Text("Current: %d", static_cast<int>(slider.background));

                ImGui::SameLine();
                if (slider.background != static_cast<Uma_ECS::Entity>(-1))
                {
                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Set");

                    // Check if entity exists and has Image component
                    if (coordinator.HasActiveEntity(slider.background))
                    {
                        auto& imageArray = coordinator.GetComponentArray<Uma_UI::Image>();
                        if (!imageArray.Has(slider.background))
                        {
                            ImGui::SameLine();
                            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No Image");
                        }
                    }
                    else
                    {
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Invalid");
                    }

                    ImGui::SameLine();
                    if (ImGui::SmallButton("Clear##bg"))
                    {
                        slider.background = static_cast<Uma_ECS::Entity>(-1);
                        m_hasUnsavedEdit = true;
                    }
                }
                else
                {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "None");
                }

                // Fill Entity
                ImGui::Text("Fill:");
                ImGui::SameLine(120);

                int fillEntityID = static_cast<int>(slider.fill);
                ImGui::SetNextItemWidth(80);
                if (ImGui::InputInt("##Fill Entity", &fillEntityID))
                {
                    if (fillEntityID < 0)
                        slider.fill = static_cast<Uma_ECS::Entity>(-1);
                    else
                        slider.fill = static_cast<Uma_ECS::Entity>(fillEntityID);
                    m_hasUnsavedEdit = true;
                }

                ImGui::SameLine();
                if (slider.fill != static_cast<Uma_ECS::Entity>(-1))
                {
                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Set");

                    if (coordinator.HasActiveEntity(slider.fill))
                    {
                        auto& rectTransformArray = coordinator.GetComponentArray<Uma_UI::RectTransform>();
                        if (!rectTransformArray.Has(slider.fill))
                        {
                            ImGui::SameLine();
                            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No RectTransform");
                        }
                    }
                    else
                    {
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Invalid");
                    }

                    ImGui::SameLine();
                    if (ImGui::SmallButton("Clear##fill"))
                    {
                        slider.fill = static_cast<Uma_ECS::Entity>(-1);
                        m_hasUnsavedEdit = true;
                    }
                }
                else
                {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "None");
                }

                // Handle Entity (Required)
                ImGui::Text("Handle:");
                ImGui::SameLine(120);

                int handleEntityID = static_cast<int>(slider.handle);
                ImGui::SetNextItemWidth(80);
                if (ImGui::InputInt("##Handle Entity", &handleEntityID))
                {
                    if (handleEntityID < 0)
                        slider.handle = static_cast<Uma_ECS::Entity>(-1);
                    else
                        slider.handle = static_cast<Uma_ECS::Entity>(handleEntityID);
                    m_hasUnsavedEdit = true;
                }

                ImGui::SameLine();
                if (slider.handle != static_cast<Uma_ECS::Entity>(-1))
                {
                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Set");

                    if (coordinator.HasActiveEntity(slider.handle))
                    {
                        auto& rectTransformArray = coordinator.GetComponentArray<Uma_UI::RectTransform>();
                        auto& imageArray = coordinator.GetComponentArray<Uma_UI::Image>();

                        if (!rectTransformArray.Has(slider.handle))
                        {
                            ImGui::SameLine();
                            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No RectTransform");
                        }
                        if (!imageArray.Has(slider.handle))
                        {
                            ImGui::SameLine();
                            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No Image");
                        }
                    }
                    else
                    {
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Invalid");
                    }

                    ImGui::SameLine();
                    if (ImGui::SmallButton("Clear##handle"))
                    {
                        slider.handle = static_cast<Uma_ECS::Entity>(-1);
                        m_hasUnsavedEdit = true;
                    }
                }
                else
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "None (Required!)");
                }

                ImGui::Spacing();
                ImGui::Separator();

                // Helper buttons
                if (ImGui::Button("Auto-Assign from Children", ImVec2(-1, 0)))
                {
                    auto& transformArray = coordinator.GetComponentArray<Uma_ECS::Transform>();
                    if (transformArray.Has(entity))
                    {
                        auto& transform = transformArray.GetData(entity);

                        if (transform.children.size() >= 1)
                            slider.background = transform.children[0];
                        if (transform.children.size() >= 2)
                            slider.fill = transform.children[1];
                        if (transform.children.size() >= 3)
                            slider.handle = transform.children[2]; // Handle is most important

                        m_hasUnsavedEdit = true;
                    }
                }
                ImGui::SameLine();
                ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Assigns first children as:\n"
                        "1st child ? Background\n"
                        "2nd child ? Fill\n"
                        "3rd child ? Handle (required)");
                }

                if (ImGui::Button("Clear All References", ImVec2(-1, 0)))
                {
                    slider.background = static_cast<Uma_ECS::Entity>(-1);
                    slider.fill = static_cast<Uma_ECS::Entity>(-1);
                    slider.handle = static_cast<Uma_ECS::Entity>(-1);
                    m_hasUnsavedEdit = true;
                }

                ImGui::Spacing();

                ImGui::SeparatorText("Colors");

                float normalColor[4] = {
                    slider.normalColour.r, slider.normalColour.g,
                    slider.normalColour.b, slider.normalColour.a
                };
                if (ImGui::ColorEdit4("Normal Color", normalColor))
                {
                    slider.normalColour.r = normalColor[0];
                    slider.normalColour.g = normalColor[1];
                    slider.normalColour.b = normalColor[2];
                    slider.normalColour.a = normalColor[3];
                    m_hasUnsavedEdit = true;
                }

                float highlightColor[4] = {
                    slider.highlightColour.r, slider.highlightColour.g,
                    slider.highlightColour.b, slider.highlightColour.a
                };
                if (ImGui::ColorEdit4("Highlight Color", highlightColor))
                {
                    slider.highlightColour.r = highlightColor[0];
                    slider.highlightColour.g = highlightColor[1];
                    slider.highlightColour.b = highlightColor[2];
                    slider.highlightColour.a = highlightColor[3];
                    m_hasUnsavedEdit = true;
                }

                float disabledColor[4] = {
                    slider.disabledColour.r, slider.disabledColour.g,
                    slider.disabledColour.b, slider.disabledColour.a
                };
                if (ImGui::ColorEdit4("Disabled Color", disabledColor))
                {
                    slider.disabledColour.r = disabledColor[0];
                    slider.disabledColour.g = disabledColor[1];
                    slider.disabledColour.b = disabledColor[2];
                    slider.disabledColour.a = disabledColor[3];
                    m_hasUnsavedEdit = true;
                }

                ImGui::Spacing();

                ImGui::SeparatorText("Callback");

                static char scriptBuffer[256];
                strncpy(scriptBuffer, slider.scriptName.c_str(), 255);
                scriptBuffer[255] = '\0';
                if (ImGui::InputText("Script Name", scriptBuffer, 256))
                {
                    slider.scriptName = scriptBuffer;
                    m_hasUnsavedEdit = true;
                }
                ImGui::SameLine();
                ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Lua script with OnDrag function");
                }

                ImGui::Spacing();

                ImGui::SeparatorText("Runtime State");

                ImGui::BeginDisabled();

                ImGui::Text("Is Dragging:");
                ImGui::SameLine();
                ImGui::TextColored(
                    slider.isDragging ? ImVec4(0.2f, 1.0f, 0.2f, 1.0f) : ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
                    "%s", slider.isDragging ? "Yes" : "No"
                );

                ImGui::Text("Is Hovered:");
                ImGui::SameLine();
                ImGui::TextColored(
                    slider.isHovered ? ImVec4(0.2f, 1.0f, 0.2f, 1.0f) : ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
                    "%s", slider.isHovered ? "Yes" : "No"
                );

                ImGui::Text("Current Color:");
                ImGui::SameLine();
                Uma_UI::Color currentColour = (!slider.interactable) ? slider.disabledColour : (slider.isDragging || slider.isHovered) ? slider.highlightColour : slider.normalColour;
                ImGui::ColorButton("##currentcolor",
                    ImVec4(currentColour.r, currentColour.g, currentColour.b, currentColour.a),
                    ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoPicker);

                ImGui::EndDisabled();

                ImGui::Spacing();

                ImGui::SeparatorText("Quick Actions");

                if (ImGui::Button("Reset to Min", ImVec2(-1, 0)))
                {
                    slider.value = slider.minValue;
                    m_hasUnsavedEdit = true;
                }

                if (ImGui::Button("Reset to Max", ImVec2(-1, 0)))
                {
                    slider.value = slider.maxValue;
                    m_hasUnsavedEdit = true;
                }

                // End tracking
                EndComponentEdit(entity, coordinator, "Slider");

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_UI::Checkbox>())
        {
            if (ImGui::CollapsingHeader("Checkbox", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::Button("Remove Component##Checkbox"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_UI::Checkbox>>(
                        &coordinator,
                        entity,
                        "Remove Checkbox"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));

                    return true;
                }

                auto& checkbox = coordinator.GetComponent<Uma_UI::Checkbox>(entity);
                ImGui::Indent();

                // Begin tracking
                BeginComponentEdit(entity, coordinator);

                if (ImGui::Checkbox("Interactable", &checkbox.interactable))
                {
                    m_hasUnsavedEdit = true;
                }

                ImGui::Spacing();
                ImGui::Text("Current State: %s",
                    checkbox.currentState == Uma_UI::CheckboxState::Normal ? "Normal" :
                    checkbox.currentState == Uma_UI::CheckboxState::Hovered ? "Hovered" :
                    checkbox.currentState == Uma_UI::CheckboxState::Pressed ? "Pressed" : "Disabled");


                // Checkmark Entity (Required)
                ImGui::Text("Checkmark:");
                ImGui::SameLine(120);

                int checkmarkEntityID = static_cast<int>(checkbox.checkmark);
                ImGui::SetNextItemWidth(80);
                if (ImGui::InputInt("##Checkmark Entity", &checkmarkEntityID))
                {
                    if (checkmarkEntityID < 0)
                        checkbox.checkmark = static_cast<Uma_ECS::Entity>(-1);
                    else
                        checkbox.checkmark = static_cast<Uma_ECS::Entity>(checkmarkEntityID);
                    m_hasUnsavedEdit = true;
                }

                ImGui::SameLine();
                if (checkbox.checkmark != static_cast<Uma_ECS::Entity>(-1))
                {
                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Set");

                    if (coordinator.HasActiveEntity(checkbox.checkmark))
                    {
                        auto& rectTransformArray = coordinator.GetComponentArray<Uma_UI::RectTransform>();
                        auto& imageArray = coordinator.GetComponentArray<Uma_UI::Image>();

                        if (!rectTransformArray.Has(checkbox.checkmark))
                        {
                            ImGui::SameLine();
                            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No RectTransform");
                        }
                        if (!imageArray.Has(checkbox.checkmark))
                        {
                            ImGui::SameLine();
                            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No Image");
                        }
                    }
                    else
                    {
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Invalid");
                    }

                    ImGui::SameLine();
                    if (ImGui::SmallButton("Clear##checkmark"))
                    {
                        checkbox.checkmark = static_cast<Uma_ECS::Entity>(-1);
                        m_hasUnsavedEdit = true;
                    }
                }
                else
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "None (Required!)");
                }

                ImGui::Spacing();

                // Background Entity
                ImGui::Text("Background:");
                ImGui::SameLine(120);

                int bgEntityID = static_cast<int>(checkbox.background);
                ImGui::SetNextItemWidth(80);
                if (ImGui::InputInt("##Background Entity", &bgEntityID))
                {
                    if (bgEntityID < 0)
                        checkbox.background = static_cast<Uma_ECS::Entity>(-1);
                    else
                        checkbox.background = static_cast<Uma_ECS::Entity>(bgEntityID);
                    m_hasUnsavedEdit = true;
                }

                ImGui::SameLine();
                if (checkbox.background != static_cast<Uma_ECS::Entity>(-1))
                {
                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Set");

                    if (coordinator.HasActiveEntity(checkbox.background))
                    {
                        auto& rectTransformArray = coordinator.GetComponentArray<Uma_UI::RectTransform>();
                        auto& imageArray = coordinator.GetComponentArray<Uma_UI::Image>();

                        if (!rectTransformArray.Has(checkbox.background))
                        {
                            ImGui::SameLine();
                            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No RectTransform");
                        }
                        if (!imageArray.Has(checkbox.background))
                        {
                            ImGui::SameLine();
                            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No Image");
                        }
                    }
                    else
                    {
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Invalid");
                    }

                    ImGui::SameLine();
                    if (ImGui::SmallButton("Clear##bg"))
                    {
                        checkbox.background = static_cast<Uma_ECS::Entity>(-1);
                        m_hasUnsavedEdit = true;
                    }
                }
                else
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "None");
                }

                ImGui::Spacing();
                ImGui::Separator();

                static char buffer[256];
                strncpy(buffer, checkbox.scriptName.c_str(), 255);
                buffer[255] = '\0';
                if (ImGui::InputText("Script Name", buffer, 256))
                {
                    checkbox.scriptName = buffer;
                    m_hasUnsavedEdit = true;
                }
                ImGui::Separator();

                ImGui::Text("Checkbox Colors");

                float normalColour[4] = { checkbox.normalColour.r, checkbox.normalColour.g, checkbox.normalColour.b, checkbox.normalColour.a };
                if (ImGui::ColorEdit4("Normal Colour", normalColour))
                {
                    checkbox.normalColour.r = normalColour[0];
                    checkbox.normalColour.g = normalColour[1];
                    checkbox.normalColour.b = normalColour[2];
                    checkbox.normalColour.a = normalColour[3];
                    m_hasUnsavedEdit = true;
                }

                float hoverColour[4] = { checkbox.hoverColour.r, checkbox.hoverColour.g, checkbox.hoverColour.b, checkbox.hoverColour.a };
                if (ImGui::ColorEdit4("Hover Colour", hoverColour))
                {
                    checkbox.hoverColour.r = hoverColour[0];
                    checkbox.hoverColour.g = hoverColour[1];
                    checkbox.hoverColour.b = hoverColour[2];
                    checkbox.hoverColour.a = hoverColour[3];
                    m_hasUnsavedEdit = true;
                }

                float pressedColour[4] = { checkbox.pressedColour.r, checkbox.pressedColour.g, checkbox.pressedColour.b, checkbox.pressedColour.a };
                if (ImGui::ColorEdit4("Pressed Colour", pressedColour))
                {
                    checkbox.pressedColour.r = pressedColour[0];
                    checkbox.pressedColour.g = pressedColour[1];
                    checkbox.pressedColour.b = pressedColour[2];
                    checkbox.pressedColour.a = pressedColour[3];
                    m_hasUnsavedEdit = true;
                }

                float disabledColour[4] = { checkbox.disabledColour.r, checkbox.disabledColour.g, checkbox.disabledColour.b, checkbox.disabledColour.a };
                if (ImGui::ColorEdit4("Disabled Colour", disabledColour))
                {
                    checkbox.disabledColour.r = disabledColour[0];
                    checkbox.disabledColour.g = disabledColour[1];
                    checkbox.disabledColour.b = disabledColour[2];
                    checkbox.disabledColour.a = disabledColour[3];
                    m_hasUnsavedEdit = true;
                }

                float checkedColour[4] = { checkbox.checkedColour.r, checkbox.checkedColour.g, checkbox.checkedColour.b, checkbox.checkedColour.a };
                if (ImGui::ColorEdit4("Checked Colour", checkedColour))
                {
                    checkbox.checkedColour.r = checkedColour[0];
                    checkbox.checkedColour.g = checkedColour[1];
                    checkbox.checkedColour.b = checkedColour[2];
                    checkbox.checkedColour.a = checkedColour[3];
                    m_hasUnsavedEdit = true;
                }

                ImGui::Separator();

                ImGui::Text("Checkmark Colors");
                float checkmarkNormalColour[4] = { checkbox.checkmarkNormalColour.r, checkbox.checkmarkNormalColour.g, checkbox.checkmarkNormalColour.b, checkbox.checkmarkNormalColour.a };
                if (ImGui::ColorEdit4("Checkmark Colour", checkmarkNormalColour))
                {
                    checkbox.checkmarkNormalColour.r = checkmarkNormalColour[0];
                    checkbox.checkmarkNormalColour.g = checkmarkNormalColour[1];
                    checkbox.checkmarkNormalColour.b = checkmarkNormalColour[2];
                    checkbox.checkmarkNormalColour.a = checkmarkNormalColour[3];
                    m_hasUnsavedEdit = true;
                }
                float checkmarkDisabledColour[4] = { checkbox.checkmarkDisabledColour.r, checkbox.checkmarkDisabledColour.g, checkbox.checkmarkDisabledColour.b, checkbox.checkmarkDisabledColour.a };
                if (ImGui::ColorEdit4("Checkmark Disabled Colour", checkmarkDisabledColour))
                {
                    checkbox.checkmarkDisabledColour.r = checkmarkDisabledColour[0];
                    checkbox.checkmarkDisabledColour.g = checkmarkDisabledColour[1];
                    checkbox.checkmarkDisabledColour.b = checkmarkDisabledColour[2];
                    checkbox.checkmarkDisabledColour.a = checkmarkDisabledColour[3];
                    m_hasUnsavedEdit = true;
                }

                // End tracking
                EndComponentEdit(entity, coordinator, "Checkbox");

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_UI::Canvas>())
        {
            if (ImGui::CollapsingHeader("Canvas", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::Button("Remove Component##Canvas"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_UI::Canvas>>(
                        &coordinator,
                        entity,
                        "Remove Canvas"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));

                    return true;
                }

                auto& canvas = coordinator.GetComponent<Uma_UI::Canvas>(entity);
                ImGui::Indent();

                // Begin tracking
                BeginComponentEdit(entity, coordinator);

                if (ImGui::DragInt("Sorting Order", &canvas.sortingOrder, 1.0f, -100, 100))
                {
                    if (coordinator.HasComponent<Uma_UI::RectTransform>(entity))
                    {
                        coordinator.GetComponent<Uma_UI::RectTransform>(entity).isDirty;
                    }
                    m_hasUnsavedEdit = true;
                }

                ImGui::Separator();
                ImGui::Text("Reference Resolution");

                float refResolution[2] = { canvas.referenceResolution.x, canvas.referenceResolution.y };
                if (ImGui::DragFloat2("Resolution", refResolution, 1.0f, 1.0f, 10000.0f, "%.0f"))
                {
                    canvas.referenceResolution = Vec2(refResolution[0], refResolution[1]);
                    if (coordinator.HasComponent<Uma_UI::RectTransform>(entity))
                    {
                        coordinator.GetComponent<Uma_UI::RectTransform>(entity).isDirty;
                    }
                    m_hasUnsavedEdit = true;
                }

                ImGui::Separator();
                ImGui::Text("Scale Mode");

                const char* scaleModes[] = { "Constant Pixel Size", "Scale With Screen Size", "Constant Physical Size" };
                int currentScaleMode = static_cast<int>(canvas.scaleMode);
                if (ImGui::Combo("Scale Mode", &currentScaleMode, scaleModes, IM_ARRAYSIZE(scaleModes)))
                {
                    canvas.scaleMode = static_cast<Uma_UI::CanvasScaleMode>(currentScaleMode);
                    if (coordinator.HasComponent<Uma_UI::RectTransform>(entity))
                    {
                        coordinator.GetComponent<Uma_UI::RectTransform>(entity).isDirty;
                    }
                    m_hasUnsavedEdit = true;
                }

                if (canvas.scaleMode == Uma_UI::CanvasScaleMode::ScaleWithScreenSize)
                {
                    if (ImGui::SliderFloat("Match Width/Height", &canvas.matchWidthOrHeight, 0.0f, 1.0f, "%.2f"))
                    {
                        if (coordinator.HasComponent<Uma_UI::RectTransform>(entity))
                        {
                            coordinator.GetComponent<Uma_UI::RectTransform>(entity).isDirty;
                        }
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
                if (ImGui::Button("Remove Component##Text"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_UI::Text>>(
                        &coordinator,
                        entity,
                        "Remove Text"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));

                    return true;
                }

                auto& text = coordinator.GetComponent<Uma_UI::Text>(entity);

                ImGui::Indent();

                // Begin tracking
                BeginComponentEdit(entity, coordinator);

                static char textContentBuffer[1024];
                strncpy(textContentBuffer, text.text.c_str(), 1023);
                textContentBuffer[1023] = '\0';
                ImGui::Text("Text Content");
                if (ImGui::InputTextMultiline("##Text Content", textContentBuffer, 1024, ImVec2(-1, 80)))
                {
                    text.text = textContentBuffer;
                    m_hasUnsavedEdit = true;
                }

                ImGui::Separator();
                ImGui::Text("Sorting Order");
                if (ImGui::InputInt("##Text Sorting Order", &text.sortingOrder, 1, 0, 0)) m_hasUnsavedEdit = true;

                ImGui::Separator();
                ImGui::Text("Font Settings");

                ImGui::Separator();
                ImGui::Text("Font Settings");

                ImGui::Text("Font Path: %s", text.fontPath.empty() ? "(None)" : text.fontPath.c_str());

                // Create a visible drop zone with visual feedback
                ImVec2 dropZoneSize = ImVec2(ImGui::GetContentRegionAvail().x, 60.0f);
                ImVec2 cursorPos = ImGui::GetCursorScreenPos();

                // Draw a border box
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                ImU32 bgColor = IM_COL32(40, 40, 60, 100);

                // Background
                drawList->AddRectFilled(cursorPos,
                    ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                    bgColor, 4.0f);

                // Center text in the drop zone
                ImVec2 textSize = ImGui::CalcTextSize("Drag & Drop Font Here");
                ImVec2 textPos = ImVec2(
                    cursorPos.x + (dropZoneSize.x - textSize.x) * 0.5f,
                    cursorPos.y + (dropZoneSize.y - textSize.y) * 0.5f - 10.0f
                );
                drawList->AddText(textPos, IM_COL32(150, 150, 150, 255), "Drag & Drop Font Here");

                // Supported formats text
                ImVec2 formatTextSize = ImGui::CalcTextSize("(.ttf, .otf)");
                ImVec2 formatTextPos = ImVec2(
                    cursorPos.x + (dropZoneSize.x - formatTextSize.x) * 0.5f,
                    cursorPos.y + (dropZoneSize.y - formatTextSize.y) * 0.5f + 10.0f
                );
                drawList->AddText(formatTextPos, IM_COL32(100, 100, 100, 255), "(.ttf, .otf)");

                // Invisible button for the drop zone
                ImGui::SetCursorScreenPos(cursorPos);
                ImGui::InvisibleButton("##FontDropZone", dropZoneSize);

                bool isHovered = ImGui::IsItemHovered();

                // Drag and Drop for Fonts
                if (ImGui::BeginDragDropTarget())
                {
                    // Highlight the drop zone when dragging over
                    drawList->AddRect(cursorPos,
                        ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                        IM_COL32(100, 200, 255, 255), 4.0f, 0, 3.0f);

                    // Show glow effect
                    drawList->AddRectFilled(cursorPos,
                        ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                        IM_COL32(100, 150, 255, 50), 4.0f);

                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                    {
                        const auto* data = static_cast<const Uma_Engine::FilePayload*>(payload->Data);
                        std::string fullPath = data->filepath;
                        std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

                        std::filesystem::path p(fullPath);
                        std::string ext = p.extension().string();

                        // Convert to lowercase for comparison
                        std::transform(ext.begin(), ext.end(), ext.begin(),
                            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                        if (ext == ".ttf" || ext == ".otf")
                        {
                            std::string relativePath = fullPath;
                            size_t assetsPos = fullPath.find("Assets/");
                            if (assetsPos != std::string::npos)
                            {
                                relativePath = fullPath.substr(assetsPos);
                            }

                            text.fontPath = relativePath;
                            m_hasUnsavedEdit = true;
                        }
                        else
                        {
                            m_popupErrorMessage = "Invalid file type for Font!\nExpected: .ttf, .otf";
                            ImGui::OpenPopup("Invalid File Format");
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
                else if (isHovered)
                {
                    // Subtle hover effect when not dragging
                    drawList->AddRect(cursorPos,
                        ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                        IM_COL32(100, 150, 200, 200), 4.0f, 0, 2.0f);
                }

                // Move cursor past the drop zone
                ImGui::SetCursorScreenPos(ImVec2(cursorPos.x, cursorPos.y + dropZoneSize.y + 5.0f));

                if (ImGui::DragFloat("Font Size", &text.fontSize, 1.0f, 1.0f, 200.0f, "%.1f"))
                {
                    m_hasUnsavedEdit = true;
                }

                ImGui::Separator();
                ImGui::Text("Appearance");

                float textColor[4] = { text.color.r, text.color.g, text.color.b, text.color.a };
                if (ImGui::ColorEdit4("Text Color", textColor))
                {
                    text.color.r = textColor[0];
                    text.color.g = textColor[1];
                    text.color.b = textColor[2];
                    text.color.a = textColor[3];
                    m_hasUnsavedEdit = true;
                }

                const char* alignments[] = { "Left", "Center", "Right" };
                int currentAlignment = static_cast<int>(text.alignment);
                if (ImGui::Combo("Alignment", &currentAlignment, alignments, IM_ARRAYSIZE(alignments)))
                {
                    text.alignment = static_cast<Uma_UI::TextAlignment>(currentAlignment);
                    m_hasUnsavedEdit = true;
                }

                if (ImGui::Checkbox("Text Visible", &text.visible))
                {
                    m_hasUnsavedEdit = true;
                }

                // End tracking
                EndComponentEdit(entity, coordinator, "Text");

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_UI::Effects>())
        {
            if (ImGui::CollapsingHeader("Effects", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::Button("Remove Component##Effects"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_UI::Effects>>(
                        &coordinator,
                        entity,
                        "Remove Effects"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));
                    return true;
                }

                auto& effects = coordinator.GetComponent<Uma_UI::Effects>(entity);
                ImGui::Indent();

                // Begin tracking
                BeginComponentEdit(entity, coordinator);

                // Play on Enable
                if (ImGui::Checkbox("Play On Enable", &effects.playOnEnable))
                {
                    m_hasUnsavedEdit = true;
                }

                ImGui::Separator();

                // Effect Clips List
                ImGui::Text("Effect Clips (%zu)", effects.clips.size());

                // Add new clip button
                if (ImGui::Button("Add Effect Clip"))
                {
                    Uma_UI::EffectClip newClip;
                    effects.clips.push_back(newClip);
                    m_hasUnsavedEdit = true;
                }

                ImGui::SameLine();

                // Breathing loop preset button
                if (ImGui::Button("Add Breathing Loop"))
                {
                    // Create first clip: scale up (normal scale -> larger)
                    Uma_UI::EffectClip breathingUp;
                    breathingUp.property = Uma_UI::EffectProperty::Scale;
                    breathingUp.easing = Uma_UI::EasingType::EaseInOutQuad;
                    breathingUp.duration = 1.0f;
                    breathingUp.delay = 0.0f;
                    breathingUp.loop = true;
                    breathingUp.startVec2 = Vec2(1.0f, 1.0f);
                    breathingUp.endVec2 = Vec2(1.1f, 1.1f);

                    // Create second clip: scale down (larger -> normal scale)
                    Uma_UI::EffectClip breathingDown;
                    breathingDown.property = Uma_UI::EffectProperty::Scale;
                    breathingDown.easing = Uma_UI::EasingType::EaseInOutQuad;
                    breathingDown.duration = 1.0f;
                    breathingDown.delay = 1.0f;  // Start after first clip
                    breathingDown.loop = true;
                    breathingDown.startVec2 = Vec2(1.1f, 1.1f);
                    breathingDown.endVec2 = Vec2(1.0f, 1.0f);

                    effects.clips.push_back(breathingUp);
                    effects.clips.push_back(breathingDown);
                    m_hasUnsavedEdit = true;
                }

                // More preset buttons
                ImGui::SameLine();
                if (ImGui::Button("Fade In"))
                {
                    Uma_UI::EffectClip fadeIn;
                    fadeIn.property = Uma_UI::EffectProperty::Alpha;
                    fadeIn.easing = Uma_UI::EasingType::EaseOutQuad;
                    fadeIn.duration = 0.5f;
                    fadeIn.delay = 0.0f;
                    fadeIn.loop = false;
                    fadeIn.startFloat = 0.0f;
                    fadeIn.endFloat = 1.0f;
                    effects.clips.push_back(fadeIn);
                    m_hasUnsavedEdit = true;
                }

                ImGui::SameLine();
                if (ImGui::Button("Pulse"))
                {
                    Uma_UI::EffectClip pulse;
                    pulse.property = Uma_UI::EffectProperty::Scale;
                    pulse.easing = Uma_UI::EasingType::EaseOutElastic;
                    pulse.duration = 0.6f;
                    pulse.delay = 0.0f;
                    pulse.loop = false;
                    pulse.startVec2 = Vec2(0.5f, 0.5f);
                    pulse.endVec2 = Vec2(1.0f, 1.0f);
                    effects.clips.push_back(pulse);
                    m_hasUnsavedEdit = true;
                }

                ImGui::SameLine();
                if (ImGui::Button("Bounce In"))
                {
                    Uma_UI::EffectClip bounceIn;
                    bounceIn.property = Uma_UI::EffectProperty::Scale;
                    bounceIn.easing = Uma_UI::EasingType::EaseOutBounce;
                    bounceIn.duration = 0.8f;
                    bounceIn.delay = 0.0f;
                    bounceIn.loop = false;
                    bounceIn.startVec2 = Vec2(0.0f, 0.0f);
                    bounceIn.endVec2 = Vec2(1.0f, 1.0f);
                    effects.clips.push_back(bounceIn);
                    m_hasUnsavedEdit = true;
                }

                ImGui::SameLine();
                if (ImGui::Button("Fill Bar"))
                {
                    Uma_UI::EffectClip fillBar;
                    fillBar.name = "FillBar";
                    fillBar.property = Uma_UI::EffectProperty::FillAmount;
                    fillBar.easing = Uma_UI::EasingType::EaseOutQuad;
                    fillBar.duration = 1.0f;
                    fillBar.delay = 0.0f;
                    fillBar.loop = false;
                    fillBar.startFloat = 0.0f;
                    fillBar.endFloat = 1.0f;
                    effects.clips.push_back(fillBar);
                    m_hasUnsavedEdit = true;
                }

                ImGui::SameLine();
                if (ImGui::Button("Sprite Anim"))
                {
                    Uma_UI::EffectClip spriteAnim;
                    spriteAnim.name = "SpriteAnim";
                    spriteAnim.property = Uma_UI::EffectProperty::SpritesheetFrame;
                    spriteAnim.easing = Uma_UI::EasingType::Linear;
                    spriteAnim.duration = 1.0f;
                    spriteAnim.delay = 0.0f;
                    spriteAnim.loop = true;
                    spriteAnim.startFrame = 0;
                    spriteAnim.endFrame = 7;
                    spriteAnim.fps = 12.0f;
                    spriteAnim.pingPong = false;
                    effects.clips.push_back(spriteAnim);
                    m_hasUnsavedEdit = true;
                }

                ImGui::Separator();

                // Display each clip
                for (size_t i = 0; i < effects.clips.size(); ++i)
                {
                    ImGui::PushID(static_cast<int>(i));
                    auto& clip = effects.clips[i];

                    std::string headerLabel = "Clip " + std::to_string(i);
                    if (!clip.name.empty())
                    {
                        headerLabel += " (" + clip.name + ")";
                    }

                    headerLabel += "###Clip";

                    if (ImGui::TreeNode(headerLabel.c_str()))
                    {
                        // Clip name field
                        static char nameBuffer[64];
                        strncpy(nameBuffer, clip.name.c_str(), 63);
                        nameBuffer[63] = '\0';
                        if (ImGui::InputText("Name", nameBuffer, 64))
                        {
                            clip.name = nameBuffer;
                            m_hasUnsavedEdit = true;
                        }
                        if (ImGui::IsItemHovered())
                        {
                            ImGui::SetTooltip("Optional name for referencing this clip from code");
                        }

                        ImGui::Separator();

                        // Property Type
                        const char* properties[] = { "Position", "Scale", "ColorTint", "Alpha", "Fill Amount", "Spritesheet Frame" };
                        int currentProperty = static_cast<int>(clip.property);
                        if (ImGui::Combo("Property", &currentProperty, properties, IM_ARRAYSIZE(properties)))
                        {
                            clip.property = static_cast<Uma_UI::EffectProperty>(currentProperty);
                            m_hasUnsavedEdit = true;
                        }

                        // Easing Type
                        const char* easingTypes[] = {
                            "Linear", "EaseInQuad", "EaseOutQuad", "EaseInOutQuad",
                            "EaseInCubic", "EaseOutCubic", "EaseInOutCubic",
                            "EaseInQuart", "EaseOutQuart", "EaseInOutQuart",
                            "EaseInElastic", "EaseOutElastic", "EaseInOutElastic",
                            "EaseInBounce", "EaseOutBounce", "EaseInOutBounce"
                        };
                        int currentEasing = static_cast<int>(clip.easing);
                        if (ImGui::Combo("Easing", &currentEasing, easingTypes, IM_ARRAYSIZE(easingTypes)))
                        {
                            clip.easing = static_cast<Uma_UI::EasingType>(currentEasing);
                            m_hasUnsavedEdit = true;
                        }

                        // Timing
                        if (ImGui::DragFloat("Duration", &clip.duration, 0.01f, 0.0f, 100.0f))
                        {
                            m_hasUnsavedEdit = true;
                        }

                        if (ImGui::DragFloat("Delay", &clip.delay, 0.01f, 0.0f, 100.0f))
                        {
                            m_hasUnsavedEdit = true;
                        }

                        if (ImGui::Checkbox("Loop", &clip.loop))
                        {
                            m_hasUnsavedEdit = true;
                        }

                        if (ImGui::Checkbox("Apply to Children", &clip.applyToChildren))
                        {
                            m_hasUnsavedEdit = true;
                        }
                        if (ImGui::IsItemHovered())
                        {
                            ImGui::SetTooltip("Recursively apply this effect to all child entities.");
                        }

                        ImGui::Separator();

                        // Property-specific values
                        switch (clip.property)
                        {
                        case Uma_UI::EffectProperty::Position:
                        {
                            float startVec[2] = { clip.startVec2.x, clip.startVec2.y };
                            if (ImGui::DragFloat2("Start", startVec, 0.1f))
                            {
                                clip.startVec2.x = startVec[0];
                                clip.startVec2.y = startVec[1];
                                m_hasUnsavedEdit = true;
                            }

                            float endVec[2] = { clip.endVec2.x, clip.endVec2.y };
                            if (ImGui::DragFloat2("End", endVec, 0.1f))
                            {
                                clip.endVec2.x = endVec[0];
                                clip.endVec2.y = endVec[1];
                                m_hasUnsavedEdit = true;
                            }
                            break;
                        }
                        case Uma_UI::EffectProperty::Scale:
                        {
                            // Display as percentage (multiply by 100)
                            float startPercent[2] = { clip.startVec2.x * 100.0f, clip.startVec2.y * 100.0f };
                            if (ImGui::DragFloat2("Start (%)", startPercent, 1.0f, 0.0f, 500.0f))
                            {
                                clip.startVec2.x = startPercent[0] / 100.0f;
                                clip.startVec2.y = startPercent[1] / 100.0f;
                                m_hasUnsavedEdit = true;
                            }

                            float endPercent[2] = { clip.endVec2.x * 100.0f, clip.endVec2.y * 100.0f };
                            if (ImGui::DragFloat2("End (%)", endPercent, 1.0f, 0.0f, 500.0f))
                            {
                                clip.endVec2.x = endPercent[0] / 100.0f;
                                clip.endVec2.y = endPercent[1] / 100.0f;
                                m_hasUnsavedEdit = true;
                            }

                            break;
                        }
                        case Uma_UI::EffectProperty::ColorTint:
                        {
                            float startColor[4] = { clip.startColor.r, clip.startColor.g, clip.startColor.b, clip.startColor.a };
                            if (ImGui::ColorEdit4("Start Color", startColor))
                            {
                                clip.startColor.r = startColor[0];
                                clip.startColor.g = startColor[1];
                                clip.startColor.b = startColor[2];
                                clip.startColor.a = startColor[3];
                                m_hasUnsavedEdit = true;
                            }

                            float endColor[4] = { clip.endColor.r, clip.endColor.g, clip.endColor.b, clip.endColor.a };
                            if (ImGui::ColorEdit4("End Color", endColor))
                            {
                                clip.endColor.r = endColor[0];
                                clip.endColor.g = endColor[1];
                                clip.endColor.b = endColor[2];
                                clip.endColor.a = endColor[3];
                                m_hasUnsavedEdit = true;
                            }
                            break;
                        }
                        case Uma_UI::EffectProperty::Alpha:
                        {
                            if (ImGui::DragFloat("Start Alpha", &clip.startFloat, 0.01f, 0.0f, 1.0f))
                            {
                                m_hasUnsavedEdit = true;
                            }

                            if (ImGui::DragFloat("End Alpha", &clip.endFloat, 0.01f, 0.0f, 1.0f))
                            {
                                m_hasUnsavedEdit = true;
                            }
                            ImGui::TextDisabled("Applies to Image and/or Text on this entity.");
                            break;
                        }
                        case Uma_UI::EffectProperty::FillAmount:
                        {
                            if (ImGui::DragFloat("Start Fill", &clip.startFloat, 0.01f, 0.0f, 1.0f))
                            {
                                m_hasUnsavedEdit = true;
                            }

                            if (ImGui::DragFloat("End Fill", &clip.endFloat, 0.01f, 0.0f, 1.0f))
                            {
                                m_hasUnsavedEdit = true;
                            }
                            ImGui::TextDisabled("Requires Image with Fill Direction set.");
                            break;
                        }
                        case Uma_UI::EffectProperty::SpritesheetFrame:
                        {
                            if (ImGui::DragInt("Start Frame", &clip.startFrame, 1.0f, 0, 1000))
                            {
                                clip.endFrame = std::max(clip.endFrame, clip.startFrame);
                                m_hasUnsavedEdit = true;
                            }

                            if (ImGui::DragInt("End Frame", &clip.endFrame, 1.0f, clip.startFrame, 1000))
                            {
                                m_hasUnsavedEdit = true;
                            }

                            if (ImGui::DragFloat("FPS", &clip.fps, 0.5f, 0.1f, 120.0f, "%.1f"))
                            {
                                m_hasUnsavedEdit = true;
                            }

                            if (ImGui::Checkbox("Ping Pong", &clip.pingPong))
                            {
                                m_hasUnsavedEdit = true;
                            }
                            if (ImGui::IsItemHovered())
                            {
                                ImGui::SetTooltip("Play forward then in reverse, alternating each loop.");
                            }

                            // Live frame preview
                            ImGui::TextDisabled("Preview frame: %d", clip.GetCurrentFrame());
                            ImGui::TextDisabled("Total frames: %d", clip.endFrame - clip.startFrame + 1);
                            ImGui::TextDisabled("Requires Image with Sprite Sheet Grid set.");
                            break;
                        }
                        }

                        ImGui::Separator();

                        // Runtime state (read-only)
                        ImGui::Text("Runtime State:");
                        ImGui::BulletText("Playing: %s", clip.isPlaying ? "Yes" : "No");
                        ImGui::BulletText("Current Time: %.2f", clip.currentTime);
                        ImGui::BulletText("Progress: %.1f%%", clip.GetProgress() * 100.0f);
                        ImGui::BulletText("Complete: %s", clip.IsComplete() ? "Yes" : "No");

                        ImGui::Separator();

                        // Playback controls
                        if (ImGui::Button("Play"))
                        {
                            clip.Play();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Pause"))
                        {
                            clip.Pause();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Stop"))
                        {
                            clip.Stop();
                        }

                        ImGui::Separator();

                        // Remove clip button
                        if (ImGui::Button("Remove Clip"))
                        {
                            effects.clips.erase(effects.clips.begin() + i);
                            m_hasUnsavedEdit = true;
                            ImGui::TreePop();
                            ImGui::PopID();
                            break;
                        }

                        ImGui::TreePop();
                    }

                    ImGui::PopID();
                }

                ImGui::Separator();

                // Global playback controls
                ImGui::Text("Global Controls:");
                if (ImGui::Button("Play All"))
                {
                    effects.PlayAll();
                }
                ImGui::SameLine();
                if (ImGui::Button("Stop All"))
                {
                    effects.StopAll();
                }
                ImGui::SameLine();
                if (ImGui::Button("Reset All"))
                {
                    effects.ResetAll();
                }

                // End tracking
                EndComponentEdit(entity, coordinator, "Effects");

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_UI::Dialogue>())
        {
            if (ImGui::CollapsingHeader("DialogueData", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::Button("Remove Component##DialogueData"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_UI::Dialogue>>(
                        &coordinator,
                        entity,
                        "Remove DialogueData"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));

                    return true;
                }

                auto& dialogueData = coordinator.GetComponent<Uma_UI::Dialogue>(entity);

                ImGui::Indent();

                // Begin tracking
                BeginComponentEdit(entity, coordinator);

                // Sequence list
                ImGui::Text("Sequences (%zu)", dialogueData.sequences.size());
                ImGui::Separator();

                // Buffer for new-sequence id input (persists across frames)
                static char newSeqIdBuffer[128] = "";

                for (int seqIdx = 0; seqIdx < static_cast<int>(dialogueData.sequences.size()); ++seqIdx)
                {
                    auto& seq = dialogueData.sequences[seqIdx];

                    // Per-sequence collapsing header labelled by id
                    char seqLabel[256];
                    snprintf(seqLabel, sizeof(seqLabel), "Sequence [%d]: \"%s\"##seq%d",
                        seqIdx, seq.id.c_str(), seqIdx);

                    bool seqOpen = ImGui::CollapsingHeader(seqLabel);

                    // Drag-drop reorder handle (right-click context menu)
                    if (ImGui::BeginPopupContextItem())
                    {
                        if (seqIdx > 0 && ImGui::MenuItem("Move Up"))
                        {
                            std::swap(dialogueData.sequences[seqIdx], dialogueData.sequences[seqIdx - 1]);
                            m_hasUnsavedEdit = true;
                            ImGui::EndPopup();
                            break; // iterator invalidated
                        }
                        if (seqIdx < static_cast<int>(dialogueData.sequences.size()) - 1 && ImGui::MenuItem("Move Down"))
                        {
                            std::swap(dialogueData.sequences[seqIdx], dialogueData.sequences[seqIdx + 1]);
                            m_hasUnsavedEdit = true;
                            ImGui::EndPopup();
                            break;
                        }
                        if (ImGui::MenuItem("Delete Sequence"))
                        {
                            dialogueData.sequences.erase(dialogueData.sequences.begin() + seqIdx);
                            m_hasUnsavedEdit = true;
                            ImGui::EndPopup();
                            break;
                        }
                        ImGui::EndPopup();
                    }

                    if (seqOpen)
                    {
                        ImGui::Indent();

                        // Sequence ID
                        static char seqIdBuffer[128];
                        strncpy(seqIdBuffer, seq.id.c_str(), 127);
                        seqIdBuffer[127] = '\0';
                        char seqIdLabel[64];
                        snprintf(seqIdLabel, sizeof(seqIdLabel), "ID##seqid%d", seqIdx);
                        if (ImGui::InputText(seqIdLabel, seqIdBuffer, 128))
                        {
                            seq.id = seqIdBuffer;
                            m_hasUnsavedEdit = true;
                        }
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Unique identifier for this sequence.\nReferenced from Lua via StartDialogueSequence(entity, id)");

                        ImGui::Separator();
                        ImGui::Text("Lines (%zu)", seq.lines.size());

                        // Lines
                        for (int lineIdx = 0; lineIdx < static_cast<int>(seq.lines.size()); ++lineIdx)
                        {
                            auto& line = seq.lines[lineIdx];

                            char lineLabel[64];
                            snprintf(lineLabel, sizeof(lineLabel), "Line %d##lineheader%d_%d", lineIdx, seqIdx, lineIdx);

                            bool lineOpen = ImGui::CollapsingHeader(lineLabel);

                            // Right-click context on each line header
                            char lineCtxId[64];
                            snprintf(lineCtxId, sizeof(lineCtxId), "##linectx%d_%d", seqIdx, lineIdx);
                            if (ImGui::BeginPopupContextItem(lineCtxId))
                            {
                                if (lineIdx > 0 && ImGui::MenuItem("Move Up"))
                                {
                                    std::swap(seq.lines[lineIdx], seq.lines[lineIdx - 1]);
                                    m_hasUnsavedEdit = true;
                                    ImGui::EndPopup();
                                    break;
                                }
                                if (lineIdx < static_cast<int>(seq.lines.size()) - 1 && ImGui::MenuItem("Move Down"))
                                {
                                    std::swap(seq.lines[lineIdx], seq.lines[lineIdx + 1]);
                                    m_hasUnsavedEdit = true;
                                    ImGui::EndPopup();
                                    break;
                                }
                                if (ImGui::MenuItem("Delete Line"))
                                {
                                    seq.lines.erase(seq.lines.begin() + lineIdx);
                                    m_hasUnsavedEdit = true;
                                    ImGui::EndPopup();
                                    break;
                                }
                                ImGui::EndPopup();
                            }

                            if (lineOpen)
                            {
                                ImGui::Indent();

                                // Speaker
                                static char speakerBuf[128];
                                strncpy(speakerBuf, line.speaker.c_str(), 127);
                                speakerBuf[127] = '\0';
                                char speakerLabel[64];
                                snprintf(speakerLabel, sizeof(speakerLabel), "Speaker##sp%d_%d", seqIdx, lineIdx);
                                if (ImGui::InputText(speakerLabel, speakerBuf, 128))
                                {
                                    line.speaker = speakerBuf;
                                    m_hasUnsavedEdit = true;
                                }
                                if (ImGui::IsItemHovered())
                                    ImGui::SetTooltip("Displayed in the name box. Leave empty to hide the name box.");

                                // Body text
                                static char textBuf[2048];
                                strncpy(textBuf, line.text.c_str(), 2047);
                                textBuf[2047] = '\0';
                                char textLabel[64];
                                snprintf(textLabel, sizeof(textLabel), "Text##txt%d_%d", seqIdx, lineIdx);
                                ImGui::Text("Text");
                                if (ImGui::InputTextMultiline(textLabel, textBuf, 2048, ImVec2(-1, 70)))
                                {
                                    line.text = textBuf;
                                    m_hasUnsavedEdit = true;
                                }

                                // Portrait path with drag-drop
                                ImGui::Text("Portrait: %s", line.portrait.empty() ? "(None)" : line.portrait.c_str());

                                ImVec2 portraitDropZoneSize = ImVec2(ImGui::GetContentRegionAvail().x, 40.0f);
                                ImVec2 portraitCursorPos = ImGui::GetCursorScreenPos();
                                ImDrawList* drawList = ImGui::GetWindowDrawList();

                                drawList->AddRectFilled(portraitCursorPos,
                                    ImVec2(portraitCursorPos.x + portraitDropZoneSize.x, portraitCursorPos.y + portraitDropZoneSize.y),
                                    IM_COL32(40, 40, 60, 100), 4.0f);

                                ImVec2 hintSize = ImGui::CalcTextSize("Drop Portrait Texture Here (.png, .jpg)");
                                drawList->AddText(
                                    ImVec2(portraitCursorPos.x + (portraitDropZoneSize.x - hintSize.x) * 0.5f,
                                        portraitCursorPos.y + (portraitDropZoneSize.y - hintSize.y) * 0.5f),
                                    IM_COL32(150, 150, 150, 255),
                                    "Drop Portrait Texture Here (.png, .jpg)");

                                char portraitDropId[64];
                                snprintf(portraitDropId, sizeof(portraitDropId), "##portraitdrop%d_%d", seqIdx, lineIdx);
                                ImGui::SetCursorScreenPos(portraitCursorPos);
                                ImGui::InvisibleButton(portraitDropId, portraitDropZoneSize);
                                bool portraitHovered = ImGui::IsItemHovered();

                                if (ImGui::BeginDragDropTarget())
                                {
                                    drawList->AddRect(portraitCursorPos,
                                        ImVec2(portraitCursorPos.x + portraitDropZoneSize.x, portraitCursorPos.y + portraitDropZoneSize.y),
                                        IM_COL32(100, 200, 255, 255), 4.0f, 0, 3.0f);
                                    drawList->AddRectFilled(portraitCursorPos,
                                        ImVec2(portraitCursorPos.x + portraitDropZoneSize.x, portraitCursorPos.y + portraitDropZoneSize.y),
                                        IM_COL32(100, 150, 255, 50), 4.0f);

                                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                                    {
                                        const auto* data = static_cast<const Uma_Engine::FilePayload*>(payload->Data);
                                        std::string fullPath = data->filepath;
                                        std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

                                        std::filesystem::path fsp(fullPath);
                                        std::string ext = fsp.extension().string();
                                        std::transform(ext.begin(), ext.end(), ext.begin(),
                                            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                                        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg")
                                        {
                                            size_t assetsPos = fullPath.find("Assets/");
                                            line.portrait = (assetsPos != std::string::npos)
                                                ? fullPath.substr(assetsPos) : fullPath;
                                            m_hasUnsavedEdit = true;
                                        }
                                        else
                                        {
                                            m_popupErrorMessage = "Invalid file type for Portrait!\nExpected: .png, .jpg, .jpeg";
                                            ImGui::OpenPopup("Invalid File Format");
                                        }
                                    }
                                    ImGui::EndDragDropTarget();
                                }
                                else if (portraitHovered)
                                {
                                    drawList->AddRect(portraitCursorPos,
                                        ImVec2(portraitCursorPos.x + portraitDropZoneSize.x, portraitCursorPos.y + portraitDropZoneSize.y),
                                        IM_COL32(100, 150, 200, 200), 4.0f, 0, 2.0f);
                                }

                                // Clear portrait button
                                ImGui::SetCursorScreenPos(ImVec2(portraitCursorPos.x, portraitCursorPos.y + portraitDropZoneSize.y + 4.0f));
                                if (!line.portrait.empty())
                                {
                                    char clearLabel[64];
                                    snprintf(clearLabel, sizeof(clearLabel), "Clear##clearPortrait%d_%d", seqIdx, lineIdx);
                                    if (ImGui::SmallButton(clearLabel))
                                    {
                                        line.portrait.clear();
                                        m_hasUnsavedEdit = true;
                                    }
                                }

                                ImGui::Unindent();
                            }
                        } // end lines loop

                        ImGui::Separator();

                        // Add Line button
                        char addLineLabel[64];
                        snprintf(addLineLabel, sizeof(addLineLabel), "+ Add Line##addline%d", seqIdx);
                        if (ImGui::Button(addLineLabel))
                        {
                            seq.lines.push_back(Uma_UI::DialogueLine{});
                            m_hasUnsavedEdit = true;
                        }

                        ImGui::Unindent();
                    }
                } // end sequences loop

                ImGui::Separator();

                // Add new sequence
                ImGui::Text("New Sequence ID:");
                ImGui::SetNextItemWidth(-80.0f);
                ImGui::InputText("##newSeqId", newSeqIdBuffer, 128);
                ImGui::SameLine();
                if (ImGui::Button("+ Add##addseq"))
                {
                    Uma_UI::DialogueSequence newSeq;
                    newSeq.id = (newSeqIdBuffer[0] != '\0') ? newSeqIdBuffer : "sequence";
                    // Ensure id uniqueness by appending count if needed
                    std::string baseId = newSeq.id;
                    int suffix = 1;
                    while (dialogueData.FindSequence(newSeq.id) != nullptr)
                    {
                        newSeq.id = baseId + "_" + std::to_string(suffix++);
                    }
                    newSeq.lines.push_back(Uma_UI::DialogueLine{});
                    dialogueData.sequences.push_back(std::move(newSeq));
                    newSeqIdBuffer[0] = '\0';
                    m_hasUnsavedEdit = true;
                }

                ImGui::Separator();
                ImGui::TextDisabled("Tip: Right-click sequence or line headers to reorder / delete.");
                ImGui::TextDisabled("Triggered from Lua via StartDialogueSequence(entity, id)");

                // End tracking
                EndComponentEdit(entity, coordinator, "DialogueData");

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::Cutscene>())
        {
            if (ImGui::CollapsingHeader("Cutscene", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::Button("Remove Component##Cutscene"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_ECS::Cutscene>>(
                        &coordinator,
                        entity,
                        "Remove Cutscene"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));
                    return true;
                }

                auto& cutscene = coordinator.GetComponent<Uma_ECS::Cutscene>(entity);
                ImGui::Indent();
                BeginComponentEdit(entity, coordinator);

                // Play Once checkbox
                if (ImGui::Checkbox("Play Once##cutscenePlayOnce", &cutscene.playOnce))
                {
                    m_hasUnsavedEdit = true;
                }
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("If checked, cutscene only triggers once per game session.");

                // Has Played (read-only indicator)
                ImGui::TextDisabled("Has Played: %s", cutscene.hasPlayed ? "Yes" : "No");
                ImGui::SameLine();
                if (ImGui::SmallButton("Reset##cutsceneReset"))
                {
                    cutscene.hasPlayed = false;
                    m_hasUnsavedEdit = true;
                }

                ImGui::Separator();
                ImGui::Text("Actions (%zu)", cutscene.actions.size());
                ImGui::Separator();

                const char* actionTypeNames[] = {
                    "Set Camera To Position",
                    "Lerp Camera To Position",
                    "Play Dialogue",
                    "Wait",
                    "Return Camera To Player",
                    "Shake Camera",
                    "Lerp Camera Zoom"
                };
                constexpr int actionTypeCount = 7;

                for (int actionIdx = 0; actionIdx < static_cast<int>(cutscene.actions.size()); ++actionIdx)
                {
                    auto& action = cutscene.actions[actionIdx];

                    int typeInt = static_cast<int>(action.type);
                    const char* typeName = (typeInt >= 0 && typeInt < actionTypeCount) ? actionTypeNames[typeInt] : "Unknown";

                    char actionLabel[256];
                    snprintf(actionLabel, sizeof(actionLabel), "[%d] %s##action%d",
                        actionIdx, typeName, actionIdx);

                    bool actionOpen = ImGui::CollapsingHeader(actionLabel);

                    // Right-click context menu
                    char actionCtxId[64];
                    snprintf(actionCtxId, sizeof(actionCtxId), "##actionctx%d", actionIdx);
                    if (ImGui::BeginPopupContextItem(actionCtxId))
                    {
                        if (actionIdx > 0 && ImGui::MenuItem("Move Up"))
                        {
                            std::swap(cutscene.actions[actionIdx], cutscene.actions[actionIdx - 1]);
                            m_hasUnsavedEdit = true;
                            ImGui::EndPopup();
                            break;
                        }
                        if (actionIdx < static_cast<int>(cutscene.actions.size()) - 1 && ImGui::MenuItem("Move Down"))
                        {
                            std::swap(cutscene.actions[actionIdx], cutscene.actions[actionIdx + 1]);
                            m_hasUnsavedEdit = true;
                            ImGui::EndPopup();
                            break;
                        }
                        if (ImGui::MenuItem("Delete Action"))
                        {
                            cutscene.actions.erase(cutscene.actions.begin() + actionIdx);
                            m_hasUnsavedEdit = true;
                            ImGui::EndPopup();
                            break;
                        }
                        ImGui::EndPopup();
                    }

                    if (actionOpen)
                    {
                        ImGui::Indent();

                        // Action type combo
                        char typeLabel[64];
                        snprintf(typeLabel, sizeof(typeLabel), "Type##actiontype%d", actionIdx);
                        if (ImGui::Combo(typeLabel, &typeInt, actionTypeNames, actionTypeCount))
                        {
                            action.type = static_cast<Uma_ECS::CutsceneActionType>(typeInt);
                            m_hasUnsavedEdit = true;
                        }

                        // Show relevant fields based on type
                        switch (action.type)
                        {
                        case Uma_ECS::CutsceneActionType::SetCameraToPosition:
                        {
                            char posLabel[64];
                            snprintf(posLabel, sizeof(posLabel), "Position##campos%d", actionIdx);
                            float pos[2] = { action.targetPosition.x, action.targetPosition.y };
                            if (ImGui::DragFloat2(posLabel, pos, 0.1f))
                            {
                                action.targetPosition = Vec2(pos[0], pos[1]);
                                m_hasUnsavedEdit = true;
                            }
                            break;
                        }
                        case Uma_ECS::CutsceneActionType::LerpCameraToPosition:
                        {
                            char posLabel[64];
                            snprintf(posLabel, sizeof(posLabel), "Target Position##lerppos%d", actionIdx);
                            float pos[2] = { action.targetPosition.x, action.targetPosition.y };
                            if (ImGui::DragFloat2(posLabel, pos, 0.1f))
                            {
                                action.targetPosition = Vec2(pos[0], pos[1]);
                                m_hasUnsavedEdit = true;
                            }
                            char durLabel[64];
                            snprintf(durLabel, sizeof(durLabel), "Duration (s)##lerpdur%d", actionIdx);
                            if (ImGui::DragFloat(durLabel, &action.duration, 0.05f, 0.1f, 30.0f))
                            {
                                m_hasUnsavedEdit = true;
                            }
                            break;
                        }
                        case Uma_ECS::CutsceneActionType::PlayDialogue:
                        {
                            static char seqIdBuf[128];
                            strncpy(seqIdBuf, action.dialogueSequenceId.c_str(), 127);
                            seqIdBuf[127] = '\0';
                            char seqLabel[64];
                            snprintf(seqLabel, sizeof(seqLabel), "Sequence ID##dlgseq%d", actionIdx);
                            if (ImGui::InputText(seqLabel, seqIdBuf, 128))
                            {
                                action.dialogueSequenceId = seqIdBuf;
                                m_hasUnsavedEdit = true;
                            }
                            if (ImGui::IsItemHovered())
                                ImGui::SetTooltip("ID of the dialogue sequence to play.\nMust match an ID in the Dialogue component on this entity.");
                            break;
                        }
                        case Uma_ECS::CutsceneActionType::Wait:
                        {
                            char waitLabel[64];
                            snprintf(waitLabel, sizeof(waitLabel), "Duration (s)##waitdur%d", actionIdx);
                            if (ImGui::DragFloat(waitLabel, &action.duration, 0.05f, 0.0f, 60.0f))
                            {
                                m_hasUnsavedEdit = true;
                            }
                            break;
                        }
                        case Uma_ECS::CutsceneActionType::ReturnCameraToPlayer:
                        {
                            ImGui::TextDisabled("Re-enables camera follow on player.");
                            break;
                        }
                        case Uma_ECS::CutsceneActionType::ShakeCamera:
                        {
                            char intLabel[64];
                            snprintf(intLabel, sizeof(intLabel), "Intensity##shakeint%d", actionIdx);
                            if (ImGui::DragFloat(intLabel, &action.shakeIntensity, 0.05f, 0.0f, 10.0f))
                            {
                                m_hasUnsavedEdit = true;
                            }
                            char durLabel[64];
                            snprintf(durLabel, sizeof(durLabel), "Duration (s)##shakedur%d", actionIdx);
                            if (ImGui::DragFloat(durLabel, &action.duration, 0.05f, 0.0f, 10.0f))
                            {
                                m_hasUnsavedEdit = true;
                            }
                            break;
                        }
                        case Uma_ECS::CutsceneActionType::LerpCameraZoom:
                        {
                            char zoomLabel[64];
                            snprintf(zoomLabel, sizeof(zoomLabel), "Target Zoom##zoomtarget%d", actionIdx);
                            if (ImGui::DragFloat(zoomLabel, &action.targetZoom, 0.1f, 0.1f, 50.0f))
                            {
                                m_hasUnsavedEdit = true;
                            }
                            char durLabel[64];
                            snprintf(durLabel, sizeof(durLabel), "Duration (s)##zoomdur%d", actionIdx);
                            if (ImGui::DragFloat(durLabel, &action.duration, 0.05f, 0.1f, 30.0f))
                            {
                                m_hasUnsavedEdit = true;
                            }
                            break;
                        }
                        }

                        ImGui::Unindent();
                    }
                } // end actions loop

                ImGui::Separator();

                // Add action buttons
                if (ImGui::Button("+ Set Camera##addSetCam"))
                {
                    Uma_ECS::CutsceneAction a;
                    a.type = Uma_ECS::CutsceneActionType::SetCameraToPosition;
                    cutscene.actions.push_back(a);
                    m_hasUnsavedEdit = true;
                }
                ImGui::SameLine();
                if (ImGui::Button("+ Lerp Camera##addLerpCam"))
                {
                    Uma_ECS::CutsceneAction a;
                    a.type = Uma_ECS::CutsceneActionType::LerpCameraToPosition;
                    cutscene.actions.push_back(a);
                    m_hasUnsavedEdit = true;
                }
                ImGui::SameLine();
                if (ImGui::Button("+ Dialogue##addDlg"))
                {
                    Uma_ECS::CutsceneAction a;
                    a.type = Uma_ECS::CutsceneActionType::PlayDialogue;
                    cutscene.actions.push_back(a);
                    m_hasUnsavedEdit = true;
                }
                if (ImGui::Button("+ Wait##addWait"))
                {
                    Uma_ECS::CutsceneAction a;
                    a.type = Uma_ECS::CutsceneActionType::Wait;
                    cutscene.actions.push_back(a);
                    m_hasUnsavedEdit = true;
                }
                ImGui::SameLine();
                if (ImGui::Button("+ Return Camera##addRetCam"))
                {
                    Uma_ECS::CutsceneAction a;
                    a.type = Uma_ECS::CutsceneActionType::ReturnCameraToPlayer;
                    cutscene.actions.push_back(a);
                    m_hasUnsavedEdit = true;
                }
                ImGui::SameLine();
                if (ImGui::Button("+ Shake Camera##addShakeCam"))
                {
                    Uma_ECS::CutsceneAction a;
                    a.type = Uma_ECS::CutsceneActionType::ShakeCamera;
                    a.shakeIntensity = 1.0f;
                    a.duration = 0.25f;
                    cutscene.actions.push_back(a);
                    m_hasUnsavedEdit = true;
                }
                //ImGui::SameLine();
                if (ImGui::Button("+ Lerp Zoom##addLerpZoom"))
                {
                    Uma_ECS::CutsceneAction a;
                    a.type = Uma_ECS::CutsceneActionType::LerpCameraZoom;
                    a.targetZoom = 10.0f;
                    a.duration = 1.0f;
                    cutscene.actions.push_back(a);
                    m_hasUnsavedEdit = true;
                }

                ImGui::Separator();
                ImGui::TextDisabled("Tip: Add Collider (Trigger + CL_PICKUP + CL_PLAYER mask)");
                ImGui::TextDisabled("+ Rigidbody + LuaScript (CutsceneTrigger.lua)");
                ImGui::TextDisabled("to activate this cutscene on player collision.");

                EndComponentEdit(entity, coordinator, "Cutscene");
                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::Prefab>())
        {
            if (ImGui::CollapsingHeader("Prefab"))
            {
                if (ImGui::Button("Remove Component##Prefab"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_ECS::Prefab>>(
                        &coordinator,
                        entity,
                        "Remove Prefab"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));

                    return true;
                }

                auto& prefab = coordinator.GetComponent<Uma_ECS::Prefab>(entity);
                ImGui::Indent();

                // begin tracking
                BeginComponentEdit(entity, coordinator);

                // Prefab Path (read-only display)
                ImGui::Text("Prefab Path:");
                ImGui::TextWrapped("%s", prefab.prefabPath.empty() ? "(None)" : prefab.prefabPath.c_str());

                // Make it editable if needed
                static char prefabPathBuffer[512];
                strncpy(prefabPathBuffer, prefab.prefabPath.c_str(), 511);
                prefabPathBuffer[511] = '\0';
                if (ImGui::InputText("Path", prefabPathBuffer, 512))
                {
                    prefab.prefabPath = prefabPathBuffer;
                    m_hasUnsavedEdit = true;
                }

                ImGui::Separator();

                // Is Root flag
                if (ImGui::Checkbox("Is Root Entity", &prefab.isRoot))
                {
                    m_hasUnsavedEdit = true;
                }

                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Marks this as the root entity of the prefab instance");
                }

                ImGui::Separator();
                ImGui::TextDisabled("This entity is linked to a prefab asset");

                // end tracking
                EndComponentEdit(entity, coordinator, "Prefab");

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::Projectile>())
        {
            if (ImGui::CollapsingHeader("Projectile"))
            {
                if (ImGui::Button("Remove Component##Projectile"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_ECS::Projectile>>(
                        &coordinator,
                        entity,
                        "Remove Projectile"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));

                    return true;
                }

                auto& projectile = coordinator.GetComponent<Uma_ECS::Projectile>(entity);
                ImGui::Indent();

                // begin tracking
                BeginComponentEdit(entity, coordinator);

                // Damage
                if (ImGui::DragInt("Damage", &projectile.mStats.damage, 1.0f, 0, 1000))
                {
                    m_hasUnsavedEdit = true;
                }

                // Damage
                if (ImGui::DragFloat("Speed", &projectile.mStats.speed, 1.0f, 0, 300.f))
                {
                    m_hasUnsavedEdit = true;
                }

                ImGui::Separator();

                // Fade Over Time
                if (ImGui::Checkbox("Fade Over Time##FadeOverTime", &projectile.mStats.fadeOVerTime))
                {
                    m_hasUnsavedEdit = true;
                }

                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Makes the projectile gradually fade out during its lifetime");
                }

                // Life Time
                if (ImGui::DragFloat("Life Time", &projectile.mStats.lifeTime, 0.1f, 0.0f, 60.0f))
                {
                    m_hasUnsavedEdit = true;
                }

                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("How long the projectile exists before being destroyed (seconds)");
                }

                ImGui::Separator();

                if (ImGui::Checkbox("Fade After Animation##fadeAfterAnimEnded", &projectile.mStats.fadeAfterAnimEnded))
                {
                    m_hasUnsavedEdit = true;
                }

                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("It will die when the animation ends (must have animator component)");
                }

                ImGui::Separator();

                ImGui::TextDisabled("This entity is marked as a projectile");

                // end tracking
                EndComponentEdit(entity, coordinator, "Projectile");

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::Tilemap>())
        {
            if (ImGui::CollapsingHeader("Tilemap"))
            {
                if (ImGui::Button("Remove Component##Tilemap"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_ECS::Tilemap>>(
                        &coordinator,
                        entity,
                        "Remove Projectile"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));

                    return true;
                }

                ImGui::Indent();

                // begin tracking
                BeginComponentEdit(entity, coordinator);

                auto& tilemap = coordinator.GetComponent<Uma_ECS::Tilemap>(entity);

                // === Map Properties Section ===
                if (ImGui::TreeNode("Map Properties"))
                {
                    // Editable map dimensions
                    int mapWidth = tilemap.mapWidth;
                    int mapHeight = tilemap.mapHeight;

                    if (ImGui::DragInt("Map Width (tiles)", &mapWidth, 1.0f, 1, 1000))
                    {
                        if (mapWidth != tilemap.mapWidth)
                        {
                            tilemap.mapWidth = mapWidth;
                            // Resize all layers
                            for (auto& layer : tilemap.layers)
                            {
                                layer.width = static_cast<unsigned int>(mapWidth);
                                layer.tiles.resize(static_cast<size_t>(mapWidth * layer.height), -1);
                            }
                            m_hasUnsavedEdit = true;
                        }
                    }

                    if (ImGui::DragInt("Map Height (tiles)", &mapHeight, 1.0f, 1, 1000))
                    {
                        if (mapHeight != tilemap.mapHeight)
                        {
                            tilemap.mapHeight = mapHeight;
                            // Resize all layers
                            for (auto& layer : tilemap.layers)
                            {
                                layer.height = static_cast<unsigned int>(mapHeight);
                                layer.tiles.resize(static_cast<size_t>(layer.width * mapHeight), -1);
                            }
                            m_hasUnsavedEdit = true;
                        }
                    }

                    int tileSize = tilemap.tileSize;
                    if (ImGui::DragInt("Tile Size (pixels)", &tileSize, 1.0f, 1, 256))
                    {
                        tilemap.tileSize = tileSize;
                        m_hasUnsavedEdit = true;
                    }

                    ImGui::Separator();
                    ImGui::Text("Tilemap Size: %dx%d units",
                        tilemap.mapWidth,
                        tilemap.mapHeight);

                    ImGui::Text("World Size: %dx%d units",
                        tilemap.mapWidth * tilemap.tileSize,
                        tilemap.mapHeight * tilemap.tileSize);

                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Tileset Properties"))
                {
                    ImGui::Text("Texture Path: %s", tilemap.tileset.texturePath.c_str());

                    // Create a visible drop zone with visual feedback
                    ImVec2 dropZoneSize = ImVec2(ImGui::GetContentRegionAvail().x, 60.0f);
                    ImVec2 cursorPos = ImGui::GetCursorScreenPos();

                    ImDrawList* drawList = ImGui::GetWindowDrawList();
                    ImU32 bgColor = IM_COL32(40, 40, 60, 100);

                    // Background
                    drawList->AddRectFilled(cursorPos,
                        ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                        bgColor, 4.0f);

                    // Center text in the drop zone
                    ImVec2 textSize = ImGui::CalcTextSize("Drag & Drop Texture Here");
                    ImVec2 textPos = ImVec2(
                        cursorPos.x + (dropZoneSize.x - textSize.x) * 0.5f,
                        cursorPos.y + (dropZoneSize.y - textSize.y) * 0.5f - 10.0f
                    );
                    drawList->AddText(textPos, IM_COL32(150, 150, 150, 255), "Drag & Drop Texture Here");

                    // Supported formats text
                    ImVec2 formatTextSize = ImGui::CalcTextSize("(.png, .jpg, .jpeg, .bmp)");
                    ImVec2 formatTextPos = ImVec2(
                        cursorPos.x + (dropZoneSize.x - formatTextSize.x) * 0.5f,
                        cursorPos.y + (dropZoneSize.y - formatTextSize.y) * 0.5f + 10.0f
                    );
                    drawList->AddText(formatTextPos, IM_COL32(100, 100, 100, 255), "(.png, .jpg, .jpeg, .bmp)");

                    // Invisible button for the drop zone
                    ImGui::SetCursorScreenPos(cursorPos);
                    ImGui::InvisibleButton("##TextureDropZone", dropZoneSize);

                    bool isHovered = ImGui::IsItemHovered();

                    // Drag and Drop Target
                    if (ImGui::BeginDragDropTarget())
                    {
                        // Highlight the drop zone when dragging over
                        drawList->AddRect(cursorPos,
                            ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                            IM_COL32(100, 200, 255, 255), 4.0f, 0, 3.0f);

                        // Show glow effect
                        drawList->AddRectFilled(cursorPos,
                            ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                            IM_COL32(100, 150, 255, 50), 4.0f);

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                        {
                            const auto* data = static_cast<const Uma_Engine::FilePayload*>(payload->Data);
                            std::string fullPath = data->filepath;
                            std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

                            std::filesystem::path p(fullPath);
                            std::string ext = p.extension().string();

                            // Convert to lowercase for comparison
                            std::transform(ext.begin(), ext.end(), ext.begin(),
                                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp")
                            {
                                std::string relativePath = fullPath;
                                size_t assetsPos = fullPath.find("Assets/");
                                if (assetsPos != std::string::npos)
                                {
                                    relativePath = fullPath.substr(assetsPos);
                                }

                                tilemap.tileset.texturePath = relativePath;
                                tilemap.tileset.texture = nullptr;
                                m_hasUnsavedEdit = true;
                            }
                            else
                            {
                                m_popupErrorMessage = "Invalid file type for Sprite!\nExpected: .png, .jpg, .jpeg, .bmp";
                                ImGui::OpenPopup("Invalid File Format");
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                    else if (isHovered)
                    {
                        // Subtle hover effect when not dragging
                        drawList->AddRect(cursorPos,
                            ImVec2(cursorPos.x + dropZoneSize.x, cursorPos.y + dropZoneSize.y),
                            IM_COL32(100, 150, 200, 200), 4.0f, 0, 2.0f);
                    }

                    float gridArray[2] = { static_cast<float>(tilemap.tileset.columns), static_cast<float>(tilemap.tileset.rows) };
                    if (ImGui::DragFloat2("Grids (Col, Row)", gridArray, 1.0f, 1.0f, 100.0f, "%.0f"))
                    {
                        tilemap.tileset.columns = static_cast<int>(gridArray[0]);
                        tilemap.tileset.rows = static_cast<int>(gridArray[1]);
                        m_hasUnsavedEdit = true;
                    }
                    ImGui::TreePop();
                }

                ImGui::Separator();

                // === Layers Section ===
                if (ImGui::TreeNode("Layers", "Layers (%zu)", tilemap.layers.size()))
                {
                    int layerToDelete = -1;
                    for (size_t i = 0; i < tilemap.layers.size(); i++)
                    {
                        ImGui::PushID(static_cast<int>(i));

                        auto& layer = tilemap.layers[i];

                        // Layer header with visibility toggle
                        bool visible = tilemap.layerVisibility[i];
                        ImGui::Checkbox("##vis", &visible);
                        tilemap.layerVisibility[i] = visible;

                        ImGui::SameLine();

                        // Layer name as tree node
                        bool layerOpen = ImGui::TreeNode("LayerNode", "%s", layer.name.c_str());

                        // Right-click menu for layer operations
                        if (ImGui::BeginPopupContextItem())
                        {
                            if (ImGui::MenuItem("Move Up", nullptr, false, i > 0))
                            {
                                std::swap(tilemap.layers[i], tilemap.layers[i - 1]);

                                bool tempVis = tilemap.layerVisibility[i];
                                tilemap.layerVisibility[i] = tilemap.layerVisibility[i - 1];
                                tilemap.layerVisibility[i - 1] = tempVis;

                                std::swap(tilemap.layerNames[i], tilemap.layerNames[i - 1]);
                            }
                            if (ImGui::MenuItem("Move Down", nullptr, false, i < tilemap.layers.size() - 1))
                            {
                                std::swap(tilemap.layers[i], tilemap.layers[i + 1]);

                                bool tempVis = tilemap.layerVisibility[i];
                                tilemap.layerVisibility[i] = tilemap.layerVisibility[i + 1];
                                tilemap.layerVisibility[i + 1] = tempVis;

                                std::swap(tilemap.layerNames[i], tilemap.layerNames[i + 1]);
                            }
                            ImGui::Separator();
                            if (ImGui::MenuItem("Delete", nullptr, false, tilemap.layers.size() > 1))
                            {
                                /* tilemap.RemoveLayer(static_cast<int>(i));
                                 ImGui::PopID();
                                 ImGui::EndPopup();
                                 if (layerOpen) ImGui::TreePop();*/

                                layerToDelete = static_cast<int>(i);

                                ImGui::EndPopup();
                                ImGui::PopID();
                                break;
                            }
                            ImGui::EndPopup();

                            m_hasUnsavedEdit = true;
                        }

                        if (layerOpen)
                        {
                            ImGui::Indent();

                            // Editable layer name
                            char nameBuffer[256];
                            strncpy(nameBuffer, layer.name.c_str(), sizeof(nameBuffer) - 1);
                            nameBuffer[sizeof(nameBuffer) - 1] = '\0';

                            if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
                            {
                                layer.name = nameBuffer;
                                tilemap.layerNames[i] = nameBuffer;

                                m_hasUnsavedEdit = true;
                            }

                            // Render layer dropdown (same as Sprite)
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
                            unsigned int rl = static_cast<unsigned int>(layer.renderLayer);
                            while (rl >>= 1) ++currentRenderLayer;
                            if (ImGui::Combo("Render Layer", &currentRenderLayer, renderLayerNames, IM_ARRAYSIZE(renderLayerNames)))
                            {
                                layer.renderLayer = (1u << currentRenderLayer);

                                m_hasUnsavedEdit = true;
                            }

                            // Render order (same as Sprite)
                            ImGui::Separator();
                            ImGui::Text("Render Order");
                            ImGui::InputInt("##Layer Render Order", &layer.renderOrder, 1, 0, 0);

                            // Tint color (RGB)
                            float tintColorArray[3] = { layer.tintColor.x, layer.tintColor.y, layer.tintColor.z };
                            if (ImGui::ColorEdit3("Tint Color", tintColorArray))
                            {
                                layer.tintColor.x = tintColorArray[0];
                                layer.tintColor.y = tintColorArray[1];
                                layer.tintColor.z = tintColorArray[2];
                                m_hasUnsavedEdit = true;
                            }

                            // Alpha (opacity)
                            if (ImGui::SliderFloat("Alpha", &layer.alpha, 0.0f, 1.0f, "%.2f")) m_hasUnsavedEdit = true;

                            // Show layer size info
                            ImGui::Separator();
                            ImGui::Text("Size: %ux%u tiles", layer.width, layer.height);

                            // Count non-empty tiles
                            int filledTiles = 0;
                            for (int tile : layer.tiles)
                            {
                                if (tile >= 0) filledTiles++;
                            }
                            ImGui::Text("Filled: %d / %zu tiles (%.1f%%)",
                                filledTiles,
                                layer.tiles.size(),
                                layer.tiles.size() > 0 ? (filledTiles * 100.0f) / layer.tiles.size() : 0.0f);

                            // Clear layer button
                            if (ImGui::Button("Clear Layer", ImVec2(-1, 0)))
                            {
                                std::fill(layer.tiles.begin(), layer.tiles.end(), -1);

                                m_hasUnsavedEdit = true;
                            }

                            ImGui::Unindent();
                            ImGui::TreePop();
                        }

                        ImGui::PopID();
                    }

                    if (layerToDelete != -1)
                    {
                        tilemap.RemoveLayer(layerToDelete);
                        m_hasUnsavedEdit = true;
                    }

                    ImGui::Separator();

                    // Add new layer button
                    if (ImGui::Button("+ Add Layer", ImVec2(-1, 0)))
                    {
                        int newIndex = static_cast<int>(tilemap.layers.size());
                        std::string newName = "Layer " + std::to_string(newIndex);
                        tilemap.CreateLayer(
                            newName,
                            static_cast<unsigned int>(tilemap.mapWidth),
                            static_cast<unsigned int>(tilemap.mapHeight),
                            newIndex
                        );

                        m_hasUnsavedEdit = true;
                    }

                    ImGui::TreePop();
                }

                ImGui::Separator();

                if (m_playState == PlayState::Stopped)
                {
                    // === Edit Mode Section ===
                    ImGui::PushStyleColor(ImGuiCol_Button,
                        tilemap.isInEditMode ? ImVec4(0.8f, 0.3f, 0.2f, 1.0f) : ImVec4(0.2f, 0.6f, 0.8f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                        tilemap.isInEditMode ? ImVec4(0.9f, 0.4f, 0.3f, 1.0f) : ImVec4(0.3f, 0.7f, 0.9f, 1.0f));

                    const char* buttonText = tilemap.isInEditMode ? "Exit Edit Mode" : "Enter Edit Mode";
                    if (ImGui::Button(buttonText, ImVec2(-1, 40)))
                    {
                        //tilemap.isInEditMode = !tilemap.isInEditMode;

                        TilemapEditorManager* tilemapEditorManager = pSystemManager->GetSystem<TilemapEditorManager>();

                        if (!tilemap.isInEditMode)
                        {
                            // Enter edit mode
                            tilemapEditorManager->OpenEditor(entity);
                        }
                        else
                        {
                            // Exit edit mode
                            tilemapEditorManager->CloseEditor();
                        }
                    }

                    ImGui::PopStyleColor(2);
                }

                // Show status message when in edit mode
                if (tilemap.isInEditMode)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.9f, 0.2f, 1.0f));
                    ImGui::TextWrapped("EDIT MODE ACTIVE - Click tiles in the scene view to paint");
                    ImGui::PopStyleColor();
                }

                EndComponentEdit(entity, coordinator, "Tilemap");

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::FSM>())
        {
            if (ImGui::CollapsingHeader("FSM"))
            {
                if (ImGui::Button("Remove Component##FSM"))
                {
                    auto cmd = std::make_unique<Uma_Editor::EntityRemoveComponentCmd<Uma_ECS::FSM>>(
                        &coordinator,
                        entity,
                        "Remove FSM"
                    );
                    commandHistory.ExecuteCommand(std::move(cmd));
                    return true;
                }

                auto& fsm = coordinator.GetComponent<Uma_ECS::FSM>(entity);
                auto& luaScripts = coordinator.GetComponent<Uma_ECS::LuaScript>(entity);  // Assuming component type is LuaScripts

                ImGui::Indent();
                BeginComponentEdit(entity, coordinator);

                static int selectedScriptIndex = -1;

                // Build list of script names
                std::vector<std::string> scriptNames;
                for (const auto& script : luaScripts.scripts) {
                    scriptNames.push_back(script.scriptName);
                }

                const char* previewText = scriptNames.empty() ? "No scripts" : "Select script";
                if (selectedScriptIndex >= 0 && selectedScriptIndex < (int)scriptNames.size()) {
                    previewText = scriptNames[selectedScriptIndex].c_str();
                }

                if (ImGui::BeginCombo("Script Name##FSM", previewText)) {
                    for (int i = 0; i < (int)scriptNames.size(); ++i) {
                        bool isSelected = (i == selectedScriptIndex);
                        if (ImGui::Selectable(scriptNames[i].c_str(), isSelected)) {
                            selectedScriptIndex = i;
                        }
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                if (ImGui::Button("Add State from Script", ImVec2(ImGui::GetContentRegionAvail().x, 0))
                    && selectedScriptIndex >= 0
                    && selectedScriptIndex < (int)scriptNames.size())
                {
                    std::string stateName = scriptNames[selectedScriptIndex];
                    fsm.AddStates(stateName, true);
                    // Optional: reset selection
                    selectedScriptIndex = -1;
                }

                ImGui::Separator();

                std::vector<std::string> stateKeys;
                for (const auto& p : fsm.states) stateKeys.push_back(p.first);

                for (size_t i = 0; i < stateKeys.size(); ++i)
                {
                    const std::string& key = stateKeys[i];
                    auto it = fsm.states.find(key);
                    if (it == fsm.states.end()) continue;

                    ImGui::PushID((int)i);

                    if (ImGui::TreeNode(key.c_str()))
                    {
                        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(200, 50, 50, 255));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 75, 75, 255));
                        if (ImGui::Button("Remove"))
                        {
                            fsm.states.erase(key);
                        }
                        ImGui::PopStyleColor(2);
                        ImGui::TreePop();
                    }

                    ImGui::PopID();
                }

                EndComponentEdit(entity, coordinator, "FSM");

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
        (void)coord;

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

        BeginComponentEdit(m_selectedEntity, coordinator);
        if (ImGui::InputText("##name", textureBuffer, 256))
        {
            tf.name = textureBuffer;
            m_hasUnsavedEdit = true;
        }
        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            EndComponentEdit(m_selectedEntity, coordinator, "Rename", true);
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
                Uma_ECS::Sprite defaultSprite;
                defaultSprite.texturePath = "Assets/whitePixel.png";
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_ECS::Sprite>>(
                    &coordinator,
                    m_selectedEntity,
                    defaultSprite,
                    "Add Sprite"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::RigidBody>()) && ImGui::MenuItem("RigidBody"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_ECS::RigidBody>>(
                    &coordinator,
                    m_selectedEntity,
                    Uma_ECS::RigidBody{},
                    "Add RigidBody"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::Collider>()) && ImGui::MenuItem("Collider"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_ECS::Collider>>(
                    &coordinator,
                    m_selectedEntity,
                    Uma_ECS::Collider{},
                    "Add Collider"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::Camera>()) && ImGui::MenuItem("Camera"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_ECS::Camera>>(
                    &coordinator,
                    m_selectedEntity,
                    Uma_ECS::Camera{},
                    "Add Camera"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::LuaScript>()) && ImGui::MenuItem("LuaScript"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_ECS::LuaScript>>(
                    &coordinator,
                    m_selectedEntity,
                    Uma_ECS::LuaScript{},
                    "Add LuaScript"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::Enemy>()) && ImGui::MenuItem("Enemy"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_ECS::Enemy>>(
                    &coordinator,
                    m_selectedEntity,
                    Uma_ECS::Enemy{},
                    "Add Enemy"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::Animator>()) && ImGui::MenuItem("Animator"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_ECS::Animator>>(
                    &coordinator,
                    m_selectedEntity,
                    Uma_ECS::Animator{},
                    "Add Animator"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::AudioComponent>()) && ImGui::MenuItem("AudioComponent"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_ECS::AudioComponent>>(
                    &coordinator,
                    m_selectedEntity,
                    Uma_ECS::AudioComponent{},
                    "Add AudioComponent"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::AudioListener>()) && ImGui::MenuItem("AudioListener"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_ECS::AudioListener>>(
                    &coordinator,
                    m_selectedEntity,
                    Uma_ECS::AudioListener{},
                    "Add AudioListener"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::PathFinding>()) && ImGui::MenuItem("PathFinding"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_ECS::PathFinding>>(
                    &coordinator,
                    m_selectedEntity,
                    Uma_ECS::PathFinding{},
                    "Add PathFinding"
                );
                commandHistory.ExecuteCommand(std::move(cmd));

                pEventSystem->Emit<CallPathFindToBake>();
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::Projectile>()) && ImGui::MenuItem("Projectile"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_ECS::Projectile>>(
                    &coordinator,
                    m_selectedEntity,
                    Uma_ECS::Projectile{},
                    "Add Projectile"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::Prefab>()) && ImGui::MenuItem("Prefab"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_ECS::Prefab>>(
                    &coordinator,
                    m_selectedEntity,
                    Uma_ECS::Prefab{},
                    "Add Prefab"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::ParticleEmitter>()) && ImGui::MenuItem("ParticleEmitter"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_ECS::ParticleEmitter>>(
                    &coordinator,
                    m_selectedEntity,
                    Uma_ECS::ParticleEmitter{},
                    "Add ParticleEmitter"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_UI::RectTransform>()) && ImGui::MenuItem("RectTransform"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_UI::RectTransform>>(
                    &coordinator,
                    m_selectedEntity,
                    Uma_UI::RectTransform{},
                    "Add RectTransform"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_UI::Image>()) && ImGui::MenuItem("Image"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_UI::Image>>(
                    &coordinator,
                    m_selectedEntity,
                    Uma_UI::Image{},
                    "Add Image"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_UI::Button>()) && ImGui::MenuItem("Button"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_UI::Button>>(
                    &coordinator,
                    m_selectedEntity,
                    Uma_UI::Button{},
                    "Add Button"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_UI::Slider>()) && ImGui::MenuItem("Slider"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_UI::Slider>>(
                    &coordinator,
                    m_selectedEntity,
                    Uma_UI::Slider{},
                    "Add Slider"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_UI::Checkbox>()) && ImGui::MenuItem("Checkbox"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_UI::Checkbox>>(
                    &coordinator,
                    m_selectedEntity,
                    Uma_UI::Checkbox{},
                    "Add Checkbox"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_UI::Canvas>()) && ImGui::MenuItem("Canvas"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_UI::Canvas>>(
                    &coordinator,
                    m_selectedEntity,
                    Uma_UI::Canvas{},
                    "Add Canvas"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_UI::Text>()) && ImGui::MenuItem("Text"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_UI::Text>>(
                    &coordinator,
                    m_selectedEntity,
                    Uma_UI::Text{},
                    "Add Text"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_UI::Effects>()) && ImGui::MenuItem("Effects"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_UI::Effects>>(
                    &coordinator,
                    m_selectedEntity,
                    Uma_UI::Effects{},
                    "Add Effects"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_UI::Dialogue>()) && ImGui::MenuItem("Dialogue"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_UI::Dialogue>>(
                    &coordinator,
                    m_selectedEntity,
                    Uma_UI::Dialogue{},
                    "Add Dialogue"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::Cutscene>()) && ImGui::MenuItem("Cutscene"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_ECS::Cutscene>>(
                    &coordinator,
                    m_selectedEntity,
                    Uma_ECS::Cutscene{},
                    "Add Cutscene"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::Tilemap>()) && ImGui::MenuItem("Tilemap"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_ECS::Tilemap>>(
                    &coordinator,
                    m_selectedEntity,
                    Uma_ECS::Tilemap{},
                    "Add Tilemap"
                );
                commandHistory.ExecuteCommand(std::move(cmd));

                // by default add one layer
                auto& tilemap = coordinator.GetComponent<Uma_ECS::Tilemap>(m_selectedEntity);
                tilemap.CreateLayer(
                    "Layer0",
                    static_cast<unsigned int>(tilemap.mapWidth),
                    static_cast<unsigned int>(tilemap.mapHeight),
                    0
                );
            }
            if (!coordinator.GetEntitySignature(m_selectedEntity).test(coordinator.GetComponentType<Uma_ECS::FSM>()) && ImGui::MenuItem("FSM"))
            {
                auto cmd = std::make_unique<Uma_Editor::EntityAddComponentCmd<Uma_ECS::FSM>>(
                    &coordinator,
                    m_selectedEntity,
                    Uma_ECS::FSM{},
                    "Add StateMachine"
                );
                commandHistory.ExecuteCommand(std::move(cmd));
            }

            ImGui::EndPopup();
        }

        // Error popup
        if (ImGui::BeginPopupModal("Invalid File Format", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Error:");
            ImGui::TextWrapped("%s", m_popupErrorMessage.c_str());
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Center the OK button
            float buttonWidth = 120.0f;
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - buttonWidth) * 0.5f);

            if (ImGui::Button("OK", ImVec2(buttonWidth, 0)))
            {
                ImGui::CloseCurrentPopup();
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

    void ImguiManager::OpenScriptInVSCode(const std::string& filepath)
    {
#ifdef _WIN32
        std::string command = "code \"" + filepath + "\"";
        system(command.c_str());
#elif __APPLE__
        std::string command = "code \"" + filepath + "\"";
        system(command.c_str());
#elif __linux__
        std::string command = "code \"" + filepath + "\"";
        system(command.c_str());
#endif
    }
}