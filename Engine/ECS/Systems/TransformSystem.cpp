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

            // Rotate local position by parent rotation
            float cosRot = std::cos(parentWorldRot);
            float sinRot = std::sin(parentWorldRot);

            Vec2 rotatedPos;
            rotatedPos.x = tf.position.x * cosRot - tf.position.y * sinRot;
            rotatedPos.y = tf.position.x * sinRot + tf.position.y * cosRot;

            // Scale by parent scale and add parent position
            tf.worldPosition.x = parentWorldPos.x + rotatedPos.x * parentWorldScale.x;
            tf.worldPosition.y = parentWorldPos.y + rotatedPos.y * parentWorldScale.y;

            // Combine scales
            tf.worldScale.x = tf.scale.x * parentWorldScale.x;
            tf.worldScale.y = tf.scale.y * parentWorldScale.y;

            // Add rotations
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