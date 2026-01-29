#pragma once

#include "../Core/Types.hpp"
#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"

#include "../Components/Tilemap.h"
#include "Systems/Graphics.hpp"
#include "Systems/ResourcesManager.hpp"

namespace Uma_ECS
{
    struct tilemapLayerInfo
    {
        Entity tilemapId;
        int index;
        int sortingOrder;
    };

    class TilemapSystem : public ECSSystem
    {
    public:
        void Init(Coordinator* coord, Uma_Engine::Graphics* graphics, Uma_Engine::ResourcesManager* resourcesManager);

        void Update(float dt);

        void Shutdown();

    private:

        Coordinator* pCoordinator = nullptr;
        Uma_Engine::Graphics* pGraphics = nullptr;
        Uma_Engine::ResourcesManager* pResourcesManager = nullptr;
    };
}