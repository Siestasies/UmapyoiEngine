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

#include "Components/Sprite.h"
#include "Components/Transform.h"
#include "Components/Camera.h"
#include "Components/Collider.h"
#include "Components/Player.h"


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

        auto& srArray = pCoordinator->GetComponentArray<Sprite>();
        auto& tfArray = pCoordinator->GetComponentArray<Transform>();
        auto& camArray = pCoordinator->GetComponentArray<Camera>();
        auto& cArray = pCoordinator->GetComponentArray<Collider>();
        auto& pArray = pCoordinator->GetComponentArray<Player>();

        // one camera for now
        Entity camera = camArray.GetEntity(0);
        auto& cam_tf = tfArray.GetData(camera);
        auto& cam_c = camArray.GetData(camera);

        pGraphics->SetCamInfo(cam_tf.position, 10);

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

            // if use native
            // the the tf scale will be scaling the tex size with its aspect ratio
            // else it will be using the transform scale
            Vec2 spriteScale;
            if (sr.UseNativeSize)
            {
                spriteScale = sr.texture->GetNativeSize();
                spriteScale.x *= tf.scale.x;
                spriteScale.y *= tf.scale.y;
            }
            else
            {
                spriteScale = tf.scale;
            }

            sorted_sprites[sr.texture->tex_id].push_back(Uma_Engine::Sprite_Info
                {
                    .tex_id = sr.texture->tex_id,
                    //.tex_size = sr.texture->tex_size,
                    .pos = tf.position,
                    .scale = spriteScale,
                    .rot = tf.rotation.x,
                    .rot_speed = tf.rotation.y,
                });
        }

        for (const auto& pair : sorted_sprites)
        {
            const std::vector<Uma_Engine::Sprite_Info>& sprites = pair.second;

            pGraphics->DrawSpritesInstanced(
                pair.first,
                sprites
            );
        }

        for (const auto& entity : aEntities)
        {
            auto& c = cArray.GetData(entity);
            auto& tf = tfArray.GetData(entity);

            // Debug draw
            if (!c.showBBox)
            {
                continue;
            }

            for (size_t i = 0; i < c.shapes.size(); i++)
            {
                if (!c.shapes[i].isActive) continue;
                if (i >= c.bounds.size()) continue;

                const auto& shape = c.shapes[i];
                const auto& bounds = c.bounds[i];

                LayerMask effectiveLayer = c.GetEffectiveLayer(i);
                LayerMask effectiveMask = c.GetEffectiveMask(i);

                float r = 1.f, g = 0.f, b = 0.f;

                if (shape.purpose == ColliderPurpose::Trigger)
                {
                    // Triggers: Blue
                    r = 0.f; g = 0.f; b = 1.f;
                }
                else if (shape.purpose == ColliderPurpose::Environment)
                {
                    // Walls: Green
                    r = 0.f; g = 1.f; b = 0.f;
                }
                else if (shape.purpose == ColliderPurpose::Physics)
                {
                    // Check what it collides with
                    if (effectiveMask & CL_WALL)
                    {
                        // Feet (collides with walls): green
                        r = 0.f; g = 1.f; b = 0.f;
                    }
                    else if (effectiveMask & CL_ENEMY || effectiveMask & CL_PLAYER)
                    {
                        // Body (collides with enemies / player): Red
                        r = 1.f; g = 0.f; b = 0.f;
                    }
                    else
                    {
                        // Other physics: Purple
                        r = 1.f; g = 0.f; b = 1.f;
                    }
                }

                pGraphics->DrawDebugRect(bounds, r, g, b);
            }
        }
    }
}