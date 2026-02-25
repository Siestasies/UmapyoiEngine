#pragma once

#include "EventType.h"
#include "../UI/Core/FeedbackTypes.h"

namespace Uma_UI
{
    /*!
     * \struct SpawnFeedbackEvent
     * \brief  Request to display a floating number at a world-space position.
     *
     *  Inherits Uma_Engine::Event so it works with EventSystem::Emit<>() and
     *  EventSystem::Subscribe<SpawnFeedbackEvent, FeedbackSystem>().
     *  Priority is left at Normal (default) — numbers are cosmetic, not critical.
     */
    struct SpawnFeedbackEvent : public Uma_Engine::Event
    {
        float            worldX = 0.0f;
        float            worldY = 0.0f;
        std::string      value;
        FeedbackType     type = FeedbackType::Normal;

        SpawnFeedbackEvent() = default;
        SpawnFeedbackEvent(float wx, float wy, const std::string val, FeedbackType t)
            : worldX(wx), worldY(wy), value(val), type(t) {
        }
    };
}