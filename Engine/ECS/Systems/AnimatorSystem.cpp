/*!
\file    AnimatorSystem.cpp
\par     Project: GAM200
\par     Course: CSD2401
\par     Section A
\par     Software Engineering Project 3

\author Javier Chua Dong Qing (100%)
\par     E-mail: javierdongqing.chua@digipen.edu
\par     DigiPen login: javierdongqing.chua

\brief
Implements the AnimatorSystem which iterates over all Animator
components to advance their animation frames based on delta time

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "AnimatorSystem.hpp"
#include "Components/Animator.h"

namespace Uma_ECS
{
    void AnimatorSystem::Init(Coordinator* c, Uma_Engine::ResourcesManager* rm)
    {
        pCoordinator = c;
        pResourcesManager = rm;
    }

    void AnimatorSystem::Update(float dt)
    {
        if (!aEntities.size()) return;

        auto& animatorArray = pCoordinator->GetComponentArray<Animator>();

        for (const auto& entity : aEntities)
        {
            if (!pCoordinator->IsActiveInHierarchy(entity))
                continue;

            auto& animator = animatorArray.GetData(entity);

            // Only play the initial clip if haven't initialized this beofre
            if (!animator.isInitialized)
            {
                if (animator.autoPlay && !animator.initialClip.empty())
                {
                    animator.animator.Play(animator.initialClip);
                }
                animator.isInitialized = true;
            }

            // Update animation timer and advance frames
            animator.animator.Update(dt);

            // Calculate and store current frame UVs
            animator.animator.GetUVs(animator.uvOffset, animator.uvSize);

            // Resolve active texture for current clip
            animator.activeTexture = nullptr;
            const auto& clips = animator.animator.GetClips();
            const std::string& currentClip = animator.animator.GetCurrentClip();

            if (!clips.empty() && clips.find(currentClip) != clips.end())
            {
                const auto& clipData = clips.at(currentClip);
                if (!clipData.texturePath.empty() && pResourcesManager)
                {
                    auto clipTexture = pResourcesManager->GetTexture(clipData.texturePath);
                    if (clipTexture && clipTexture->tex_id != 0)
                    {
                        animator.activeTexture = clipTexture;
                    }
                }
            }
        }
    }
}