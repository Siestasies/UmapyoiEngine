#pragma once

#include "Core/System.hpp"
#include "Core/Coordinator.hpp"
#include "Systems/Graphics.hpp"
#include "Systems/ResourcesManager.hpp"
#include <random>

namespace Uma_ECS
{
    class ParticleSystem : public ECSSystem
    {
    public:
        void Init(Uma_Engine::Graphics* g, Uma_Engine::ResourcesManager* rm, Coordinator* c);
        void Update(float dt);

    private:
        void SpawnParticle(ParticleEmitter& emitter, const Vec2& position);
        void SpawnScreenFillParticle(ParticleEmitter& emitter);
        void UpdateParticle(Particle& p, ParticleEmitter& emitter, float dt);

        float Random(float min, float max);

        Coordinator* pCoordinator = nullptr;
        Uma_Engine::Graphics* pGraphics = nullptr;
        Uma_Engine::ResourcesManager* pResourcesManager = nullptr;
        std::default_random_engine generator;
        std::uniform_real_distribution<float> distribution{ 0.0f, 1.0f };
    };
}