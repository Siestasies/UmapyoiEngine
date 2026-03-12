/*!
\file   PlayFabEditorManager.cpp
\par    Project: GAM250
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
 * Implements the PlayFab editor panel for credential management, connection
 * status display, and admin Title Data CRUD operations.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "PlayFabEditorManager.h"

#include "PlayFab/Core/PlayFabManager.h"
#include "PlayFab/Core/Admin/PlayFabAdminManager.h"
#include "PlayFab/Core/Player/PlayFabPlayerManager.h"
#include "PlayFab/Core/PlayFabConfig.h"
#include "Core/EngineConfigSerializer.h"
#include "Core/FilePaths.h"

#include <algorithm>
#include <cstring>

namespace Uma_Engine
{
    // =========================================================================
    //  ISystem lifecycle
    // =========================================================================

    void PlayFabEditorManager::Init()
    {
        pPlayFabManager = pSystemManager->GetSystem<PlayFabManager>();

        // Pre-populate credential fields from existing config file
        PlayFabConfig config;
        EngineConfigSerializer serializer;
        serializer.Register(&config);
        serializer.load(Uma_FilePath::CONFIG_ROOT + "playfab_dev.json");

        if (!config.titleId.empty())
            strncpy_s(titleIdBuf, config.titleId.c_str(), sizeof(titleIdBuf) - 1);

        if (!config.secretKey.empty())
            strncpy_s(secretKeyBuf, config.secretKey.c_str(), sizeof(secretKeyBuf) - 1);

        if (!config.customId.empty())
            strncpy_s(playerCustomIdBuf, config.customId.c_str(), sizeof(playerCustomIdBuf) - 1);
    }

    void PlayFabEditorManager::Update(float dt)
    {
        if (statusTimer > 0.f)
            statusTimer -= dt;

        if (showWindow)
            RenderMainWindow();
    }

    void PlayFabEditorManager::Shutdown()
    {
        // PlayFabManager owns the SDK lifecycle — nothing to clean up here
    }


    // =========================================================================
    //  Main window
    // =========================================================================

    void PlayFabEditorManager::RenderMainWindow()
    {
        ImGui::Begin("PlayFab Manager", &showWindow);

        RenderCredentialsSection();
        ImGui::Separator();
        RenderConnectionStatus();

        // Only show admin tools when admin API is fully authenticated
        if (pPlayFabManager && pPlayFabManager->IsReady()
            && pPlayFabManager->HasAdminAccess()
            && pPlayFabManager->Admin().IsReady())
        {
            ImGui::Separator();
            RenderAdminToolsSection();
        }

        // Player tools — shown when SDK is ready (login doesn't require admin)
        if (pPlayFabManager && pPlayFabManager->IsReady())
        {
            ImGui::Separator();
            RenderPlayerToolsSection();
        }

        // Status bar
        if (statusTimer > 0.f && !statusMessage.empty())
        {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "%s", statusMessage.c_str());
        }

        ImGui::End();
    }


    // =========================================================================
    //  Credentials section
    // =========================================================================

    void PlayFabEditorManager::RenderCredentialsSection()
    {
        if (ImGui::CollapsingHeader("Credentials", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::InputText("Title ID", titleIdBuf, sizeof(titleIdBuf));
            ImGui::InputText("Secret Key", secretKeyBuf, sizeof(secretKeyBuf),
                             ImGuiInputTextFlags_Password);

            if (ImGui::Button("Save & Connect"))
            {
                SaveCredentialsToFile();

                if (pPlayFabManager)
                {
                    pPlayFabManager->SetCredentials(
                        std::string(titleIdBuf),
                        std::string(secretKeyBuf),
                        [this]()
                        {
                            statusMessage = "Admin connected successfully!";
                            statusTimer = 5.f;
                        },
                        [this](HRESULT hr, const std::string& msg)
                        {
                            (void)hr;
                            statusMessage = "Connection failed: " + msg;
                            statusTimer = 8.f;
                        }
                    );
                    pPlayFabManager->Init();
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Save Only"))
            {
                SaveCredentialsToFile();
            }
        }
    }


    // =========================================================================
    //  Connection status
    // =========================================================================

    void PlayFabEditorManager::RenderConnectionStatus()
    {
        ImGui::Text("Connection Status:");

        if (!pPlayFabManager)
        {
            ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "  PlayFabManager not found");
            return;
        }

        // SDK initialized?
        if (pPlayFabManager->IsReady())
            ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "  SDK: Initialized");
        else
            ImGui::TextColored(ImVec4(1.f, 0.5f, 0.f, 1.f), "  SDK: Not Initialized");

        // Admin API status
        if (pPlayFabManager->HasAdminAccess())
        {
            if (pPlayFabManager->Admin().IsReady())
                ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "  Admin: Connected");
            else
                ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "  Admin: Authenticating...");
        }
        else
        {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.f), "  Admin: No secret key provided");
        }
    }


    // =========================================================================
    //  Admin tools (tab bar)
    // =========================================================================

    void PlayFabEditorManager::RenderAdminToolsSection()
    {
        if (ImGui::CollapsingHeader("Admin Tools", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::BeginTabBar("AdminTabs"))
            {
                if (ImGui::BeginTabItem("Title Data"))
                {
                    RenderTitleDataPanel();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Statistics"))
                {
                    RenderStatisticsPanel();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Leaderboards"))
                {
                    RenderLeaderboardsPanel();
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
    }


    // =========================================================================
    //  Title Data panel
    // =========================================================================

    void PlayFabEditorManager::RenderTitleDataPanel()
    {
        // ── Refresh button ───────────────────────────────────────────
        if (ImGui::Button("Refresh") && !titleDataFetching)
        {
            FetchAllTitleData();
        }
        ImGui::SameLine();
        if (titleDataFetching)
            ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Fetching...");
        else if (titleDataLoaded)
            ImGui::Text("%zu entries", cachedTitleData.size());

        ImGui::Separator();

        // ── Add new entry ────────────────────────────────────────────
        ImGui::Text("Add Entry:");
        ImGui::PushItemWidth(200.f);
        ImGui::InputText("Key##new", newKeyBuf, sizeof(newKeyBuf));
        ImGui::PopItemWidth();
        ImGui::InputTextMultiline("Value##new", newValueBuf, sizeof(newValueBuf),
                                  ImVec2(-50.f, 60.f));

        if (ImGui::Button("Add##addentry"))
        {
            std::string k(newKeyBuf);
            std::string v(newValueBuf);
            if (!k.empty())
            {
                SaveTitleDataEntry(k, v);
                memset(newKeyBuf, 0, sizeof(newKeyBuf));
                memset(newValueBuf, 0, sizeof(newValueBuf));
            }
        }

        ImGui::Separator();

        // ── Existing entries table ───────────────────────────────────
        if (!titleDataLoaded)
        {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.f),
                               "Press Refresh to load title data.");
            return;
        }

        if (cachedTitleData.empty())
        {
            ImGui::Text("No title data entries found.");
            return;
        }

        if (ImGui::BeginTable("TitleDataTable", 3,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg
            | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY,
            ImVec2(0.f, 300.f)))
        {
            ImGui::TableSetupColumn("Key",     ImGuiTableColumnFlags_WidthFixed,   150.f);
            ImGui::TableSetupColumn("Value",   ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed,   130.f);
            ImGui::TableHeadersRow();

            for (auto& [key, value] : cachedTitleData)
            {
                ImGui::TableNextRow();
                ImGui::PushID(key.c_str());

                // Key column
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(key.c_str());

                // Value column
                ImGui::TableSetColumnIndex(1);
                if (editingKey == key)
                {
                    ImGui::InputTextMultiline("##editval", editValueBuf,
                        sizeof(editValueBuf), ImVec2(-1.f, 40.f));
                }
                else
                {
                    ImGui::TextWrapped("%s", value.c_str());
                }

                // Actions column
                ImGui::TableSetColumnIndex(2);
                if (editingKey == key)
                {
                    if (ImGui::Button("Save##save"))
                    {
                        SaveTitleDataEntry(key, std::string(editValueBuf));
                        editingKey.clear();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel##cancel"))
                    {
                        editingKey.clear();
                    }
                }
                else
                {
                    if (ImGui::Button("Edit##edit"))
                    {
                        editingKey = key;
                        strncpy_s(editValueBuf, value.c_str(), sizeof(editValueBuf) - 1);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Delete##del"))
                    {
                        DeleteTitleDataEntry(key);
                    }
                }

                ImGui::PopID();
            }

            ImGui::EndTable();
        }
    }


    // =========================================================================
    //  Async data operations
    // =========================================================================

    void PlayFabEditorManager::FetchAllTitleData()
    {
        titleDataFetching = true;

        pPlayFabManager->Admin().GetTitleData(
            {},  // empty = fetch all keys
            [this](const std::unordered_map<std::string, std::string>& data)
            {
                cachedTitleData  = data;
                titleDataLoaded  = true;
                titleDataFetching = false;
                statusMessage = "Title data loaded (" + std::to_string(data.size()) + " entries)";
                statusTimer = 3.f;
            },
            [this](HRESULT hr, const std::string& msg)
            {
                (void)hr;
                titleDataFetching = false;
                statusMessage = "Failed to fetch title data: " + msg;
                statusTimer = 5.f;
            }
        );
    }

    void PlayFabEditorManager::SaveTitleDataEntry(const std::string& key, const std::string& value)
    {
        pPlayFabManager->Admin().SetTitleData(
            key, value,
            [this, key, value]()
            {
                cachedTitleData[key] = value;
                statusMessage = "Saved: " + key;
                statusTimer = 3.f;
            },
            [this, key](HRESULT hr, const std::string& msg)
            {
                (void)hr;
                statusMessage = "Failed to save '" + key + "': " + msg;
                statusTimer = 5.f;
            }
        );
    }

    void PlayFabEditorManager::DeleteTitleDataEntry(const std::string& key)
    {
        pPlayFabManager->Admin().DeleteTitleData(
            key,
            [this, key]()
            {
                cachedTitleData.erase(key);
                if (editingKey == key)
                    editingKey.clear();
                statusMessage = "Deleted: " + key;
                statusTimer = 3.f;
            },
            [this, key](HRESULT hr, const std::string& msg)
            {
                (void)hr;
                statusMessage = "Failed to delete '" + key + "': " + msg;
                statusTimer = 5.f;
            }
        );
    }


    // =========================================================================
    //  Statistics panel
    // =========================================================================

    void PlayFabEditorManager::RenderStatisticsPanel()
    {
        // ── Refresh ──────────────────────────────────────────────────
        if (ImGui::Button("Refresh##stats") && !statsFetching)
        {
            FetchStatisticDefinitions();
        }
        ImGui::SameLine();
        if (statsFetching)
            ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Fetching...");
        else if (statsLoaded)
            ImGui::Text("%zu definitions", cachedStatNames.size());

        ImGui::Separator();

        // ── Create new statistic ─────────────────────────────────────
        if (ImGui::TreeNode("Create New Statistic"))
        {
            ImGui::InputText("Name##statname", newStatNameBuf, sizeof(newStatNameBuf));
            ImGui::InputText("Column Name##statcol", newStatColumnBuf, sizeof(newStatColumnBuf));

            const char* aggregationItems[] = { "Last", "Min", "Max", "Sum" };
            ImGui::Combo("Aggregation", &newStatAggregation, aggregationItems, 4);

            if (ImGui::Button("Create##createstat"))
            {
                CreateStatisticDefinition();
            }

            ImGui::TreePop();
        }

        ImGui::Separator();

        // ── List existing statistics ─────────────────────────────────
        if (!statsLoaded)
        {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.f),
                               "Press Refresh to load statistics.");
            return;
        }

        if (cachedStatNames.empty())
        {
            ImGui::Text("No statistic definitions found.");
            return;
        }

        if (ImGui::BeginTable("StatsTable", 2,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg
            | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY,
            ImVec2(0.f, 250.f)))
        {
            ImGui::TableSetupColumn("Name",    ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 80.f);
            ImGui::TableHeadersRow();

            for (auto& name : cachedStatNames)
            {
                ImGui::TableNextRow();
                ImGui::PushID(name.c_str());

                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(name.c_str());

                ImGui::TableSetColumnIndex(1);
                if (ImGui::Button("Delete##del"))
                {
                    DeleteStatisticDefinition(name);
                }

                ImGui::PopID();
            }

            ImGui::EndTable();
        }
    }

    void PlayFabEditorManager::FetchStatisticDefinitions()
    {
        statsFetching = true;

        pPlayFabManager->Admin().ListStatisticDefinitions(
            [this](const std::vector<std::string>& names)
            {
                cachedStatNames = names;
                statsLoaded     = true;
                statsFetching   = false;
                statusMessage = "Statistics loaded (" + std::to_string(names.size()) + " definitions)";
                statusTimer = 3.f;
            },
            [this](HRESULT hr, const std::string& msg)
            {
                (void)hr;
                statsFetching = false;
                statusMessage = "Failed to fetch statistics: " + msg;
                statusTimer = 5.f;
            }
        );
    }

    void PlayFabEditorManager::CreateStatisticDefinition()
    {
        std::string name(newStatNameBuf);
        std::string column(newStatColumnBuf);

        if (name.empty() || column.empty())
        {
            statusMessage = "Stat name and column name are required.";
            statusTimer = 3.f;
            return;
        }

        // Map combo index to SDK enum
        PFStatisticsStatisticAggregationMethod aggregation;
        switch (newStatAggregation)
        {
        case 0:  aggregation = PFStatisticsStatisticAggregationMethod::Last; break;
        case 1:  aggregation = PFStatisticsStatisticAggregationMethod::Min;  break;
        case 2:  aggregation = PFStatisticsStatisticAggregationMethod::Max;  break;
        case 3:  aggregation = PFStatisticsStatisticAggregationMethod::Sum;  break;
        default: aggregation = PFStatisticsStatisticAggregationMethod::Max;  break;
        }

        pPlayFabManager->Admin().CreateStatisticDefinition(
            name, column, aggregation,
            [this, name]()
            {
                cachedStatNames.push_back(name);
                memset(newStatNameBuf, 0, sizeof(newStatNameBuf));
                memset(newStatColumnBuf, 0, sizeof(newStatColumnBuf));
                statusMessage = "Created statistic: " + name;
                statusTimer = 3.f;
            },
            [this, name](HRESULT hr, const std::string& msg)
            {
                (void)hr;
                statusMessage = "Failed to create '" + name + "': " + msg;
                statusTimer = 5.f;
            }
        );
    }

    void PlayFabEditorManager::DeleteStatisticDefinition(const std::string& name)
    {
        pPlayFabManager->Admin().DeleteStatisticDefinition(
            name,
            [this, name]()
            {
                auto it = std::find(cachedStatNames.begin(), cachedStatNames.end(), name);
                if (it != cachedStatNames.end())
                    cachedStatNames.erase(it);
                statusMessage = "Deleted statistic: " + name;
                statusTimer = 3.f;
            },
            [this, name](HRESULT hr, const std::string& msg)
            {
                (void)hr;
                statusMessage = "Failed to delete '" + name + "': " + msg;
                statusTimer = 5.f;
            }
        );
    }


    // =========================================================================
    //  Leaderboards panel
    // =========================================================================

    void PlayFabEditorManager::RenderLeaderboardsPanel()
    {
        // ── Refresh ──────────────────────────────────────────────────
        if (ImGui::Button("Refresh##lb") && !leaderboardsFetching)
        {
            FetchLeaderboardDefinitions();
        }
        ImGui::SameLine();
        if (leaderboardsFetching)
            ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Fetching...");
        else if (leaderboardsLoaded)
            ImGui::Text("%zu definitions", cachedLeaderboardNames.size());

        ImGui::Separator();

        // ── Create new leaderboard ───────────────────────────────────
        if (ImGui::TreeNode("Create New Leaderboard"))
        {
            ImGui::InputText("Name##lbname", newLBNameBuf, sizeof(newLBNameBuf));
            ImGui::InputText("Column Name##lbcol", newLBColumnBuf, sizeof(newLBColumnBuf));

            const char* sortItems[] = { "Descending (high first)", "Ascending (low first)" };
            ImGui::Combo("Sort Direction", &newLBSortDirection, sortItems, 2);

            ImGui::InputInt("Size Limit", &newLBSizeLimit);
            if (newLBSizeLimit < 1)    newLBSizeLimit = 1;
            if (newLBSizeLimit > 10000) newLBSizeLimit = 10000;

            if (ImGui::Button("Create##createlb"))
            {
                CreateLeaderboardDefinition();
            }

            ImGui::TreePop();
        }

        ImGui::Separator();

        // ── List existing leaderboards ───────────────────────────────
        if (!leaderboardsLoaded)
        {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.f),
                               "Press Refresh to load leaderboards.");
            return;
        }

        if (cachedLeaderboardNames.empty())
        {
            ImGui::Text("No leaderboard definitions found.");
            return;
        }

        if (ImGui::BeginTable("LeaderboardsTable", 3,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg
            | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY,
            ImVec2(0.f, 200.f)))
        {
            ImGui::TableSetupColumn("Name",    ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("View",    ImGuiTableColumnFlags_WidthFixed, 60.f);
            ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 80.f);
            ImGui::TableHeadersRow();

            for (auto& name : cachedLeaderboardNames)
            {
                ImGui::TableNextRow();
                ImGui::PushID(name.c_str());

                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(name.c_str());

                ImGui::TableSetColumnIndex(1);
                if (ImGui::Button("View##view"))
                {
                    FetchLeaderboardEntries(name);
                }

                ImGui::TableSetColumnIndex(2);
                if (ImGui::Button("Delete##del"))
                {
                    DeleteLeaderboardDefinition(name);
                }

                ImGui::PopID();
            }

            ImGui::EndTable();
        }

        // ── Leaderboard entries viewer ───────────────────────────────
        if (!viewingLeaderboard.empty())
        {
            ImGui::Separator();
            ImGui::Text("Entries: %s", viewingLeaderboard.c_str());

            if (lbEntriesFetching)
            {
                ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Fetching entries...");
            }
            else if (cachedLBEntries.empty())
            {
                ImGui::Text("No entries found.");
            }
            else if (ImGui::BeginTable("LBEntriesTable", 4,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg
                | ImGuiTableFlags_ScrollY,
                ImVec2(0.f, 200.f)))
            {
                ImGui::TableSetupColumn("Rank",         ImGuiTableColumnFlags_WidthFixed, 50.f);
                ImGui::TableSetupColumn("Entity ID",    ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Display Name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Score",        ImGuiTableColumnFlags_WidthFixed, 100.f);
                ImGui::TableHeadersRow();

                for (auto& entry : cachedLBEntries)
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%d", entry.rank);

                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(entry.entityId.c_str());

                    ImGui::TableSetColumnIndex(2);
                    ImGui::TextUnformatted(entry.displayName.c_str());

                    ImGui::TableSetColumnIndex(3);
                    ImGui::TextUnformatted(entry.score.c_str());
                }

                ImGui::EndTable();
            }
        }
    }

    void PlayFabEditorManager::FetchLeaderboardDefinitions()
    {
        leaderboardsFetching = true;

        pPlayFabManager->Admin().ListLeaderboardDefinitions(
            [this](const std::vector<std::string>& names)
            {
                cachedLeaderboardNames = names;
                leaderboardsLoaded     = true;
                leaderboardsFetching   = false;
                statusMessage = "Leaderboards loaded (" + std::to_string(names.size()) + " definitions)";
                statusTimer = 3.f;
            },
            [this](HRESULT hr, const std::string& msg)
            {
                (void)hr;
                leaderboardsFetching = false;
                statusMessage = "Failed to fetch leaderboards: " + msg;
                statusTimer = 5.f;
            }
        );
    }

    void PlayFabEditorManager::CreateLeaderboardDefinition()
    {
        std::string name(newLBNameBuf);
        std::string column(newLBColumnBuf);

        if (name.empty() || column.empty())
        {
            statusMessage = "Leaderboard name and column name are required.";
            statusTimer = 3.f;
            return;
        }

        PFLeaderboardsLeaderboardSortDirection sortDir =
            (newLBSortDirection == 0)
                ? PFLeaderboardsLeaderboardSortDirection::Descending
                : PFLeaderboardsLeaderboardSortDirection::Ascending;

        pPlayFabManager->Admin().CreateLeaderboardDefinition(
            name, column, sortDir, static_cast<int32_t>(newLBSizeLimit),
            [this, name]()
            {
                cachedLeaderboardNames.push_back(name);
                memset(newLBNameBuf, 0, sizeof(newLBNameBuf));
                memset(newLBColumnBuf, 0, sizeof(newLBColumnBuf));
                statusMessage = "Created leaderboard: " + name;
                statusTimer = 3.f;
            },
            [this, name](HRESULT hr, const std::string& msg)
            {
                (void)hr;
                statusMessage = "Failed to create '" + name + "': " + msg;
                statusTimer = 5.f;
            }
        );
    }

    void PlayFabEditorManager::DeleteLeaderboardDefinition(const std::string& name)
    {
        pPlayFabManager->Admin().DeleteLeaderboardDefinition(
            name,
            [this, name]()
            {
                auto it = std::find(cachedLeaderboardNames.begin(), cachedLeaderboardNames.end(), name);
                if (it != cachedLeaderboardNames.end())
                    cachedLeaderboardNames.erase(it);

                if (viewingLeaderboard == name)
                {
                    viewingLeaderboard.clear();
                    cachedLBEntries.clear();
                }
                statusMessage = "Deleted leaderboard: " + name;
                statusTimer = 3.f;
            },
            [this, name](HRESULT hr, const std::string& msg)
            {
                (void)hr;
                statusMessage = "Failed to delete '" + name + "': " + msg;
                statusTimer = 5.f;
            }
        );
    }

    void PlayFabEditorManager::FetchLeaderboardEntries(const std::string& name)
    {
        viewingLeaderboard = name;
        lbEntriesFetching  = true;
        cachedLBEntries.clear();

        pPlayFabManager->Admin().GetLeaderboard(
            name, 100, nullptr,
            [this](const std::vector<LeaderboardEntry>& entries, uint32_t version)
            {
                (void)version;
                cachedLBEntries.clear();
                cachedLBEntries.reserve(entries.size());

                for (auto& e : entries)
                {
                    CachedLeaderboardEntry cached;
                    cached.entityId    = e.entityId;
                    cached.displayName = e.displayName;
                    cached.rank        = e.rank;
                    cached.score       = e.scores.empty() ? "" : e.scores[0];
                    cachedLBEntries.push_back(std::move(cached));
                }

                lbEntriesFetching = false;
                statusMessage = "Loaded " + std::to_string(entries.size()) + " leaderboard entries";
                statusTimer = 3.f;
            },
            [this](HRESULT hr, const std::string& msg)
            {
                (void)hr;
                lbEntriesFetching = false;
                statusMessage = "Failed to fetch leaderboard entries: " + msg;
                statusTimer = 5.f;
            }
        );
    }


    // =========================================================================
    //  Player tools section
    // =========================================================================

    void PlayFabEditorManager::RenderPlayerToolsSection()
    {
        if (ImGui::CollapsingHeader("Player Tools"))
        {
            if (ImGui::BeginTabBar("PlayerTabs"))
            {
                if (ImGui::BeginTabItem("Login"))
                {
                    RenderPlayerLoginPanel();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Player Data"))
                {
                    RenderPlayerDataPanel();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("My Stats"))
                {
                    RenderPlayerStatsPanel();
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Leaderboard"))
                {
                    RenderPlayerLeaderboardPanel();
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
    }


    // =========================================================================
    //  Player Login panel
    // =========================================================================

    void PlayFabEditorManager::RenderPlayerLoginPanel()
    {
        bool loggedIn = pPlayFabManager->Player().IsLoggedIn();

        // Status indicator
        if (loggedIn)
            ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Player: Logged In");
        else
            ImGui::TextColored(ImVec4(1.f, 0.5f, 0.f, 1.f), "Player: Not Logged In");

        ImGui::Separator();

        if (!loggedIn)
        {
            ImGui::InputText("Custom ID", playerCustomIdBuf, sizeof(playerCustomIdBuf));
            ImGui::Checkbox("Create Account If Not Exists", &playerCreateAccount);

            if (ImGui::Button("Login"))
            {
                std::string customId(playerCustomIdBuf);
                if (!customId.empty())
                {
                    pPlayFabManager->LoginWithCustomID(
                        customId, playerCreateAccount,
                        [this]()
                        {
                            statusMessage = "Player logged in successfully!";
                            statusTimer = 5.f;
                        },
                        [this](HRESULT hr, const std::string& msg)
                        {
                            (void)hr;
                            statusMessage = "Player login failed: " + msg;
                            statusTimer = 5.f;
                        }
                    );
                }
                else
                {
                    statusMessage = "Custom ID cannot be empty.";
                    statusTimer = 3.f;
                }
            }
        }
        else
        {
            if (ImGui::Button("Logout"))
            {
                pPlayFabManager->Logout();
                // Clear player caches
                cachedPlayerData.clear();
                playerDataLoaded = false;
                playerStatQueried = false;
                cachedPlayerLBEntries.clear();
                playerLBViewingName.clear();
                statusMessage = "Player logged out.";
                statusTimer = 3.f;
            }
        }
    }


    // =========================================================================
    //  Player Data panel
    // =========================================================================

    void PlayFabEditorManager::RenderPlayerDataPanel()
    {
        if (!pPlayFabManager->Player().IsLoggedIn())
        {
            ImGui::TextColored(ImVec4(1.f, 0.5f, 0.f, 1.f), "Login required to use Player Data.");
            return;
        }

        // ── Refresh ──────────────────────────────────────────────────
        if (ImGui::Button("Refresh##playerdata") && !playerDataFetching)
        {
            PlayerFetchAllData();
        }
        ImGui::SameLine();
        if (playerDataFetching)
            ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Fetching...");
        else if (playerDataLoaded)
            ImGui::Text("%zu entries", cachedPlayerData.size());

        ImGui::Separator();

        // ── Write new entry ──────────────────────────────────────────
        ImGui::Text("Write Data:");
        ImGui::PushItemWidth(200.f);
        ImGui::InputText("Key##pnew", playerNewKeyBuf, sizeof(playerNewKeyBuf));
        ImGui::PopItemWidth();
        ImGui::InputTextMultiline("Value##pnew", playerNewValueBuf, sizeof(playerNewValueBuf),
                                  ImVec2(-50.f, 60.f));

        if (ImGui::Button("Write##pwrite"))
        {
            std::string k(playerNewKeyBuf);
            std::string v(playerNewValueBuf);
            if (!k.empty())
            {
                PlayerWriteData(k, v);
                memset(playerNewKeyBuf, 0, sizeof(playerNewKeyBuf));
                memset(playerNewValueBuf, 0, sizeof(playerNewValueBuf));
            }
        }

        ImGui::Separator();

        // ── Existing entries table ───────────────────────────────────
        if (!playerDataLoaded)
        {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.f),
                               "Press Refresh to load player data.");
            return;
        }

        if (cachedPlayerData.empty())
        {
            ImGui::Text("No player data entries found.");
            return;
        }

        if (ImGui::BeginTable("PlayerDataTable", 3,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg
            | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY,
            ImVec2(0.f, 250.f)))
        {
            ImGui::TableSetupColumn("Key",     ImGuiTableColumnFlags_WidthFixed,   150.f);
            ImGui::TableSetupColumn("Value",   ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed,   100.f);
            ImGui::TableHeadersRow();

            for (auto& [key, value] : cachedPlayerData)
            {
                ImGui::TableNextRow();
                ImGui::PushID(key.c_str());

                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(key.c_str());

                ImGui::TableSetColumnIndex(1);
                if (playerEditingKey == key)
                {
                    ImGui::InputTextMultiline("##peditval", playerEditValueBuf,
                        sizeof(playerEditValueBuf), ImVec2(-1.f, 40.f));
                }
                else
                {
                    ImGui::TextWrapped("%s", value.c_str());
                }

                ImGui::TableSetColumnIndex(2);
                if (playerEditingKey == key)
                {
                    if (ImGui::Button("Save##psave"))
                    {
                        PlayerWriteData(key, std::string(playerEditValueBuf));
                        playerEditingKey.clear();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("X##pcancel"))
                    {
                        playerEditingKey.clear();
                    }
                }
                else
                {
                    if (ImGui::Button("Edit##pedit"))
                    {
                        playerEditingKey = key;
                        strncpy_s(playerEditValueBuf, value.c_str(), sizeof(playerEditValueBuf) - 1);
                    }
                }

                ImGui::PopID();
            }

            ImGui::EndTable();
        }
    }

    void PlayFabEditorManager::PlayerFetchAllData()
    {
        playerDataFetching = true;

        pPlayFabManager->Player().ReadMultipleData(
            {},  // empty = fetch all
            [this](const std::unordered_map<std::string, std::string>& data)
            {
                cachedPlayerData  = data;
                playerDataLoaded  = true;
                playerDataFetching = false;
                statusMessage = "Player data loaded (" + std::to_string(data.size()) + " entries)";
                statusTimer = 3.f;
            },
            [this](HRESULT hr, const std::string& msg)
            {
                (void)hr;
                playerDataFetching = false;
                statusMessage = "Failed to fetch player data: " + msg;
                statusTimer = 5.f;
            }
        );
    }

    void PlayFabEditorManager::PlayerWriteData(const std::string& key, const std::string& value)
    {
        pPlayFabManager->Player().WriteData(
            key, value,
            [this, key, value]()
            {
                cachedPlayerData[key] = value;
                statusMessage = "Player data saved: " + key;
                statusTimer = 3.f;
            },
            [this, key](HRESULT hr, const std::string& msg)
            {
                (void)hr;
                statusMessage = "Failed to write '" + key + "': " + msg;
                statusTimer = 5.f;
            }
        );
    }


    // =========================================================================
    //  Player Stats panel
    // =========================================================================

    void PlayFabEditorManager::RenderPlayerStatsPanel()
    {
        if (!pPlayFabManager->Player().IsLoggedIn())
        {
            ImGui::TextColored(ImVec4(1.f, 0.5f, 0.f, 1.f), "Login required to use Player Stats.");
            return;
        }

        // ── Read stat ────────────────────────────────────────────────
        ImGui::Text("Read Statistic:");
        ImGui::PushItemWidth(200.f);
        ImGui::InputText("Stat Name##query", playerStatQueryBuf, sizeof(playerStatQueryBuf));
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Get##getstat") && !playerStatFetching)
        {
            std::string name(playerStatQueryBuf);
            if (!name.empty())
                PlayerFetchStat(name);
        }

        if (playerStatFetching)
        {
            ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Fetching...");
        }
        else if (playerStatQueried)
        {
            ImGui::Text("  %s = %.2f", playerStatResultName.c_str(), playerStatResultValue);
        }

        ImGui::Separator();

        // ── Submit score ─────────────────────────────────────────────
        ImGui::Text("Submit Score:");
        ImGui::PushItemWidth(200.f);
        ImGui::InputText("Stat Name##submit", playerSubmitStatBuf, sizeof(playerSubmitStatBuf));
        ImGui::PopItemWidth();
        ImGui::InputDouble("Score Value", &playerSubmitScoreValue, 0.0, 0.0, "%.2f");

        if (ImGui::Button("Submit##submitscore"))
        {
            std::string name(playerSubmitStatBuf);
            if (!name.empty())
            {
                PlayerSubmitScore(name, playerSubmitScoreValue);
            }
        }
    }

    void PlayFabEditorManager::PlayerFetchStat(const std::string& statName)
    {
        playerStatFetching = true;

        pPlayFabManager->Player().GetMyStats(
            statName,
            [this](const std::string& name, double value)
            {
                playerStatResultName  = name;
                playerStatResultValue = value;
                playerStatQueried     = true;
                playerStatFetching    = false;
                statusMessage = name + " = " + std::to_string(value);
                statusTimer = 3.f;
            },
            [this](HRESULT hr, const std::string& msg)
            {
                (void)hr;
                playerStatFetching = false;
                statusMessage = "Failed to get stat: " + msg;
                statusTimer = 5.f;
            }
        );
    }

    void PlayFabEditorManager::PlayerSubmitScore(const std::string& statName, double value)
    {
        pPlayFabManager->Player().SubmitScore(
            statName, value,
            [this, statName, value]()
            {
                statusMessage = "Submitted " + statName + " = " + std::to_string(value);
                statusTimer = 3.f;
            },
            [this, statName](HRESULT hr, const std::string& msg)
            {
                (void)hr;
                statusMessage = "Failed to submit '" + statName + "': " + msg;
                statusTimer = 5.f;
            }
        );
    }


    // =========================================================================
    //  Player Leaderboard panel
    // =========================================================================

    void PlayFabEditorManager::RenderPlayerLeaderboardPanel()
    {
        if (!pPlayFabManager->Player().IsLoggedIn())
        {
            ImGui::TextColored(ImVec4(1.f, 0.5f, 0.f, 1.f), "Login required to use Leaderboards.");
            return;
        }

        ImGui::PushItemWidth(200.f);
        ImGui::InputText("Leaderboard Name##plb", playerLBNameBuf, sizeof(playerLBNameBuf));
        ImGui::PopItemWidth();

        ImGui::InputInt("Page Size##plb", &playerLBPageSize);
        if (playerLBPageSize < 1)    playerLBPageSize = 1;
        if (playerLBPageSize > 1000) playerLBPageSize = 1000;

        ImGui::Checkbox("Around Me", &playerLBAroundMe);

        if (!playerLBAroundMe)
        {
            ImGui::InputInt("Start Position (1-based)##plb", &playerLBStartPos);
            if (playerLBStartPos < 1) playerLBStartPos = 1;
        }

        if (ImGui::Button("Fetch##plbfetch") && !playerLBFetching)
        {
            std::string name(playerLBNameBuf);
            if (!name.empty())
            {
                if (playerLBAroundMe)
                    PlayerFetchLeaderboardAroundMe(name, static_cast<uint32_t>(playerLBPageSize));
                else
                    PlayerFetchLeaderboard(name, static_cast<uint32_t>(playerLBPageSize),
                                           static_cast<uint32_t>(playerLBStartPos));
            }
        }

        // ── Results ──────────────────────────────────────────────────
        if (playerLBFetching)
        {
            ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Fetching...");
        }
        else if (!playerLBViewingName.empty())
        {
            ImGui::Separator();
            ImGui::Text("Results: %s (%zu entries)", playerLBViewingName.c_str(),
                        cachedPlayerLBEntries.size());

            if (!cachedPlayerLBEntries.empty() &&
                ImGui::BeginTable("PlayerLBTable", 4,
                    ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg
                    | ImGuiTableFlags_ScrollY,
                    ImVec2(0.f, 200.f)))
            {
                ImGui::TableSetupColumn("Rank",         ImGuiTableColumnFlags_WidthFixed, 50.f);
                ImGui::TableSetupColumn("Entity ID",    ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Display Name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Score",        ImGuiTableColumnFlags_WidthFixed, 100.f);
                ImGui::TableHeadersRow();

                for (auto& entry : cachedPlayerLBEntries)
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%d", entry.rank);

                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(entry.entityId.c_str());

                    ImGui::TableSetColumnIndex(2);
                    ImGui::TextUnformatted(entry.displayName.c_str());

                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%.2f", entry.score);
                }

                ImGui::EndTable();
            }
        }
    }

    void PlayFabEditorManager::PlayerFetchLeaderboard(const std::string& name,
                                                       uint32_t pageSize, uint32_t startPos)
    {
        playerLBFetching    = true;
        playerLBViewingName = name;
        cachedPlayerLBEntries.clear();

        pPlayFabManager->Player().GetLeaderboard(
            name, pageSize, startPos,
            [this](const std::vector<LeaderboardPlayerEntry>& entries)
            {
                cachedPlayerLBEntries.clear();
                cachedPlayerLBEntries.reserve(entries.size());
                for (auto& e : entries)
                {
                    CachedPlayerLBEntry cached;
                    cached.entityId    = e.entityId;
                    cached.displayName = e.displayName;
                    cached.rank        = e.rank;
                    cached.score       = e.score;
                    cachedPlayerLBEntries.push_back(std::move(cached));
                }
                playerLBFetching = false;
                statusMessage = "Loaded " + std::to_string(entries.size()) + " entries";
                statusTimer = 3.f;
            },
            [this](HRESULT hr, const std::string& msg)
            {
                (void)hr;
                playerLBFetching = false;
                statusMessage = "Failed to fetch leaderboard: " + msg;
                statusTimer = 5.f;
            }
        );
    }

    void PlayFabEditorManager::PlayerFetchLeaderboardAroundMe(const std::string& name,
                                                               uint32_t pageSize)
    {
        playerLBFetching    = true;
        playerLBViewingName = name;
        cachedPlayerLBEntries.clear();

        pPlayFabManager->Player().GetLeaderboardAroundMe(
            name, pageSize,
            [this](const std::vector<LeaderboardPlayerEntry>& entries)
            {
                cachedPlayerLBEntries.clear();
                cachedPlayerLBEntries.reserve(entries.size());
                for (auto& e : entries)
                {
                    CachedPlayerLBEntry cached;
                    cached.entityId    = e.entityId;
                    cached.displayName = e.displayName;
                    cached.rank        = e.rank;
                    cached.score       = e.score;
                    cachedPlayerLBEntries.push_back(std::move(cached));
                }
                playerLBFetching = false;
                statusMessage = "Loaded " + std::to_string(entries.size()) + " entries (around me)";
                statusTimer = 3.f;
            },
            [this](HRESULT hr, const std::string& msg)
            {
                (void)hr;
                playerLBFetching = false;
                statusMessage = "Failed to fetch leaderboard: " + msg;
                statusTimer = 5.f;
            }
        );
    }


    // =========================================================================
    //  Config persistence
    // =========================================================================

    void PlayFabEditorManager::SaveCredentialsToFile()
    {
        PlayFabConfig config;
        config.titleId  = std::string(titleIdBuf);
        config.secretKey = std::string(secretKeyBuf);
        config.customId = std::string(playerCustomIdBuf);

        EngineConfigSerializer serializer;
        serializer.Register(&config);
        serializer.save(Uma_FilePath::CONFIG_ROOT + "playfab_dev.json");

        statusMessage = "Credentials saved to playfab_dev.json";
        statusTimer = 3.f;
    }

} // namespace Uma_Engine
