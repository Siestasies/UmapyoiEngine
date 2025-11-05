#include "ImguiManager.h"
#include "SceneManager.h"  // Include here in .cpp instead of .h

//#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"

#include "Core/FilePaths.h"

#include <unordered_map>

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
        , m_showSystemsWindow(true)
        , m_historyOffset(0)
        , pEventSystem(nullptr)
        , pResourcesManager(nullptr)
        , mEntityCount(0)
        , windowWidth(1920)
        , windowHeight(1080)
        , m_selectedEntity(static_cast<Uma_ECS::Entity>(-1))
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

        // Padding adjustments (similar to Unity�s compact UI)
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
        pEventSystem->Subscribe<DebugLogEvent>([this](const DebugLogEvent& e) { AddConsoleLog(e.message); });
        pEventSystem->Subscribe<EntityCreatedEvent>([this](const EntityCreatedEvent& e) { mEntityCount = e.entityCnt; });
        pEventSystem->Subscribe<EntityDestroyedEvent>([this](const EntityDestroyedEvent& e) { mEntityCount = e.entityCnt; });
        pEventSystem->Subscribe<SceneInfoRequest>([this](const SceneInfoRequest& e)
            { sceneNames = e.sceneNames; scenePaths = e.scenePaths; activeSceneIndex = e.activeSceneIndex; });
        pEventSystem->Subscribe<IMGUIStopRequest>([this](const IMGUIStopRequest& e)
             { m_playState = PlayState::Stopped; });

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

        CreateDockspace();

        // play stop bar
        CreateEditorControlBar();

        SceneManagerWindow();

        // call for windows to be shown
        float currentFps = deltaTime > 0.0f ? (1.0f / deltaTime) : 0.0f;
        CreateDebugWindows(currentFps, deltaTime);

        fileBrowser.Render();

        resourcesWindow.Render();

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

            // scene view / game view indicator


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
                if (m_playState == PlayState::Stopped || m_playState == PlayState::Paused)
                {
                    pEventSystem->Emit<PlaySceneRequest>();
                    m_playState = PlayState::Playing;
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

            if (isPaused)
            {
                ImGui::PopStyleColor(3);
            }

            ImGui::SameLine();

            // Stop Button
            if (ImGui::Button("Stop", ImVec2(buttonWidth, 0)))
            {
                pEventSystem->Emit<StopSceneRequest>();
                m_playState = PlayState::Stopped;
                pEventSystem->Emit<ReLoadSceneRequestEvent>();
            }

            // Show current state text on the right
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

    void ImguiManager::CreateDebugWindows(float fps, float deltaTime)
    {
        if (!m_initialized)
            return;

        CreateEngineDebugWindow(fps, deltaTime);
        CreatePerformanceWindow();

        CreateHierarchyWindow();
        CreateInspectorWindow();

		CreateSystemsWindow();
		CreateEntityDebugWindow();
		CreateConsoleWindow();
		CreateEntityPropertyWindow();
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

    void ImguiManager::CreateEntityDebugWindow()
    {
        bool b = true;
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
        if (ImGui::Button("Spawn 2.5k in VP", { 160, 50 }))
        {
            pEventSystem->Emit<ShowEntityInVPRequestEvent>();
        }

        ImGui::End();
    }

    void ImguiManager::CreateEntityPropertyWindow()
    {
        bool b = true;
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
            ImGui::DockBuilderDockWindow("Hierarchy", dock_id_left);
            ImGui::DockBuilderDockWindow("Inspector", dock_id_right);
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

    // ========== HIERARCHY WINDOW IMPLEMENTATION ==========

    void ImguiManager::CreateHierarchyWindow()
    {
        bool b = true;
        ImGui::Begin("Hierarchy", &b);

        // Header with entity count
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
            m_selectedEntity = entity;
        }

        // Right-click context menu
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Create New"))
            {
                pEventSystem->Emit<SpawnEntityRequestEvent>();
            }

            if (ImGui::MenuItem("Create Child"))
            {
                Uma_ECS::Entity child = coordinator.CreateEntity();
                coordinator.AddComponent(child, Uma_ECS::Transform{
                    .position = Vec2(0, 0),
                    .rotation = Vec2(0, 0),
                    .scale = Vec2(1, 1)
                    });
                coordinator.SetParent(child, entity);
            }

            if (ImGui::MenuItem("Duplicate"))
            {
                Uma_ECS::Entity duplicate = coordinator.DuplicateEntity(entity);

                // If the entity had a parent, set the duplicate to have the same parent
                if (transform.parent.has_value())
                {
                    coordinator.SetParent(duplicate, transform.parent.value());
                }
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Delete"))
            {
                pEventSystem->Emit<DestroyEntityRequestEvent>(entity);
                if (m_selectedEntity == entity)
                {
                    m_selectedEntity = static_cast<Uma_ECS::Entity>(-1);
                }
            }

            if (ImGui::MenuItem("Delete with Children"))
            {
                coordinator.DestroyEntityAndChildren(entity);
                if (m_selectedEntity == entity)
                {
                    m_selectedEntity = static_cast<Uma_ECS::Entity>(-1);
                }
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

    std::string ImguiManager::GetEntityDisplayName(Uma_ECS::Entity entity, Uma_ECS::Coordinator& coordinator)
    {
        std::string name = "Entity " + std::to_string(entity);

        // Add component indicators
        if (coordinator.GetComponentArray<Uma_ECS::Player>().Has(entity))
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

                float position[2] = { transform.position.x, transform.position.y };
                if (ImGui::DragFloat2("Position", position, 0.1f))
                {
                    transform.position = Vec2(position[0], position[1]);
                    transform.isDirty = true;
                }

                float rotation = transform.rotation.x;
                if (ImGui::DragFloat("Rotation", &rotation, 1.0f))
                {
                    transform.rotation.x = rotation;
                    transform.isDirty = true;
                }

                float scale[2] = { transform.scale.x, transform.scale.y };
                if (ImGui::DragFloat2("Scale", scale, 0.01f))
                {
                    transform.scale = Vec2(scale[0], scale[1]);
                    transform.isDirty = true;
                }

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

                float velocity[2] = { rb.velocity.x, rb.velocity.y };
                if (ImGui::DragFloat2("Velocity", velocity, 0.1f))
                {
                    rb.velocity = Vec2(velocity[0], velocity[1]);
                }

                float acceleration[2] = { rb.acceleration.x, rb.acceleration.y };
                if (ImGui::DragFloat2("Acceleration", acceleration, 0.1f))
                {
                    rb.acceleration = Vec2(acceleration[0], acceleration[1]);
                }

                ImGui::DragFloat("Accel Strength", &rb.accel_strength, 0.1f, 0.0f, 1000.0f);
                ImGui::DragFloat("Friction Coefficient", &rb.fric_coeff, 0.01f, 0.0f, 10.0f);

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

                ImGui::Text("Texture: %s", sprite.textureName.c_str());

                // Texture name input
                static char textureBuffer[256];
                strncpy(textureBuffer, sprite.textureName.c_str(), 255);
                textureBuffer[255] = '\0';
                if (ImGui::InputText("Texture Name", textureBuffer, 256))
                {
                    sprite.textureName = textureBuffer;
                    sprite.texture = nullptr; // Will reload
                }

                ImGui::Checkbox("Flip X", &sprite.flipX);
                ImGui::Checkbox("Flip Y", &sprite.flipY);
                ImGui::Checkbox("Use Native Size", &sprite.UseNativeSize);

                // Render layer
                int renderLayer = static_cast<int>(sprite.renderLayer);
                if (ImGui::InputInt("Render Layer", &renderLayer))
                {
                    sprite.renderLayer = static_cast<Uma_ECS::LayerMask>(renderLayer);
                }

                ImGui::Separator();
                if (sprite.texture)
                {
                    ImGui::Text("Texture ID: %u", sprite.texture->tex_id);
                    ImGui::Text("Native Size: %.0f x %.0f", sprite.texture->GetNativeSize().x, sprite.texture->GetNativeSize().y);
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

                ImGui::Checkbox("Show Bounding Box", &collider.showBBox);

                ImGui::Separator();
                ImGui::Text("Default Settings");

                int defaultLayer = static_cast<int>(collider.defaultLayer);
                if (ImGui::InputInt("Default Layer", &defaultLayer))
                {
                    collider.defaultLayer = static_cast<Uma_ECS::LayerMask>(defaultLayer);
                }

                int defaultMask = static_cast<int>(collider.defaultMask);
                if (ImGui::InputInt("Default Mask", &defaultMask))
                {
                    collider.defaultMask = static_cast<Uma_ECS::LayerMask>(defaultMask);
                }

                ImGui::Separator();
                ImGui::Text("Shapes: %zu", collider.shapes.size());

                for (size_t i = 0; i < collider.shapes.size(); ++i)
                {
                    ImGui::PushID(static_cast<int>(i));

                    auto& shape = collider.shapes[i];

                    if (ImGui::TreeNode("Shape", "Shape %zu %s", i, shape.isActive ? "" : "(Inactive)"))
                    {
                        ImGui::Checkbox("Active", &shape.isActive);
                        ImGui::Checkbox("Auto Fit to Sprite", &shape.autoFitToSprite);

                        float size[2] = { shape.size.x, shape.size.y };
                        if (ImGui::DragFloat2("Size", size, 0.1f, 0.0f, 100.0f))
                        {
                            shape.size = Vec2(size[0], size[1]);
                        }

                        float offset[2] = { shape.offset.x, shape.offset.y };
                        if (ImGui::DragFloat2("Offset", offset, 0.1f))
                        {
                            shape.offset = Vec2(offset[0], offset[1]);
                        }

                        // Purpose dropdown
                        const char* purposes[] = { "Physics", "Environment", "Trigger" };
                        int currentPurpose = static_cast<int>(shape.purpose);
                        if (ImGui::Combo("Purpose", &currentPurpose, purposes, 3))
                        {
                            shape.purpose = static_cast<Uma_ECS::ColliderPurpose>(currentPurpose);
                        }

                        int layer = static_cast<int>(shape.layer);
                        if (ImGui::InputInt("Layer", &layer))
                        {
                            shape.layer = static_cast<Uma_ECS::LayerMask>(layer);
                        }

                        int mask = static_cast<int>(shape.colliderMask);
                        if (ImGui::InputInt("Collider Mask", &mask))
                        {
                            shape.colliderMask = static_cast<Uma_ECS::LayerMask>(mask);
                        }

                        if (ImGui::Button("Remove Shape"))
                        {
                            collider.shapes.erase(collider.shapes.begin() + i);
                            collider.bounds.resize(collider.shapes.size());
                            ImGui::TreePop();
                            ImGui::PopID();
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
                }

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::Camera>())
        {
            if (ImGui::CollapsingHeader("Camera"))
            {
                auto& camera = coordinator.GetComponent<Uma_ECS::Camera>(entity);
                ImGui::Indent();

                ImGui::DragFloat("Zoom", &camera.mZoom, 0.1f, 0.1f, 10.0f);
                ImGui::Checkbox("Follow Player", &camera.followPlayer);

                ImGui::Separator();
                ImGui::Text("Camera Controls");
                if (ImGui::Button("Reset Zoom"))
                {
                    camera.mZoom = 1.0f;
                }

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::Player>())
        {
            if (ImGui::CollapsingHeader("Player"))
            {
                auto& player = coordinator.GetComponent<Uma_ECS::Player>(entity);
                ImGui::Indent();

                ImGui::DragFloat("Speed", &player.mSpeed, 0.1f, 0.0f, 100.0f);

                ImGui::Separator();
                ImGui::Text("Player Tag Component");
                ImGui::TextDisabled("This entity is marked as the player");

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::Enemy>())
        {
            if (ImGui::CollapsingHeader("Enemy"))
            {
                auto& enemy = coordinator.GetComponent<Uma_ECS::Enemy>(entity);
                ImGui::Indent();

                ImGui::DragFloat("Speed", &enemy.mSpeed, 0.1f, 0.0f, 100.0f);

                ImGui::Separator();
                ImGui::Text("Enemy Tag Component");
                ImGui::TextDisabled("This entity is marked as an enemy");

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::Animator>())
        {
            if (ImGui::CollapsingHeader("Animator"))
            {
                auto& animator = coordinator.GetComponent<Uma_ECS::Animator>(entity);
                ImGui::Indent();

                ImGui::Checkbox("Auto Play", &animator.autoPlay);

                static char initialClipBuffer[128];
                strncpy(initialClipBuffer, animator.initialClip.c_str(), 127);
                initialClipBuffer[127] = '\0';
                if (ImGui::InputText("Initial Clip", initialClipBuffer, 128))
                {
                    animator.initialClip = initialClipBuffer;
                }

                ImGui::Separator();
                ImGui::Text("Current State");
                ImGui::Text("UV Offset: (%.3f, %.3f)", animator.uvOffset.x, animator.uvOffset.y);
                ImGui::Text("UV Size: (%.3f, %.3f)", animator.uvSize.x, animator.uvSize.y);

                ImGui::Separator();
                const auto& clips = animator.animator.GetClips();
                ImGui::Text("Animation Clips: %zu", clips.size());

                if (!clips.empty())
                {
                    for (const auto& [clipName, clip] : clips)
                    {
                        ImGui::PushID(clipName.c_str());

                        if (ImGui::Button(clipName.c_str(), ImVec2(150, 0)))
                        {
                            animator.animator.Play(clipName);
                        }

                        ImGui::SameLine();
                        ImGui::Text("Frames: %d, Speed: %.2f, Loop: %s",
                            clip.frameCount,
                            clip.speed,
                            clip.loop ? "Yes" : "No");

                        ImGui::PopID();
                    }
                }
                else
                {
                    ImGui::TextDisabled("No animation clips available");
                }

                ImGui::Unindent();
            }
        }
        else if (type == coordinator.GetComponentType<Uma_ECS::LuaScript>())
        {
            if (ImGui::CollapsingHeader("LuaScript"))
            {
                auto& luaScript = coordinator.GetComponent<Uma_ECS::LuaScript>(entity);
                ImGui::Indent();

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
                        ImGui::Checkbox("Enabled", &script.isEnabled);

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
                                        }
                                    }
                                    else
                                    {
                                        if (ImGui::DragFloat(var.name.c_str(), &val, 0.1f))
                                        {
                                            var.value = val;
                                            script.isVariableDirty = true;
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
                            luaScript.scripts.erase(luaScript.scripts.begin() + i);
                            ImGui::TreePop();
                            ImGui::PopID();
                            break;
                        }

                        ImGui::TreePop();
                    }

                    ImGui::PopID();
                }

                ImGui::Separator();

                if (ImGui::Button("Add Script", ImVec2(-1, 0)))
                {
                    ImGui::OpenPopup("AddScriptPopup");
                }
                if (ImGui::BeginPopup("AddScriptPopup"))
                {
                    // Texture name input
                    static char textureBuffer[256];
                    textureBuffer[255] = '\0';
                    if (ImGui::InputText("##Script Name", textureBuffer, 256))
                    {
                        mScriptName.clear();
                        mScriptName = textureBuffer;
                        mScriptName = "Assets/Scripts/" + mScriptName + ".lua";
                    }
                    ImGui::Separator();
                    if (FileBrowser::fileExists(mScriptName))
                    {
                        if (ImGui::Button("Add", ImVec2(-1, 0)))
                        {
                            luaScript.AddScript(mScriptName);
                            pEventSystem->Emit<CallLuaToInitScript>(entity);
                        }
                    }
                    else
                    {
                        float child_width = ImGui::GetWindowSize().x;
                        float text_width;
                        if (mScriptName == "Assets/Scripts/.lua")
                            text_width = ImGui::CalcTextSize("Enter Script Name").x;
                        else
                            text_width = ImGui::CalcTextSize("Script Not Found").x;

                        ImGui::SetCursorPosX((child_width - text_width) * 0.5f);
                        if (mScriptName == "Assets/Scripts/.lua")
                            ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Enter Script Name");
                        else
                            ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Script Not Found");
                    }
                    ImGui::EndPopup();
                }

                ImGui::Unindent();
            }
        }
        else
        {
            return false;
        }
        return true;
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
        std::string entityName = GetEntityDisplayName(m_selectedEntity, coordinator);
        ImGui::Text("Name: %s", entityName.c_str());

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

            ImGui::EndPopup();
        }
        ImGui::End();
    }

    void ImguiManager::SceneManagerWindow()
    {
        ImGui::Begin("Scene Manager");

        // Button to create a new scene
        if (ImGui::Button("Create New Scene")) {
            pEventSystem->Emit<StopSceneRequest>();
            pEventSystem->Emit<CreateNewSceneRequest>();
        }

        // Button to delete the selected scene
        if (ImGui::Button("Remove Current Scene"))
        {
            pEventSystem->Emit<StopSceneRequest>();
            pEventSystem->Emit<DeleteCurrSceneRequest>(sceneNames[activeSceneIndex]);
        }

        if (ImGui::Button("Save Current Scene"))
        {
            pEventSystem->Emit<SaveCurrSceneRequest>(sceneNames[activeSceneIndex]);
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
                pEventSystem->Emit<StopSceneRequest>();
                pEventSystem->Emit<LoadSceneRequestEvent>(sceneNames[i]);
            }
        }
        ImGui::EndChild();

        ImGui::End();
    }

    // In your ImGui editor code (probably in EditorLayer or similar)
}
