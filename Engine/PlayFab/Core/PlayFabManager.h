/*!
\file   PlayFabManager.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Engine-level facade for the PlayFab integration in Uma Engine.

Implements ISystem so it participates in the engine lifecycle
(Init / Update / Shutdown) via SystemManager. Owns the two sub-managers:

    PlayFabAdminManager   -- Server/Admin API  (dev builds only)
    PlayFabPlayerManager  -- Client/Player API (runtime, release-safe)

=== SDK lifecycle (PlayFab Services C SDK) ===

Init() does three things in order:
  1. PFInitialize()                             -- boots the SDK
  2. PFServiceConfigCreateHandle()              -- creates service config from TitleId
  3. PFAuthenticationGetEntityWithSecretKeyAsync()
                                               -- authenticates as the title entity
                                                  using the Developer Secret Key, async.
                                                  On completion the PFEntityHandle is
                                                  passed to PlayFabAdminManager via
                                                  SetTitleEntityHandle().

Admin().IsReady() becomes true only after step 3 completes.
Player() is usable immediately after Init() returns (player login is separate).

Shutdown() closes all handles in reverse order then calls PFUninitializeAsync().

=== Player Authentication ===

Three login paths are provided, all async. On success the resulting
PFEntityHandle is forwarded to PlayFabPlayerManager via
SetPlayerEntityHandle(), after which Player().IsLoggedIn() returns true.

    LoginWithCustomID()    -- guest / random account (GUID stored locally)
    Logout()               -- close player handle, clears Player().IsLoggedIn()

=== Usage ===

    // Engine boot
    auto* pfb = systemManager.RegisterSystem<PlayFabManager>();
    pfb->SetCredentials("YOUR_TITLE_ID", "YOUR_DEV_SECRET_KEY");
    systemManager.Init();

    // Player login (game code)
    pfb->LoginWithCustomID("some-guid", true,
        []{ UMA_INFO("Logged in!"); },
        [](HRESULT hr, const std::string& msg){ UMA_ERROR(msg); });

    // Admin API — ready asynchronously, poll or use the callback
    if (pfb->Admin().IsReady())
        pfb->Admin().GetTitleData(...);

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

// ── Engine ────────────────────────────────────────────────────────────────────
#include "Core/SystemType.h"

// ── Sub-managers ──────────────────────────────────────────────────────────────
#include "Admin/PlayFabAdminManager.h"
#include "Player/PlayFabPlayerManager.h"

// ── PlayFab Services SDK ──────────────────────────────────────────────────────
#include <playfab/core/PFCore.h>
#include <playfab/core/PFServiceConfig.h>
#include <playfab/core/PFAuthentication.h>
#include <playfab/core/PFEntity.h>
#include <playfab/services/PFServices.h>

// ── XAsync / XTaskQueue (GDK / PlayFab Services on Win32) ─────────────────────
#include <XAsync.h>
#include <XTaskQueue.h>

// ── Standard library ──────────────────────────────────────────────────────────
#include <functional>
#include <memory>
#include <string>

namespace Uma_Engine
{
    // =========================================================================
    //  Callback aliases (player auth)
    // =========================================================================

    /// Called when a player login or registration completes successfully.
    using OnPlayerLoginSuccess = std::function<void()>;

    /// Called when a player login or registration fails.
    using OnPlayerLoginFailure = std::function<void(HRESULT hr, const std::string& msg)>;


    // =========================================================================
    //  PlayFabManager   (ISystem facade)
    // =========================================================================

    /*!
    \class  PlayFabManager
    \brief  ISystem facade that owns the PlayFab SDK lifecycle and the two
            sub-managers (Admin and Player).

    Register and configure before systemManager.Init():

    \code
        auto* pfb = systemManager.RegisterSystem<PlayFabManager>();
        pfb->SetCredentials("TITLE_ID", "DEV_SECRET_KEY"); // omit key in release
        systemManager.Init();
    \endcode

    Admin API becomes available asynchronously after Init(). Poll
    Admin().IsReady() or supply an onAdminReady callback to SetCredentials().
    */
    class PlayFabManager : public ISystem
    {
    public:
        PlayFabManager() = default;
        ~PlayFabManager() = default;

        // Non-copyable, non-movable (owned by SystemManager)
        PlayFabManager(const PlayFabManager&) = delete;
        PlayFabManager& operator=(const PlayFabManager&) = delete;
        PlayFabManager(PlayFabManager&&) = delete;
        PlayFabManager& operator=(PlayFabManager&&) = delete;


        // ── Configuration — call BEFORE Init() ───────────────────────────────

        /*!
        \brief  Supplies the PlayFab Title ID and Developer Secret Key.

        Must be called before systemManager.Init(). The secret key is only
        required for Admin API access; omit it in release builds.

        \param  titleId       PlayFab Title ID (e.g. "1A2B3C").
        \param  devSecretKey  Developer Secret Key. Empty = no Admin access.
        \param  onAdminReady  Optional — fired when admin auth succeeds and
                              Admin().IsReady() becomes true.
        \param  onAdminFailed Optional — fired if admin auth fails.
        */
        void SetCredentials(
            const std::string& titleId,
            const std::string& devSecretKey = "",
            OnAdminSuccess     onAdminReady = nullptr,
            OnAdminFailure     onAdminFailed = nullptr
        );


        // ── ISystem ───────────────────────────────────────────────────────────

        /*!
        \brief  Initialises the PlayFab SDK, creates the service config, and
                kicks off title-entity authentication if a secret key was given.
        \pre    SetCredentials() has been called with a non-empty titleId.
        */
        void Init()             override;

        /*!
        \brief  Required by ISystem contract. Unused — SDK manages its own threads.
        \param  dt  Delta time (not used).
        */
        void Update(float dt)   override;

        /*!
        \brief  Closes all PlayFab handles and uninitialises the SDK.
        */
        void Shutdown()         override;


        // ── State queries ─────────────────────────────────────────────────────

        /*!
        \brief  Returns true after Init() has successfully initialised the SDK
                and created the service config handle.
        */
        [[nodiscard]] bool IsReady()       const;

        /*!
        \brief  Returns true if a Developer Secret Key was supplied.
                Does NOT mean async auth has completed — use Admin().IsReady().
        */
        [[nodiscard]] bool HasAdminAccess() const;


        // ── Player Authentication ─────────────────────────────────────────────

        /*!
        \brief  Logs in as a player using a custom ID string.

        Suitable for guest / anonymous accounts. Pass a locally stored GUID
        as the customId so the same account is recovered on subsequent launches.
        Set createAccount to true to auto-create an account on first use.

        On success the resulting PFEntityHandle is forwarded to
        PlayFabPlayerManager and Player().IsLoggedIn() becomes true.

        \param  customId       Arbitrary unique string identifying this player.
        \param  createAccount  If true, creates the account when it does not exist.
        \param  onSuccess      Optional — called when login succeeds.
        \param  onFailure      Optional — receives HRESULT and message on failure.
        \pre    IsReady() == true
        */
        void LoginWithCustomID(
            const std::string& customId,
            bool                  createAccount = true,
            OnPlayerLoginSuccess  onSuccess = nullptr,
            OnPlayerLoginFailure  onFailure = nullptr
        );

        /*!
        \brief  Closes the current player entity handle and clears the logged-in
                state in PlayFabPlayerManager. Safe to call if not logged in.
        */
        void Logout();


        // ── Sub-manager accessors ─────────────────────────────────────────────

        /*!
        \brief  Returns a reference to the Admin sub-manager.
        \note   Check Admin().IsReady() before making any Admin API calls.
        \pre    IsReady() == true
        */
        PlayFabAdminManager& Admin();
        const PlayFabAdminManager& Admin()  const;

        /*!
        \brief  Returns a reference to the Player sub-manager.
        \note   Check Player().IsLoggedIn() before making any Player API calls.
        \pre    IsReady() == true
        */
        PlayFabPlayerManager& Player();
        const PlayFabPlayerManager& Player() const;

        // healper to generate UUID4 for customid usage
        std::string GenerateUUID4();


    private:
        // ── Async context structs ─────────────────────────────────────────────
        // XAsyncBlock is always the first member so the static callback can
        // safely cast XAsyncBlock* back to the full context struct.

        /// Context for PFAuthenticationGetEntityWithSecretKeyAsync (admin auth).
        struct TitleAuthContext
        {
            XAsyncBlock     async{};    ///< Must be first.
            PlayFabManager* manager;    ///< Back-pointer.
        };

        /// Context for PFAuthenticationLoginWithCustomIDAsync.
        struct LoginWithCustomIDContext
        {
            XAsyncBlock          async{};       ///< Must be first.
            PlayFabManager* manager;       ///< Back-pointer.
            OnPlayerLoginSuccess onSuccess;
            OnPlayerLoginFailure onFailure;
        };

        // ── Internal helpers ──────────────────────────────────────────────────

        /*!
        \brief  Allocates a TitleAuthContext and calls
                PFAuthenticationGetEntityWithSecretKeyAsync.
                Called from Init() when a secret key is present.
        */
        void AuthenticateAsTitle();


        // ── Static async callbacks ────────────────────────────────────────────

        /*!
        \brief  SDK completion callback for GetEntityWithSecretKey (admin).
                Retrieves the PFEntityHandle and passes it to PlayFabAdminManager.
        */
        static void CALLBACK OnGetEntityWithSecretKeyComplete(XAsyncBlock* async);

        /*!
        \brief  SDK completion callback for LoginWithCustomID.
                Retrieves the PFEntityHandle and passes it to PlayFabPlayerManager.
        */
        static void CALLBACK OnLoginWithCustomIDComplete(XAsyncBlock* async);


        // ── Private helpers ───────────────────────────────────────────────────

        /*!
        \brief  Calls onFailure (if set) with the given HRESULT and a
                formatted message. Safe to call with a null onFailure.
        */
        static void DispatchError(
            HRESULT                      hr,
            const std::string& context,
            const OnAdminFailure& onFailure
        );

        // ── Credentials ───────────────────────────────────────────────────────
        std::string    m_titleId;
        std::string    m_devSecretKey;      ///< Empty in release builds.

        OnAdminSuccess m_onAdminReady;      ///< Fired when admin auth succeeds.
        OnAdminFailure m_onAdminFailed;     ///< Fired when admin auth fails.


        // ── SDK handles ───────────────────────────────────────────────────────

        /// Created in Init(). Required for all login calls (holds TitleId +
        /// endpoint). Closed in Shutdown().
        PFServiceConfigHandle m_serviceConfig{ nullptr };

        /// Obtained after GetEntityWithSecretKeyAsync completes.
        /// Passed to PlayFabAdminManager::SetTitleEntityHandle() and kept here
        /// so Shutdown() can close it via PFEntityCloseHandle().
        PFEntityHandle m_titleEntityHandle{ nullptr };


        // ── Task queue ────────────────────────────────────────────────────────
        /// Explicit task queue for all PlayFab async operations.
        /// Created in Init(), dispatched in Update(), drained in Shutdown().
        XTaskQueueHandle m_taskQueue{ nullptr };

        // ── State ─────────────────────────────────────────────────────────────
        bool m_ready{ false };


        // ── Sub-managers ──────────────────────────────────────────────────────
        std::unique_ptr<PlayFabAdminManager>  m_adminManager;
        std::unique_ptr<PlayFabPlayerManager> m_playerManager;
    };

} // namespace Uma_Engine