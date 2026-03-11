/*!
\file   PlayFabPlayerTest.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Integration test for PlayFabPlayerManager — exercises every public method
via an already-initialised PlayFabManager.

=== How to use ===

    Call from your EditorApplication after PlayFabManager is initialised:

        PlayFabManager* pfm = GetPlayFabManager();
        Uma_Engine::TestPlayerManager(pfm);

    The function is non-blocking — it fires LoginWithCustomID and all
    subsequent tests run inside async callbacks on SDK background threads.
    Results are logged via Debugger::Log.

=== Prerequisites ===

    - PlayFabManager must be initialised (IsReady() == true)
    - A statistic definition "TestHighScore" must exist in PlayFab Game Manager
    - A leaderboard definition "TestLeaderboard" must exist in PlayFab Game Manager
    - If these don't exist, those specific tests will log errors but won't crash

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "../Core/PlayFabManager.h"
#include "Debugging/Debugger.hpp"

#include <string>
#include <sstream>

namespace Uma_Engine
{
    // This was the test script to test for the playfabplayermanager features
    // and these are working
    // im leaving these code here for references if u need to use the functions and u are lost

    //void EditorApplication::TestPlayerManagerConfiguration()
    //{
    //    PlayFabManager* playfabManager = GetPlayFabManager();
    //    TestPlayerManager(playfabManager);
    //}

    void TestPlayerManager(PlayFabManager* playfabManager)
    {
        if (!playfabManager || !playfabManager->IsReady())
        {
            Debugger::Log(WarningLevel::eError,
                "[PlayerTest] PlayFabManager is not ready — cannot run tests");
            return;
        }

        Debugger::Log(WarningLevel::eInfo,
            "=== PlayFabPlayerManager Integration Test ===");

        // -- Login with Custom ID, then fire all tests in the callback --------
        playfabManager->LoginWithCustomID(
            "UmaTestPlayer_CustomID_002",
            true,   // createAccount
            [playfabManager]()
            {
                Debugger::Log(WarningLevel::eInfo,
                    "[PlayerTest] LoginWithCustomID OK — player logged in");

                auto& player = playfabManager->Player();

                // ── helpers ──────────────────────────────────────────────────
                auto onFail = [](HRESULT, const std::string& msg)
                    {
                        Debugger::Log(WarningLevel::eWarning, msg);
                    };

                // =============================================================
                //  1. Account — GetAccountInfo
                // =============================================================
                player.GetAccountInfo(
                    [](const std::unordered_map<std::string, std::string>& info)
                    {
                        std::string out = "[PlayerTest | GetAccountInfo] OK —";
                        for (auto const& [k, v] : info)
                            out += " " + k + "=\"" + v + "\"";
                        Debugger::Log(WarningLevel::eInfo, out);
                    },
                    onFail
                );

                // =============================================================
                //  2. Account — SetDisplayName
                // =============================================================
                player.SetDisplayName(
                    "UmaTestPlayer1",
                    []()
                    {
                        Debugger::Log(WarningLevel::eInfo,
                            "[PlayerTest | SetDisplayName] OK — name set to \"UmaTestPlayer\"");
                    },
                    onFail
                );

                // =============================================================
                //  3. Player Data — WriteData then ReadData (chained)
                // =============================================================
                player.WriteData(
                    "test_key", "hello_from_uma",
                    [playfabManager, onFail]()
                    {
                        Debugger::Log(WarningLevel::eInfo,
                            "[PlayerTest | WriteData] OK — wrote test_key = \"hello_from_uma\"");

                        // Read it back
                        playfabManager->Player().ReadData(
                            "test_key",
                            [](const std::string& value)
                            {
                                Debugger::Log(WarningLevel::eInfo,
                                    "[PlayerTest | ReadData] OK — test_key = \"" + value + "\"");
                            },
                            onFail
                        );
                    },
                    onFail
                );
                player.WriteData(
                    "test_key", "hello_from_uma",
                    [playfabManager, onFail]()
                    {
                        Debugger::Log(WarningLevel::eInfo,
                            "[PlayerTest | WriteData] OK — wrote test_key = \"hello_from_uma\"");

                        // Read it back
                        playfabManager->Player().ReadData(
                            "test_key",
                            [](const std::string& value)
                            {
                                Debugger::Log(WarningLevel::eInfo,
                                    "[PlayerTest | ReadData] OK — test_key = \"" + value + "\"");
                            },
                            onFail
                        );
                    },
                    onFail
                );
                player.WriteData(
                    "test_key", "hello_from_uma",
                    [playfabManager, onFail]()
                    {
                        Debugger::Log(WarningLevel::eInfo,
                            "[PlayerTest | WriteData] OK — wrote test_key = \"hello_from_uma\"");

                        // Read it back
                        playfabManager->Player().ReadData(
                            "test_key",
                            [](const std::string& value)
                            {
                                Debugger::Log(WarningLevel::eInfo,
                                    "[PlayerTest | ReadData] OK — test_key = \"" + value + "\"");
                            },
                            onFail
                        );
                    },
                    onFail
                );

                // =============================================================
                //  4. Player Data — WriteData then ReadMultipleData (chained)
                // =============================================================
                player.WriteData(
                    "test_key_2", "second_value",
                    [playfabManager, onFail]()
                    {
                        Debugger::Log(WarningLevel::eInfo,
                            "[PlayerTest | WriteData] OK — wrote test_key_2 = \"second_value\"");

                        playfabManager->Player().ReadMultipleData(
                            { "test_key", "test_key_2" },
                            [](const std::unordered_map<std::string, std::string>& data)
                            {
                                std::string out = "[PlayerTest | ReadMultipleData] OK —";
                                for (auto const& [k, v] : data)
                                    out += " " + k + "=\"" + v + "\"";
                                Debugger::Log(WarningLevel::eInfo, out);
                            },
                            onFail
                        );
                    },
                    onFail
                );

                // =============================================================
                //  5. Title Data — GetTitleData (read-only from client)
                // =============================================================
                player.GetTitleData(
                    "game version",
                    [](const std::string& value)
                    {
                        Debugger::Log(WarningLevel::eInfo,
                            "[PlayerTest | GetTitleData] OK — game version = \"" + value + "\"");
                    },
                    onFail
                );

                // =============================================================
                //  6. Statistics — SubmitScore then GetMyStats (chained)
                //
                //  PREREQUISITE: stat definition "TestHighScore" must exist
                // =============================================================
                player.SubmitScore(
                    "TestHighScore", 42000,
                    [playfabManager, onFail]()
                    {
                        Debugger::Log(WarningLevel::eInfo,
                            "[PlayerTest | SubmitScore] OK — submitted 42000 to TestHighScore");

                        playfabManager->Player().GetMyStats(
                            "TestHighScore",
                            [](const std::string& statName, double value)
                            {
                                std::ostringstream oss;
                                oss << "[PlayerTest | GetMyStats] OK — "
                                    << statName << " = " << value;
                                Debugger::Log(WarningLevel::eInfo, oss.str());
                            },
                            onFail
                        );
                    },
                    onFail
                );

                // =============================================================
                //  7. Leaderboards — GetLeaderboard (top 10)
                //
                //  PREREQUISITE: leaderboard definition "TestLeaderboard" must exist
                // =============================================================
                player.GetLeaderboard(
                    "TestLeaderboard",
                    10,     // pageSize
                    1,      // startPos (1-based)
                    [](const std::vector<LeaderboardPlayerEntry>& entries)
                    {
                        Debugger::Log(WarningLevel::eInfo,
                            "[PlayerTest | GetLeaderboard] OK — "
                            + std::to_string(entries.size()) + " entries");

                        for (auto const& e : entries)
                        {
                            std::ostringstream oss;
                            oss << "  #" << e.rank
                                << " " << e.displayName
                                << " [" << e.entityId << "]"
                                << " — " << e.score;
                            Debugger::Log(WarningLevel::eInfo, oss.str());
                        }
                    },
                    onFail
                );

                // =============================================================
                //  8. Leaderboards — GetLeaderboardAroundMe
                // =============================================================
                player.GetLeaderboardAroundMe(
                    "TestLeaderboard",
                    5,
                    [](const std::vector<LeaderboardPlayerEntry>& entries)
                    {
                        Debugger::Log(WarningLevel::eInfo,
                            "[PlayerTest | GetLeaderboardAroundMe] OK — "
                            + std::to_string(entries.size()) + " entries");

                        for (auto const& e : entries)
                        {
                            std::ostringstream oss;
                            oss << "  #" << e.rank
                                << " " << e.displayName
                                << " [" << e.entityId << "]"
                                << " — " << e.score;
                            Debugger::Log(WarningLevel::eInfo, oss.str());
                        }
                    },
                    onFail
                );

                Debugger::Log(WarningLevel::eInfo,
                    "[PlayerTest] All tests dispatched — results will arrive async");
            },
            [](HRESULT, const std::string& msg)
            {
                Debugger::Log(WarningLevel::eError,
                    "[PlayerTest] LoginWithCustomID FAILED — " + msg);
            }
        );
    }

} // namespace Uma_Engine
