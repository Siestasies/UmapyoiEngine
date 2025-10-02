/*!
\file   RenderingSystem.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Implements sprite batching and rendering system that groups sprites by texture ID for instanced drawing.

Loads textures on-demand through ResourcesManager if not cached in SpriteRenderer component.
Queries camera transform and zoom from Camera component to configure graphics viewport.
Validates texture handles before rendering and logs warnings for invalid textures. Builds sorted map of sprites
grouped by texture ID, then submits batched draw calls through Graphics API for optimal performance.
Supports single camera setup with entity at index 0.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "RenderingSystem.hpp"
#include "Core/Coordinator.hpp"

#include "Components/SpriteRenderer.h"
#include "Components/Transform.h"
#include "Components/Camera.h"


#include "Debugging/Debugger.hpp"

#include <cassert>
#include <sstream>

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

        auto& srArray = pCoordinator->GetComponentArray<SpriteRenderer>();
        auto& tfArray = pCoordinator->GetComponentArray<Transform>();
        auto& camArray = pCoordinator->GetComponentArray<Camera>();

        // one camera for now
        Entity camera = camArray.GetEntity(0);
        auto& cam_tf = tfArray.GetData(camera);
        auto& cam_c = camArray.GetData(camera);

        pGraphics->SetCamInfo(cam_tf.position, cam_c.mZoom);

        // Iterate over the smaller array for efficiency (here, RigidBody)

        std::unordered_map<unsigned int, std::vector<Uma_Engine::Sprite_Info>> sorted_sprites;

        for (const auto& entity : aEntities)
        {
            auto& sr = srArray.GetData(entity);
            auto& tf = tfArray.GetData(entity);

            // Load texture if not already loaded
            if (!sr.texture)
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

            sorted_sprites[sr.texture->tex_id].push_back(Uma_Engine::Sprite_Info
                {
                    .tex_id = sr.texture->tex_id,
                    .tex_size = sr.texture->tex_size,
                    .pos = tf.position,
                    .scale = tf.scale,
                    .rot = tf.rotation.x,
                    .rot_speed = tf.rotation.y
                });

            /*if (!sr.texture)
            {
                sr.texture = pResourcesManager->GetTexture(sr.textureName);
            }
            else
            {
                if (sr.texture->tex_id == 0)
                {
                    std::stringstream log;
                    log << "Entity(" << entity << ") texture is not valid.";
                    Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning, log.str());
                    continue;
                }

                pGraphics->DrawSprite(sr.texture->tex_id, tf.scale, tf.position);
            }*/
        }

        for (const auto& pair : sorted_sprites)
        {
            const std::vector<Uma_Engine::Sprite_Info>& sprites = pair.second;

            pGraphics->DrawSpritesInstanced(
                pair.first,
                //sprites[0].tex_size,
                sprites
            );
        }
    }
}