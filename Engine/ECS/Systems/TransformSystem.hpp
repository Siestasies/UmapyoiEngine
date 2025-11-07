/*!
\file   TransformSystem.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines transform system for hierarchical world-space coordinate calculation.

Manages recursive update of world transforms for entities with parent-child relationships.
Computes world position, scale, and rotation by applying parent transforms to local transforms.
Root entities (no parent) use local transforms as world transforms. Handles proper rotation
application via trigonometric transformation matrices. Updates entire hierarchy from root to
leaves ensuring transform coherence across entity trees. Critical system for spatial queries,
rendering, and physics operations requiring world-space coordinates.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "Core/System.hpp"
#include "Core/Coordinator.hpp"

#include "Math/Math.h"

namespace Uma_ECS
{
    class TransformSystem : public ECSSystem
    {
    public:
        /**
         * \brief Initializes transform system with coordinator reference
         * \param c Pointer to ECS coordinator
         */
        void Init(Coordinator* c) { pCoordinator = c; }

        /**
         * \brief Updates all entity world transforms starting from root entities
         */
        void UpdateWorldTransform();

    private:
        /**
         * \brief Recursively updates entity hierarchy transforms from parent to children
         * \param entity Current entity to update
         * \param parentWorldPos Parent's world position
         * \param parentWorldScale Parent's world scale
         * \param parentWorldRot Parent's world rotation in radians
         */
        void UpdateHierarchyRecursive(
            Entity entity,
            const Vec2& parentWorldPos,
            const Vec2& parentWorldScale,
            float parentWorldRot);

        Coordinator* pCoordinator;
    };
}