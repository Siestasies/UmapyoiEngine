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

=== Usage ===

    // Engine boot
    auto* pfb = systemManager.RegisterSystem<PlayFabManager>();
    pfb->SetCredentials("YOUR_TITLE_ID", "YOUR_DEV_SECRET_KEY");
    systemManager.Init();

    // Game code — Admin is ready asynchronously, poll or use the callback
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

// ── XAsync (GDK / PlayFab Services on Win32) ──────────────────────────────────
#include <XAsync.h>

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
        void Init() override;

        /*!
        \brief  Required by ISystem contract. Unused — SDK manages its own threads.
        \param  dt  Delta time (not used).
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
        \brief  Returns true if a Developer Secret Key was supplied.
                Does NOT mean async auth has completed — use Admin().IsReady().
        */
        [[nodiscard]] bool HasAdminAccess() const;


        // ── Sub-manager accessors ─────────────────────────────────────────────

        /*!
        \brief  Returns a reference to the Admin sub-manager.
        \note   Check Admin().IsReady() before making any Admin API calls.
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
        // ── Async context struct ──────────────────────────────────────────────
        // Used for the one async operation PlayFabManager itself owns:
        // PFAuthenticationGetEntityWithSecretKeyAsync.
        // XAsyncBlock is first so the static callback can cast back safely.

        struct TitleAuthContext
        {
            XAsyncBlock     async{};        // must be first
            PlayFabManager* manager;        // back-pointer to set handle + call callbacks
        };


        // ── Internal helpers ──────────────────────────────────────────────────

        /*!
        \brief  Allocates a TitleAuthContext and calls
                PFAuthenticationGetEntityWithSecretKeyAsync.
                Called from Init() when a secret key is present.
        */
        void AuthenticateAsTitle();


        // ── Private helpers ───────────────────────────────────────────────────

        /*!
        \brief  Calls onFailure (if set) with the given HRESULT and a
                formatted message. Safe to call with a null onFailure.
        */
        static void DispatchError(
            HRESULT               hr,
            const std::string& context,
            const OnManagerFailure& onFailure
        );

        /*!
        \brief  SDK completion callback for GetEntityWithSecretKey.
                Retrieves the PFEntityHandle and hands it to PlayFabAdminManager.
        */
        static void CALLBACK OnGetEntityWithSecretKeyComplete(XAsyncBlock* async);


        // ── Credentials ───────────────────────────────────────────────────────
        std::string    m_titleId;
        std::string    m_devSecretKey;      ///< Empty in release builds.

        OnAdminSuccess m_onAdminReady;      ///< Fired when admin auth succeeds.
        OnAdminFailure m_onAdminFailed;     ///< Fired when admin auth fails.


        // ── SDK handles ───────────────────────────────────────────────────────

        /// Created in Init(). Required for PFServiceConfig-based API calls
        /// (client/player login etc.). Closed in Shutdown().
        PFServiceConfigHandle m_serviceConfig{ nullptr };

        /// Obtained after GetEntityWithSecretKeyAsync completes.
        /// Passed to PlayFabAdminManager::SetTitleEntityHandle() and then
        /// kept here so Shutdown() can close it via PFEntityCloseHandle().
        PFEntityHandle m_titleEntityHandle{ nullptr };


        // ── State ─────────────────────────────────────────────────────────────
        bool m_ready{ false };


        // ── Sub-managers ──────────────────────────────────────────────────────
        std::unique_ptr<PlayFabAdminManager>  m_adminManager;
        std::unique_ptr<PlayFabPlayerManager> m_playerManager;
    };

} // namespace Uma_Engine