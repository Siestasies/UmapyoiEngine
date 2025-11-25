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
        void Init(Uma_Engine::Graphics* g, Uma_Engine::ResourcesManager* rm, Coordinator* c);
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