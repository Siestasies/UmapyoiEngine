#include "StateMachineSystem.hpp"
#include "../AI/StateMachine.hpp"

void Uma_ECS::StateMachineSystem::Update(float dt)
{
    auto& smArray = gCoordinator->GetComponentArray<FSM>();
    for (auto const& entity : aEntities)
    {
        auto& sm = smArray.GetData(entity);
        sm.Update(sm, dt);
    }
}

