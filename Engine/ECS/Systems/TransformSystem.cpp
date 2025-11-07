/*!
\file   TransformSystem.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Implements hierarchical transform system for world-space coordinate propagation.

UpdateWorldTransform identifies root entities and triggers recursive updates.
UpdateHierarchyRecursive applies parent transforms to children using rotation matrices,
position offsets, and scale multiplication. Root entities copy local to world transforms.
Child entities compute rotated position via cos/sin transformation, then apply parent scale
and position. Recursively processes entire entity hierarchy ensuring proper transform
inheritance through arbitrary tree depths. Marks transforms as clean after update.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "TransformSystem.hpp"

#include "Components/Transform.h"

namespace Uma_ECS
{
    void TransformSystem::UpdateWorldTransform()
    {
        auto& tfArray = pCoordinator->GetComponentArray<Transform>();

        // finds all the root entities (entities with no parent)
        std::vector<Entity> rootEntities;
        for (size_t i = 0; i < tfArray.Size(); i++)
        {
            Entity entity = tfArray.GetEntity(i);
            auto& tf = tfArray.GetComponentAt(i);

            if (!tf.parent.has_value())
            {
                rootEntities.push_back(entity);
            }
        }

        // Update each root and its children recursively
        for (Entity root : rootEntities)
        {
            UpdateHierarchyRecursive(root, Vec2{ 0, 0 }, Vec2{ 1, 1 }, 0.0f);
        }
    }

    void TransformSystem::UpdateHierarchyRecursive(Entity entity, const Vec2& parentWorldPos, const Vec2& parentWorldScale, float parentWorldRot)
    {
        auto& tfArray = pCoordinator->GetComponentArray<Transform>();
        auto& tf = tfArray.GetData(entity);

        // check if it has a parent
        if (!tf.parent.has_value())
        {
            // Root entity - local == world
            tf.worldPosition = tf.position;
            tf.worldScale = tf.scale;
            tf.worldRotation = tf.rotation.x;
        }
        else
        {
            // Child entity - apply parent transforms

            // ADD DEBUG LOGGING HERE
            /*std::cout << "Entity " << entity << " (child):\n";
            std::cout << "  Local pos: (" << tf.position.x << ", " << tf.position.y << ")\n";
            std::cout << "  Parent world pos: (" << parentWorldPos.x << ", " << parentWorldPos.y << ")\n";
            std::cout << "  Parent world scale: (" << parentWorldScale.x << ", " << parentWorldScale.y << ")\n";*/

            float cosRot = std::cos(parentWorldRot);
            float sinRot = std::sin(parentWorldRot);

            Vec2 rotatedPos;
            rotatedPos.x = tf.position.x * cosRot - tf.position.y * sinRot;
            rotatedPos.y = tf.position.x * sinRot + tf.position.y * cosRot;

            //std::cout << "  Rotated pos: (" << rotatedPos.x << ", " << rotatedPos.y << ")\n";

            tf.worldPosition.x = parentWorldPos.x + rotatedPos.x * parentWorldScale.x;
            tf.worldPosition.y = parentWorldPos.y + rotatedPos.y * parentWorldScale.y;

            //std::cout << "  Final world pos: (" << tf.worldPosition.x << ", " << tf.worldPosition.y << ")\n";

            tf.worldScale.x = tf.scale.x * parentWorldScale.x;
            tf.worldScale.y = tf.scale.y * parentWorldScale.y;
            tf.worldRotation = parentWorldRot + tf.rotation.x;
        }

        tf.isDirty = false;

        // Recursively update children
        for (Entity child : tf.children)
        {
            UpdateHierarchyRecursive(child, tf.worldPosition, tf.worldScale, tf.worldRotation);
        }
    }
}