/*!
\file   FeedbackSystem.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Declares FeedbackSystem — a pooled, single-canvas floating damage number
system integrated with the engine's EventSystem.

ARCHITECTURE
────────────────────────────────────────────────────────────────────────────
  Canvas tier  (1 entity, created once on Init, never destroyed mid-session)
  │   Canvas            sortingOrder = 999, ConstantPixelSize
  │   RectTransform     full-screen stretch (anchorMin=0,0 anchorMax=1,1)
  │   Transform         identity
  │
  └─ Text tier  (up to kPoolSize entities, children of the single Canvas)
       Text              content, font, size, color (alpha drives fade)
       RectTransform     anchoredPosition updated each frame; pivot=(0.5,0.5)
       Transform         identity  (parent = Canvas entity)

  All Text entities are created once in Init and reused via the pool.
  Spawning a new number writes into an idle slot — zero ECS create/destroy
  calls on any hot path.

EVENT INTEGRATION
────────────────────────────────────────────────────────────────────────────
  Any system or Lua script emits SpawnFeedbackEvent through EventSystem.
  FeedbackSystem subscribes to it in Init() under the FeedbackSystem
  type tag (matching the existing Subscribe<T,U> pattern from EventSystem.h).

  Lua global:
      SpawnDamageNumber(worldX, worldY, amount [, typeString])
  typeString: "normal" | "crit" | "heal" | "playerhit"

POOL EVICTION
────────────────────────────────────────────────────────────────────────────
  When all kPoolSize slots are live, the slot furthest through its lifetime
  (closest to expiry, most faded) is evicted — no visible pop.

All content (C) 2026 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../Core/FeedbackTypes.h"

#include "../../ECS/Core/Coordinator.hpp"
#include "../../Core/EventSystem.h"
#include "../../Systems/Graphics.hpp"
#include "../../Events/ApplicationEvents.h"   // LoadSceneRequestEvent

#include <string>
#include <cstdint>

namespace Uma_UI { class UISystem; }   // forward-declare

namespace Uma_UI
{
    class FeedbackSystem
    {
    public:
        FeedbackSystem()  = default;
        ~FeedbackSystem() = default;

        /*!
         * \brief Creates the shared Canvas, pre-allocates the Text pool,
         *        and subscribes to SpawnFeedbackEvent.
         */
        void Init(Uma_ECS::Coordinator*    coord,
                  Uma_Engine::Graphics*    gfx,
                  Uma_Engine::EventSystem* events,
                  UISystem*                uiSystem,
                  const std::string&       fontPath = "Assets/Fonts/Fujimaru-Regular.ttf");

        /*!
         * \brief Advances all live slots. Call every frame before UISystem::Update().
         */
        void Update(float dt);

        /*!
         * \brief Destroys the Canvas hierarchy and unsubscribes from events.
         */
        void Shutdown();

        /*!
         * \brief Tears down the current Canvas/pool and rebuilds them fresh.
         *
         * Call this whenever the scene changes. Internally it is also wired
         * to LoadSceneRequestEvent via the EventSystem so it fires automatically
         * without requiring a manual call from the application layer.
         *
         * Safe to call even if Init() has not been called yet.
         */
        void OnSceneTransition();

        /*!
         * \brief Direct C++ spawn (also the EventSystem handler target).
         */
        void Spawn(float worldX, float worldY, const std::string& value = "",
                   FeedbackType type = FeedbackType::Normal);

        /*!
         * \brief Convenience overload that maps a boolean critical flag to FeedbackType.
         * \param worldX World-space X position.
         * \param worldY World-space Y position.
         * \param value Text to display.
         * \param isCrit If true, uses Critical type; otherwise Normal.
         */
        void Spawn(float worldX, float worldY, const std::string& value, bool isCrit)
        {
            Spawn(worldX, worldY, value,
                  isCrit ? FeedbackType::Critical : FeedbackType::Normal);
        }

        /*!
         * \brief Returns the entity ID of the feedback canvas.
         * \return Canvas entity ID.
         */
        Uma_ECS::Entity GetCanvasEntity() const { return mCanvasEntity; }

        /*!
         * \brief Sets the font path used for feedback text entities.
         * \param fontPath Relative path to the font file.
         */
        void SetNumberFont(const std::string& fontPath) { mFontPath = fontPath; }

    private:
        Uma_ECS::Coordinator*    pCoordinator = nullptr;
        Uma_Engine::Graphics*    pGraphics    = nullptr;
        Uma_Engine::EventSystem* pEventSystem = nullptr;
        UISystem*                pUISystem    = nullptr;
        std::string              mFontPath;

        // Tracks whether Init() has completed — guards OnSceneTransition early calls
        bool mInitialized = false;

        // Single shared canvas
        Uma_ECS::Entity mCanvasEntity = static_cast<Uma_ECS::Entity>(-1);

        // Fixed pool
        Slot mPool[FeedbackConfig::kPoolSize]{};

        /*!
         * \brief Converts world-space coordinates to NDC.
         * \param wx World X position.
         * \param wy World Y position.
         * \return Position in NDC space.
         */
        Vec2  WorldToNDC(float wx, float wy) const;

        /*!
         * \brief Acquires a pool slot, evicting the furthest-elapsed slot if full.
         * \return Index into mPool.
         */
        int   AcquireSlot();

        /*!
         * \brief Generates a pseudo-random jitter value using xorshift.
         * \return Random float in [0,1].
         */
        float NextJitter();
        uint32_t mRandState = 0x9E3779B9u;

        /*!
         * \brief Creates the shared canvas entity for feedback numbers.
         */
        void BuildCanvas();

        /*!
         * \brief Pre-allocates pooled text entities as children of the canvas.
         */
        void BuildPooledTextEntities();

        /*!
         * \brief Hides a slot by moving it off-screen and setting alpha to zero.
         * \param slot The pool slot to hide.
         */
        void HideSlot(Slot& slot);
    };

} // namespace Uma_UI
