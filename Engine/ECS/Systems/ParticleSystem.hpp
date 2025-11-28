/*!
\file    ParticleSystem.hpp
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

#pragma once
#include "Core/System.hpp"
#include "Core/Coordinator.hpp"
#include "Systems/Graphics.hpp"
#include "Systems/ResourcesManager.hpp"
#include <random>

namespace Uma_ECS
{
    struct EmitterInstance;  // Forward declaration
    struct Particle;

    class ParticleSystem : public ECSSystem
    {
    public:
        /*!
        \brief Initializes the particle system with required engine subsystems
        \param g Pointer to the Graphics system for rendering
        \param rm Pointer to ResourcesManager for texture retrieval
        \param c Pointer to the ECS Coordinator
        */
        void Init(Uma_Engine::Graphics* g, Uma_Engine::ResourcesManager* rm, Coordinator* c);

        /*!
        \brief Main update loop for particles

        1. Calculates camera bounds (for culling/screen-fill effects)
        2. Spawns new particles based on emitter rules
        3. Updates existing particle physics and visuals
        4. Submits active particles to the Graphics system via Instanced Rendering

        \param dt Delta time in seconds.
        */
        void Update(float dt);

    private:
        Uma_Engine::Graphics* pGraphics = nullptr;
        Uma_Engine::ResourcesManager* pResourcesManager = nullptr;
        Coordinator* pCoordinator = nullptr;

        // Random number generation
        std::default_random_engine generator;
        std::uniform_real_distribution<float> distribution{ 0.0f, 1.0f };

        // Particle functions
        void SpawnParticle(EmitterInstance& emitter, const Vec2& emitterPos);
        void SpawnScreenFillParticle(EmitterInstance& emitter);
        void UpdateParticle(Particle& p, EmitterInstance& emitter, float dt);
        float Random(float min, float max);
    };
}