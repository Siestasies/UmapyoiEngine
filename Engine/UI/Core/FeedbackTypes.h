#pragma once

#include "../Core/UITypes.h"

namespace Uma_UI
{
    // =========================================================================
    //  Tuning constants
    // =========================================================================
    namespace FeedbackConfig
    {
        constexpr int   kPoolSize = 32;
        constexpr float kLifetime = 1.2f;    // seconds per number
        constexpr float kLifetimeMana = 0.85f;   // mana numbers fade faster — secondary info
        constexpr float kRiseSpeed = 80.0f;   // px/sec upward drift
        constexpr float kFallSpeed = 40.0f;   // px/sec downward drift (mana spend)
        constexpr float kSpreadRadius = 28.0f;   // max horizontal jitter (px)
        constexpr float kFontSizeNormal = 2.f;
        constexpr float kFontSizeAffinity = 3.f;
        constexpr float kFontSizeCrit = 3.f;
        constexpr float kFontSizeMana = 1.f;   // mana numbers are smaller — subordinate to damage
        constexpr float kFontSizeWarning = 1.f;
        constexpr float kFadeStartFraction = 0.55f;   // fraction of lifetime before alpha fades
        constexpr float kPunchDuration = 0.10f;   // seconds for spawn scale punch-in
        constexpr float kPunchPeak = 1.5f;    // peak scale multiplier during punch

        inline Color NormalColor() { return { 1.0f, 1.0f, 1.0f, 1.0f }; }  // white (normal)
        inline Color AffinityColor() { return { 1.0f, 0.92f, 0.2f, 1.0f }; } // warm yellow (ability)
        inline Color CritColor() { return { 1.0f, 0.55f, 0.0f, 1.0f }; } // orange (critical)
        inline Color HealColor() { return { 0.1f, 1.0f, 0.1f, 1.0f }; }  // bright green
        inline Color PlayerHitColor() { return { 1.0f, 0.1f, 0.1f, 1.0f }; }  // red
        inline Color ManaSpendColor() { return { 0.1f, 0.45f, 1.0f, 1.0f }; } // deep blue
        inline Color ManaGainColor() { return { 0.2f, 0.75f, 1.0f, 1.0f }; } // lighter blue
        inline Color WarningColor() { return { 1.0f, 0.3f, 0.3f, 1.0f }; }   // red-pink for warnings
    }

    // =========================================================================
    //  Type tag
    // =========================================================================
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