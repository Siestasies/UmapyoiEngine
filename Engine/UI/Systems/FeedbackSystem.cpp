/*!
\file   FeedbackSystem.cpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Implementation of FeedbackSystem. See FeedbackSystem.h for design notes.

All content (C) 2026 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "FeedbackSystem.h"
#include "UISystem.h"

#include "../ECS/Components/Transform.h"
#include "../UI/Components/Canvas.h"
#include "../UI/Components/RectTransform.h"
#include "../UI/Components/Text.h"

#include "../Events/FeedbackEvents.h"
#include "../Events/IMGUIEvents.h"

#include "Debugging/Debugger.hpp"
#include "Events/ApplicationEvents.h"   // LoadSceneRequestEvent, ResetSceneRequest

#include <cmath>
#include <string>
#include <algorithm>

namespace Uma_UI
{
    void FeedbackSystem::Init(Uma_ECS::Coordinator*    coord,
                                  Uma_Engine::Graphics*    gfx,
                                  Uma_Engine::EventSystem* events,
                                  UISystem*                uiSystem,
                                  const std::string&       fontPath)
    {
        pCoordinator = coord;
        pGraphics    = gfx;
        pEventSystem = events;
        pUISystem    = uiSystem;
        mFontPath    = fontPath;

        BuildCanvas();
        BuildPooledTextEntities();

        pEventSystem->Subscribe<SpawnNumberEvent, FeedbackSystem>(
            [this](const SpawnNumberEvent& e)
            {
                Spawn(e.worldX, e.worldY, e.amount, e.type);
            });

        pEventSystem->Subscribe<Uma_Engine::LoadSceneRequestEvent, FeedbackSystem>(
            [this](const Uma_Engine::LoadSceneRequestEvent&)
            {
                OnSceneTransition();
            });

        pEventSystem->Subscribe<Uma_Engine::ResetSceneRequest, FeedbackSystem>(
            [this](const Uma_Engine::ResetSceneRequest&)
            {
                OnSceneTransition();
            });

        mInitialized = true;

        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
            "[FeedbackSystem] Initialized. Canvas entity = "
            + std::to_string(mCanvasEntity)
            + ", pool size = " + std::to_string(NumberConfig::kPoolSize));
    }

    void FeedbackSystem::Update(float dt)
    {
        if (!pCoordinator || !pGraphics) return;

        Vec2 screen = pGraphics->GetSceneViewport();
        bool anyChanged = false;

        for (auto& slot : mPool)
        {
            if (!slot.alive) continue;

            slot.elapsed += dt;
            const float t = slot.elapsed / slot.lifetime;  // 0 → 1

            if (t >= 1.0f)
            {
                HideSlot(slot);
                anyChanged = true;
                continue;
            }

            anyChanged = true;

            const float ndcRise = (NumberConfig::kRiseSpeed / (screen.y * 0.5f)) * slot.elapsed;

            const float pixX = (slot.baseNdcX + slot.jitterNdcX) * (screen.x * 0.5f);
            const float pixY = (slot.baseNdcY + ndcRise)          * (screen.y * 0.5f);

            float alpha = 1.0f;
            if (t > NumberConfig::kFadeStartFraction)
            {
                const float fadeSpan = 1.0f - NumberConfig::kFadeStartFraction;
                alpha = 1.0f - ((t - NumberConfig::kFadeStartFraction) / fadeSpan);
                alpha = std::max(0.0f, alpha);
            }

            float scaleMul = 1.0f;
            if (slot.elapsed < NumberConfig::kPunchDuration)
            {
                const float punchT = slot.elapsed / NumberConfig::kPunchDuration;
                const float eased  = 1.0f - (1.0f - punchT) * (1.0f - punchT); // EaseOutQuad
                scaleMul = 1.0f + (NumberConfig::kPunchPeak - 1.0f) * eased;
            }

            if (!pCoordinator->HasActiveEntity(slot.textEntity)) continue;

            if (pCoordinator->HasComponent<RectTransform>(slot.textEntity))
            {
                auto& rt = pCoordinator->GetComponent<RectTransform>(slot.textEntity);
                rt.anchoredPosition.x = pixX;
                rt.anchoredPosition.y = pixY;
                rt.sizeDelta.x = slot.baseFontSize * scaleMul * 5.0f;
                rt.sizeDelta.y = slot.baseFontSize * scaleMul * 2.0f;
                rt.isDirty = true;
            }

            if (pCoordinator->HasComponent<Text>(slot.textEntity))
            {
                auto& txt = pCoordinator->GetComponent<Text>(slot.textEntity);
                txt.color.a  = alpha;
                txt.fontSize = slot.baseFontSize * scaleMul;
            }
        }

        if (anyChanged && pUISystem)
            pUISystem->MarkAllDirty();
    }

    void FeedbackSystem::OnSceneTransition()
    {
        if (!mInitialized) return;

        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
            "[FeedbackSystem] Scene transition detected — rebuilding pool.");

        if (mCanvasEntity != static_cast<Uma_ECS::Entity>(-1)
            && pCoordinator->HasActiveEntity(mCanvasEntity))
        {
            pCoordinator->DestroyEntityAndChildren(mCanvasEntity);
        }

        mCanvasEntity = static_cast<Uma_ECS::Entity>(-1);
        for (auto& slot : mPool)
        {
            slot.alive      = false;
            slot.elapsed    = 0.0f;
            slot.textEntity = static_cast<Uma_ECS::Entity>(-1);
        }

        BuildCanvas();
        BuildPooledTextEntities();

        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
            "[FeedbackSystem] Pool rebuilt. New canvas entity = "
            + std::to_string(mCanvasEntity));
    }

    void FeedbackSystem::Shutdown()
    {
        if (pEventSystem)
            pEventSystem->UnsubscribeSystem<FeedbackSystem>();

        if (mCanvasEntity != static_cast<Uma_ECS::Entity>(-1)
            && pCoordinator
            && pCoordinator->HasActiveEntity(mCanvasEntity))
        {
            pCoordinator->DestroyEntityAndChildren(mCanvasEntity);
        }

        mInitialized  = false;
        mCanvasEntity = static_cast<Uma_ECS::Entity>(-1);
        for (auto& slot : mPool)
        {
            slot.alive      = false;
            slot.textEntity = static_cast<Uma_ECS::Entity>(-1);
        }

        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
            "[FeedbackSystem] Shutdown complete.");
    }

    void FeedbackSystem::Spawn(float worldX, float worldY, int amount,
                                   NumberType type)
    {
        if (!pCoordinator || !pGraphics) return;

        std::string displayText;
        if (type == NumberType::Heal)
            displayText = "+" + std::to_string(amount);
        else
            displayText = std::to_string(amount);

        if (type == NumberType::Affinity)
            displayText += "!";
        else if (type == NumberType::Critical)
            displayText += "!!";

        Color color;
        float fontSize;
        switch (type)
        {
        case NumberType::Affinity:
            color = NumberConfig::AffinityColor();
            fontSize = NumberConfig::kFontSizeAffinity;
            break;
        case NumberType::Critical:
            color    = NumberConfig::CritColor();
            fontSize = NumberConfig::kFontSizeCrit;
            break;
        case NumberType::Heal:
            color    = NumberConfig::HealColor();
            fontSize = NumberConfig::kFontSizeNormal;
            break;
        case NumberType::PlayerHit:
            color    = NumberConfig::PlayerHitColor();
            fontSize = NumberConfig::kFontSizeNormal;
            break;
        case NumberType::ManaSpend:
            color = NumberConfig::ManaSpendColor();
            fontSize = NumberConfig::kFontSizeMana;
            break;
        case NumberType::ManaGain:
            color = NumberConfig::ManaGainColor();
            fontSize = NumberConfig::kFontSizeMana;
            break;
        default:
            color    = NumberConfig::NormalColor();
            fontSize = NumberConfig::kFontSizeNormal;
            break;
        }

        Vec2 ndc = WorldToNDC(worldX, worldY);

        Vec2 screen = pGraphics->GetSceneViewport();
        float jitterNdc = (NextJitter() * 2.0f - 1.0f)
                          * (NumberConfig::kSpreadRadius / (screen.x * 0.5f));

        int idx = AcquireSlot();
        NumberSlot& slot = mPool[idx];

        slot.baseNdcX    = ndc.x;
        slot.baseNdcY    = ndc.y;
        slot.jitterNdcX  = jitterNdc;
        slot.elapsed     = 0.0f;
        slot.lifetime    = NumberConfig::kLifetime;
        slot.baseFontSize = fontSize;
        slot.alive       = true;

        if (!pCoordinator->HasActiveEntity(slot.textEntity))
        {
            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning,
                "[FeedbackSystem] Slot " + std::to_string(idx)
                + " has invalid text entity — skipping spawn.");
            slot.alive = false;
            return;
        }

        if (pCoordinator->HasComponent<Text>(slot.textEntity))
        {
            auto& txt    = pCoordinator->GetComponent<Text>(slot.textEntity);
            txt.text     = displayText;
            txt.fontSize = fontSize * NumberConfig::kPunchPeak;
            txt.color    = color;
            txt.color.a  = 1.0f;
            txt.visible  = true;
        }

        if (pCoordinator->HasComponent<RectTransform>(slot.textEntity))
        {
            auto& rt = pCoordinator->GetComponent<RectTransform>(slot.textEntity);
            rt.anchoredPosition.x = (ndc.x + jitterNdc) * (screen.x * 0.5f);
            rt.anchoredPosition.y = ndc.y                * (screen.y * 0.5f);
            rt.sizeDelta.x = fontSize * NumberConfig::kPunchPeak * 5.0f;
            rt.sizeDelta.y = fontSize * NumberConfig::kPunchPeak * 2.0f;
            rt.isDirty = true;
        }

        pCoordinator->SetActive(slot.textEntity, true);
    }

    void FeedbackSystem::BuildCanvas()
    {
        mCanvasEntity = pCoordinator->CreateEntity();

        Uma_ECS::Transform tf{};
        pCoordinator->AddComponent(mCanvasEntity, tf);

        Canvas canvas{};
        canvas.sortingOrder = 999;
        canvas.scaleMode    = CanvasScaleMode::ConstantPixelSize;
        canvas.scaleFactor  = 1.0f;
        pCoordinator->AddComponent(mCanvasEntity, canvas);

        RectTransform rt{};
        rt.anchorMin        = { 0.0f, 0.0f };
        rt.anchorMax        = { 1.0f, 1.0f };
        rt.pivot            = { 0.5f, 0.5f };
        rt.anchoredPosition = { 0.0f, 0.0f };
        rt.sizeDelta        = { 0.0f, 0.0f };
        rt.isDirty          = true;
        pCoordinator->AddComponent(mCanvasEntity, rt);
    }

    void FeedbackSystem::BuildPooledTextEntities()
    {
        for (int i = 0; i < NumberConfig::kPoolSize; ++i)
        {
            NumberSlot& slot = mPool[i];

            Uma_ECS::Entity ent = pCoordinator->CreateEntity();
            slot.textEntity = ent;
            slot.alive      = false;

            Uma_ECS::Transform tf{};
            pCoordinator->AddComponent(ent, tf);

            RectTransform rt{};
            rt.anchorMin        = { 0.5f, 0.5f };
            rt.anchorMax        = { 0.5f, 0.5f };
            rt.pivot            = { 0.5f, 0.5f };
            rt.anchoredPosition = { -9999.0f, -9999.0f }; // off-screen initially
            rt.sizeDelta        = { 200.0f, 80.0f };
            rt.isDirty          = true;
            pCoordinator->AddComponent(ent, rt);

            Text txt{};
            txt.text     = "";
            txt.fontPath = mFontPath;
            txt.fontSize = NumberConfig::kFontSizeNormal;
            txt.color    = { 1.0f, 1.0f, 1.0f, 0.0f }; // alpha = 0
            txt.alignment  = TextAlignment::Center;
            txt.visible    = false;
            txt.sortingOrder = 1;
            pCoordinator->AddComponent(ent, txt);

            pCoordinator->SetParent(ent, mCanvasEntity);

            pCoordinator->SetActive(ent, false);
        }
    }

    void FeedbackSystem::HideSlot(NumberSlot& slot)
    {
        slot.alive   = false;
        slot.elapsed = 0.0f;

        if (!pCoordinator->HasActiveEntity(slot.textEntity)) return;

        if (pCoordinator->HasComponent<RectTransform>(slot.textEntity))
        {
            auto& rt = pCoordinator->GetComponent<RectTransform>(slot.textEntity);
            rt.anchoredPosition = { -9999.0f, -9999.0f };
            rt.isDirty = true;
        }

        if (pCoordinator->HasComponent<Text>(slot.textEntity))
        {
            auto& txt   = pCoordinator->GetComponent<Text>(slot.textEntity);
            txt.color.a = 0.0f;
            txt.visible = false;
            txt.text    = "";
        }

        pCoordinator->SetActive(slot.textEntity, false);
    }

    int FeedbackSystem::AcquireSlot()
    {
        for (int i = 0; i < NumberConfig::kPoolSize; ++i)
        {
            if (!mPool[i].alive) return i;
        }

        int   bestIdx = 0;
        float bestT   = -1.0f;
        for (int i = 0; i < NumberConfig::kPoolSize; ++i)
        {
            float t = mPool[i].elapsed / mPool[i].lifetime;
            if (t > bestT) { bestT = t; bestIdx = i; }
        }

        HideSlot(mPool[bestIdx]);
        return bestIdx;
    }

    Vec2 FeedbackSystem::WorldToNDC(float wx, float wy) const
    {
        Vec2 screen = pGraphics->GetSceneViewport();        // (width, height) in pixels
        Vec2 pixel  = pGraphics->WorldToScreen({ wx, wy }); // pixel coords, origin top-left

        return {
            (pixel.x / screen.x) * 2.0f - 1.0f,           // NDC x  [-1, 1]
            (pixel.y / screen.y) * 2.0f - 1.0f             // NDC y  [-1, 1]
        };
    }

    // =========================================================================
    //  Private — NextJitter   (xorshift32, returns [0,1])
    // =========================================================================
    float FeedbackSystem::NextJitter()
    {
        mRandState ^= mRandState << 13;
        mRandState ^= mRandState >> 17;
        mRandState ^= mRandState << 5;
        return static_cast<float>(mRandState & 0xFFFFu) / 65535.0f;
    }
}