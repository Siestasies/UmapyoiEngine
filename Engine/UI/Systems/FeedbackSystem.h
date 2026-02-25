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
  Any system or Lua script emits SpawnNumberEvent through EventSystem.
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
         *        and subscribes to SpawnNumberEvent.
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
        void Spawn(float worldX, float worldY, int amount,
                   NumberType type = NumberType::Normal);

        void Spawn(float worldX, float worldY, int amount, bool isCrit)
        {
            Spawn(worldX, worldY, amount,
                  isCrit ? NumberType::Critical : NumberType::Normal);
        }

        Uma_ECS::Entity GetCanvasEntity() const { return mCanvasEntity; }

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
        NumberSlot mPool[NumberConfig::kPoolSize]{};

        // Helpers
        Vec2  WorldToNDC(float wx, float wy) const;
        int   AcquireSlot();      // returns index into mPool; evicts furthest-elapsed if full
        float NextJitter();       // xorshift pseudo-random in [0,1]
        uint32_t mRandState = 0x9E3779B9u;

        void BuildCanvas();
        void BuildPooledTextEntities();
        void HideSlot(NumberSlot& slot);  // moves off-screen, alpha=0
    };

} // namespace Uma_UI
