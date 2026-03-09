/*!
\file   PlayFabAdminManager.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

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
  - TitleData : GetTitleData, SetTitleData, SetTitleDataBatch, DeleteTitleData

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

// ── PlayFab Services SDK ──────────────────────────────────────────────────────
#include <playfab/core/PFEntity.h>
#include <playfab/services/PFTitleDataManagement.h>
#include <playfab/services/PFTitleDataManagementTypes.h>

// ── XAsync (GDK / PlayFab Services on Win32) ──────────────────────────────────
#include <XAsync.h>

namespace Uma_Engine
{
    // =========================================================================
    //  Callback aliases
    // =========================================================================

    /// Fired on success when key-value data is returned (e.g. GetTitleData).
    using OnAdminDataSuccess = std::function<void(const std::unordered_map<std::string, std::string>& data)>;

    /// Fired on success when no payload is expected (e.g. SetTitleData).
    using OnAdminSuccess = std::function<void()>;

    /// Fired on any Admin API failure.
    /// \param hr       The HRESULT returned by the failed operation.
    /// \param message  Human-readable description of the failure.
    using OnAdminFailure = std::function<void(HRESULT hr, const std::string& message)>;


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

        Calls PFTitleDataManagementServerGetTitleDataAsync. The two-step
        GetResultSize / GetResult retrieval is handled internally; the caller
        only receives the final map in onSuccess.

        \param  keys        Keys to fetch. Empty vector fetches ALL keys.
        \param  onSuccess   Called with an unordered_map<string,string> on success.
        \param  onFailure   Called with HRESULT + description on failure.

        \pre    IsReady() == true

        \par Example
        \code
            pfb->Admin().GetTitleData(
                { "SeasonId", "MatchSettings" },
                [](const auto& kv) { auto id = kv.at("SeasonId"); },
                [](HRESULT hr, const auto& msg) { LOG_ERROR(msg); }
            );
        \endcode
        */
        void GetTitleData(
            const std::vector<std::string>& keys,
            OnAdminDataSuccess               onSuccess,
            OnAdminFailure                   onFailure = nullptr
        );

        /*!
        \brief  Writes a single TitleData key-value pair (creates or overwrites).

        Calls PFTitleDataManagementServerSetTitleDataAsync.
        Result is status-only; no payload is retrieved.

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
        \brief  Writes multiple TitleData key-value pairs in one logical call.

        Dispatches one independent SetTitleData call per key. Each key fires
        its own success/failure callback.

        \param  kvPairs     Map of { key -> value } to write.
        \param  onSuccess   Called for each key that succeeds.
        \param  onFailure   Called for each key that fails.

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
        // Each async call heap-allocates one of these. XAsyncBlock is the
        // FIRST member so that the static callback can safely cast
        // XAsyncBlock* back to the full context type. The callback is
        // responsible for `delete`-ing the context when done.

        /// Context carried through a GetTitleData async operation.
        struct GetTitleDataContext
        {
            XAsyncBlock        async{};   // must be first
            OnAdminDataSuccess onSuccess;
            OnAdminFailure     onFailure;
        };

        /// Context carried through a SetTitleData async operation.
        struct SetTitleDataContext
        {
            XAsyncBlock    async{};       // must be first
            OnAdminSuccess onSuccess;
            OnAdminFailure onFailure;
        };


        // ── Static completion callbacks ───────────────────────────────────────

        static void CALLBACK OnGetTitleDataComplete(XAsyncBlock* async);
        static void CALLBACK OnSetTitleDataComplete(XAsyncBlock* async);


        // ── Private helpers ───────────────────────────────────────────────────

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
    };

} // namespace Uma_Engine