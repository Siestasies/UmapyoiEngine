/*!
\file   RenderingSystem.cpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (Everything else)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\co-author Javier Chua Dong Qing (EditorCamera, Animator, FlipScale, Particle Layering)
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
#include "UI/Components/Image.h"

#include "UI/Components/Canvas.h"


#include "Debugging/Debugger.hpp"

#include <cassert>
#include <sstream>
#include <algorithm>
#include <map>
#include <unordered_set>

namespace Uma_ECS
{
    void RenderingSystem::Init(Uma_Engine::Graphics* g, Uma_Engine::ResourcesManager* rm, Coordinator* c)
    {
        pCoordinator = c;
        pGraphics = g;
        pResourcesManager = rm;

        if (pResourcesManager)
        {
            pResourcesManager->SetCoordinator(c);
        }
    }

    void RenderingSystem::Update(float dt, bool enableCullMode)
    {
        isCullMode = enableCullMode;

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
        auto& particleArray = pCoordinator->GetComponentArray<ParticleEmitter>();
        auto& rbArray = pCoordinator->GetComponentArray<RigidBody>();

        size_t estimatedSpriteCount = aEntities.size();

        Vec2 cameraPos(0, 0);
        float cameraZoom = 1.0f;
        int viewportWidth = static_cast<int>(pGraphics->GetSceneViewport().x);
        int viewportHeight = static_cast<int>(pGraphics->GetSceneViewport().y);

        if (camArray.Size() > 0)
        {
            Entity camera = camArray.GetEntity(0);
            auto& cam_tf = tfArray.GetData(camera);
            auto& cam_c = camArray.GetData(camera);

            cameraPos = cam_tf.position;
            cameraZoom = cam_c.mZoom;

            // Only update graphics camera if not using editor camera
            if (mUpdateCamera)
            {
                pGraphics->SetCamInfo(cameraPos, cameraZoom);
            }
        }

        float halfWidth = (viewportWidth * 0.5f) / cameraZoom;
        float halfHeight = (viewportHeight * 0.5f) / cameraZoom;
        Vec2 camMin = cameraPos - Vec2(halfWidth, halfHeight);
        Vec2 camMax = cameraPos + Vec2(halfWidth, halfHeight);

        float cullingBuffer = 100.0f;
        camMin.x -= cullingBuffer;
        camMin.y -= cullingBuffer;
        camMax.x += cullingBuffer;
        camMax.y += cullingBuffer;

        for (const auto& entity : tmArray.GetAllEntities())
        {
            int hierarchyOrder = pCoordinator->GetHierarchyIndex(entity);
            if (hierarchyOrder == -1) continue;

            auto& tilemap = tmArray.GetData(entity);
            auto& tf = tfArray.GetData(entity);

            // loading of the texture
            if (!tilemap.tileset.texture || tilemap.tileset.texture->tex_id == 0)
            {
                tilemap.tileset.texture = pResourcesManager->GetTexture(tilemap.tileset.texturePath);
            }

            // Verify texture is valid before using it
            if (!tilemap.tileset.texture || tilemap.tileset.texture->tex_id == 0)
            {
                std::stringstream log;
                log << "Entity(" << entity << ") tileset's texture is not valid.";
                Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning, log.str());
                continue;
            }

            for (int layerIdx = 0; layerIdx < tilemap.layers.size(); layerIdx++)
            {
                if (!tilemap.layerVisibility[layerIdx]) continue;

                TileLayer& layer = tilemap.layers[layerIdx];

                Vec2 tileWorldSize = { tilemap.tileSize * tf.scale.x, tilemap.tileSize * tf.scale.y };

                int minCol = (std::max)(0, int((camMin.x - tf.worldPosition.x) / tileWorldSize.x));
                int maxCol = (std::min)(int(layer.width), int((camMax.x - tf.worldPosition.x) / tileWorldSize.x) + 1);
                int minRow = (std::max)(0, int((camMin.y - tf.worldPosition.y) / tileWorldSize.y));
                int maxRow = (std::min)(int(layer.height), int((camMin.y - tf.worldPosition.y) / tileWorldSize.y));

                int visibleTiles = (std::max)(0, int((maxCol - minCol) * (maxRow - minRow)));
                estimatedSpriteCount += visibleTiles;
            }
        }

        allSprites.reserve(aEntities.size() + particleArray.Size());

        // caching the hierarchy order
        std::unordered_map<Entity, int> hierarchyCache;
        hierarchyCache.reserve(tmArray.Size() + aEntities.size());

        for (const auto& entity : aEntities)
        {
            hierarchyCache[entity] = pCoordinator->GetHierarchyIndex(entity);
        }

        for (const auto& entity : tmArray.GetAllEntities())
        {
            hierarchyCache[entity] = pCoordinator->GetHierarchyIndex(entity);
        }

        // Gather all sprite info with layer data
        for (const auto& entity : aEntities)
        {
            if (!pCoordinator->IsActiveInHierarchy(entity))
                continue;

            auto& sr = srArray.GetData(entity);
            auto& tf = tfArray.GetData(entity);

            if (!sr.texture || sr.texture->tex_id == 0)
            {
                sr.texture = pResourcesManager->GetTexture(sr.texturePath);
            }

            if (!sr.texture || sr.texture->tex_id == 0)
            {
                std::stringstream log;
                log << "Failed to load texture: " << sr.texturePath;
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

            if (isCullMode && !IsSpriteVisible(tf.worldPosition, spriteScale, camMin, camMax))
            {
                continue;
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

            // Add offset
            Vec2 spritePos = tf.worldPosition + sr.spriteOffset;

            // Get UV coordinates from animator if present
            Vec2 uvOffset(0.0f, 0.0f);
            Vec2 uvSize(1.0f, 1.0f);

            std::shared_ptr<Uma_Engine::Texture> activeTexture = sr.texture;

            if (animatorArray.Has(entity))
            {
                auto& animator = animatorArray.GetData(entity);
                uvOffset = animator.uvOffset;
                uvSize = animator.uvSize;

                if (animator.activeTexture)
                {
                    activeTexture = animator.activeTexture;
                }
            }
            else
            {
                sr.GetUVs(uvOffset, uvSize);
            }

            // Check for material override
            unsigned int shaderId = 0;
            const std::unordered_map<std::string, MaterialValue>* matProps = nullptr;
            const std::vector<Uma_Engine::UniformInfo>* matUniforms = nullptr;

            if (pCoordinator->HasComponent<SpriteMaterial>(entity))
            {
                auto& mat = pCoordinator->GetComponent<SpriteMaterial>(entity);
                if (!mat.effectName.empty())
                {
                    auto* effect = pResourcesManager->GetEffect(mat.effectName);
                    if (effect && effect->shaderProgramID != 0)
                    {
                        shaderId = effect->shaderProgramID;
                        matProps = &mat.properties;
                        matUniforms = &effect->uniforms;
                    }
                }
            }

            allSprites.push_back(LayeredSprite
                {
                    .info = Uma_Engine::Sprite_Info{
                        .tex_id = activeTexture->tex_id,
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
                    .texId = activeTexture->tex_id,
                    .entityId = entity,
                    .shaderId = shaderId,
                    .properties = matProps,
                    .uniforms = matUniforms
                });
        }

        for (const auto& entity : tmArray.GetAllEntities())
        {
            if (!pCoordinator->IsActiveInHierarchy(entity)) continue;

            auto& tilemap = tmArray.GetData(entity);
            auto& tf = tfArray.GetData(entity);

            int hierarchyOrder = hierarchyCache[entity];
            if (hierarchyOrder == -1) continue;

            if (!tilemap.tileset.texture || tilemap.tileset.texture->tex_id == 0)
            {
                tilemap.tileset.texture = pResourcesManager->GetTexture(tilemap.tileset.texturePath);
            }

            // Verify texture is valid before using it
            if (!tilemap.tileset.texture || tilemap.tileset.texture->tex_id == 0)
            {
                std::stringstream log;
                log << "tilemap(" << entity << ") has no valid texture";
                Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning, log.str());
                continue;
            }

            for (int layerIdx = 0; layerIdx < tilemap.layers.size(); layerIdx++)
            {
                TileLayer& layer = tilemap.layers[layerIdx];

                if (!tilemap.layerVisibility[layerIdx]) continue;

                // Calculate the the visible tile range
                float tileWorldSizeX = tilemap.tileSize * tf.scale.x;
                float tileWorldSizeY = tilemap.tileSize * tf.scale.y;

                int minCol = (std::max)(0, int((camMin.x - tf.worldPosition.x) / tileWorldSizeX));
                int maxCol = (std::min)(int(layer.width), int((camMax.x - tf.worldPosition.x) / tileWorldSizeX) + 1);
                int minRow = (std::max)(0, int((tf.worldPosition.y - camMax.y) / tileWorldSizeY));
                int maxRow = (std::min)(int(layer.height), int((tf.worldPosition.y - camMin.y) / tileWorldSizeY) + 1);

                // Only iterate visible tiles

                if (!isCullMode)
                {
                    minCol = 0;
                    maxCol = tilemap.mapWidth;
                    minRow = 0;
                    maxRow = tilemap.mapHeight;
                }

                for (int row = minRow; row < maxRow; row++)
                {
                    for (int col = minCol; col < maxCol; col++)
                    {
                        int i = row * layer.width + col;

                        // Skip empty tiles (tile ID 0 or -1)
                        if (layer.tiles[i] < 0) continue;

                        // Grid position - snap to pixel grid to prevent sub-pixel seams
                        Vec2 tilePos = tf.worldPosition + Vec2(
                            col * tileWorldSizeX,
                            -(row * tileWorldSizeY)
                        );
                        tilePos.x = std::round(tilePos.x);
                        tilePos.y = std::round(tilePos.y);

                        // Tileset indices for UVs
                        int tileset_row = layer.tiles[i] / int(tilemap.tileset.columns);
                        int tileset_col = layer.tiles[i] % int(tilemap.tileset.columns);

                        Vec2 uvOffset(0.0f, 0.0f);
                        Vec2 uvSize(1.0f, 1.0f);
                        tilemap.tileset.GetUVs(uvOffset, uvSize, Vec2(static_cast<float>(tileset_col), static_cast<float>(tileset_row)));

                        allSprites.push_back(LayeredSprite
                            {
                                .info = Uma_Engine::Sprite_Info{
                                    .tex_id = tilemap.tileset.texture->tex_id,
                                    .pos = tilePos,
                                    .scale = Vec2(tileWorldSizeX, tileWorldSizeY),
                                    .rot = tf.worldRotation,
                                    .rot_speed = tf.rotation.y,
                                    .uvOffset = uvOffset,
                                    .uvSize = uvSize,
                                    .tintColor = layer.tintColor,
                                    .alpha = layer.alpha
                                },
                                .layer = layer.renderLayer,
                                .order = layer.renderOrder,
                                .hierarchyOrder = hierarchyOrder,
                                .texId = tilemap.tileset.texture->tex_id,
                                .entityId = entity
                            });
                    }
                }
            }
        }

        // Gather ALL particle entities
        std::vector<Entity> particleEntities = particleArray.GetAllEntities();

        for (const auto& entity : particleEntities)
        {
            if (!pCoordinator->IsActiveInHierarchy(entity))
                continue;

            auto& component = particleArray.GetData(entity);
            int hierarchyOrder = pCoordinator->GetHierarchyIndex(entity);
            if (hierarchyOrder == -1) continue;

            // Loop through each emitter in this component
            for (int emitterIdx = 0; emitterIdx < component.GetEmitterCount(); ++emitterIdx)
            {
                auto* emitter = component.GetEmitter(emitterIdx);
                if (!emitter || !emitter->isActive) continue;

                // Get texture
                auto texture = pResourcesManager->GetTexture(emitter->texturePath);
                if (!texture || texture->tex_id == 0)
                {
                    std::stringstream log;
                    log << "ParticleEmitter '" << emitter->name << "': Invalid texture '" << emitter->texturePath << "'";
                    Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning, log.str());
                    continue;
                }

                // Add each active particle as a LayeredSprite
                for (const auto& particle : emitter->particles)
                {
                    if (!particle.active) continue;

                    allSprites.push_back(LayeredSprite{
                        .info = Uma_Engine::Sprite_Info{
                            .tex_id = texture->tex_id,
                            .pos = particle.position,
                            .scale = Vec2(particle.scale, particle.scale),
                            .rot = particle.rotation,
                            .rot_speed = 0.0f,
                            .uvOffset = Vec2(0.0f, 0.0f),
                            .uvSize = Vec2(1.0f, 1.0f),
                            .tintColor = particle.color,
                            .alpha = particle.opacity
                        },
                        .layer = emitter->renderLayer,
                        .order = emitter->renderOrder,
                        .hierarchyOrder = hierarchyOrder,
                        .texId = texture->tex_id,
                        .entityId = entity
                        });
                }
            }
        }
    }

    bool RenderingSystem::IsSpriteVisible(const Vec2& spritePos, const Vec2& spriteScale,
        const Vec2& camMin, const Vec2& camMax)
    {
        float halfSpriteWidth = spriteScale.x * 0.5f;
        float halfSpriteHeight = spriteScale.y * 0.5f;

        float spriteLeft = spritePos.x - halfSpriteWidth;
        float spriteRight = spritePos.x + halfSpriteWidth;
        float spriteTop = spritePos.y + halfSpriteHeight;
        float spriteBottom = spritePos.y - halfSpriteHeight;

        if (spriteRight < camMin.x) return false;   // Too far left
        if (spriteLeft > camMax.x) return false;    // Too far right
        if (spriteBottom > camMax.y) return false;  // Too far up
        if (spriteTop < camMin.y) return false;     // Too far down

        return true;  // Sprite is visible
    }

    void RenderingSystem::RenderWorldSprites(std::vector<LayeredSprite>& allSprites)
    {
        std::stable_sort(allSprites.begin(), allSprites.end(),
            [](const LayeredSprite& a, const LayeredSprite& b)
            {
                if (a.layer != b.layer)
                    return a.layer < b.layer;
                if (a.order != b.order)
                    return a.order < b.order;
                if (a.hierarchyOrder != b.hierarchyOrder)
                    return a.hierarchyOrder < b.hierarchyOrder;
                if (a.shaderId != b.shaderId)
                    return a.shaderId < b.shaderId;
                if (a.texId != b.texId)
                    return a.texId < b.texId;
                return a.entityId < b.entityId;
            });

        // Batch within same layer+order+hierarchy+shader only
        std::map<unsigned int, std::vector<Uma_Engine::Sprite_Info>> sorted_sprites;

        if (!allSprites.empty())
        {
            LayerMask currentLayer = allSprites[0].layer;
            int currentOrder = allSprites[0].order;
            int currentHierarchy = allSprites[0].hierarchyOrder;
            unsigned int currentShader = allSprites[0].shaderId;

            // Track material pointers for current batch (all sprites in a shader batch share the same material)
            const std::unordered_map<std::string, Uma_ECS::MaterialValue>* currentProps = allSprites[0].properties;
            const std::vector<Uma_Engine::UniformInfo>* currentUniforms = allSprites[0].uniforms;

            for (const auto& layeredSprite : allSprites)
            {
                // Flush if ANY of the sorting criteria changed
                if (layeredSprite.layer != currentLayer ||
                    layeredSprite.order != currentOrder ||
                    layeredSprite.hierarchyOrder != currentHierarchy ||
                    layeredSprite.shaderId != currentShader)
                {
                    // Render all batches from previous group
                    for (const auto& pair : sorted_sprites)
                    {
                        pGraphics->DrawSpritesInstanced(pair.first, pair.second,
                            currentShader, currentProps, currentUniforms);
                    }
                    sorted_sprites.clear();

                    // Update current sorting group
                    currentLayer = layeredSprite.layer;
                    currentOrder = layeredSprite.order;
                    currentHierarchy = layeredSprite.hierarchyOrder;
                    currentShader = layeredSprite.shaderId;
                    currentProps = layeredSprite.properties;
                    currentUniforms = layeredSprite.uniforms;
                }

                // Add to batch
                sorted_sprites[layeredSprite.texId].push_back(layeredSprite.info);
            }

            // Render remaining batches
            for (const auto& pair : sorted_sprites)
            {
                pGraphics->DrawSpritesInstanced(pair.first, pair.second,
                    currentShader, currentProps, currentUniforms);
            }
        }
    }

    void RenderingSystem::GatherUIElements(std::vector<UIDrawCommand>& uiDrawCommands)
    {
        auto& canvasArray = pCoordinator->GetComponentArray<Uma_UI::Canvas>();
        //auto& tfArray = pCoordinator->GetComponentArray<Uma_ECS::Transform>();
        auto& rtfArray = pCoordinator->GetComponentArray<Uma_UI::RectTransform>();

        std::vector<Entity> sortedCanvasIds = canvasArray.GetAllEntities();

        // cache hierarchy order
        std::unordered_map<Entity, int> hierarchyCache;
        hierarchyCache.reserve(sortedCanvasIds.size());

        for (const auto& canvasId : sortedCanvasIds)
        {
            hierarchyCache[canvasId] = pCoordinator->GetHierarchyIndex(canvasId);
        }

        std::sort(sortedCanvasIds.begin(), sortedCanvasIds.end(),
            [&](const Entity& lhs, const Entity& rhs)
            {
                int lhs_sorting_order = canvasArray.GetData(lhs).sortingOrder;
                int rhs_sorting_order = canvasArray.GetData(rhs).sortingOrder;

                int lhs_hierarchyOrder = hierarchyCache[lhs];
                int rhs_hierarchyOrder = hierarchyCache[rhs];

                if (lhs_sorting_order != rhs_sorting_order)
                {
                    return lhs_sorting_order < rhs_sorting_order;
                }

                if (lhs_hierarchyOrder != rhs_hierarchyOrder)
                {
                    return lhs_hierarchyOrder < rhs_hierarchyOrder;
                }

                return false;
            });

        for (const auto& canvasId : sortedCanvasIds)
        {
            std::vector<Entity> childrenList;

            GetAllChildren(canvasId, childrenList);

            //auto& canvas = canvasArray.GetData(canvasId);

            for (int i = 0; i < childrenList.size(); i++)
            {
                Entity childUI = childrenList[i];

                if (!pCoordinator->IsActiveInHierarchy(childUI) || !rtfArray.Has(childUI)) continue;

                auto& rectTransform = rtfArray.GetData(childUI);

                if (pCoordinator->HasComponent<Uma_UI::Text>(childUI))
                {
                    UIDrawCommand::Type uiType = UIDrawCommand::UI_TEXT;

                    auto& textComp = pCoordinator->GetComponent<Uma_UI::Text>(childUI);
                    if (!textComp.visible || textComp.text.empty())
                    {
                        continue;
                    }

                    Uma_Engine::FontData* uiFont = pResourcesManager->GetFont(textComp.fontPath);

                    // Get font, if null add to load container
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

                    if (!image.visible) continue;

                    if (!image.texture || image.texture->tex_id == 0 || image.change)
                    {
                        image.texture = pResourcesManager->GetTexture(image.texturePath);
                        image.change = false;
                    }

                    if (!image.texture || image.texture->tex_id == 0)
                    {
                        std::stringstream log;
                        log << "Entity(" << childUI << ") failed to load texture: " << image.texturePath;
                        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning, log.str());
                        continue;
                    }

                    Vec2 uvOffset, uvSize;
                    image.GetUVs(uvOffset, uvSize);

                    Uma_Engine::Sprite_Info spriteInfo = Uma_Engine::Sprite_Info
                    {
                        .tex_id = image.texture->tex_id,
                        .pos = rectTransform.computedRect.Center(),
                        .scale = rectTransform.computedRect.Size(),
                        .rot = 0.0f,
                        .rot_speed = 0.0f,
                        .uvOffset = uvOffset,
                        .uvSize = uvSize,
                        .tintColor = image.color.ToVec3(),
                        .alpha = image.color.a,
                        .fillDirection = static_cast<int>(image.fillDirection),
                        .fillAmount = image.fillAmount
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
                std::stable_sort(uiDrawCommands.begin(), uiDrawCommands.end(),
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
        std::unordered_set<Entity> visited;
        visited.insert(parent);

        // Iterative DFS using an explicit stack to avoid call-stack overflow
        // and to detect circular parent-child references
        std::vector<Entity> stack;

        if (!pCoordinator->HasComponent<Transform>(parent)) return;

        const auto& rootTf = pCoordinator->GetComponent<Transform>(parent);
        // Push in reverse so children come out in original order
        for (auto it = rootTf.children.rbegin(); it != rootTf.children.rend(); ++it)
            stack.push_back(*it);

        while (!stack.empty())
        {
            Entity current = stack.back();
            stack.pop_back();

            if (visited.count(current))
            {
                Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning,
                    "Circular parent-child relationship detected for entity: " + std::to_string(current));
                continue;
            }
            visited.insert(current);
            childrenList.push_back(current);

            if (!pCoordinator->HasComponent<Transform>(current)) continue;

            const auto& tf = pCoordinator->GetComponent<Transform>(current);
            for (auto it = tf.children.rbegin(); it != tf.children.rend(); ++it)
                stack.push_back(*it);
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
                    command.spriteInfo.alpha,
                    command.spriteInfo.fillDirection,
                    command.spriteInfo.fillAmount);

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