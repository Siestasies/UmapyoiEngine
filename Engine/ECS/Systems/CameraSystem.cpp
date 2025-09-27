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
        auto& pArray = pCoordinator->GetComponentArray<Player>();
        auto& tfArray = pCoordinator->GetComponentArray<Transform>();
        auto& camArray = pCoordinator->GetComponentArray<Camera>();

        // one camera for now
        Entity camera = camArray.GetEntity(0);
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



