#include "RenderingSystem.hpp"
#include "Core/Coordinator.hpp"

#include "Components/SpriteRenderer.h"
#include "Components/Transform.h"

#include "Debugging/Debugger.hpp"

#include <cassert>
#include <sstream>

namespace Uma_ECS
{
    void RenderingSystem::Update(float dt)
    {
        // OLD METHOD TO ITERATE THRU EVERY ENTITIES
        // NOT IN USED, THIS IS EXPENSIVE
        // ACCESSING THE ARRAY DIRECTLY
        // IS FASTER
        {
        //for (auto const& entity : aEntities)
        //{
        //    auto const& sr = pCoordinator->GetComponent<SpriteRenderer>(entity);
        //    auto const& tf = pCoordinator->GetComponent<Transform>(entity);

        //    // check if texture is valid
        //    if (sr.texture->tex_id == 0)
        //    {
        //        std::stringstream log;
        //        log << "Entity(" << entity << ") texture is not valid.";
        //        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning, log.str());
        //        continue;
        //    }

        //    pGraphics->DrawSprite(sr.texture->tex_id, tf.scale, tf.position);
        //}
        }

        auto& srArray = pCoordinator->GetComponentArray<SpriteRenderer>();
        auto& tfArray = pCoordinator->GetComponentArray<Transform>();

        // Iterate over the smaller array for efficiency (here, RigidBody)
        for (size_t i = 0; i < srArray.Size(); ++i)
        {
            Entity e = srArray.GetEntity(i);

            if (tfArray.Has(e))  // check if Transform exists
            {
                auto& sr = srArray.GetComponentAt(i);
                auto& tf = tfArray.GetData(e);

                if (sr.texture->tex_id == 0)
                {
                    std::stringstream log;
                    log << "Entity(" << e << ") texture is not valid.";
                    Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning, log.str());
                    continue;
                }

                pGraphics->DrawSprite(sr.texture->tex_id, tf.scale, tf.position);
            }
        }
    }
}