/*!
\file   PlayFabEditorManager.h
\par    Project: GAM250
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
 * Editor-only ImGui interface for PlayFab configuration and administration.
 * Provides:
 * - Credential input (Title ID, Secret Key) with persistence to playfab_dev.json
 * - Real-time connection status display
 * - Admin tools for Title Data CRUD operations
 * - Extensible tab-based layout for future admin features

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../Core/SystemType.h"

#include <imgui.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace Uma_Engine
{
    class PlayFabManager;

    /**
     * @class PlayFabEditorManager
     * @brief ISystem that provides an ImGui-based PlayFab configuration and admin panel
     *
     * This system renders editor windows for:
     * - Entering and saving PlayFab credentials (Title ID, Secret Key)
     * - Displaying connection status (SDK init, admin auth)
     * - Managing Title Data (view, add, edit, delete entries)
     *
     * Credentials are persisted to Configs/playfab_dev.json using the existing
     * PlayFabConfig serializer. The admin tools section is only shown when
     * the PlayFab admin API is fully authenticated and ready.
     */
    class PlayFabEditorManager : public ISystem
    {
    public:

        /**
         * @brief Initialize the PlayFab editor system
         *
         * Acquires a reference to PlayFabManager and pre-populates credential
         * input fields from the existing playfab_dev.json config file.
         */
        void Init() override;

        /**
         * @brief Update the PlayFab editor each frame
         *
         * Renders the PlayFab Manager ImGui window with credentials,
         * connection status, and admin tools sections.
         *
         * @param dt Delta time in seconds since last frame
         */
        void Update(float dt) override;

        /**
         * @brief Clean up editor resources on shutdown
         */
        void Shutdown() override;

        /**
         * @brief Toggle visibility of the PlayFab Manager window
         */
        void ToggleWindow() { showWindow = !showWindow; }

        /**
         * @brief Check if the PlayFab Manager window is visible
         */
        bool IsWindowVisible() const { return showWindow; }

    private:
        // ── ImGui rendering methods ──────────────────────────────────
        void RenderMainWindow();
        void RenderCredentialsSection();
        void RenderConnectionStatus();
        void RenderAdminToolsSection();

        // ── Title Data sub-panel ─────────────────────────────────────
        void RenderTitleDataPanel();
        void FetchAllTitleData();
        void SaveTitleDataEntry(const std::string& key, const std::string& value);
        void DeleteTitleDataEntry(const std::string& key);

        // ── Statistics sub-panel ─────────────────────────────────────
        void RenderStatisticsPanel();
        void FetchStatisticDefinitions();
        void CreateStatisticDefinition();
        void DeleteStatisticDefinition(const std::string& name);

        // ── Leaderboards sub-panel ───────────────────────────────────
        void RenderLeaderboardsPanel();
        void FetchLeaderboardDefinitions();
        void CreateLeaderboardDefinition();
        void DeleteLeaderboardDefinition(const std::string& name);
        void FetchLeaderboardEntries(const std::string& name);

        // ── Player tools ─────────────────────────────────────────────
        void RenderPlayerToolsSection();
        void RenderPlayerLoginPanel();
        void RenderPlayerDataPanel();
        void RenderPlayerStatsPanel();
        void RenderPlayerLeaderboardPanel();

        // Player data async operations
        void PlayerFetchAllData();
        void PlayerWriteData(const std::string& key, const std::string& value);
        void PlayerFetchStat(const std::string& statName);
        void PlayerSubmitScore(const std::string& statName, double value);
        void PlayerFetchLeaderboard(const std::string& name, uint32_t pageSize, uint32_t startPos);
        void PlayerFetchLeaderboardAroundMe(const std::string& name, uint32_t pageSize);

        // ── Config persistence ───────────────────────────────────────
        void SaveCredentialsToFile();

        // ── Systems ──────────────────────────────────────────────────
        PlayFabManager* pPlayFabManager = nullptr;

        // ── Window visibility ────────────────────────────────────────
        bool showWindow = false;

        // ── Credentials input buffers ────────────────────────────────
        char titleIdBuf[128]   = {};
        char secretKeyBuf[256] = {};

        // ── Title Data cache ─────────────────────────────────────────
        std::unordered_map<std::string, std::string> cachedTitleData;
        bool titleDataLoaded   = false;
        bool titleDataFetching = false;

        // ── Add new entry buffers ────────────────────────────────────
        char newKeyBuf[256]    = {};
        char newValueBuf[4096] = {};

        // ── Edit mode state ──────────────────────────────────────────
        std::string editingKey;
        char editValueBuf[4096] = {};

        // ── Statistics cache ──────────────────────────────────────────
        std::vector<std::string> cachedStatNames;
        bool statsLoaded   = false;
        bool statsFetching = false;

        // Create stat input buffers
        char newStatNameBuf[256]   = {};
        char newStatColumnBuf[256] = {};
        int  newStatAggregation    = 2;  // 0=Last, 1=Min, 2=Max, 3=Sum

        // ── Leaderboards cache ───────────────────────────────────────
        std::vector<std::string> cachedLeaderboardNames;
        bool leaderboardsLoaded   = false;
        bool leaderboardsFetching = false;

        // Create leaderboard input buffers
        char newLBNameBuf[256]   = {};
        char newLBColumnBuf[256] = {};
        int  newLBSortDirection  = 0;  // 0=Descending, 1=Ascending
        int  newLBSizeLimit      = 1000;

        // Leaderboard entries viewer
        std::string  viewingLeaderboard;
        bool         lbEntriesFetching = false;
        struct CachedLeaderboardEntry
        {
            std::string entityId;
            std::string displayName;
            int32_t     rank = 0;
            std::string score;
        };
        std::vector<CachedLeaderboardEntry> cachedLBEntries;

        // ── Player login state ────────────────────────────────────────
        char playerCustomIdBuf[256] = {};
        bool playerCreateAccount    = true;

        // ── Player data cache ────────────────────────────────────────
        std::unordered_map<std::string, std::string> cachedPlayerData;
        bool playerDataLoaded   = false;
        bool playerDataFetching = false;

        char playerNewKeyBuf[256]    = {};
        char playerNewValueBuf[4096] = {};

        // Player data edit mode
        std::string playerEditingKey;
        char playerEditValueBuf[4096] = {};

        // ── Player stats state ───────────────────────────────────────
        char playerStatQueryBuf[256] = {};
        std::string playerStatResultName;
        double      playerStatResultValue = 0.0;
        bool        playerStatQueried     = false;
        bool        playerStatFetching    = false;

        char   playerSubmitStatBuf[256] = {};
        double playerSubmitScoreValue   = 0.0;

        // ── Player leaderboard state ─────────────────────────────────
        char     playerLBNameBuf[256] = {};
        int      playerLBPageSize     = 100;
        int      playerLBStartPos     = 1;
        bool     playerLBAroundMe     = false;

        struct CachedPlayerLBEntry
        {
            std::string entityId;
            std::string displayName;
            int         rank  = 0;
            double      score = 0.0;
        };
        std::vector<CachedPlayerLBEntry> cachedPlayerLBEntries;
        bool playerLBFetching = false;
        std::string playerLBViewingName;

        // ── Status / feedback messages ───────────────────────────────
        std::string statusMessage;
        float statusTimer = 0.f;
    };
}
