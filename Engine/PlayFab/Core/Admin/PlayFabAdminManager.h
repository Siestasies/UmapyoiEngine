/*!
\file   PlayFabAdminManager.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Provides Admin-tier PlayFab API operations for Uma Engine.

Owned by PlayFabManager. Receives a title-entity PFEntityHandle from
PlayFabManager after it authenticates with the Developer Secret Key via
PFAuthenticationGetEntityWithSecretKeyAsync. All API calls in this class
require that handle and must NEVER be used in release/public builds.

Each async operation heap-allocates its own XAsyncBlock (plus any context
it needs) and frees it inside the completion callback, so callers do not
manage async lifetimes manually.

Current scope:
  - TitleData    : GetTitleData, SetTitleData, SetTitleDataBatch, DeleteTitleData
  - Statistics   : CreateStatisticDefinition, DeleteStatisticDefinition,
                   UpdateStatistics, GetStatistics, IncrementStatisticVersion,
                   ListStatisticDefinitions
  - Leaderboards : CreateLeaderboardDefinition, DeleteLeaderboardDefinition,
                   GetLeaderboard, GetLeaderboardAroundEntity,
                   UpdateLeaderboardEntries, IncrementLeaderboardVersion,
                   ListLeaderboardDefinitions
  - CloudScript  : ExecuteFunction

Planned scope:
  - Player management : GetPlayerProfile, BanPlayer
  - Economy / Catalog : GetCatalogItems, UpdateCatalogItems

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

// ── Standard library ──────────────────────────────────────────────────────────
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <Windows.h>
#include <queue>

// ── PlayFab Services SDK ──────────────────────────────────────────────────────
#include <playfab/core/PFEntity.h>
#include <playfab/services/PFTitleDataManagement.h>
#include <playfab/services/PFTitleDataManagementTypes.h>
#include <playfab/services/PFStatistics.h>
#include <playfab/services/PFStatisticsTypes.h>
#include <playfab/services/PFLeaderboards.h>
#include <playfab/services/PFLeaderboardsTypes.h>
#include <playfab/services/PFCloudScript.h>
#include <playfab/services/PFCloudScriptTypes.h>
#include <playfab/services/PFServices.h>

// ── XAsync (GDK / PlayFab Services on Win32) ──────────────────────────────────
#include <XAsync.h>
#include <XTaskQueue.h>


namespace Uma_Engine
{
    // =========================================================================
    //  Shared data types returned to callers
    // =========================================================================

    /// A single leaderboard entry as returned by GetLeaderboard / GetLeaderboardAroundEntity.
    struct LeaderboardEntry
    {
        std::string              entityId;    ///< PlayFab entity ID.
        std::string              displayName; ///< Display name, may be empty.
        int32_t                  rank;        ///< 1-based position on the leaderboard.
        std::vector<std::string> scores;      ///< One element per leaderboard column.
        std::string              metadata;    ///< Opaque metadata blob, may be empty.
    };

    /// Summary of a statistic value for a single entity.
    struct StatisticValue
    {
        std::string              name;    ///< Statistic name.
        int32_t                  version; ///< Current version of the statistic.
        std::vector<std::string> scores;  ///< Column scores as strings.
    };


    // =========================================================================
    //  Callback aliases
    // =========================================================================

    /// Fired on success when key-value data is returned (e.g. GetTitleData).
    using OnAdminDataSuccess = std::function<void(const std::unordered_map<std::string, std::string>& data)>;

    /// Fired on success when no payload is expected (e.g. SetTitleData, CreateStatisticDefinition).
    using OnAdminSuccess = std::function<void()>;

    /// Fired on any Admin API failure.
    /// \param hr       The HRESULT returned by the failed operation.
    /// \param message  Human-readable description of the failure.
    using OnAdminFailure = std::function<void(HRESULT hr, const std::string& message)>;

    /// Fired when a leaderboard page is successfully retrieved.
    using OnLeaderboardSuccess = std::function<void(const std::vector<LeaderboardEntry>& entries, uint32_t version)>;

    /// Fired when GetStatistics succeeds.
    using OnStatisticsSuccess = std::function<void(const std::vector<StatisticValue>& stats)>;

    /// Fired when a new version number is returned (IncrementStatisticVersion / IncrementLeaderboardVersion).
    using OnVersionSuccess = std::function<void(uint32_t newVersion)>;

    /// Fired when ExecuteFunction succeeds.
    /// \param functionName  Name of the Azure Function that executed.
    /// \param resultJson    The raw JSON string returned by the function. Empty if the function
    ///                      returned null or the result was too large.
    using OnFunctionSuccess = std::function<void(const std::string& functionName, const std::string& resultJson)>;


    // =========================================================================
    //  PlayFabAdminManager
    // =========================================================================

    /*!
    \class  PlayFabAdminManager
    \brief  Wraps PlayFab Server/Admin API calls that require a title entity.

    Not a standalone ISystem. Owned by PlayFabManager, which provides the
    title PFEntityHandle after authenticating with the Developer Secret Key.
    Access via:

        systemManager.GetSystem<PlayFabManager>()->Admin()

    All methods assert that a valid title entity handle has been set.

    \warning
    All methods require a title entity handle obtained via the Developer
    Secret Key. Do NOT call or compile into release builds.
    */
    class PlayFabAdminManager
    {
    public:
        PlayFabAdminManager() = default;
        ~PlayFabAdminManager() = default;

        // Non-copyable, non-movable
        PlayFabAdminManager(const PlayFabAdminManager&) = delete;
        PlayFabAdminManager& operator=(const PlayFabAdminManager&) = delete;
        PlayFabAdminManager(PlayFabAdminManager&&) = delete;
        PlayFabAdminManager& operator=(PlayFabAdminManager&&) = delete;


        // ── Called internally by PlayFabManager ───────────────────────────────

        /*!
        \brief  Stores the title entity handle obtained via
                PFAuthenticationGetEntityWithSecretKeyAsync.

        Called by PlayFabManager once admin auth completes. Do not call
        from game code.

        \param  titleEntityHandle  Borrowed handle — owned and closed by
                                   PlayFabManager. Valid for this manager's
                                   entire lifetime.
        */
        void SetTitleEntityHandle(PFEntityHandle titleEntityHandle);

        /*!
        \brief  Returns true once a valid title entity handle has been set.
        */
        [[nodiscard]] bool IsReady() const;


        // =====================================================================
        //  Title Data
        //  Global, game-wide key-value store on the PlayFab backend.
        //  Ideal for season configs, balance tables, and feature flags.
        // =====================================================================

        /*!
        \brief  Fetches one or more TitleData entries from PlayFab.

        \param  keys        Keys to fetch. Empty vector fetches ALL keys.
        \param  onSuccess   Called with an unordered_map<string,string> on success.
        \param  onFailure   Called with HRESULT + description on failure.

        \pre    IsReady() == true
        */
        void GetTitleData(
            const std::vector<std::string>& keys,
            OnAdminDataSuccess               onSuccess,
            OnAdminFailure                   onFailure = nullptr
        );

        /*!
        \brief  Writes a single TitleData key-value pair (creates or overwrites).

        \param  key         The TitleData key.
        \param  value       The string value to store.
        \param  onSuccess   Optional callback on success.
        \param  onFailure   Optional callback on failure.

        \pre    IsReady() == true
        */
        void SetTitleData(
            const std::string& key,
            const std::string& value,
            OnAdminSuccess     onSuccess = nullptr,
            OnAdminFailure     onFailure = nullptr
        );

        /*!
        \brief  Writes multiple TitleData key-value pairs sequentially.

        Chains each SetTitleData call inside the previous one's success callback,
        so writes are serialised one-at-a-time. This avoids E_PF_CONCURRENT_EDIT_ERROR
        which PlayFab returns when multiple writes hit the same title's data store
        simultaneously.

        \param  kvPairs     Map of { key -> value } to write. Written in an
                            unspecified but non-concurrent order.
        \param  onSuccess   Called once after every key has been written successfully.
        \param  onFailure   Called on the first key that fails; remaining keys are
                            not written after a failure.

        \pre    IsReady() == true
        */
        void SetTitleDataBatch(
            const std::unordered_map<std::string, std::string>& kvPairs,
            OnAdminSuccess                                       onSuccess = nullptr,
            OnAdminFailure                                       onFailure = nullptr
        );

        /*!
        \brief  Clears a TitleData key by writing an empty string.

        PlayFab has no explicit delete endpoint for TitleData; writing an
        empty value is the documented convention to "clear" a key.

        \param  key         The TitleData key to clear.
        \param  onSuccess   Optional callback on success.
        \param  onFailure   Optional callback on failure.

        \pre    IsReady() == true
        */
        void DeleteTitleData(
            const std::string& key,
            OnAdminSuccess     onSuccess = nullptr,
            OnAdminFailure     onFailure = nullptr
        );


        // =====================================================================
        //  Statistics  (PlayFab v2 Statistics API)
        //  Per-entity numeric stats that drive leaderboards.
        //  Stats must be defined before values can be written.
        // =====================================================================

        /*!
        \brief  Creates a new statistic definition on the title.

        A definition specifies the stat name, the column name (a sub-identifier
        used when writing/reading scores), per-column aggregation method
        (Last / Min / Max / Sum), and an automatic version-reset schedule.
        Must be called once before any entity can have values for that stat.

        \param  name            Stat name (a-Z, 0-9, '_', '-', '.', max 150 chars).
                                This is the identifier used in UpdateStatistics /
                                GetStatistics calls.
        \param  columnName      Name of the single score column within this stat
                                (e.g. "score", "time_ms"). Must be distinct from the
                                stat name. Used by PlayFab internally to identify the
                                column when the stat has multiple columns.
        \param  aggregation     How scores are aggregated: Last, Min, Max, Sum.
        \param  onSuccess       Called when the definition is created.
        \param  onFailure       Called on failure.

        \pre    IsReady() == true

        \par Example
        \code
            pfb->Admin().CreateStatisticDefinition(
                "HighScore", "score",
                PFStatisticsStatisticAggregationMethod::Max,
                []() { LOG_INFO("Stat created"); },
                [](HRESULT hr, const auto& msg) { LOG_ERROR(msg); }
            );
        \endcode
        */
        void CreateStatisticDefinition(
            const std::string& name,
            const std::string& columnName,
            PFStatisticsStatisticAggregationMethod     aggregation,
            OnAdminSuccess                             onSuccess = nullptr,
            OnAdminFailure                             onFailure = nullptr
        );

        /*!
        \brief  Deletes an existing statistic definition and all associated data.

        \warning Destructive. All entity values for this stat will be lost.

        \param  name        Name of the stat definition to delete.
        \param  onSuccess   Called on success.
        \param  onFailure   Called on failure.

        \pre    IsReady() == true
        */
        void DeleteStatisticDefinition(
            const std::string& name,
            OnAdminSuccess     onSuccess = nullptr,
            OnAdminFailure     onFailure = nullptr
        );

        /*!
        \brief  Writes or updates one statistic value for a given entity.

        The score string must be a valid int64 (e.g. "42000"). The actual
        stored value depends on the definition's aggregation method.

        \param  entityId    PlayFab entity ID of the target.
        \param  entityType  Entity type string (e.g. "title_player_account").
        \param  statName    Name of the statistic to update.
        \param  score       Score value as a string integer.
        \param  onSuccess   Called on success.
        \param  onFailure   Called on failure.

        \pre    IsReady() == true
        */
        void UpdateStatistics(
            const std::string& entityId,
            const std::string& entityType,
            const std::string& statName,
            const std::string& score,
            OnAdminSuccess     onSuccess = nullptr,
            OnAdminFailure     onFailure = nullptr
        );

        /*!
        \brief  Retrieves all current statistics for a given entity.

        \param  entityId    PlayFab entity ID to query.
        \param  entityType  Entity type string.
        \param  onSuccess   Called with a vector of StatisticValue on success.
        \param  onFailure   Called on failure.

        \pre    IsReady() == true

        \par Example
        \code
            pfb->Admin().GetStatistics(
                playerId, "title_player_account",
                [](const auto& stats) {
                    for (auto& s : stats)
                        LOG_INFO(s.name + ": " + s.scores[0]);
                },
                nullptr
            );
        \endcode
        */
        void GetStatistics(
            const std::string& entityId,
            const std::string& entityType,
            OnStatisticsSuccess  onSuccess,
            OnAdminFailure       onFailure = nullptr
        );

        /*!
        \brief  Bumps the version of a statistic, effectively resetting it for a
                new season or round. Old version data is archived by PlayFab.

        \param  name        Statistic name to reset.
        \param  onSuccess   Called with the new version number on success.
        \param  onFailure   Called on failure.

        \pre    IsReady() == true
        */
        void IncrementStatisticVersion(
            const std::string& name,
            OnVersionSuccess   onSuccess = nullptr,
            OnAdminFailure     onFailure = nullptr
        );

        /*!
        \brief  Lists all statistic definitions registered for this title.

        Useful for engine tooling or an admin debug panel.

        \param  onSuccess   Called with a vector of definition names on success.
        \param  onFailure   Called on failure.

        \pre    IsReady() == true
        */
        void ListStatisticDefinitions(
            std::function<void(const std::vector<std::string>& names)> onSuccess,
            OnAdminFailure                                              onFailure = nullptr
        );


        // =====================================================================
        //  Leaderboards  (PlayFab v2 Leaderboards API)
        //  Ranked views over statistic data.
        //  A leaderboard definition must exist before entries can be queried.
        // =====================================================================

        /*!
        \brief  Creates a new leaderboard definition.

        Leaderboard columns must match the columns of the linked statistic (if
        any). Sort direction (Descending / Ascending) is fixed at creation time.

        \param  name            Leaderboard name, unique per title.
        \param  columnName      Name for the single score column.
        \param  sortDirection   Descending (high score first) or Ascending.
        \param  sizeLimit       Maximum entries stored (e.g. 1000).
        \param  onSuccess       Called on success.
        \param  onFailure       Called on failure.

        \pre    IsReady() == true

        \par Example
        \code
            pfb->Admin().CreateLeaderboardDefinition(
                "GlobalHighScore", "Score",
                PFLeaderboardsLeaderboardSortDirection::Descending,
                1000,
                []() { LOG_INFO("Leaderboard ready"); },
                nullptr
            );
        \endcode
        */
        void CreateLeaderboardDefinition(
            const std::string& name,
            const std::string& columnName,
            PFLeaderboardsLeaderboardSortDirection   sortDirection,
            int32_t                                  sizeLimit,
            OnAdminSuccess                           onSuccess = nullptr,
            OnAdminFailure                           onFailure = nullptr
        );

        /*!
        \brief  Deletes a leaderboard definition and all its entries.

        \warning Destructive. All rankings for this leaderboard are lost.

        \param  name        Leaderboard name to delete.
        \param  onSuccess   Called on success.
        \param  onFailure   Called on failure.

        \pre    IsReady() == true
        */
        void DeleteLeaderboardDefinition(
            const std::string& name,
            OnAdminSuccess     onSuccess = nullptr,
            OnAdminFailure     onFailure = nullptr
        );

        /*!
        \brief  Fetches a page of the global leaderboard from the top.

        \param  leaderboardName  Name of the leaderboard to query.
        \param  pageSize         Number of entries to return (1–1000).
        \param  startPosition    1-based starting rank (nullptr = from position 1).
        \param  onSuccess        Called with a sorted vector of LeaderboardEntry.
        \param  onFailure        Called on failure.

        \pre    IsReady() == true

        \par Example
        \code
            pfb->Admin().GetLeaderboard(
                "GlobalHighScore", 10, nullptr,
                [](const auto& entries, uint32_t ver) {
                    for (auto& e : entries)
                        LOG_INFO("#" + std::to_string(e.rank) + " " + e.displayName);
                },
                nullptr
            );
        \endcode
        */
        void GetLeaderboard(
            const std::string& leaderboardName,
            uint32_t            pageSize,
            const uint32_t* startPosition,
            OnLeaderboardSuccess onSuccess,
            OnAdminFailure       onFailure = nullptr
        );

        /*!
        \brief  Fetches leaderboard entries centered around a specific entity.

        Returns entries ranked above and below the target entity, useful for
        showing a player their rank in context.

        \param  leaderboardName      Name of the leaderboard.
        \param  entityId             Target entity ID.
        \param  entityType           Target entity type.
        \param  maxSurroundingEntries  Total entries to return around the target.
        \param  onSuccess            Called with the surrounding entry window.
        \param  onFailure            Called on failure.

        \pre    IsReady() == true
        */
        void GetLeaderboardAroundEntity(
            const std::string& leaderboardName,
            const std::string& entityId,
            const std::string& entityType,
            uint32_t            maxSurroundingEntries,
            OnLeaderboardSuccess onSuccess,
            OnAdminFailure       onFailure = nullptr
        );

        /*!
        \brief  Server-side direct write to a leaderboard entry.

        Bypasses the player-facing SubmitScore path. Use for admin corrections
        or test seeding. Score is passed as a string integer.

        \param  leaderboardName  Leaderboard to update.
        \param  entityId         Entity whose entry to set.
        \param  score            Score as a string (e.g. "95000").
        \param  metadata         Optional opaque metadata (<50 UTF-8 bytes).
        \param  onSuccess        Called on success.
        \param  onFailure        Called on failure.

        \pre    IsReady() == true
        */
        void UpdateLeaderboardEntries(
            const std::string& leaderboardName,
            const std::string& entityId,
            const std::string& score,
            const std::string& metadata = "",
            OnAdminSuccess     onSuccess = nullptr,
            OnAdminFailure     onFailure = nullptr
        );

        /*!
        \brief  Bumps the leaderboard version, archiving current rankings and
                starting a fresh season.

        \param  name        Leaderboard to reset.
        \param  onSuccess   Called with the new version number.
        \param  onFailure   Called on failure.

        \pre    IsReady() == true
        */
        void IncrementLeaderboardVersion(
            const std::string& name,
            OnVersionSuccess   onSuccess = nullptr,
            OnAdminFailure     onFailure = nullptr
        );

        /*!
        \brief  Lists all leaderboard definitions registered for this title.

        \param  onSuccess   Called with a vector of definition names on success.
        \param  onFailure   Called on failure.

        \pre    IsReady() == true
        */
        void ListLeaderboardDefinitions(
            std::function<void(const std::vector<std::string>& names)> onSuccess,
            OnAdminFailure                                              onFailure = nullptr
        );


        // =====================================================================
        //  CloudScript / Azure Functions
        //  Server-side logic executed in Azure. Bridges the C++ SDK to your
        //  deployed Azure Function App (SubmitScore, GetSessionKey, etc.).
        // =====================================================================

        /*!
        \brief  Invokes a registered Azure Function by name.

        The function receives the entity context and the JSON parameter blob.
        The returned JSON string is forwarded to onSuccess. Use this as the
        bridge to your SubmitScore and GetSessionKey Azure Functions.

        \param  functionName     Name of the Azure Function (as registered in
                                 PlayFab Game Manager → Automation → Functions).
        \param  functionParamJson  JSON string to pass as FunctionArgument, or
                                   empty string for no arguments.
        \param  onSuccess        Called with (functionName, resultJson).
        \param  onFailure        Called on failure.

        \pre    IsReady() == true

        \par Example
        \code
            pfb->Admin().ExecuteFunction(
                "SubmitScore",
                R"({"score":42000,"hmac":"abc123"})",
                [](const auto& fn, const auto& result) {
                    LOG_INFO(fn + " returned: " + result);
                },
                [](HRESULT hr, const auto& msg) { LOG_ERROR(msg); }
            );
        \endcode
        */
        void ExecuteFunction(
            const std::string& functionName,
            const std::string& functionParamJson,
            OnFunctionSuccess   onSuccess,
            OnAdminFailure      onFailure = nullptr
        );

        /*!
        \brief  Queues a TitleData write for deferred execution.

        Pushes a key-value pair onto the internal write queue without
        launching an async operation immediately. The queued writes are
        drained one at a time by Update().

        \param  key         The TitleData key to write.
        \param  value       The string value to store.
        \param  onSuccess   Optional per-key success callback.
        \param  onFailure   Optional per-key failure callback.
        */
        void QueueTitleData(const std::string& key, const std::string& value,
            OnAdminSuccess onSuccess = nullptr,
            OnAdminFailure onFailure = nullptr);

        /*!
        \brief  Processes queued TitleData writes and polls async operations.

        Called every frame from PlayFabManager::Update(). Drains the
        internal write queue one entry at a time to avoid concurrent
        edit errors on the PlayFab backend.
        */
        void Update();


        // =====================================================================
        //  (Planned) Player Management
        // =====================================================================

        // void GetPlayerProfile(const std::string& playFabId, ...);
        // void BanPlayer(const std::string& playFabId, int durationHours, ...);
        // void RevokeInventoryItem(const std::string& playFabId, ...);


        // =====================================================================
        //  (Planned) Economy / Catalog
        // =====================================================================

        // void GetCatalogItems(const std::string& catalogVersion, ...);
        // void UpdateCatalogItems(...);


    private:
        // ── Async context structs ─────────────────────────────────────────────
        // Each async call heap-allocates one of these. XAsyncBlock must be
        // the LAST member so the struct's address equals the context pointer
        // stored in async.context, making the reinterpret_cast in the callback
        // correct. The callback is responsible for `delete`-ing the context.

        struct GetTitleDataContext
        {
            OnAdminDataSuccess onSuccess;
            OnAdminFailure     onFailure;
            XAsyncBlock        async{};
        };

        struct SetTitleDataContext
        {
            OnAdminSuccess onSuccess;
            OnAdminFailure onFailure;
            XAsyncBlock    async{};
        };

        // ── Statistics contexts ───────────────────────────────────────────────

        struct NoPayloadContext
        {
            OnAdminSuccess onSuccess;
            OnAdminFailure onFailure;
            XAsyncBlock    async{};
        };

        struct GetStatisticsContext
        {
            OnStatisticsSuccess onSuccess;
            OnAdminFailure      onFailure;
            XAsyncBlock         async{};
        };

        struct IncrementStatVersionContext
        {
            OnVersionSuccess onSuccess;
            OnAdminFailure   onFailure;
            XAsyncBlock      async{};
        };

        struct ListStatDefinitionsContext
        {
            std::function<void(const std::vector<std::string>&)> onSuccess;
            OnAdminFailure                                        onFailure;
            XAsyncBlock                                           async{};
        };

        // ── Leaderboard contexts ──────────────────────────────────────────────

        struct GetLeaderboardContext
        {
            OnLeaderboardSuccess onSuccess;
            OnAdminFailure       onFailure;
            XAsyncBlock          async{};
        };

        struct IncrementLBVersionContext
        {
            OnVersionSuccess onSuccess;
            OnAdminFailure   onFailure;
            XAsyncBlock      async{};
        };

        struct ListLBDefinitionsContext
        {
            std::function<void(const std::vector<std::string>&)> onSuccess;
            OnAdminFailure                                        onFailure;
            XAsyncBlock                                           async{};
        };

        // ── CloudScript context ───────────────────────────────────────────────

        struct ExecuteFunctionContext
        {
            OnFunctionSuccess onSuccess;
            OnAdminFailure    onFailure;
            XAsyncBlock       async{};
        };


        // ── Static completion callbacks ───────────────────────────────────────

        // TitleData

        /*!
        \brief  Completion callback for GetTitleData async operation.
        \param  async  Pointer to the completed XAsyncBlock.
        */
        static void CALLBACK OnGetTitleDataComplete(XAsyncBlock* async);

        /*!
        \brief  Completion callback for SetTitleData async operation.
        \param  async  Pointer to the completed XAsyncBlock.
        */
        static void CALLBACK OnSetTitleDataComplete(XAsyncBlock* async);

        // Statistics

        /*!
        \brief  Completion callback for async operations that return no payload.
        \param  async  Pointer to the completed XAsyncBlock.
        */
        static void CALLBACK OnNoPayloadComplete(XAsyncBlock* async);

        /*!
        \brief  Completion callback for GetStatistics async operation.
        \param  async  Pointer to the completed XAsyncBlock.
        */
        static void CALLBACK OnGetStatisticsComplete(XAsyncBlock* async);

        /*!
        \brief  Completion callback for IncrementStatisticVersion async operation.
        \param  async  Pointer to the completed XAsyncBlock.
        */
        static void CALLBACK OnIncrementStatVersionComplete(XAsyncBlock* async);

        /*!
        \brief  Completion callback for ListStatisticDefinitions async operation.
        \param  async  Pointer to the completed XAsyncBlock.
        */
        static void CALLBACK OnListStatDefinitionsComplete(XAsyncBlock* async);

        // Leaderboards

        /*!
        \brief  Completion callback for GetLeaderboard async operation.
        \param  async  Pointer to the completed XAsyncBlock.
        */
        static void CALLBACK OnGetLeaderboardComplete(XAsyncBlock* async);

        /*!
        \brief  Completion callback for IncrementLeaderboardVersion async operation.
        \param  async  Pointer to the completed XAsyncBlock.
        */
        static void CALLBACK OnIncrementLBVersionComplete(XAsyncBlock* async);

        /*!
        \brief  Completion callback for ListLeaderboardDefinitions async operation.
        \param  async  Pointer to the completed XAsyncBlock.
        */
        static void CALLBACK OnListLBDefinitionsComplete(XAsyncBlock* async);

        // CloudScript

        /*!
        \brief  Completion callback for ExecuteFunction async operation.
        \param  async  Pointer to the completed XAsyncBlock.
        */
        static void CALLBACK OnExecuteFunctionComplete(XAsyncBlock* async);


        // ── Private helpers ───────────────────────────────────────────────────

        /// Parses a PFLeaderboardsGetEntityLeaderboardResponse into a vector of LeaderboardEntry.
        static std::vector<LeaderboardEntry> ParseLeaderboardResponse(
            const PFLeaderboardsGetEntityLeaderboardResponse* response
        );

        /*!
        \brief  Calls onFailure (if set) with the given HRESULT and a
                formatted message. Safe to call with a null onFailure.
        */
        static void DispatchError(
            HRESULT               hr,
            const std::string& context,
            const OnAdminFailure& onFailure
        );


        // ── State ─────────────────────────────────────────────────────────────

        /// Borrowed handle — owned and closed by PlayFabManager.
        /// nullptr until SetTitleEntityHandle() is called.
        PFEntityHandle m_titleEntityHandle{ nullptr };

        struct TitleDataWriteCommand
        {
            std::string    key;
            std::string    value;
            OnAdminSuccess onSuccess; // per-key callback, optional
            OnAdminFailure onFailure;
        };

        /*!
        \brief  Sends a single SetTitleData request from the internal queue.

        Called internally by Update() to process one queued write at a time.
        On completion the next queued entry is dispatched.

        \param  key         The TitleData key to write.
        \param  value       The string value to store.
        \param  onSuccess   Optional callback on success.
        \param  onFailure   Optional callback on failure.
        */
        void ProcessSetTitleDataQueue(
            const std::string& key,
            const std::string& value,
            OnAdminSuccess     onSuccess = nullptr,
            OnAdminFailure     onFailure = nullptr
        );

        std::queue<TitleDataWriteCommand> m_titleDataQueue;
        bool                              m_titleDataWriteInFlight{ false };
    };

} // namespace Uma_Engine