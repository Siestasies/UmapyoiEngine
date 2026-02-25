#pragma once

#include "EventType.h"
#include "../UI/Core/FeedbackTypes.h"

namespace Uma_UI
{
    /*!
     * \struct SpawnNumberEvent
     * \brief  Request to display a floating number at a world-space position.
     *
     *  Inherits Uma_Engine::Event so it works with EventSystem::Emit<>() and
     *  EventSystem::Subscribe<SpawnNumberEvent, FeedbackSystem>().
     *  Priority is left at Normal (default) — numbers are cosmetic, not critical.
     */
    struct SpawnNumberEvent : public Uma_Engine::Event
    {
        float            worldX = 0.0f;
        float            worldY = 0.0f;
        int              amount = 0;
        NumberType type = NumberType::Normal;

        SpawnNumberEvent() = default;
        SpawnNumberEvent(float wx, float wy, int amt, NumberType t)
            : worldX(wx), worldY(wy), amount(amt), type(t) {
        }
    };
}