/*!
\file   CameraSystem.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

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

namespace Uma_ECS
{
    void CameraSystem::Update(float dt)
    {
        (void)dt;
        if (aEntities.size() == 0) return;

        auto& pArray = pCoordinator->GetComponentArray<Player>();
        auto& tfArray = pCoordinator->GetComponentArray<Transform>();
        auto& camArray = pCoordinator->GetComponentArray<Camera>();

        // one camera for now
        Entity camera = aEntities[0];
        auto& cam_tf = tfArray.GetData(camera);
        auto& cam_c = camArray.GetData(camera);

        if (cam_c.followPlayer)
        {
            Entity player = pArray.GetEntity(0);
            auto& player_tf = tfArray.GetData(player);

            cam_tf.position = player_tf.position;
        }
    }
}



