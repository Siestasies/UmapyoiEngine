/*!
\file   FeedbackTypes.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines configuration constants, colour presets, type tags, and pool slot
structures for the floating feedback number system used to display damage,
healing, mana, and warning values in the game UI.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../Core/UITypes.h"

namespace Uma_UI
{
    // =========================================================================
    //  Tuning constants
    // =========================================================================
    /*!
     * \namespace FeedbackConfig
     * \brief Tuning constants and color presets for the floating feedback number system.
     */
    namespace FeedbackConfig
    {
        constexpr int   kPoolSize = 32;
        constexpr float kLifetime = 1.2f;    // seconds per number
        constexpr float kLifetimeMana = 0.85f;   // mana numbers fade faster � secondary info
        constexpr float kRiseSpeed = 80.0f;   // px/sec upward drift
        constexpr float kFallSpeed = 40.0f;   // px/sec downward drift (mana spend)
        constexpr float kSpreadRadius = 28.0f;   // max horizontal jitter (px)
        constexpr float kFontSizeNormal = 2.f;
        constexpr float kFontSizeAffinity = 3.f;
        constexpr float kFontSizeCrit = 3.f;
        constexpr float kFontSizeMana = 1.f;   // mana numbers are smaller � subordinate to damage
        constexpr float kFontSizeWarning = 1.f;
        constexpr float kFadeStartFraction = 0.55f;   // fraction of lifetime before alpha fades
        constexpr float kPunchDuration = 0.10f;   // seconds for spawn scale punch-in
        constexpr float kPunchPeak = 1.5f;    // peak scale multiplier during punch

        /*! \brief Returns white color for normal damage numbers. \return Color for normal feedback. */
        inline Color NormalColor() { return { 1.0f, 1.0f, 1.0f, 1.0f }; }
        /*! \brief Returns warm yellow for affinity/ability numbers. \return Color for affinity feedback. */
        inline Color AffinityColor() { return { 1.0f, 0.92f, 0.2f, 1.0f }; }
        /*! \brief Returns orange for critical hit numbers. \return Color for critical feedback. */
        inline Color CritColor() { return { 1.0f, 0.55f, 0.0f, 1.0f }; }
        /*! \brief Returns bright green for healing numbers. \return Color for heal feedback. */
        inline Color HealColor() { return { 0.1f, 1.0f, 0.1f, 1.0f }; }
        /*! \brief Returns red for player hit numbers. \return Color for player-hit feedback. */
        inline Color PlayerHitColor() { return { 1.0f, 0.1f, 0.1f, 1.0f }; }
        /*! \brief Returns deep blue for mana spend numbers. \return Color for mana-spend feedback. */
        inline Color ManaSpendColor() { return { 0.1f, 0.45f, 1.0f, 1.0f }; }
        /*! \brief Returns lighter blue for mana gain numbers. \return Color for mana-gain feedback. */
        inline Color ManaGainColor() { return { 0.2f, 0.75f, 1.0f, 1.0f }; }
        /*! \brief Returns red-pink for warning messages. \return Color for warning feedback. */
        inline Color WarningColor() { return { 1.0f, 0.3f, 0.3f, 1.0f }; }
    }

    // =========================================================================
    //  Type tag
    // =========================================================================
    /*!
     * \enum FeedbackType
     * \brief Type tag controlling the visual style of a floating feedback number.
     */
    enum class FeedbackType : uint8_t
    {
        Normal = 0,
        Affinity = 1,
        Critical = 2,
        Heal = 3,
        PlayerHit = 4,
        ManaSpend = 5,   // blue, drifts downward, prefix "-"
        ManaGain = 6,    // cyan, drifts upward,   prefix "+"
        Warning = 7
    };

    // =========================================================================
    //  Internal pool slot  (one per pre-created Text entity)
    // =========================================================================
    /*!
     * \struct Slot
     * \brief Internal pool slot representing one pre-created floating text entity.
     */
    struct Slot
    {
        Uma_ECS::Entity textEntity = static_cast<Uma_ECS::Entity>(-1);

        float baseNdcX = 0.0f;   // NDC x at spawn
        float baseNdcY = 0.0f;   // NDC y at spawn
        float jitterNdcX = 0.0f;   // fixed horizontal jitter chosen at spawn
        float elapsed = 0.0f;
        float lifetime = FeedbackConfig::kLifetime;
        float baseFontSize = FeedbackConfig::kFontSizeNormal;

        bool fall = false;
        bool  alive = false;
    };
}