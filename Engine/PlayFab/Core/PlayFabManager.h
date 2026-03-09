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
  1. PFInitialize()               -- boots the SDK and its task queue
  2. PFServiceConfigCreateHandle() -- creates the service config from TitleId
  3. PFAuthenticationGetEntityWithSecretKeyAsync()
                                  -- authenticates as the title entity using
                                     the Developer Secret Key, async.
                                     On completion, the resulting PFEntityHandle
                                     is handed to PlayFabAdminManager via
                                     SetTitleEntityHandle().

Admin().IsReady() becomes true only after step 3 completes.
Player() is ready immediately after Init() returns (player login is separate).

Shutdown() closes handles in reverse order and calls PFUninitializeAsync().

=== Usage ===

    // Engine boot
    auto* pfb = systemManager.RegisterSystem<PlayFabManager>();
    pfb->SetCredentials("YOUR_TITLE_ID", "YOUR_DEV_SECRET_KEY");
    systemManager.Init();

    // Later in game code -- check Admin().IsReady() before calling Admin APIs
    if (pfb->Admin().IsReady())
        pfb->Admin().GetTitleData(...);

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

// ── Engine ────────────────────────────────────────────────────────────────────
#include "Core/SystemType.h"

// ── Sub-managers ──────────────────────────────────────────────────────────────
#include "PlayFabAdminManager.h"
//#include "PlayFabPlayerManager.h"

// ── PlayFab Services SDK ──────────────────────────────────────────────────────
#include <playfab/core/PFCore.h>
#include <playfab/core/PFServiceConfig.h>
#include <playfab/core/PFAuthentication.h>
#include <playfab/core/PFEntity.h>

// ── Standard library ──────────────────────────────────────────────────────────
#include <memory>
#include <string>

namespace Uma_Engine
{
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

    Admin API becomes available asynchronously after Init() — poll
    Admin().IsReady() or supply an OnAdminReady callback via SetCredentials.
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
        required for Admin API access; leave it empty in release builds.

        Optionally supply onAdminReady to be notified when the async
        secret-key authentication completes and Admin().IsReady() becomes true.

        \param  titleId         PlayFab Title ID (e.g. "1A2B3C").
        \param  devSecretKey    Developer Secret Key. Empty = no Admin access.
        \param  onAdminReady    Optional callback fired when admin auth succeeds.
        \param  onAdminFailed   Optional callback fired if admin auth fails.
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
                kicks off title-entity authentication (if a secret key was given).

        \pre    SetCredentials() has been called with a non-empty titleId.
        */
        void Init() override;

        /*!
        \brief  Required by ISystem contract; unused (SDK uses its own threads).
        \param  dt  Delta time — not used.
        */
        void Update(float dt) override;

        /*!
        \brief  Closes all PlayFab handles and uninitialises the SDK.
        */
        void Shutdown() override;


        // ── State queries ─────────────────────────────────────────────────────

        /*!
        \brief  Returns true after Init() has successfully initialised the SDK
                and created the service config handle.
        */
        [[nodiscard]] bool IsReady() const;

        /*!
        \brief  Returns true if a Developer Secret Key was provided.
                Does NOT imply the async auth has completed — use
                Admin().IsReady() for that.
        */
        [[nodiscard]] bool HasAdminAccess() const;


        // ── Sub-manager accessors ─────────────────────────────────────────────

        /*!
        \brief  Returns a reference to the Admin sub-manager.

        \note   Admin().IsReady() must be true before making Admin API calls.
                The handle is set asynchronously after Init().

        \pre    IsReady() == true
        */
        PlayFabAdminManager& Admin();

        /*!
        \brief  Returns a reference to the Player sub-manager.

        \pre    IsReady() == true
        */
        //PlayFabPlayerManager& Player();

        const PlayFabAdminManager& Admin()  const;
        //const PlayFabPlayerManager& Player() const;


    private:
        // ── Internal helpers ──────────────────────────────────────────────────

        /*!
        \brief  Kicks off PFAuthenticationGetEntityWithSecretKeyAsync.
                Called from Init() when a secret key is present.
        */
        void AuthenticateAsTitle();

        /// XAsyncBlock completion callback for PFAuthenticationGetEntityWithSecretKeyAsync.
        static void CALLBACK OnGetEntityWithSecretKeyComplete(XAsyncBlock* async);


        // ── Credentials ───────────────────────────────────────────────────────
        std::string    m_titleId;
        std::string    m_devSecretKey;      ///< Empty in release builds.

        /// Optional callbacks supplied via SetCredentials().
        OnAdminSuccess m_onAdminReady;
        OnAdminFailure m_onAdminFailed;


        // ── SDK handles ───────────────────────────────────────────────────────

        /// Created in Init() via PFServiceConfigCreateHandle.
        /// Used for all API calls and for player logins.
        PFServiceConfigHandle m_serviceConfig{ nullptr };

        /// Obtained after PFAuthenticationGetEntityWithSecretKeyAsync completes.
        /// Passed to PlayFabAdminManager::SetTitleEntityHandle().
        /// Closed in Shutdown() via PFEntityCloseHandle.
        PFEntityHandle m_titleEntityHandle{ nullptr };


        // ── State ─────────────────────────────────────────────────────────────
        bool m_ready{ false };


        // ── Sub-managers ──────────────────────────────────────────────────────
        std::unique_ptr<PlayFabAdminManager>  m_adminManager;
        //std::unique_ptr<PlayFabPlayerManager> m_playerManager;
    };

} // namespace Uma_Engine