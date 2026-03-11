// Temporary test — delete before shipping
#include <httpClient/httpClient.h>
#include <XAsync.h>
#include <XTaskQueue.h>
#include <playfab/core/PFAuthentication.h>
#include <playfab/core/PFServiceConfig.h>
#include <playfab/services/PFTitleDataManagement.h>
#include <playfab/services/PFTitleDataManagementTypes.h>
#include <playfab/services/PFServices.h>
#include "Debugging/Debugger.hpp"
#include <string>
#include <sstream>

static const char* TITLE_ID = "FF2A5";
static const char* SECRET_KEY = "8O6GRYXHKMF1MFK677WUYUMTIC11PPHIO8EHYHXTD39HO4ARZC";
static const char* ENDPOINT = "https://FF2A5.playfabapi.com";

namespace Uma_Engine
{
    struct TestContext
    {
        XTaskQueueHandle      taskQueue{ nullptr };
        PFServiceConfigHandle serviceConfig{ nullptr };
        PFEntityHandle        entityHandle{ nullptr };
        bool                  done{ false };
    };

    static void DoSetTitleData(TestContext* ctx)
    {
        struct SetContext
        {
            TestContext* test;
            XAsyncBlock  async{};
        };
        auto* sc = new SetContext{ ctx };
        sc->async.queue = ctx->taskQueue;
        sc->async.context = sc;
        sc->async.callback = [](XAsyncBlock* ab)
            {
                auto* sc = static_cast<SetContext*>(ab->context);
                HRESULT hr = XAsyncGetStatus(ab, false);
                if (SUCCEEDED(hr))
                    Debugger::Log(WarningLevel::eInfo, "[PlayFab] ServerSetTitleData OK");
                else
                {
                    std::ostringstream oss;
                    oss << "[PlayFab] ServerSetTitleData FAILED: 0x" << std::hex << hr;
                    Debugger::Log(WarningLevel::eError, oss.str());
                }
                sc->test->done = true;
                delete sc;
            };

        PFTitleDataManagementSetTitleDataRequest req{};
        req.key = "game version";
        req.value = "HelloFromUmapyoi";

        HRESULT hr = PFTitleDataManagementServerSetTitleDataAsync(
            ctx->entityHandle,
            &req,
            &sc->async);

        if (FAILED(hr))
        {
            std::ostringstream oss;
            oss << "[PlayFab] ServerSetTitleDataAsync dispatch FAILED: 0x" << std::hex << hr;
            Debugger::Log(WarningLevel::eError, oss.str());
            ctx->done = true;
            delete sc;
        }
    }

