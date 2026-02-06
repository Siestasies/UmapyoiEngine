/*!
\file    ParticleSystem.cpp
\par     Project: GAM200
\par     Course: CSD2401
\par     Section A
\par     Software Engineering Project 3

\author Javier Chua Dong Qing (100%)
\par     E-mail: javierdongqing.chua@digipen.edu
\par     DigiPen login: javierdongqing.chua

\brief
Defines the ECS System for updating and rendering of all particles.
It handles: Spawning logic, Physics integration, Visual updates, Rendering

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "ParticleSystem.hpp"
#include "Components/ParticleEmitter.h"
#include "Components/Transform.h"
#include "Systems/ResourcesTypes.hpp"
#include "Components/Camera.h"
#include "Debugging/Debugger.hpp"
#include <cmath>
#include <sstream>
#include <algorithm>

namespace Uma_ECS
{
    void ParticleSystem::Init(Uma_Engine::Graphics* g, Uma_Engine::ResourcesManager* rm, Coordinator* c)
    {
        pCoordinator = c;
        pGraphics = g;
        pResourcesManager = rm;
    }

    void ParticleSystem::Update(float dt)
    {
        if (!aEntities.size()) return;

        auto& emitterArray = pCoordinator->GetComponentArray<ParticleEmitter>();
        auto& tfArray = pCoordinator->GetComponentArray<Transform>();
        auto& camArray = pCoordinator->GetComponentArray<Camera>();

        // Calculate screen bounds from camera for ScreenFill mode
        Vec2 screenMin = { 0, 0 };
        Vec2 screenMax = { 1920, 1080 };

        if (camArray.Size() > 0)
        {
            Entity camera = camArray.GetEntity(0);
            auto& cam_tf = tfArray.GetData(camera);
            auto& cam_c = camArray.GetData(camera);

            int width = pGraphics->GetViewportWidth();
            int height = pGraphics->GetViewportHeight();
            float zoom = cam_c.mZoom;

            float halfWidth = (width * 0.5f) / zoom;
            float halfHeight = (height * 0.5f) / zoom;

            screenMin = Vec2(cam_tf.position.x - halfWidth, cam_tf.position.y - halfHeight);
            screenMax = Vec2(cam_tf.position.x + halfWidth, cam_tf.position.y + halfHeight);
        }

        for (const auto& entity : aEntities)
        {
            if (!pCoordinator->IsActiveInHierarchy(entity))
                continue;

            auto& component = emitterArray.GetData(entity);
            auto& transform = tfArray.GetData(entity);

            // Loop through all emitters in this entity
            for (auto& emitter : component.emitters)
            {
                // Update screen bounds for ScreenFill mode
                if (emitter.mode == EmitterMode::ScreenFill)
                {
                    emitter.screenMin = screenMin;
                    emitter.screenMax = screenMax;
                }

                // Emission logic
                if (emitter.isActive)
                {
                    if (emitter.mode == EmitterMode::Burst)
                    {
                        if (!emitter.initialized)
                        {
                            // Initial burst
                            for (int i = 0; i < emitter.maxParticles; ++i)
                            {
                                SpawnParticle(emitter, transform.worldPosition);
                            }
                            emitter.initialized = true;
                            emitter.burstTimer = 0.0f;
                        }
                        else if (emitter.emission.loop)
                        {
                            // Handle looping burst
                            emitter.burstTimer += dt;

                            // Check if all particles are dead
                            bool allDead = true;
                            for (const auto& p : emitter.particles)
                            {
                                if (p.active)
                                {
                                    allDead = false;
                                    break;
                                }
                            }

                            if (allDead && emitter.burstTimer >= emitter.emission.loopDelay)
                            {
                                // Trigger new burst
                                for (int i = 0; i < emitter.maxParticles; ++i)
                                {
                                    SpawnParticle(emitter, transform.worldPosition);
                                }
                                emitter.burstTimer = 0.0f;
                            }
                        }
                    }
                    else if (emitter.mode == EmitterMode::Continuous)
                    {
                        emitter.emissionTimer += dt;
                        float interval = 1.0f / emitter.emission.emissionRate;

                        while (emitter.emissionTimer >= interval)
                        {
                            SpawnParticle(emitter, transform.worldPosition);
                            emitter.emissionTimer -= interval;
                        }
                    }
                    else if (emitter.mode == EmitterMode::ScreenFill)
                    {
                        // Gradual initial spawn
                        if (!emitter.initialized)
                        {
                            emitter.emissionTimer = 0.0f;
                            emitter.initialized = true;
                        }

                        // Spawn particles gradually over first second
                        if (emitter.particles.size() < static_cast<size_t>(emitter.maxParticles))
                        {
                            emitter.emissionTimer += dt;
                            float spawnInterval = 1.0f / (emitter.maxParticles * 2.0f); // Spawn over 0.5 seconds

                            while (emitter.emissionTimer >= spawnInterval &&
                                emitter.particles.size() < static_cast<size_t>(emitter.maxParticles))
                            {
                                SpawnScreenFillParticle(emitter, true); // true = initial spawn
                                emitter.emissionTimer -= spawnInterval;
                            }
                        }

                        // Continuous respawning for off-screen particles
                        int inactiveCount = 0;
                        for (const auto& p : emitter.particles)
                        {
                            if (!p.active) inactiveCount++;
                        }

                        for (int i = 0; i < inactiveCount; ++i)
                        {
                            SpawnScreenFillParticle(emitter, false); // false = respawn at top
                        }
                    }
                }

                // Update particles
                for (auto& p : emitter.particles)
                {
                    if (!p.active) continue;
                    UpdateParticle(p, emitter, dt);
                }

                // Render particles
                /*auto texture = pResourcesManager->GetTexture(emitter.textureName);
                if (!texture || texture->tex_id == 0)
                {
                    std::stringstream log;
                    log << "ParticleEmitter '" << emitter.name << "': Invalid texture '" << emitter.textureName << "'";
                    Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning, log.str());
                    continue;
                }

                std::vector<Uma_Engine::Sprite_Info> instanceData;
                instanceData.reserve(emitter.maxParticles);

                for (const auto& particle : emitter.particles)
                {
                    if (!particle.active) continue;

                    Uma_Engine::Sprite_Info instance;
                    instance.pos = particle.position;
                    instance.scale = Vec2(particle.scale, particle.scale);
                    instance.rot = particle.rotation;
                    instance.uvOffset = Vec2(0.0f, 0.0f);
                    instance.uvSize = Vec2(1.0f, 1.0f);
                    instance.tintColor = particle.color;
                    instance.alpha = particle.opacity;

                    instanceData.push_back(instance);
                }

                if (!instanceData.empty())
                {
                    pGraphics->DrawSpritesInstanced(texture->tex_id, instanceData);
                }*/
            }
        }
    }

    void ParticleSystem::SpawnParticle(EmitterInstance& emitter, const Vec2& emitterPos)
    {
        for (auto& p : emitter.particles)
        {
            if (!p.active)
            {
                p.active = true;
                p.age = 0.0f;

                // Position with spawn offset and radius
                Vec2 spawnPos = emitterPos + emitter.spawn.spawnOffset;
                if (emitter.spawn.spawnRadius > 0.0f)
                {
                    float angle = Random(0, 360) * 0.0174533f;
                    float distance = Random(0, emitter.spawn.spawnRadius);
                    spawnPos.x += std::cos(angle) * distance;
                    spawnPos.y += std::sin(angle) * distance;
                }
                p.position = spawnPos;

                // Lifetime
                p.lifetime = Random(emitter.physics.lifetimeRange.x, emitter.physics.lifetimeRange.y);
                p.maxLifetime = p.lifetime;

                // Scale
                p.scale = Random(emitter.appearance.scaleRange.x, emitter.appearance.scaleRange.y);

                // Color
                p.color = emitter.appearance.startColor;

                // Velocity with emission cone
                float angle = Random(0, 360) * 0.0174533f;
                if (emitter.spawn.useEmissionCone)
                {
                    float centerAngle = emitter.spawn.emissionAngle * 0.0174533f;
                    float spreadHalf = (emitter.spawn.emissionSpread * 0.5f) * 0.0174533f;
                    angle = centerAngle + Random(-spreadHalf, spreadHalf);
                }

                float speed = Random(emitter.physics.speedRange.x, emitter.physics.speedRange.y);
                p.velocity = { std::cos(angle) * speed, std::sin(angle) * speed };

                // Rotation
                p.rotation = Random(0, 360);
                if (emitter.appearance.rotateParticles)
                {
                    p.rotationSpeed = Random(emitter.appearance.rotationSpeedRange.x,
                        emitter.appearance.rotationSpeedRange.y);
                }
                else
                {
                    p.rotationSpeed = 0.0f;
                }

                // Opacity
                if (emitter.appearance.randomOpacity)
                {
                    p.baseOpacity = Random(emitter.appearance.opacityRange.x, emitter.appearance.opacityRange.y);
                }
                else
                {
                    p.baseOpacity = 1.0f;
                }

                // Start with fade in if enabled
                if (emitter.fade.fadeIn)
                {
                    p.opacity = 0.0f;
                }
                else
                {
                    p.opacity = p.baseOpacity;
                }

                return;
            }
        }

        // No inactive particle found, create new one if under limit
        if (emitter.particles.size() < static_cast<size_t>(emitter.maxParticles))
        {
            emitter.particles.emplace_back();
            SpawnParticle(emitter, emitterPos);
        }
    }

    void ParticleSystem::UpdateParticle(Particle& p, EmitterInstance& emitter, float dt)
    {
        // Update age
        p.age += dt;

        // Apply physics
        p.velocity += emitter.physics.gravity * dt;

        // Apply drag
        if (emitter.physics.drag > 0.0f)
        {
            float dragFactor = 1.0f - (emitter.physics.drag * dt);
            if (dragFactor < 0.0f) dragFactor = 0.0f;
            p.velocity.x *= dragFactor;
            p.velocity.y *= dragFactor;
        }

        p.position += p.velocity * dt;

        // Update rotation
        p.rotation += p.rotationSpeed * dt;

        // Update lifetime
        p.lifetime -= dt;

        // Mode updates
        if (emitter.mode == EmitterMode::ScreenFill)
        {
            // Check if off-screen, delete
            float margin = emitter.screenFill.spawnMargin;
            bool offScreen =
                p.position.y < emitter.screenMin.y - margin ||
                p.position.x < emitter.screenMin.x - margin ||
                p.position.x > emitter.screenMax.x + margin ||
                p.position.y > emitter.screenMax.y + margin;

            if (offScreen)
            {
                p.active = false;
                return;
            }

            // Start with base opacity
            float finalOpacity = p.baseOpacity;

            // Fade in (for ScreenFill)
            if (emitter.fade.fadeIn && p.age < emitter.fade.fadeInDuration)
            {
                float fadeInProgress = p.age / emitter.fade.fadeInDuration;
                finalOpacity *= fadeInProgress;
            }

            // Edge fade for ScreenFill
            if (emitter.fade.fadeAtEdges)
            {
                float fadeDistance = emitter.fade.edgeFadeDistance;

                // Check distance from each edge
                float distFromLeft = p.position.x - emitter.screenMin.x;
                float distFromRight = emitter.screenMax.x - p.position.x;
                float distFromTop = emitter.screenMax.y - p.position.y;
                float distFromBottom = p.position.y - emitter.screenMin.y;

                // Only apply edge fade if particle is within screen bounds
                bool withinBounds = distFromLeft >= 0 && distFromRight >= 0 &&
                    distFromTop >= 0 && distFromBottom >= 0;

                if (withinBounds)
                {
                    // Find closest edge
                    float minDist = distFromLeft;
                    if (distFromRight < minDist) minDist = distFromRight;
                    if (distFromTop < minDist) minDist = distFromTop;
                    if (distFromBottom < minDist) minDist = distFromBottom;

                    if (minDist < fadeDistance)
                    {
                        float edgeFade = minDist / fadeDistance;
                        finalOpacity *= edgeFade;
                    }
                }
            }

            p.opacity = finalOpacity;
        }
        else // Burst/Continuous
        {
            // Check lifetime
            if (p.lifetime <= 0)
            {
                p.active = false;
                return;
            }

            // Start with base opacity
            float finalOpacity = p.baseOpacity;

            // Fade In
            if (emitter.fade.fadeIn && p.age < emitter.fade.fadeInDuration)
            {
                float fadeInProgress = p.age / emitter.fade.fadeInDuration;
                finalOpacity *= fadeInProgress;
            }

            // Fade Out
            if (emitter.fade.fadeOut && p.lifetime < emitter.fade.fadeOutDuration)
            {
                float fadeOutProgress = p.lifetime / emitter.fade.fadeOutDuration;
                finalOpacity *= fadeOutProgress;
            }

            p.opacity = finalOpacity;

            // Color interpolation
            if (emitter.appearance.colorLerp && p.maxLifetime > 0)
            {
                float t = 1.0f - (p.lifetime / p.maxLifetime);
                p.color.x = emitter.appearance.startColor.x + (emitter.appearance.endColor.x - emitter.appearance.startColor.x) * t;
                p.color.y = emitter.appearance.startColor.y + (emitter.appearance.endColor.y - emitter.appearance.startColor.y) * t;
                p.color.z = emitter.appearance.startColor.z + (emitter.appearance.endColor.z - emitter.appearance.startColor.z) * t;
            }
        }
    }

    float ParticleSystem::Random(float min, float max)
    {
        return min + distribution(generator) * (max - min);
    }

    void ParticleSystem::SpawnScreenFillParticle(EmitterInstance& emitter, bool initialSpawn)
    {
        for (auto& p : emitter.particles)
        {
            if (!p.active)
            {
                p.active = true;
                p.age = 0.0f;
                p.lifetime = 999.0f;
                p.maxLifetime = p.lifetime;

                // Spawn position
                if (initialSpawn)
                {
                    // Initial spawn: distribute across entire viewport
                    p.position.x = Random(emitter.screenMin.x, emitter.screenMax.x);
                    p.position.y = Random(emitter.screenMin.y, emitter.screenMax.y);
                }
                else if (emitter.screenFill.spawnAtTop)
                {
                    // Respawn: spawn above screen
                    p.position.x = Random(emitter.screenMin.x, emitter.screenMax.x);
                    p.position.y = emitter.screenMax.y + Random(0, emitter.screenFill.spawnMargin);
                }
                else
                {
                    // Respawn: anywhere in viewport
                    p.position.x = Random(emitter.screenMin.x, emitter.screenMax.x);
                    p.position.y = Random(emitter.screenMin.y, emitter.screenMax.y);
                }

                // Depth factor for layering (0.0 = far, 1.0 = near)
                float depth = Random(0.3f, 1.0f);

                // Velocity scaled by depth (farther = slower)
                float depthScale = 0.5f + (depth * 0.5f); // 0.5 to 1.0 range
                p.velocity.x = Random(emitter.screenFill.velocityXRange.x,
                    emitter.screenFill.velocityXRange.y) * depthScale;
                p.velocity.y = Random(emitter.screenFill.velocityYRange.x,
                    emitter.screenFill.velocityYRange.y) * depthScale;

                // Scale based on depth (farther = smaller)
                float baseScale = Random(emitter.appearance.scaleRange.x, emitter.appearance.scaleRange.y);
                p.scale = baseScale * depthScale;

                // Color
                p.color = emitter.appearance.startColor;

                // Rotation
                p.rotation = Random(0, 360);
                if (emitter.appearance.rotateParticles)
                {
                    p.rotationSpeed = Random(emitter.appearance.rotationSpeedRange.x,
                        emitter.appearance.rotationSpeedRange.y) * depthScale;
                }
                else
                {
                    p.rotationSpeed = 0.0f;
                }

                // Opacity based on depth (farther = more transparent)
                float baseOpacity;
                if (emitter.appearance.randomOpacity)
                {
                    baseOpacity = Random(emitter.appearance.opacityRange.x, emitter.appearance.opacityRange.y);
                }
                else
                {
                    baseOpacity = 1.0f;
                }
                p.baseOpacity = baseOpacity * (0.4f + depth * 0.6f); // 0.4 to 1.0 range

                // Start with fade in if enabled (only on respawn, not initial)
                if (emitter.fade.fadeIn && !initialSpawn)
                {
                    p.opacity = 0.0f;
                }
                else
                {
                    p.opacity = p.baseOpacity;
                }

                return;
            }
        }

        // No inactive particle found, create new one if under limit
        if (emitter.particles.size() < static_cast<size_t>(emitter.maxParticles))
        {
            emitter.particles.emplace_back();
            SpawnScreenFillParticle(emitter, initialSpawn);
        }
    }
}