#include "PickingSystem.h"
#include "../../ECS/Components/Transform.h"
#include "../../ECS/Components/Sprite.h"
#include "../../UI/Components/RectTransform.h"
#include "../../UI/Components/Canvas.h"
#include "../../UI/Helpers/Input.h"

#include <cmath>

namespace Uma_Engine
{
    Uma_ECS::Entity PickingSystem::RaycastEntity(const Vec2& screenPos, const EditorConfig& config)
    {
        if (!pCoordinator || !pGraphics)
            return static_cast<Uma_ECS::Entity>(-1);

        // Convert screen to world and NDC for both systems
        Vec2 worldPos = pGraphics->ScreenToWorld(screenPos);

        int screenWidth = pGraphics->GetViewportWidth();
        int screenHeight = pGraphics->GetViewportHeight();
        Vec2 ndcPos = Uma_UI::ScreenToNDC(screenPos.x, screenPos.y,
            static_cast<float>(screenWidth),
            static_cast<float>(screenHeight));

        // Try UI first (higher priority)
        if (config.pickUIEntities)
        {
            Uma_ECS::Entity uiHit = RaycastUIEntity(ndcPos);
            if (uiHit != static_cast<Uma_ECS::Entity>(-1))
            {
                auto& rectTransformArray = pCoordinator->GetComponentArray<Uma_UI::RectTransform>();
                if (rectTransformArray.Has(uiHit))
                {
                    const auto& rt = rectTransformArray.GetData(uiHit);
                    float halfWidth = rt.computedRect.width * 0.5f;
                    float halfHeight = rt.computedRect.height * 0.5f;

                    // Verify click is actually inside bounds
                    if (ndcPos.x >= rt.computedRect.x - halfWidth &&
                        ndcPos.x <= rt.computedRect.x + halfWidth &&
                        ndcPos.y >= rt.computedRect.y - halfHeight &&
                        ndcPos.y <= rt.computedRect.y + halfHeight)
                    {
                        return uiHit;
                    }
                }
            }
        }

        // Try game entities
        if (config.pickGameEntities)
        {
            Uma_ECS::Entity gameHit = RaycastGameEntity(worldPos, config.gamePickRadius);
            if (gameHit != static_cast<Uma_ECS::Entity>(-1))
                return gameHit;
        }

        return static_cast<Uma_ECS::Entity>(-1);
    }

    Uma_ECS::Entity PickingSystem::RaycastGameEntity(const Vec2& worldPos, float /*pickRadius*/)
    {
        if (!pCoordinator)
            return static_cast<Uma_ECS::Entity>(-1);

        auto& transformArray = pCoordinator->GetComponentArray<Uma_ECS::Transform>();
        auto& spriteArray = pCoordinator->GetComponentArray<Uma_ECS::Sprite>();
        auto& rectTransformArray = pCoordinator->GetComponentArray<Uma_UI::RectTransform>();

        Uma_ECS::Entity closest = static_cast<Uma_ECS::Entity>(-1);
        float closestDist = FLT_MAX;

        // Check all entities with transforms
        for (size_t i = 0; i < transformArray.Size(); ++i)
        {
            Uma_ECS::Entity entity = transformArray.GetEntity(i);

            // Skip UI entities (they have RectTransform)
            if (rectTransformArray.Has(entity))
                continue;

            const auto& transform = transformArray.GetComponentAt(i);

            Vec2 size(50.0f, 50.0f);  // Default size for entities without sprites

            if (spriteArray.Has(entity))
            {
                const auto& sprite = spriteArray.GetData(entity);

                // Use actual texture dimensions
                if (sprite.texture)
                {
                    size.x = static_cast<float>(sprite.texture->tex_size.x) / static_cast<float>(sprite.texture->pixelsPerUnit);
                    size.y = static_cast<float>(sprite.texture->tex_size.y) / static_cast<float>(sprite.texture->pixelsPerUnit);
                }
            }

            // Apply entity scale
            size.x *= transform.worldScale.x;
            size.y *= transform.worldScale.y;

            Vec2 halfSize = size * 0.5f;
            if (worldPos.x >= transform.worldPosition.x - halfSize.x &&
                worldPos.x <= transform.worldPosition.x + halfSize.x &&
                worldPos.y >= transform.worldPosition.y - halfSize.y &&
                worldPos.y <= transform.worldPosition.y + halfSize.y)
            {
                // Inside bounds - calculate distance from center for priority
                float dx = transform.worldPosition.x - worldPos.x;
                float dy = transform.worldPosition.y - worldPos.y;
                float dist = std::sqrt(dx * dx + dy * dy);

                if (dist < closestDist)
                {
                    closestDist = dist;
                    closest = entity;
                }
            }
        }

        return closest;
    }

    Uma_ECS::Entity PickingSystem::RaycastUIEntity(const Vec2& ndcPos)
    {
        if (!pCoordinator)
            return static_cast<Uma_ECS::Entity>(-1);

        auto& rectTransformArray = pCoordinator->GetComponentArray<Uma_UI::RectTransform>();
        auto& canvasArray = pCoordinator->GetComponentArray<Uma_UI::Canvas>();

        // Check all UI entities
        // TODO: Sort by render order/depth for proper front-to-back checking
        for (size_t i = 0; i < rectTransformArray.Size(); ++i)
        {
            Uma_ECS::Entity entity = rectTransformArray.GetEntity(i);

            if (canvasArray.Has(entity))
                continue;

            const auto& rectTransform = rectTransformArray.GetComponentAt(i);

            // computedRect is Vec4(centerX, centerY, width, height) in NDC
            float centerX = rectTransform.computedRect.x;
            float centerY = rectTransform.computedRect.y;
            float halfWidth = rectTransform.computedRect.width * 0.5f;
            float halfHeight = rectTransform.computedRect.height * 0.5f;

            if (ndcPos.x >= centerX - halfWidth &&
                ndcPos.x <= centerX + halfWidth &&
                ndcPos.y >= centerY - halfHeight &&
                ndcPos.y <= centerY + halfHeight)
            {
                return entity;  // First hit (assume front-most for now)
            }
        }

        return static_cast<Uma_ECS::Entity>(-1);
    }

    bool PickingSystem::IsGameEntity(Uma_ECS::Entity entity) const
    {
        if (!pCoordinator)
            return false;

        auto& transformArray = pCoordinator->GetComponentArray<Uma_ECS::Transform>();
        auto& rectTransformArray = pCoordinator->GetComponentArray<Uma_UI::RectTransform>();

        return transformArray.Has(entity) && !rectTransformArray.Has(entity);
    }

    bool PickingSystem::IsUIEntity(Uma_ECS::Entity entity) const
    {
        if (!pCoordinator)
            return false;

        auto& rectTransformArray = pCoordinator->GetComponentArray<Uma_UI::RectTransform>();
        return rectTransformArray.Has(entity);
    }
}