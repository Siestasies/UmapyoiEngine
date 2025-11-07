/*!
\file    AnimatorSystem.hpp
\par     Project: GAM200
\par     Course: CSD2401
\par     Section A
\par     Software Engineering Project 3

\author Javier Chua Dong Qing (100%)
\par     E-mail: javierdongqing.chua@digipen.edu
\par     DigiPen login: javierdongqing.chua

\brief
Defines the AnimatorSystem, an ECS System responsible for updating
Animator components each frame

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once
/*!
\file   AudioSystem.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Koh Kai Yang (100%)
\par    E-mail: k.kaiyang@digipen.edu
\par    DigiPen login: k.kaiyang

\brief
Implementation of the procedural map generation system that creates dynamic game environments.
This file contains the core algorithms for procedural dungeon generation, including room placement,
corridor connection, random walk generation for unique room shapes, and specialised map generation
for different node types. The implementation handles tile rendering with varied appearances based on node type,
efficient culling for visible tiles, collision detection between entities and map elements,
animation of special tiles like lava, and integration with the game's broader progression system.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"

namespace Uma_ECS
{
    /**
     * \class AnimatorSystem
     * \brief Updates all Animator components in the ECS
     *
     * This system iterates over entities with an Animator component,
     * calls the Update method on their internal SpriteAnimator,
     * and caches the resulting UV coordinates back into the component
     * for the RenderSystem to use
     */
    class AnimatorSystem : public ECSSystem
    {
    public:
        /**
         * \brief Initializes the system with a pointer to the ECS coordinator
         * \param c Pointer to the main Coordinator
         */
        void Init(Coordinator* c);
        /**
         * \brief Updates all registered Animator components
         * \param dt Delta time
         */
        void Update(float dt);

    private:
        Coordinator* pCoordinator = nullptr;
    };
}