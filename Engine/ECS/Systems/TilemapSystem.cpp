#include "TilemapSystem.hpp"

#include "ECS/Components/Transform.h"
#include "ECS/Components/Sprite.h"

namespace Uma_ECS
{
    void TilemapSystem::Init(Coordinator* coord, Uma_Engine::Graphics* graphics, Uma_Engine::ResourcesManager* resourcesManager)
    {
        pCoordinator = coord;
        pGraphics = graphics;
        pResourcesManager = resourcesManager;
    }

    void TilemapSystem::Update(float dt)
    {
        (void)dt;
    }

    void TilemapSystem::Shutdown()
    {

    }
}