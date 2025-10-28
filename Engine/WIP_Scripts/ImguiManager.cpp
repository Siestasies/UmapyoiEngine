#include "ImguiManager.h"

//#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"

namespace Uma_Engine
{
	ImguiManager::ImguiManager()
        : m_initialized(false)
        , ds_initialized(false)
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
        io.IniFilename = "imgui.ini";

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsDark();

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

        // call for windows to be shown
        float currentFps = deltaTime > 0.0f ? (1.0f / deltaTime) : 0.0f;
        CreateDebugWindows(currentFps, deltaTime);

        fileBrowser.Render();

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

    void ImguiManager::CreateDebugWindows(float fps, float deltaTime)
    {
        if (!m_initialized)
            return;

        CreateEngineDebugWindow(fps, deltaTime);
        CreatePerformanceWindow();

        CreateHierarchyWindow();
        CreateInspectorWindow();

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

    void ImguiManager::CreateSystemsWindow()
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

    void ImguiManager::CreatePerformanceWindow()
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

    void ImguiManager::CreateEngineDebugWindow(float fps, float deltaTime)
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

    void ImguiManager::CreateSerializationDebugWindow()
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

    void ImguiManager::CreateEntityDebugWindow()
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
        if (ImGui::Button("Spawn 2.5k in VP", { 160, 50 }))
        {
            pEventSystem->Emit<ShowEntityInVPRequestEvent>();
        }

        ImGui::End();
    }

    void ImguiManager::CreateEntityPropertyWindow()
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

    void ImguiManager::CreateConsoleWindow()
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

        ImGui::Spacing();

        ImGui::End();
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

    void ImguiManager::AddConsoleLog(const std::string& message)
    {
        logsVec.push_back(message);

        // dont go beyond 100 messgaes shown
        if (logsVec.size() > 100)
            logsVec.erase(logsVec.begin());
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

    bool ImguiManager::windowsInit(const char* filename)
    {
        return std::ifstream(filename).good();
    }

    void ImguiManager::CreateHierarchyWindow()
    {
        bool b = true;
        ImGui::Begin("Hierarchy", &b);

        // Header with entity count
        ImGui::Text("Scene Entities: %d", mEntityCount);
        ImGui::Separator();

        // Refresh button to update entity list
        if (ImGui::Button("Refresh"))
        {
            // TODO: Call your ECS system to get all entities
            // Example: m_sceneEntities = pEntityManager->GetAllEntities();
        }

        ImGui::SameLine();

        // Create new entity button
        if (ImGui::Button("Create Game Object"))
        {
            // TODO: Create new entity through your ECS
            // Example: pEntityManager->CreateEntity();
        }

        ImGui::Separator();

        // Scrollable region for entity list
        ImGui::BeginChild("EntityList", ImVec2(0, 0), true);

        // Display all entities
        // TODO: Replace this with actual entity retrieval from your ECS
        for (int i = 0; i < mEntityCount; ++i)
        {
            // Generate unique ID for each selectable
            ImGui::PushID(i);

            // Create selectable item for each entity
            // TODO: Get actual entity name from your ECS
            std::string entityName = "Entity " + std::to_string(i);

            // Check if this entity is selected
            //bool isSelected = (m_selectedEntity.GetID() == i);  // Adjust based on your Entity implementation

            if (ImGui::Selectable(entityName.c_str(), false))
            {
                // TODO: Set selected entity from your ECS
                // m_selectedEntity = m_sceneEntities[i];
                //m_selectedEntity = Entity(i);  // Placeholder
            }

            // Right-click context menu
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Delete"))
                {
                    // TODO: Delete entity through your ECS
                    // pEntityManager->DestroyEntity(entity);
                }

                if (ImGui::MenuItem("Duplicate"))
                {
                    // TODO: Duplicate entity through your ECS
                    // pEntityManager->DuplicateEntity(entity);
                }

                ImGui::EndPopup();
            }

            ImGui::PopID();
        }

        ImGui::EndChild();
        ImGui::End();
    }

    void ImguiManager::CreateInspectorWindow()
    {
        bool b = true;
        ImGui::Begin("Inspector", &b);

        // Check if an entity is selected
        //if (!m_selectedEntity.IsValid())  // Adjust based on your Entity implementation
        //{
        //    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No entity selected");
        //    ImGui::End();
        //    return;
        //}

        // Display entity ID
        ImGui::Text("Entity ID: %d", /*m_selectedEntity.GetID()*/1);
        ImGui::Separator();

        // Entity Name Field
        static char entityName[128] = "Entity";
        ImGui::Text("Name:");
        ImGui::SameLine();
        if (ImGui::InputText("##EntityName", entityName, sizeof(entityName)))
        {
            // TODO: Update entity name in your ECS
        }

        ImGui::Separator();
        ImGui::Spacing();

        // ========== Transform Component ==========
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // TODO: Get actual transform component from entity
            static float position[3] = { 0.0f, 0.0f, 0.0f };
            static float rotation[3] = { 0.0f, 0.0f, 0.0f };
            static float scale[3] = { 1.0f, 1.0f, 1.0f };

            ImGui::Indent();

            ImGui::Text("Position");
            if (ImGui::DragFloat3("##Position", position, 0.1f))
            {
                // TODO: Update transform component
            }

            ImGui::Text("Rotation");
            if (ImGui::DragFloat3("##Rotation", rotation, 1.0f))
            {
                // TODO: Update transform component
            }

            ImGui::Text("Scale");
            if (ImGui::DragFloat3("##Scale", scale, 0.1f))
            {
                // TODO: Update transform component
            }

            ImGui::Unindent();
        }

        // ========== Add Component Button ==========
        ImGui::Spacing();
        ImGui::Separator();

        if (ImGui::Button("Add Component", ImVec2(-1, 0)))
        {
            ImGui::OpenPopup("AddComponentPopup");
        }

        // Add Component Popup
        if (ImGui::BeginPopup("AddComponentPopup"))
        {
            ImGui::Text("Select Component Type:");
            ImGui::Separator();

            if (ImGui::MenuItem("Mesh Renderer"))
            {
                // TODO: Add mesh renderer component
            }

            if (ImGui::MenuItem("Rigidbody"))
            {
                // TODO: Add rigidbody component
            }

            if (ImGui::MenuItem("Collider"))
            {
                // TODO: Add collider component
            }

            if (ImGui::MenuItem("Script"))
            {
                // TODO: Add script component
            }

            ImGui::EndPopup();
        }
        ImGui::End();
    }

}
