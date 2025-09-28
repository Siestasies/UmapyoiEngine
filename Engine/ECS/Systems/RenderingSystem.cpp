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

        for (const auto& entity : aEntities)
        {
            auto& sr = srArray.GetData(entity);
            auto& tf = tfArray.GetData(entity);

            if (!sr.texture)
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
            }
       }
    }
}