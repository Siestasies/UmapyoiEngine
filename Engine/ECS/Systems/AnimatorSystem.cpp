#include "AnimatorSystem.hpp"
#include "Components/Animator.h"

namespace Uma_ECS
{
    void AnimatorSystem::Init(Coordinator* c)
    {
        pCoordinator = c;
    }

    void AnimatorSystem::Update(float dt)
    {
        if (!aEntities.size()) return;

        auto& animatorArray = pCoordinator->GetComponentArray<Animator>();

        for (const auto& entity : aEntities)
        {
            auto& animator = animatorArray.GetData(entity);

            // Update animation timer and advance frames
            animator.animator.Update(dt);

            // Calculate and store current frame UVs
            animator.animator.GetUVs(animator.uvOffset, animator.uvSize);
        }
    }
}