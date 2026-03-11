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
Provides Player-tier PlayFab API operations for Uma Engine.

Current scope:


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
    /// Fired on success when no payload is expected (e.g. SetTitleData, CreateStatisticDefinition).
    using OnPlayerSuccess = std::function<void()>;

    /// Fired on any Admin API failure.
    /// \param hr       The HRESULT returned by the failed operation.
    /// \param message  Human-readable description of the failure.
    using OnPlayerFailure = std::function<void(HRESULT hr, const std::string& message)>;

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


    private:

        /*!
        \brief  Calls onFailure (if set) with the given HRESULT and a
                formatted message. Safe to call with a null onFailure.
        */
        static void DispatchError(
            HRESULT               hr,
            const std::string& context,
            const OnPlayerFailure& onFailure
        );


        // ── State ─────────────────────────────────────────────────────────────

        /// Borrowed handle — owned and closed by PlayFabManager.
        /// nullptr until SetTitleEntityHandle() is called.
        PFEntityHandle m_titleEntityHandle{ nullptr };
    };

} // namespace Uma_Engine