    void TestSetTitleData()
    {
        auto* ctx = new TestContext{};

        // 0 — Initialize PlayFab Services (must be first)
        HRESULT hr = PFServicesInitialize(nullptr);
        if (FAILED(hr))
        {
            std::ostringstream oss;
            oss << "[PlayFab] PFServicesInitialize FAILED: 0x" << std::hex << hr;
            Debugger::Log(WarningLevel::eError, oss.str());
            delete ctx; return;
        }
        Debugger::Log(WarningLevel::eInfo, "[PlayFab] PFServicesInitialize OK");

        // 1 — Task queue
        hr = XTaskQueueCreate(
            XTaskQueueDispatchMode::Manual,
            XTaskQueueDispatchMode::Manual,
            &ctx->taskQueue);
        if (FAILED(hr))
        {
            std::ostringstream oss;
            oss << "[PlayFab] XTaskQueueCreate FAILED: 0x" << std::hex << hr;
            Debugger::Log(WarningLevel::eError, oss.str());
            delete ctx; return;
        }
        Debugger::Log(WarningLevel::eInfo, "[PlayFab] XTaskQueueCreate OK");

        // 2 — Service config
        hr = PFServiceConfigCreateHandle(ENDPOINT, TITLE_ID, &ctx->serviceConfig);
        if (FAILED(hr))
        {
            std::ostringstream oss;
            oss << "[PlayFab] PFServiceConfigCreateHandle FAILED: 0x" << std::hex << hr;
            Debugger::Log(WarningLevel::eError, oss.str());
            delete ctx; return;
        }
        Debugger::Log(WarningLevel::eInfo, "[PlayFab] PFServiceConfigCreateHandle OK");

        // 3 — SecretKey auth
        struct AuthContext
        {
            TestContext* test;
            XAsyncBlock  async{};
        };
        auto* ac = new AuthContext{ ctx };
        ac->async.queue = ctx->taskQueue;
        ac->async.context = ac;
        ac->async.callback = [](XAsyncBlock* ab)
            {
                auto* ac = static_cast<AuthContext*>(ab->context);
                HRESULT hr = PFAuthenticationGetEntityWithSecretKeyGetResult(ab, &ac->test->entityHandle);
                if (SUCCEEDED(hr))
                {
                    Debugger::Log(WarningLevel::eInfo, "[PlayFab] SecretKey auth OK — got entity handle");
                    DoSetTitleData(ac->test);
                }
                else
                {
                    std::ostringstream oss;
                    oss << "[PlayFab] SecretKey auth FAILED: 0x" << std::hex << hr;
                    Debugger::Log(WarningLevel::eError, oss.str());
                    ac->test->done = true;
                }
                delete ac;
            };

        PFAuthenticationGetEntityRequest req{};
        hr = PFAuthenticationGetEntityWithSecretKeyAsync(
            ctx->serviceConfig, SECRET_KEY, &req, &ac->async);
        if (FAILED(hr))
        {
            std::ostringstream oss;
            oss << "[PlayFab] GetEntityWithSecretKeyAsync dispatch FAILED: 0x" << std::hex << hr;
            Debugger::Log(WarningLevel::eError, oss.str());
            delete ac; delete ctx; return;
        }

        // 4 — Pump until done
        while (!ctx->done)
        {
            XTaskQueueDispatch(ctx->taskQueue, XTaskQueuePort::Work, 0);
            XTaskQueueDispatch(ctx->taskQueue, XTaskQueuePort::Completion, 0);
        }

        // 5 — Cleanup
        if (ctx->entityHandle) PFEntityCloseHandle(ctx->entityHandle);
        PFServiceConfigCloseHandle(ctx->serviceConfig);
        XTaskQueueCloseHandle(ctx->taskQueue);
        delete ctx;

        Debugger::Log(WarningLevel::eInfo, "[PlayFab] Test done");
    }

    // This was the test script to test for the playfabadminmanager features 
    // and these are working
    // im leaving these code here for references if u need to use the functions and u are lost
    
    //void EditorApplication::PlayFabConfiguration()
    //{
    //    // Checks for a playfab config file. If previously set up, auto-establishes
    //    // connection. If no config file is found, the function is skipped.
    //    PlayFabConfig* config = new PlayFabConfig{};  // deleted after SetCredentials below
    //    EngineConfigSerializer configSerializer;
    //    configSerializer.Register(config);
    //    configSerializer.load(Uma_FilePath::CONFIG_ROOT + "playfab_dev.json");
    //    if (config->titleId.empty()) return;

    //    PlayFabManager* playfabManager = GetPlayFabManager();
    //    playfabManager->SetCredentials(
    //        config->titleId,
    //        config->secretKey,
    //        [playfabManager]()
    //        {
    //            auto& admin = playfabManager->Admin();

    //            // ── helpers ──────────────────────────────────────────────────────
    //            auto onFail = [](HRESULT hr, const std::string& msg)
    //                {
    //                    Debugger::Log(WarningLevel::eWarning, msg);
    //                };

    //            // ================================================================
    //            //  1. TitleData  (existing smoke-test, kept for reference)
    //            // ================================================================
    //            admin.GetTitleData(
    //                { "game version" },
    //                [](const std::unordered_map<std::string, std::string>& data)
    //                {
    //                    Debugger::Log(WarningLevel::eInfo,
    //                        "[PF | GetTitleData] game version = " + data.at("game version"));
    //                },
    //                onFail
    //            );

    //            admin.SetTitleData(
    //                "test_key", "hello_uma",
    //                []() { Debugger::Log(WarningLevel::eInfo, "[PF | SetTitleData] OK"); },
    //                onFail
    //            );

    //            admin.SetTitleDataBatch(
    //                { { "batch_a", "1" }, { "batch_b", "2" } },
    //                []() { Debugger::Log(WarningLevel::eInfo, "[PF | SetTitleDataBatch] OK"); },
    //                onFail
    //            );

    //            admin.DeleteTitleData(
    //                "test_key",
    //                []() { Debugger::Log(WarningLevel::eInfo, "[PF | DeleteTitleData] OK"); },
    //                onFail
    //            );


    //            // ================================================================
    //            //  2. Statistics
    //            // ================================================================

