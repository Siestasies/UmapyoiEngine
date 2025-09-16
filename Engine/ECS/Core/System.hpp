#pragma once

//#include "../Core/Coordinator.h"
#include "../Core/Types.hpp"
#include <set>

namespace Uma_ECS
{
    //class Coordinator;

    class ECSSystem
    {
    public:
        //virtual void Init(Coordinator& c) {
        //    mCoordinator = &c;
        //}

        //virtual void Update(float dt) = 0;

        std::set<Entity> aEntities;
        //Coordinator* mCoordinator = nullptr;

    };
}
