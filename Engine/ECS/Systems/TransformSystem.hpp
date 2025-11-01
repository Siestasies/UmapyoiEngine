#pragma once

#include "Core/System.hpp"
#include "Core/Coordinator.hpp"

#include "Math/Math.h"

namespace Uma_ECS
{
    class TransformSystem : public ECSSystem
    {
    public:
        void Init(Coordinator* c) { pCoordinator = c; }

        void UpdateWorldTransform();

    private:
        void UpdateHierarchyRecursive(
            Entity entity, 
            const Vec2& parentWorldPos, 
            const Vec2& parentWorldScale, 
            float parentWorldRot);

        Coordinator* pCoordinator;
    };
}