    //            //// -- 2a. Create a stat definition --------------------------------
    //            //admin.CreateStatisticDefinition(
    //            //    "TestHighScore",
    //            //    "score",                                        // column name — distinct from stat name
    //            //    PFStatisticsStatisticAggregationMethod::Max,
    //            //    [&admin, onFail]()
    //            //    {
    //            //        Debugger::Log(WarningLevel::eInfo,
    //            //            "[PF | CreateStatisticDefinition] TestHighScore created");

    //            //        // -- 2b. Write a value for a dummy entity ----------------
    //            //        // Replace the entityId below with a real title_player_account
    //            //        // ID from your PlayFab Game Manager → Players tab.
    //            //        const std::string testEntityId = "REPLACE_WITH_REAL_ENTITY_ID";
    //            //        const std::string testEntityType = "title_player_account";

    //            //        admin.UpdateStatistics(
    //            //            testEntityId, testEntityType,
    //            //            "TestHighScore", "99000",
    //            //            [&admin, testEntityId, testEntityType, onFail]()
    //            //            {
    //            //                Debugger::Log(WarningLevel::eInfo,
    //            //                    "[PF | UpdateStatistics] wrote 99000 to TestHighScore");

    //            //                // -- 2c. Read back the value ---------------------
    //            //                admin.GetStatistics(
    //            //                    testEntityId, testEntityType,
    //            //                    [](const std::vector<Uma_Engine::StatisticValue>& stats)
    //            //                    {
    //            //                        for (auto const& s : stats)
    //            //                        {
    //            //                            std::string scores;
    //            //                            for (auto const& sc : s.scores)
    //            //                                scores += sc + " ";
    //            //                            Debugger::Log(WarningLevel::eInfo,
    //            //                                "[PF | GetStatistics] " + s.name
    //            //                                + " v" + std::to_string(s.version)
    //            //                                + " = " + scores);
    //            //                        }
    //            //                    },
    //            //                    onFail
    //            //                );
    //            //            },
    //            //            onFail
    //            //        );
    //            //    },
    //            //    onFail
    //            //);

    //            //// -- 2d. List all stat definitions --------------------------------
    //            //admin.ListStatisticDefinitions(
    //            //    [](const std::vector<std::string>& names)
    //            //    {
    //            //        for (auto const& n : names)
    //            //            Debugger::Log(WarningLevel::eInfo,
    //            //                "[PF | ListStatisticDefinitions] " + n);
    //            //    },
    //            //    onFail
    //            //);

    //            // -- 2e. Increment stat version (season reset) -------------------
    //            //        Uncomment when you actually want to roll the version.
    //            //        Doing it here would wipe your TestHighScore data above.
    //            //
    //            // admin.IncrementStatisticVersion(
    //            //     "TestHighScore",
    //            //     [](uint32_t v)
    //            //     {
    //            //         Debugger::Log(WarningLevel::eInfo,
    //            //             "[PF | IncrementStatisticVersion] new version = "
    //            //             + std::to_string(v));
    //            //     },
    //            //     onFail
    //            // );


    //            // ================================================================
    //            //  3. Leaderboards
    //            // ================================================================

    //            // -- 3a. Create a leaderboard definition -------------------------
    //            //admin.CreateLeaderboardDefinition(
    //            //    "TestLeaderboard", "Score",
    //            //    PFLeaderboardsLeaderboardSortDirection::Descending,
    //            //    100,
    //            //    [&admin, onFail]()
    //            //    {
    //            //        Debugger::Log(WarningLevel::eInfo,
    //            //            "[PF | CreateLeaderboardDefinition] TestLeaderboard created");

    //            //        // -- 3b. Server-side write an entry ----------------------
    //            //        const std::string testEntityId = "REPLACE_WITH_REAL_ENTITY_ID";

    //            //        admin.UpdateLeaderboardEntries(
    //            //            "TestLeaderboard", testEntityId, "88000", "test_meta",
    //            //            [&admin, testEntityId, onFail]()
    //            //            {
    //            //                Debugger::Log(WarningLevel::eInfo,
    //            //                    "[PF | UpdateLeaderboardEntries] wrote 88000");

