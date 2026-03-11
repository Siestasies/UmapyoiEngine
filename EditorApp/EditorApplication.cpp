/*!
\file   EditorApplication.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Implements the EditorApplication class, the specialized application layer
used when running the engine in Editor Mode. This class extends the base
Application system by registering editor-specific systems, setting up the
editor scene, and subscribing to editor/application events.

EditorApplication configures the SceneManager for editor behavior, registers
editor scripts, initializes the ImGui manager, and loads a default editor
scene used for development workflows.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "EditorApplication.h"

#include "Core/SystemManager.h"
#include "Core/EngineConfig.h"
#include "Scripts/EditorScript.h"
#include "WIP_Scripts/GameSceneScript.h"

// Systems
#include "Systems/SceneManager.h"
#include "Editor/Core/EditorSystem.h"
#include "Scripts/ImguiManager.h"
#include "../Editor/Systems/TilemapEditorManager.h"
#include "PlayFab/Core/PlayFabManager.h"

// imgui
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui_internal.h"

// Events
#include "Events/ApplicationEvents.h"

// playfab test WIP
//#include "PlayFab/Test Scripts/PlayFabRestTest.cpp"

#include "Core/EngineConfigSerializer.h"
#include "PlayFab/Core/PlayFabConfig.h"

namespace Uma_Engine
{
    /**
     * \brief Constructs the EditorApplication and initializes pointers.
     * The actual initialization work occurs in PreInit(), RegisterSystems(), and PostInit().
     */
    EditorApplication::EditorApplication()
        : Application()
        , mEditorSystem(nullptr)
        , mImguiManager(nullptr)
    {
    }

    /**
     * \brief Registers all systems required specifically for the editor.
     * This includes EditorSystem and ImguiManager, which provide UI and tool functionality.
     */
    void EditorApplication::RegisterSystems()
    {
        mEditorSystem = GetSystemManager()->RegisterSystem<EditorSystem>();
        mImguiManager = GetSystemManager()->RegisterSystem<ImguiManager>();
        mTilemapEditorManager = GetSystemManager()->RegisterSystem<TilemapEditorManager>();
    }

    /**
     * \brief Executed before engine initialization. Enables editor mode.
     */
    void EditorApplication::PreInit()
    {
        SetIsEditor(true);
    }

    void EditorApplication::PreUpdate(float dt)
    {
        (void)dt;
        // Imgui to make new frame
        ImguiStartFrame();
    }

    /**
     * \brief Subscribes editor-related events such as quit requests.
     */
    void EditorApplication::SubscribeEvents()
    {
        EventSystem* eventSystem = GetEventSystem();

        eventSystem->Subscribe<Uma_Engine::ApplicationQuitRequest, Application>(
            [this](const ApplicationQuitRequest&)
            {
                GetWindow()->Close();
            });
    }

    void EditorApplication::SetupImguiStyle()
    {
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
        ImGui_ImplGlfw_InitForOpenGL(mGraphics->GetWindow(), true);
        const char* glsl_version = "#version 130";
        ImGui_ImplOpenGL3_Init(glsl_version);
    }

    /**
     * \brief Executed after engine initialization.
     * Sets up the editor environment, loads the default editor scene,
     * and registers editor/game scripts to the SceneManager.
     */
    void EditorApplication::PostInit()
    {
        SubscribeEvents();

        GetWindow()->SetWindowMode(WindowMode::Maximized);

        SceneManager* sceneManager = GetSceneManager();
        sceneManager->SetEditorMode(true);

        // Set up Imgui
        SetupImguiStyle();

        // Register script classes for use in the editor scene
        sceneManager->RegisterScript<GameSceneScript>("GameBehaviour");
        sceneManager->RegisterScript<EditorScript>("EditorBehaviour");

        // Create the editor scene and configure it
        auto editorScene = sceneManager->CreateScene("tutorial.scn", "tutorial.scn");
        editorScene->g_EngineConfig = *GetConfig();

        sceneManager->AttachScriptToScene("tutorial.scn", "GameBehaviour");
        sceneManager->AttachScriptToScene("tutorial.scn", "EditorBehaviour");

        // Load the default scene
        sceneManager->LoadScene("tutorial.scn");

        // load and configure playfab
        PlayFabConfiguration();
    }

    bool EditorApplication::HandleInterruptions(float deltaTime)
    {
        (void)deltaTime;
        // Editor mode: Only reset input when not focused, but continue rendering
        bool isFocused = glfwGetWindowAttrib(GetGLFWWindow(), GLFW_FOCUSED);

        if (!isFocused && mWasFocused)
        {
            // Lost focus - reset input only
            if (GetInputSystem())
            {
                GetInputSystem()->ResetAllInput();
            }
            mWasFocused = false;
        }
        else if (isFocused && !mWasFocused)
        {
            // Regained focus
            mWasFocused = true;
        }

        // Editor continues rendering even when unfocused
        return true;
    }


    void EditorApplication::PostUpdate(float dt)
    {
        (void)dt;
        if (Uma_Engine::HybridInputSystem::KeyPressed(GLFW_KEY_ESCAPE))
        {
            GamePause() = !GamePause();
            GetEventSystem()->Emit<ApplicationGamePauseRequest>(GamePause());
        }

        ImguiRender();
    }

    // IMGUI SPECIFIC METHODS
    void EditorApplication::ImguiStartFrame()
    {
        // start imgui fram
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void EditorApplication::ImguiRender()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void EditorApplication::PlayFabConfiguration()
    {
        // Checks for a playfab config file. If previously set up, auto-establishes
        // connection. If no config file is found, the function is skipped.
        PlayFabConfig* config = new PlayFabConfig{};  // deleted after SetCredentials below
        EngineConfigSerializer configSerializer;
        configSerializer.Register(config);
        configSerializer.load(Uma_FilePath::CONFIG_ROOT + "playfab_dev.json");
        if (config->titleId.empty()) return;

        PlayFabManager* playfabManager = GetPlayFabManager();
        playfabManager->SetCredentials(
            config->titleId,
            config->secretKey,
            [playfabManager]()
            {
                auto& admin = playfabManager->Admin();

                // ── helpers ──────────────────────────────────────────────────────
                auto onFail = [](HRESULT hr, const std::string& msg)
                    {
                        Debugger::Log(WarningLevel::eWarning, msg);
                    };

                // ================================================================
                //  1. TitleData  (existing smoke-test, kept for reference)
                // ================================================================
                admin.GetTitleData(
                    { "game version" },
                    [](const std::unordered_map<std::string, std::string>& data)
                    {
                        Debugger::Log(WarningLevel::eInfo,
                            "[PF | GetTitleData] game version = " + data.at("game version"));
                    },
                    onFail
                );

                admin.SetTitleData(
                    "test_key", "hello_uma",
                    []() { Debugger::Log(WarningLevel::eInfo, "[PF | SetTitleData] OK"); },
                    onFail
                );

                admin.SetTitleDataBatch(
                    { { "batch_a", "1" }, { "batch_b", "2" } },
                    []() { Debugger::Log(WarningLevel::eInfo, "[PF | SetTitleDataBatch] OK"); },
                    onFail
                );

                admin.DeleteTitleData(
                    "test_key",
                    []() { Debugger::Log(WarningLevel::eInfo, "[PF | DeleteTitleData] OK"); },
                    onFail
                );


                // ================================================================
                //  2. Statistics
                // ================================================================

                //// -- 2a. Create a stat definition --------------------------------
                //admin.CreateStatisticDefinition(
                //    "TestHighScore",
                //    "score",                                        // column name — distinct from stat name
                //    PFStatisticsStatisticAggregationMethod::Max,
                //    [&admin, onFail]()
                //    {
                //        Debugger::Log(WarningLevel::eInfo,
                //            "[PF | CreateStatisticDefinition] TestHighScore created");

                //        // -- 2b. Write a value for a dummy entity ----------------
                //        // Replace the entityId below with a real title_player_account
                //        // ID from your PlayFab Game Manager → Players tab.
                //        const std::string testEntityId = "REPLACE_WITH_REAL_ENTITY_ID";
                //        const std::string testEntityType = "title_player_account";

                //        admin.UpdateStatistics(
                //            testEntityId, testEntityType,
                //            "TestHighScore", "99000",
                //            [&admin, testEntityId, testEntityType, onFail]()
                //            {
                //                Debugger::Log(WarningLevel::eInfo,
                //                    "[PF | UpdateStatistics] wrote 99000 to TestHighScore");

                //                // -- 2c. Read back the value ---------------------
                //                admin.GetStatistics(
                //                    testEntityId, testEntityType,
                //                    [](const std::vector<Uma_Engine::StatisticValue>& stats)
                //                    {
                //                        for (auto const& s : stats)
                //                        {
                //                            std::string scores;
                //                            for (auto const& sc : s.scores)
                //                                scores += sc + " ";
                //                            Debugger::Log(WarningLevel::eInfo,
                //                                "[PF | GetStatistics] " + s.name
                //                                + " v" + std::to_string(s.version)
                //                                + " = " + scores);
                //                        }
                //                    },
                //                    onFail
                //                );
                //            },
                //            onFail
                //        );
                //    },
                //    onFail
                //);

                //// -- 2d. List all stat definitions --------------------------------
                //admin.ListStatisticDefinitions(
                //    [](const std::vector<std::string>& names)
                //    {
                //        for (auto const& n : names)
                //            Debugger::Log(WarningLevel::eInfo,
                //                "[PF | ListStatisticDefinitions] " + n);
                //    },
                //    onFail
                //);

                // -- 2e. Increment stat version (season reset) -------------------
                //        Uncomment when you actually want to roll the version.
                //        Doing it here would wipe your TestHighScore data above.
                //
                // admin.IncrementStatisticVersion(
                //     "TestHighScore",
                //     [](uint32_t v)
                //     {
                //         Debugger::Log(WarningLevel::eInfo,
                //             "[PF | IncrementStatisticVersion] new version = "
                //             + std::to_string(v));
                //     },
                //     onFail
                // );


                // ================================================================
                //  3. Leaderboards
                // ================================================================

                // -- 3a. Create a leaderboard definition -------------------------
                //admin.CreateLeaderboardDefinition(
                //    "TestLeaderboard", "Score",
                //    PFLeaderboardsLeaderboardSortDirection::Descending,
                //    100,
                //    [&admin, onFail]()
                //    {
                //        Debugger::Log(WarningLevel::eInfo,
                //            "[PF | CreateLeaderboardDefinition] TestLeaderboard created");

                //        // -- 3b. Server-side write an entry ----------------------
                //        const std::string testEntityId = "REPLACE_WITH_REAL_ENTITY_ID";

                //        admin.UpdateLeaderboardEntries(
                //            "TestLeaderboard", testEntityId, "88000", "test_meta",
                //            [&admin, testEntityId, onFail]()
                //            {
                //                Debugger::Log(WarningLevel::eInfo,
                //                    "[PF | UpdateLeaderboardEntries] wrote 88000");

                //                // -- 3c. Read top 10 -----------------------------
                //                admin.GetLeaderboard(
                //                    "TestLeaderboard", 10, nullptr,
                //                    [](const std::vector<Uma_Engine::LeaderboardEntry>& entries,
                //                        uint32_t version)
                //                    {
                //                        Debugger::Log(WarningLevel::eInfo,
                //                            "[PF | GetLeaderboard] version "
                //                            + std::to_string(version)
                //                            + ", entries: "
                //                            + std::to_string(entries.size()));

                //                        for (auto const& e : entries)
                //                        {
                //                            std::string score = e.scores.empty()
                //                                ? "?" : e.scores[0];
                //                            Debugger::Log(WarningLevel::eInfo,
                //                                "  #" + std::to_string(e.rank)
                //                                + " [" + e.entityId + "] "
                //                                + e.displayName
                //                                + " — " + score);
                //                        }
                //                    },
                //                    onFail
                //                );

                //                // -- 3d. Read entries around the test entity -----
                //                admin.GetLeaderboardAroundEntity(
                //                    "TestLeaderboard",
                //                    testEntityId, "title_player_account",
                //                    5,
                //                    [](const std::vector<Uma_Engine::LeaderboardEntry>& entries,
                //                        uint32_t version)
                //                    {
                //                        Debugger::Log(WarningLevel::eInfo,
                //                            "[PF | GetLeaderboardAroundEntity] "
                //                            + std::to_string(entries.size()) + " entries");

                //                        for (auto const& e : entries)
                //                            Debugger::Log(WarningLevel::eInfo,
                //                                "  #" + std::to_string(e.rank)
                //                                + " " + e.entityId);
                //                    },
                //                    onFail
                //                );
                //            },
                //            onFail
                //        );
                //    },
                //    onFail
                //);

                //// -- 3e. List all leaderboard definitions ------------------------
                //admin.ListLeaderboardDefinitions(
                //    [](const std::vector<std::string>& names)
                //    {
                //        for (auto const& n : names)
                //            Debugger::Log(WarningLevel::eInfo,
                //                "[PF | ListLeaderboardDefinitions] " + n);
                //    },
                //    onFail
                //);

                // -- 3f. Increment leaderboard version (season reset) ------------
                //        Same caution as IncrementStatisticVersion — destructive.
                //        Uncomment only when intentionally rolling a new season.
                //
                // admin.IncrementLeaderboardVersion(
                //     "TestLeaderboard",
                //     [](uint32_t v)
                //     {
                //         Debugger::Log(WarningLevel::eInfo,
                //             "[PF | IncrementLeaderboardVersion] new version = "
                //             + std::to_string(v));
                //     },
                //     onFail
                // );


                // ================================================================
                //  4. CloudScript / Azure Functions
                // ================================================================

                // -- 4a. Call a registered Azure Function ------------------------
                //        "GetSessionKey" is the function name you register in
                //        PlayFab Game Manager → Automation → Functions.
                //        Replace with your actual function name and argument JSON.
                //admin.ExecuteFunction(
                //    "GetSessionKey",
                //    R"({"reason":"test"})",
                //    [](const std::string& fnName, const std::string& resultJson)
                //    {
                //        Debugger::Log(WarningLevel::eInfo,
                //            "[PF | ExecuteFunction] " + fnName
                //            + " returned: " + resultJson);
                //    },
                //    onFail
                //);

                //// -- 4b. Call SubmitScore via Azure Function ----------------------
                ////        This mirrors the flow your game client will use, but
                ////        invoked here from admin context for testing.
                //admin.ExecuteFunction(
                //    "SubmitScore",
                //    R"({"score":12345,"hmac":"REPLACE_WITH_REAL_HMAC"})",
                //    [](const std::string& fnName, const std::string& resultJson)
                //    {
                //        Debugger::Log(WarningLevel::eInfo,
                //            "[PF | ExecuteFunction] " + fnName
                //            + " returned: " + resultJson);
                //    },
                //    onFail
                //);
            },
            nullptr  // onAuthFailure
        );

        playfabManager->Init();
        delete config;
    }
}
