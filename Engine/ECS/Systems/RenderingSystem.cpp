/*!
\file   RenderingSystem.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (Everything else)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\co-author Javier Chua Dong Qing (EditorCamera, Animator, FlipScale)
\par       E-mail: javierdongqing.chua@digipen.edu
\par       DigiPen login: javierdongqing.chua

\brief
Implements sprite batching and rendering system that groups sprites by texture ID for instanced drawing.

Loads textures on-demand through ResourcesManager if not cached in SpriteRenderer component.
Queries camera transform and zoom from Camera component to configure graphics viewport.
Validates texture handles before rendering and logs warnings for invalid textures. Builds sorted map of sprites
grouped by texture ID, then submits batched draw calls through Graphics API for optimal performance.
Supports single camera setup with entity at index 0.
Integrates with the Animator component, using its uvOffset and uvSize for rendering

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "RenderingSystem.hpp"
#include "Core/Coordinator.hpp"

#include "Components/Sprite.h"
#include "Components/Transform.h"
#include "Components/Camera.h"
#include "Components/Collider.h"
#include "Components/Player.h"
#include "Components/Animator.h"


#include "Debugging/Debugger.hpp"

#include <cassert>
#include <sstream>
#include <algorithm>
#include <map>

namespace Uma_ECS
{
    void RenderingSystem::Init(Uma_Engine::Graphics* g, Uma_Engine::ResourcesManager* rm, Coordinator* c)
    {
        pCoordinator = c;
        pGraphics = g;
        pResourcesManager = rm;
    }

    void RenderingSystem::Update(float dt)
    {
        (void)dt;

        if (!aEntities.size()) return;

        auto& srArray = pCoordinator->GetComponentArray<Sprite>();
        auto& tfArray = pCoordinator->GetComponentArray<Transform>();
        auto& camArray = pCoordinator->GetComponentArray<Camera>();
        auto& animatorArray = pCoordinator->GetComponentArray<Animator>();
        
        auto& rbArray = pCoordinator->GetComponentArray<RigidBody>();

        // one camera for now
        if (camArray.Size() > 0)
        {
            Entity camera = camArray.GetEntity(0);
            auto& cam_tf = tfArray.GetData(camera);
            auto& cam_c = camArray.GetData(camera);

            // Only update graphics camera if not using editor camera
            if (mUpdateCamera)
            {
                pGraphics->SetCamInfo(cam_tf.position, cam_c.mZoom * 10.f);
            }
        }

        // Structure to hold sprite info WITH layer information
        struct LayeredSprite
        {
            Uma_Engine::Sprite_Info info;
            LayerMask layer;
            unsigned int texId;
            Entity entityId;
        };

        std::vector<LayeredSprite> allSprites;
        allSprites.reserve(aEntities.size());

        // Gather all sprite info with layer data
        for (const auto& entity : aEntities)
        {
            if (!pCoordinator->IsActiveInHierarchy(entity))
                continue;


            auto& sr = srArray.GetData(entity);
            auto& tf = tfArray.GetData(entity);

            // Load texture if not loaded
            // or texture became invalid (tex_id == 0)
            if (!sr.texture || sr.texture->tex_id == 0)
            {
                sr.texture = pResourcesManager->GetTexture(sr.textureName);
            }

            // Verify texture is valid before using it
            if (!sr.texture || sr.texture->tex_id == 0)
            {
                std::stringstream log;
                log << "Entity(" << entity << ") texture is not valid.";
                Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning, log.str());
                continue;
            }

            Vec2 spriteScale;
            if (sr.UseNativeSize)
            {
                spriteScale = sr.texture->GetNativeSize();
                spriteScale.x *= tf.worldScale.x;
                spriteScale.y *= tf.worldScale.y;
            }
            else
            {
                spriteScale = tf.worldScale;
            }

            if (rbArray.Has(entity) && sr.autoFlip)
            {
                auto& rb = rbArray.GetData(entity);
                if (rb.velocity.x < 0) sr.flipX = true;
                if (rb.velocity.x > 0) sr.flipX = false;
            }

            if (sr.flipX)
            {
                spriteScale.x = -spriteScale.x;
            }
            if (sr.flipY)
            {
                spriteScale.y = -spriteScale.y;
            }

            // Get UV coordinates from animator if present
            Vec2 uvOffset(0.0f, 0.0f);
            Vec2 uvSize(1.0f, 1.0f);

            if (animatorArray.Has(entity))
            {
                auto& animator = animatorArray.GetData(entity);

                // Use animator UVs only if it has clips and current clip is valid
                const auto& clips = animator.animator.GetClips();
                const std::string& currentClip = animator.animator.GetCurrentClip();

                if (!clips.empty() && clips.find(currentClip) != clips.end())
                {
                    uvOffset = animator.uvOffset;
                    uvSize = animator.uvSize;
                }
                else
                {
                    // Animator not active, use sprite's cell selection
                    sr.GetUVs(uvOffset, uvSize);
                }
            }
            else
            {
                sr.GetUVs(uvOffset, uvSize);
            }

            allSprites.push_back(LayeredSprite{
                .info = Uma_Engine::Sprite_Info{
                    .tex_id = sr.texture->tex_id,
                    .pos = tf.worldPosition,
                    .scale = spriteScale,
                    .rot = tf.worldRotation,
                    .rot_speed = tf.rotation.y,
                    .uvOffset = uvOffset,
                    .uvSize = uvSize,
                    .tintColor = sr.tintColor,
                    .alpha = sr.alpha
                },
                .layer = sr.renderLayer,
                .texId = sr.texture->tex_id,
                .entityId = entity
                });
        }

        // Sort by layer FIRST, then by texture (for batching within same layer), then by entity ID (for stability)
        std::sort(allSprites.begin(), allSprites.end(),
            [](const LayeredSprite& a, const LayeredSprite& b)
            {
                if (a.layer != b.layer)
                    return a.layer < b.layer;      // Sort by layer first
                if (a.texId != b.texId)
                    return a.texId < b.texId;      // Then by texture for batching
                return a.entityId < b.entityId;    // Finally by entity ID for deterministic ordering
            });

        // Now group by texture and render in layer order
        std::map<unsigned int, std::vector<Uma_Engine::Sprite_Info>> sorted_sprites;
        LayerMask currentLayer = allSprites.empty() ? 0 : allSprites[0].layer;

        for (const auto& layeredSprite : allSprites)
        {
            // If we've moved to a new layer, flush previous batches
            if (layeredSprite.layer != currentLayer)
            {
                if (!sorted_sprites.empty())
                {
                    // Render all batches from previous layer
                    for (const auto& pair : sorted_sprites)
                    {
                        pGraphics->DrawSpritesInstanced(pair.first, pair.second);
                    }
                    sorted_sprites.clear();
                }
                currentLayer = layeredSprite.layer;
            }

            sorted_sprites[layeredSprite.texId].push_back(layeredSprite.info);
        }

        // Render remaining batches
        for (const auto& pair : sorted_sprites)
        {
            pGraphics->DrawSpritesInstanced(pair.first, pair.second);
        }
    }
}