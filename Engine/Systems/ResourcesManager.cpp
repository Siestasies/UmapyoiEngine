#include "ResourcesManager.hpp"

#include "../Systems/Graphics.hpp"
#include "../../Core/SystemManager.h"

//Uma_Engine::Graphics* gGraphics;

namespace Uma_Engine
{
    void ResourcesManager::Init()
    {
        gGraphics = pSystemManager->GetSystem<Graphics>();
    }

    void ResourcesManager::Update(float dt)
    {

    }

    void ResourcesManager::Shutdown()
    {
        
    }
}