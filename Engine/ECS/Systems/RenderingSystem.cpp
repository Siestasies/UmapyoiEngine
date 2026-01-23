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
#include "Components/Tilemap.h"

#include "UI/Components/Canvas.h"


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
        RenderWorldPass(dt);

        RenderUIPass(dt);
    }

    void RenderingSystem::RenderWorldPass(float dt)
    {
        (void)dt;

        std::vector<LayeredSprite> sprites;

        GatherWorldSprites(sprites);
        RenderWorldSprites(sprites);
    }

    void RenderingSystem::GatherWorldSprites(std::vector<LayeredSprite>& allSprites)
    {
        if (!aEntities.size()) return;

        auto& srArray = pCoordinator->GetComponentArray<Sprite>();
        auto& tfArray = pCoordinator->GetComponentArray<Transform>();
        auto& camArray = pCoordinator->GetComponentArray<Camera>();
        auto& animatorArray = pCoordinator->GetComponentArray<Animator>();
        auto& tmArray = pCoordinator->GetComponentArray<Tilemap>();
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

            int hierarchyOrder = pCoordinator->GetHierarchyIndex(entity);
            if (hierarchyOrder == -1) continue;

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

                allSprites.push_back(LayeredSprite
                    {
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
                        .order = sr.renderOrder,
                        .hierarchyOrder = hierarchyOrder,
                        .texId = sr.texture->tex_id,
                        .entityId = entity
                    });
            }
            else if (tmArray.Has(entity))
            {
                auto& tilemap = tmArray.GetData(entity);
                for (auto& layer : tilemap.layers)
                {
                    for (int i = 0; i < layer.tiles.size(); i++)
                    {
                        int row = i / layer.width;
                        int col = i % layer.width;

                        // Grid position for placement
                        Vec2 tilePos = tf.worldPosition + Vec2(col * tilemap.tileSize * tf.scale.x, -(row * tilemap.tileSize * tf.scale.y));

                        // Tileset indices for UVs
                        int tileset_row = layer.tiles[i] / int(sr.spriteSheetGrid.x);
                        int tileset_col = layer.tiles[i] % int(sr.spriteSheetGrid.x);

                        sr.spriteCell = Vec2(tileset_col, tileset_row);
                        sr.GetUVs(uvOffset, uvSize);

                        allSprites.push_back(LayeredSprite
                            {
                                .info = Uma_Engine::Sprite_Info{
                                    .tex_id = sr.texture->tex_id,
                                    .pos = tilePos,
                                    .scale = Vec2(tilemap.tileSize * tf.scale.x, tilemap.tileSize * tf.scale.y),
                                    .rot = tf.worldRotation,
                                    .rot_speed = tf.rotation.y,
                                    .uvOffset = uvOffset,
                                    .uvSize = uvSize,
                                    .tintColor = sr.tintColor,
                                    .alpha = sr.alpha
                                },
                                .layer = layer.renderLayer,
                                .order = layer.renderOrder,
                                .hierarchyOrder = hierarchyOrder,
                                .texId = sr.texture->tex_id,
                                .entityId = entity
                            });
                    }
                }
            }
            else
            {
                sr.GetUVs(uvOffset, uvSize);

                allSprites.push_back(LayeredSprite
                    {
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
                        .order = sr.renderOrder,
                        .hierarchyOrder = hierarchyOrder,
                        .texId = sr.texture->tex_id,
                        .entityId = entity
                    });
            }
        }
    }

    void RenderingSystem::RenderWorldSprites(std::vector<LayeredSprite>& allSprites)
    {
        std::sort(allSprites.begin(), allSprites.end(),
            [](const LayeredSprite& a, const LayeredSprite& b)
            {
                if (a.layer != b.layer)
                    return a.layer < b.layer;
                if (a.order != b.order)
                    return a.order < b.order;
                if (a.hierarchyOrder != b.hierarchyOrder)
                    return a.hierarchyOrder < b.hierarchyOrder;
                if (a.texId != b.texId)
                    return a.texId < b.texId;
                return a.entityId < b.entityId;
            });

        // Batch within same layer+order+hierarchy only
        std::map<unsigned int, std::vector<Uma_Engine::Sprite_Info>> sorted_sprites;

        if (!allSprites.empty())
        {
            LayerMask currentLayer = allSprites[0].layer;
            int currentOrder = allSprites[0].order;
            int currentHierarchy = allSprites[0].hierarchyOrder;

            for (const auto& layeredSprite : allSprites)
            {
                // Flush if ANY of the sorting criteria changed
                if (layeredSprite.layer != currentLayer ||
                    layeredSprite.order != currentOrder ||
                    layeredSprite.hierarchyOrder != currentHierarchy)
                {
                    // Render all batches from previous group
                    for (const auto& pair : sorted_sprites)
                    {
                        pGraphics->DrawSpritesInstanced(pair.first, pair.second);
                    }
                    sorted_sprites.clear();

                    // Update current sorting group
                    currentLayer = layeredSprite.layer;
                    currentOrder = layeredSprite.order;
                    currentHierarchy = layeredSprite.hierarchyOrder;
                }

                // Add to batch
                sorted_sprites[layeredSprite.texId].push_back(layeredSprite.info);
            }

            // Render remaining batches
            for (const auto& pair : sorted_sprites)
            {
                pGraphics->DrawSpritesInstanced(pair.first, pair.second);
            }
        }
    }

    void RenderingSystem::GatherUIElements(std::vector<UIDrawCommand>& uiDrawCommands)
    {
        auto& canvasArray = pCoordinator->GetComponentArray<Uma_UI::Canvas>();
        auto& tfArray = pCoordinator->GetComponentArray<Uma_ECS::Transform>();
        auto& rtfArray = pCoordinator->GetComponentArray<Uma_UI::RectTransform>();

        std::vector<Entity> sortedCanvasIds = canvasArray.GetAllEntities();

        std::sort(sortedCanvasIds.begin(), sortedCanvasIds.end(),
            [&](const Entity& lhs, const Entity& rhs)
            {
                int lhs_sorting_order = canvasArray.GetData(lhs).sortingOrder;
                int rhs_sorting_order = canvasArray.GetData(rhs).sortingOrder;

                int lhs_hierarchyOrder = pCoordinator->GetHierarchyIndex(lhs);
                int rhs_hierarchyOrder = pCoordinator->GetHierarchyIndex(rhs);

                if (lhs_sorting_order != rhs_sorting_order)
                {
                    return lhs_sorting_order < rhs_sorting_order;
                }

                if (lhs_hierarchyOrder != rhs_hierarchyOrder)
                {
                    return lhs_hierarchyOrder < rhs_hierarchyOrder;
                }
            });

        for (const auto& canvasId : sortedCanvasIds)
        {
            std::vector<Entity> childrenList;

            GetAllChildren(canvasId, childrenList);

            auto& canvas = canvasArray.GetData(canvasId);

            for (int i = 0; i < childrenList.size(); i++)
            {
                Entity childUI = childrenList[i];

                if (!pCoordinator->IsActiveInHierarchy(childUI)) continue;

                auto& rectTransform = rtfArray.GetData(childUI);
                

                if (pCoordinator->HasComponent<Uma_UI::Text>(childUI))
                {
                    UIDrawCommand::Type uiType = UIDrawCommand::UI_TEXT;

                    auto& textComp = pCoordinator->GetComponent<Uma_UI::Text>(childUI);
                    if (!textComp.visible || textComp.text.empty())
                    {
                        continue;
                    }

                    Uma_Engine::FontData* uiFont = pResourcesManager->GetFont(textComp.fontName);
                    if (uiFont == nullptr)
                    {
                        std::stringstream log;
                        log << "UI object(" << childUI << ") font is not loaded or invalid.";
                        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning, log.str());
                        continue;
                    }

                    // text
                    uiDrawCommands.push_back(UIDrawCommand
                        {
                            .type = uiType,
                            .layer = RL_UI,
                            .order = textComp.sortingOrder,
                            .hierarchyOrder = i,
                            .entity = childUI,

                            .text = textComp.text,
                            .font = uiFont,
                            .fontSize = textComp.fontSize,
                            .alignment = textComp.alignment,
                            .textColor = textComp.color
                        }
                        );
                }
                if (pCoordinator->HasComponent<Uma_UI::Image>(childUI))
                {
                    UIDrawCommand::Type uiType = UIDrawCommand::UI_IMAGE;

                    auto& image = pCoordinator->GetComponent<Uma_UI::Image>(childUI);

                    if (!image.texture || image.texture->tex_id == 0)
                    {
                        image.texture = pResourcesManager->GetTexture(image.textureName);
                    }

                    // Verify texture is valid before using it
                    if (!image.texture || image.texture->tex_id == 0)
                    {
                        std::stringstream log;
                        log << "Entity(" << childUI << ") texture is not valid.";
                        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning, log.str());
                        continue;
                    }

                    Uma_Engine::Sprite_Info spriteInfo = Uma_Engine::Sprite_Info
                    {
                        .tex_id = image.texture->tex_id,

                        .pos = rectTransform.computedRect.Center(),
                        .scale = rectTransform.computedRect.Size(),
                        .rot = 0.0f,
                        .rot_speed = 0.0f,

                        .uvOffset = Vec2(0.0f, 0.0f),
                        .uvSize = Vec2(1.0f, 1.0f),

                        .tintColor = image.color.ToVec3(),
                        .alpha = image.color.a
                    };

                    // image
                    uiDrawCommands.push_back(UIDrawCommand
                        {
                            .type = uiType,
                            .layer = RL_UI,
                            .order = image.sortingOrder,
                            .hierarchyOrder = i,
                            .entity = childUI,

                            .spriteInfo = spriteInfo,
                        }
                        );
                }

                // sort based on children's sorting order
                std::sort(uiDrawCommands.begin(), uiDrawCommands.end(),
                    [&](const UIDrawCommand& lhs, const UIDrawCommand& rhs)
                    {
                        if (lhs.order != rhs.order)
                        {
                            return lhs.order < rhs.order;
                        }

                        return lhs.hierarchyOrder < rhs.hierarchyOrder;
                    });
            }
        }
    }

    void RenderingSystem::GetAllChildren(Entity parent, std::vector<Entity>& childrenList)
    {
        const auto& transform = pCoordinator->GetComponent<Transform>(parent);

        if (!transform.children.size()) return;

        for (const auto& childObj : transform.children)
        {
            childrenList.push_back(childObj);

            GetAllChildren(childObj, childrenList);
        }
    }

    void RenderingSystem::RenderUIElements(std::vector<UIDrawCommand>& uiDrawCommands)
    {
        for (const auto& command : uiDrawCommands)
        {
            switch (command.type)
            {
            case UIDrawCommand::UI_IMAGE:
            {
                pGraphics->DrawSpriteScreen(
                    command.spriteInfo.tex_id,
                    command.spriteInfo.pos,   // Position (NDC)
                    command.spriteInfo.scale, // Size (NDC)
                    command.spriteInfo.rot,
                    command.spriteInfo.uvOffset,
                    command.spriteInfo.uvSize,
                    command.spriteInfo.tintColor,
                    command.spriteInfo.alpha);

                break;
            }
            case UIDrawCommand::UI_TEXT:
            {
                auto& rectTransform = pCoordinator->GetComponent<Uma_UI::RectTransform>(command.entity);

                // Measure text width in NDC space
                float textWidthNDC = pGraphics->MeasureText(*command.font, command.text, command.fontSize);
                float alignX = 0.0f;

                // Calculate horizontal alignment based on rect bounds
                switch (command.alignment)
                {
                case Uma_UI::TextAlignment::Left:
                    alignX = rectTransform.computedRect.Left();
                    break;
                case Uma_UI::TextAlignment::Center:
                    alignX = rectTransform.computedRect.Center().x - textWidthNDC * 0.5f;
                    break;
                case Uma_UI::TextAlignment::Right:
                    alignX = rectTransform.computedRect.Right() - textWidthNDC;
                    break;
                }

                // Calculate vertical center
                float fontHeightNDC = (command.fontSize * 48.f) / static_cast<float>(pGraphics->GetSceneViewport().y) * 2.0f;
                float alignY = rectTransform.computedRect.Center().y - fontHeightNDC * 0.15f;

                pGraphics->DrawTextScreen(*command.font, command.text, alignX, alignY,
                    command.fontSize, command.textColor.r, command.textColor.g, command.textColor.b);

                break;
            }
            }
        }
    }

    void RenderingSystem::RenderUIPass(float dt)
    {
        (void)dt;

        std::vector<UIDrawCommand> UIElements;

        GatherUIElements(UIElements);
        RenderUIElements(UIElements);
    }
}