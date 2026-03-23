/*!
\file   LightSystem.cpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A

\author Javier Chua Dong Qing (100%)
\par    E-mail: javierdongqing.chua@digipen.edu
\par    DigiPen login: javierdongqing.chua

\brief
Implements the 2D light rendering pass using a multiplicative light map.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "LightSystem.hpp"
#include "../Core/Coordinator.hpp"
#include "../../Systems/Graphics.hpp"
#include "../Components/Light2D.h"
#include "../Components/SceneLighting.h"
#include "../Components/Transform.h"
#include <cmath>

namespace Uma_ECS
{
    void LightSystem::Init(Uma_Engine::Graphics* graphics, Coordinator* coordinator)
    {
        pGraphics = graphics;
        pCoordinator = coordinator;
    }

    void LightSystem::Update(float dt)
    {
        if (!pGraphics || !pCoordinator) return;

        // No SceneLighting component — skip entirely, normal rendering
        auto& slArray = pCoordinator->GetComponentArray<SceneLighting>();
        auto slEntities = slArray.GetAllEntities();
        if (slEntities.empty()) return;

        auto& sl = slArray.GetData(slEntities[0]);
        Vec3 ambient = sl.ambientColor * sl.ambientStrength;

        auto& lightArray = pCoordinator->GetComponentArray<Light2D>();

        auto& tfArray = pCoordinator->GetComponentArray<Transform>();

        pGraphics->BeginLightPass(ambient.x, ambient.y, ambient.z);

        for (const auto& entity : lightArray.GetAllEntities())
        {
            if (!pCoordinator->IsActiveInHierarchy(entity)) continue;

            auto& light = lightArray.GetData(entity);
            if (!light.enabled) continue;

            if (!tfArray.Has(entity)) continue;
            auto& tf = tfArray.GetData(entity);

            // Update flicker
            float effectiveIntensity = light.intensity;
            if (light.flickerSpeed > 0.0f && light.flickerAmount > 0.0f)
            {
                light.flickerPhase += dt;
                float flicker = std::sin(light.flickerPhase * light.flickerSpeed * 6.2831853f);
                effectiveIntensity += flicker * light.flickerAmount;
                if (effectiveIntensity < 0.0f) effectiveIntensity = 0.0f;
            }

            pGraphics->DrawLight(tf.worldPosition, light.radius, light.color,
                                 effectiveIntensity, light.innerRadius);
        }

        pGraphics->EndLightPassAndComposite();
    }
}
