#include "ResourcesManager.hpp"

#include "../Systems/Graphics.hpp"
#include "../../Core/SystemManager.h"

//Uma_Engine::Graphics* gGraphics;
#include <cassert>

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

    void ResourcesManager::Load_Texture(const std::string& texname, const std::string& filepath)
    {
        assert(gGraphics != nullptr && "Error : Graphics system is not init properply.");

        Texture tex;

        tex = gGraphics->LoadTexture(filepath);
    }
}