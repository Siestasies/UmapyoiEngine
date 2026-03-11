/*!
\file   PlayFabPlayerManager.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Player-facing PlayFab sub-manager for Uma Engine.

Owned by PlayFabManager and accessed via PlayFabManager::Player().
Requires a valid PFEntityHandle supplied by PlayFabManager after a
successful player login. All methods are no-ops (with a logged warning)
if IsLoggedIn() is false.

=== Responsibilities ===

    Authentication   -- Login / Register / Logout (delegates to PlayFabManager
                        which owns m_serviceConfig; results are handed back here
                        via SetPlayerEntityHandle())
    Account          -- GetAccountInfo, SetDisplayName
    Player Data      -- ReadData, ReadMultipleData, WriteData
    Title Data       -- GetTitleData  (read-only from client)
    Statistics       -- SubmitScore, GetMyStats
    Leaderboards     -- GetLeaderboard, GetLeaderboardAroundMe

=== Usage ===

    // After PlayFabManager::LoginWithCustomID / LoginWithEmail completes:
    auto& player = pfb->Player();
    if (player.IsLoggedIn())
    {
        player.SubmitScore("HighScore", 9999, [](bool ok){ ... });
        player.GetLeaderboard("HighScore", 100, 1, [](auto entries){ ... });
    }

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

// ── Windows (must precede XAsync / XTaskQueue) ────────────────────────────────
#include <Windows.h>

// ── PlayFab Services SDK ──────────────────────────────────────────────────────
#include <playfab/core/PFEntity.h>
#include <playfab/services/PFAccountManagement.h>
#include <playfab/services/PFPlayerDataManagement.h>
#include <playfab/services/PFTitleDataManagement.h>
#include <playfab/services/PFStatistics.h>
#include <playfab/services/PFLeaderboards.h>

// ── XAsync ────────────────────────────────────────────────────────────────────
#include <XAsync.h>

// ── Standard library ──────────────────────────────────────────────────────────
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Uma_Engine
{
    // =========================================================================
    //  Callback aliases
    // =========================================================================

    /// Generic success/failure callback (used for fire-and-forget writes).
    using OnPlayerSuccess = std::function<void()>;
    using OnPlayerFailure = std::function<void(HRESULT hr, const std::string& msg)>;

    /// Callback for a single string result (e.g. ReadData, GetTitleData).
    using OnStringResult = std::function<void(const std::string& value)>;

    /// Callback for a key-value map result (e.g. ReadMultipleData).
    using OnKVResult = std::function<void(const std::unordered_map<std::string, std::string>& data)>;

    /// Single leaderboard entry returned to the caller.
    struct LeaderboardPlayerEntry
    {
        std::string entityId;       ///< PlayFab entity ID of the ranked player.
        std::string displayName;    ///< Display name of the ranked player.
        int         rank{ 0 };      ///< 1-based rank on the leaderboard.
        double      score{ 0.0 };   ///< Score value for this entry.
    };

    /// Callback for leaderboard reads.
    using OnLeaderboardPlayerResult = std::function<void(const std::vector<LeaderboardPlayerEntry>& entries)>;

    /// Callback for stat reads (statName → value).
    using OnStatResult = std::function<void(const std::string& statName, double value)>;


    // =========================================================================
    //  PlayFabPlayerManager
    // =========================================================================

    /*!
    \class  PlayFabPlayerManager
    \brief  Handles all player-facing PlayFab API calls.

    Obtain via PlayFabManager::Player(). Do not construct directly.
    All async methods accept optional onSuccess / onFailure callbacks.
    Callbacks are invoked on the thread that calls PlayFabManager::Update().
    */
    class PlayFabPlayerManager
    {
    public:
        PlayFabPlayerManager() = default;
        ~PlayFabPlayerManager() = default;

        // Non-copyable, non-movable
        PlayFabPlayerManager(const PlayFabPlayerManager&) = delete;
        PlayFabPlayerManager& operator=(const PlayFabPlayerManager&) = delete;
        PlayFabPlayerManager(PlayFabPlayerManager&&) = delete;
        PlayFabPlayerManager& operator=(PlayFabPlayerManager&&) = delete;


        // ── Handle management (called by PlayFabManager) ──────────────────────

        /*!
        \brief  Supplies the player entity handle obtained after a successful login.
                Called internally by PlayFabManager — do not call from game code.
        \param  handle  Valid PFEntityHandle from a login completion callback.
        */
        void SetPlayerEntityHandle(PFEntityHandle handle);

        /*!
        \brief  Releases the current player entity handle and marks the manager
                as logged out. Called by PlayFabManager::Logout().
        */
        void ClearPlayerEntityHandle();

        /*!
        \brief  Returns true if a valid player entity handle is held.
        */
        [[nodiscard]] bool IsLoggedIn() const;


        // ── Account ───────────────────────────────────────────────────────────

        /*!
        \brief  Fetches the current player's account details (PlayFab ID,
                username, created date) from the service.
        \param  onSuccess  Receives a KV map of account fields on success.
        \param  onFailure  Optional — receives HRESULT and message on failure.
        */
        void GetAccountInfo(
            OnKVResult     onSuccess = nullptr,
            OnPlayerFailure onFailure = nullptr
        );

        /*!
        \brief  Sets or updates the display name shown on leaderboards and profiles.
        \param  name       The new display name (subject to PlayFab profanity filter).
        \param  onSuccess  Optional — called when the update is confirmed.
        \param  onFailure  Optional — receives HRESULT and message on failure.
        */
        void SetDisplayName(
            const std::string& name,
            OnPlayerSuccess    onSuccess = nullptr,
            OnPlayerFailure    onFailure = nullptr
        );


        // ── Player Data ───────────────────────────────────────────────────────

        /*!
        \brief  Fetches a single KV entry from the player's private user data store.
        \param  key        The data key to fetch.
        \param  onSuccess  Receives the string value on success.
        \param  onFailure  Optional — receives HRESULT and message on failure.
        */
        void ReadData(
            const std::string& key,
            OnStringResult     onSuccess = nullptr,
            OnPlayerFailure    onFailure = nullptr
        );

        /*!
        \brief  Fetches multiple KV entries in one call.
                Pass an empty vector to fetch all keys.
        \param  keys       List of data keys to fetch.
        \param  onSuccess  Receives a key-value map on success.
        \param  onFailure  Optional — receives HRESULT and message on failure.
        */
        void ReadMultipleData(
            const std::vector<std::string>& keys,
            OnKVResult                      onSuccess = nullptr,
            OnPlayerFailure                 onFailure = nullptr
        );

        /*!
        \brief  Writes a single KV pair to the player's private user data store.
                Creates the key if it does not exist; overwrites if it does.
        \param  key        The data key to write.
        \param  value      The string value to store.
        \param  onSuccess  Optional — called when the write is confirmed.
        \param  onFailure  Optional — receives HRESULT and message on failure.
        */
        void WriteData(
            const std::string& key,
            const std::string& value,
            OnPlayerSuccess    onSuccess = nullptr,
            OnPlayerFailure    onFailure = nullptr
        );


        // ── Title Data ────────────────────────────────────────────────────────

        /*!
        \brief  Reads a global KV entry set by the developer in the PlayFab
                dashboard. Read-only from the client.
        \param  key        The title data key to read.
        \param  onSuccess  Receives the string value on success.
        \param  onFailure  Optional — receives HRESULT and message on failure.
        */
        void GetTitleData(
            const std::string& key,
            OnStringResult     onSuccess = nullptr,
            OnPlayerFailure    onFailure = nullptr
        );


        // ── Statistics ────────────────────────────────────────────────────────

        /*!
        \brief  Updates a stat on the player's entity profile.
                Automatically propagates to any leaderboard linked to this stat.
        \param  statName   Name of the statistic definition to update.
        \param  value      The new score value.
        \param  onSuccess  Optional — called when the update is confirmed.
        \param  onFailure  Optional — receives HRESULT and message on failure.
        */
        void SubmitScore(
            const std::string& statName,
            double             value,
            OnPlayerSuccess    onSuccess = nullptr,
            OnPlayerFailure    onFailure = nullptr
        );

        /*!
        \brief  Reads the player's current value for a named statistic.
        \param  statName   Name of the statistic definition to query.
        \param  onSuccess  Receives the stat name and current value on success.
        \param  onFailure  Optional — receives HRESULT and message on failure.
        */
        void GetMyStats(
            const std::string& statName,
            OnStatResult       onSuccess = nullptr,
            OnPlayerFailure    onFailure = nullptr
        );


        // ── Leaderboards ──────────────────────────────────────────────────────

        /*!
        \brief  Fetches a page of the global leaderboard from a given position.
        \param  name          Name of the leaderboard definition to query.
        \param  pageSize      Number of entries to return (1 – 1000).
        \param  startPos      1-based rank position to start from. 1 = top of board.
        \param  onSuccess     Receives a vector of LeaderboardEntry on success.
        \param  onFailure     Optional — receives HRESULT and message on failure.
        */
        void GetLeaderboard(
            const std::string& name,
            uint32_t            pageSize,
            uint32_t            startPos,
            OnLeaderboardPlayerResult onSuccess = nullptr,
            OnPlayerFailure     onFailure = nullptr
        );

        /*!
        \brief  Fetches leaderboard entries centred on the current player's rank.
        \param  name          Name of the leaderboard definition to query.
        \param  pageSize      Total number of entries to return around the player.
        \param  onSuccess     Receives a vector of LeaderboardEntry on success.
        \param  onFailure     Optional — receives HRESULT and message on failure.
        */
        void GetLeaderboardAroundMe(
            const std::string& name,
            uint32_t            pageSize,
            OnLeaderboardPlayerResult onSuccess = nullptr,
            OnPlayerFailure     onFailure = nullptr
        );


    private:
        // ── Async context base ────────────────────────────────────────────────
        // All per-operation context structs embed this as their first member so
        // the static callback can safely cast XAsyncBlock* back to the context.

        struct PlayerOpContext
        {
            XAsyncBlock          async{};   ///< Must be first.
            PlayFabPlayerManager* manager;  ///< Back-pointer.
        };

        // ── Per-operation context structs ─────────────────────────────────────

        struct GetAccountInfoContext : PlayerOpContext
        {
            OnKVResult      onSuccess;
            OnPlayerFailure onFailure;
        };

        struct SetDisplayNameContext : PlayerOpContext
        {
            OnPlayerSuccess onSuccess;
            OnPlayerFailure onFailure;
        };

        struct ReadDataContext : PlayerOpContext
        {
            std::string     key;
            OnStringResult  onSuccess;
            OnPlayerFailure onFailure;
        };

        struct ReadMultipleDataContext : PlayerOpContext
        {
            OnKVResult      onSuccess;
            OnPlayerFailure onFailure;
        };

        struct WriteDataContext : PlayerOpContext
        {
            OnPlayerSuccess onSuccess;
            OnPlayerFailure onFailure;
        };

        struct GetTitleDataContext : PlayerOpContext
        {
            std::string     key;
            OnStringResult  onSuccess;
            OnPlayerFailure onFailure;
        };

        struct SubmitScoreContext : PlayerOpContext
        {
            OnPlayerSuccess onSuccess;
            OnPlayerFailure onFailure;
        };

        struct GetMyStatsContext : PlayerOpContext
        {
            std::string     statName;
            OnStatResult    onSuccess;
            OnPlayerFailure onFailure;
        };

        struct GetLeaderboardContext : PlayerOpContext
        {
            OnLeaderboardPlayerResult onSuccess;
            OnPlayerFailure     onFailure;
        };

        struct GetLeaderboardAroundMeContext : PlayerOpContext
        {
            OnLeaderboardPlayerResult onSuccess;
            OnPlayerFailure     onFailure;
        };


        // ── Static async callbacks ────────────────────────────────────────────

        static void CALLBACK OnGetAccountInfoComplete(XAsyncBlock* async);
        static void CALLBACK OnSetDisplayNameComplete(XAsyncBlock* async);
        static void CALLBACK OnReadDataComplete(XAsyncBlock* async);
        static void CALLBACK OnReadMultipleDataComplete(XAsyncBlock* async);
        static void CALLBACK OnWriteDataComplete(XAsyncBlock* async);
        static void CALLBACK OnGetTitleDataComplete(XAsyncBlock* async);
        static void CALLBACK OnSubmitScoreComplete(XAsyncBlock* async);
        static void CALLBACK OnGetMyStatsComplete(XAsyncBlock* async);
        static void CALLBACK OnGetLeaderboardComplete(XAsyncBlock* async);
        static void CALLBACK OnGetLeaderboardAroundMeComplete(XAsyncBlock* async);


        // ── Private helpers ───────────────────────────────────────────────────

        /*!
        \brief  Returns true and logs a warning if the player entity handle is
                null. Used at the top of every public method.
        */
        [[nodiscard]] bool GuardLoggedIn(const char* callerName) const;

        /*!
        \brief  Dispatches an error to onFailure (if set) with a formatted message.
                Safe to call with a null onFailure.
        */
        static void DispatchError(
            HRESULT               hr,
            const std::string& context,
            const OnPlayerFailure& onFailure
        );


        // ── State ─────────────────────────────────────────────────────────────

        /// Valid after a successful login; null before login or after logout.
        PFEntityHandle m_playerEntityHandle{ nullptr };
    };

} // namespace Uma_Engine