    //            //                // -- 3c. Read top 10 -----------------------------
    //            //                admin.GetLeaderboard(
    //            //                    "TestLeaderboard", 10, nullptr,
    //            //                    [](const std::vector<Uma_Engine::LeaderboardEntry>& entries,
    //            //                        uint32_t version)
    //            //                    {
    //            //                        Debugger::Log(WarningLevel::eInfo,
    //            //                            "[PF | GetLeaderboard] version "
    //            //                            + std::to_string(version)
    //            //                            + ", entries: "
    //            //                            + std::to_string(entries.size()));

    //            //                        for (auto const& e : entries)
    //            //                        {
    //            //                            std::string score = e.scores.empty()
    //            //                                ? "?" : e.scores[0];
    //            //                            Debugger::Log(WarningLevel::eInfo,
    //            //                                "  #" + std::to_string(e.rank)
    //            //                                + " [" + e.entityId + "] "
    //            //                                + e.displayName
    //            //                                + " — " + score);
    //            //                        }
    //            //                    },
    //            //                    onFail
    //            //                );

    //            //                // -- 3d. Read entries around the test entity -----
    //            //                admin.GetLeaderboardAroundEntity(
    //            //                    "TestLeaderboard",
    //            //                    testEntityId, "title_player_account",
    //            //                    5,
    //            //                    [](const std::vector<Uma_Engine::LeaderboardEntry>& entries,
    //            //                        uint32_t version)
    //            //                    {
    //            //                        Debugger::Log(WarningLevel::eInfo,
    //            //                            "[PF | GetLeaderboardAroundEntity] "
    //            //                            + std::to_string(entries.size()) + " entries");

    //            //                        for (auto const& e : entries)
    //            //                            Debugger::Log(WarningLevel::eInfo,
    //            //                                "  #" + std::to_string(e.rank)
    //            //                                + " " + e.entityId);
    //            //                    },
    //            //                    onFail
    //            //                );
    //            //            },
    //            //            onFail
    //            //        );
    //            //    },
    //            //    onFail
    //            //);

    //            //// -- 3e. List all leaderboard definitions ------------------------
    //            //admin.ListLeaderboardDefinitions(
    //            //    [](const std::vector<std::string>& names)
    //            //    {
    //            //        for (auto const& n : names)
    //            //            Debugger::Log(WarningLevel::eInfo,
    //            //                "[PF | ListLeaderboardDefinitions] " + n);
    //            //    },
    //            //    onFail
    //            //);

    //            // -- 3f. Increment leaderboard version (season reset) ------------
    //            //        Same caution as IncrementStatisticVersion — destructive.
    //            //        Uncomment only when intentionally rolling a new season.
    //            //
    //            // admin.IncrementLeaderboardVersion(
    //            //     "TestLeaderboard",
    //            //     [](uint32_t v)
    //            //     {
    //            //         Debugger::Log(WarningLevel::eInfo,
    //            //             "[PF | IncrementLeaderboardVersion] new version = "
    //            //             + std::to_string(v));
    //            //     },
    //            //     onFail
    //            // );


    //            // ================================================================
    //            //  4. CloudScript / Azure Functions
    //            // ================================================================

    //            // -- 4a. Call a registered Azure Function ------------------------
    //            //        "GetSessionKey" is the function name you register in
    //            //        PlayFab Game Manager → Automation → Functions.
    //            //        Replace with your actual function name and argument JSON.
    //            //admin.ExecuteFunction(
    //            //    "GetSessionKey",
    //            //    R"({"reason":"test"})",
    //            //    [](const std::string& fnName, const std::string& resultJson)
    //            //    {
    //            //        Debugger::Log(WarningLevel::eInfo,
    //            //            "[PF | ExecuteFunction] " + fnName
    //            //            + " returned: " + resultJson);
    //            //    },
    //            //    onFail
    //            //);

    //            //// -- 4b. Call SubmitScore via Azure Function ----------------------
    //            ////        This mirrors the flow your game client will use, but
    //            ////        invoked here from admin context for testing.
    //            //admin.ExecuteFunction(
    //            //    "SubmitScore",
    //            //    R"({"score":12345,"hmac":"REPLACE_WITH_REAL_HMAC"})",
    //            //    [](const std::string& fnName, const std::string& resultJson)
    //            //    {
    //            //        Debugger::Log(WarningLevel::eInfo,
    //            //            "[PF | ExecuteFunction] " + fnName
    //            //            + " returned: " + resultJson);
    //            //    },
    //            //    onFail
    //            //);
    //        },
    //        nullptr  // onAuthFailure
    //    );

    //    playfabManager->Init();
    //    delete config;
    //}

}