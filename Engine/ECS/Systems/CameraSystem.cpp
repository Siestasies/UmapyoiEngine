/*!
\file   CameraSystem.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (Everything else)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\co-author Javier Chua Dong Qing (Screenshake implementation)
\par       E-mail: javierdongqing.chua@digipen.edu
\par       DigiPen login: javierdongqing.chua

\brief
Implements camera following behavior that tracks player position when followPlayer flag is enabled.

Updates camera transform to match player transform position each frame by querying component arrays directly.
Currently supports single camera setup with entity at index 0.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "CameraSystem.hpp"

#include "Core/Coordinator.hpp"

#include "Components/Camera.h"
#include "Components/Transform.h"
#include "Components/Player.h"

#include <iostream>
#include <iomanip>
#include <cmath>

namespace Uma_ECS
{
    void CameraSystem::Update(float dt)
    {
        (void)dt;
        if (aEntities.size() == 0) return;

        if (!pCoordinator->IsActiveInHierarchy(aEntities[0]))
            return;

        auto& pArray = pCoordinator->GetComponentArray<Player>();
        auto& tfArray = pCoordinator->GetComponentArray<Transform>();
        auto& camArray = pCoordinator->GetComponentArray<Camera>();

        Entity camera = aEntities[0];
        auto& cam_tf = tfArray.GetData(camera);
        auto& cam_c = camArray.GetData(camera);

        if (cam_c.followPlayer && pArray.Size() > 0)
        {
            Entity player = pArray.GetEntity(0);
            auto& player_tf = tfArray.GetData(player);
            cam_tf.position = player_tf.worldPosition;
        }

        // Pixel-perfect snap FIRST, on the base position
        float pixelSize = 1.0f / cam_c.mZoom;
        cam_tf.position.x = std::round(cam_tf.position.x / pixelSize) * pixelSize;
        cam_tf.position.y = std::round(cam_tf.position.y / pixelSize) * pixelSize;

        // Update shake timer
        if (cam_c.mShakeTimer > 0.0f)
        {
            cam_c.mShakeTimer -= dt;

            if (cam_c.mShakeTimer <= 0.0f)
            {
                cam_c.mShakeOffset = Vec2(0.0f, 0.0f);
                cam_c.mShakeIntensity = 0.0f;
            }
            else
            {
                float offsetX = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * cam_c.mShakeIntensity;
                float offsetY = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * cam_c.mShakeIntensity;
                cam_c.mShakeOffset = Vec2(offsetX, offsetY);
            }
        }

        // Apply shake AFTER snap � shake is intentionally sub-pixel
        cam_tf.position += cam_c.mShakeOffset;
    }
}



