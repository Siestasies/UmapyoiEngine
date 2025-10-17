/*!
\file   CollisionSystem.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Implements spatial hash-based collision detection using grid partitioning for broadphase optimization.

Updates axis-aligned bounding boxes from transform and collider data, then performs narrowphase AABB tests
using both static and dynamic (continuous) collision detection with swept volume calculations.
Partitions world space into cells (CELL_SIZE) to reduce collision checks from O(n^2) to O(n) average case.
Resolves overlaps by separating objects along minimum penetration axis. Supports layer-based collision filtering
through bitmask comparison. Calculates time of first impact for moving AABBs using relative velocity projection.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "CollisionSystem.hpp"

#include "../Core/Coordinator.hpp"
#include "../Components/Collider.h"
#include "../Components/Transform.h"
#include "../Components/RigidBody.h"
#include "../Components/Sprite.h"

#include <iostream>
#include <iomanip>
#include <unordered_map>

void Uma_ECS::CollisionSystem::Update(float dt)
{
    UpdateBoundingBoxes();
    
    UpdateCollision(dt);
}

void Uma_ECS::CollisionSystem::UpdateBoundingBoxes()
{
    if (aEntities.empty()) return;

    auto& cArray = gCoordinator->GetComponentArray<Collider>();
    auto& tfArray = gCoordinator->GetComponentArray<Transform>();
    auto& sArray = gCoordinator->GetComponentArray<Sprite>();

    for (auto const& entity : aEntities)
    {
        auto& c = cArray.GetData(entity);
        auto& tf = tfArray.GetData(entity);

        // IMPORTANT: Clear bounds first, then rebuild
        c.bounds.clear();  // ADD THIS LINE

        Vec2 spriteSize{ 1.0f, 1.0f };
        if (sArray.Has(entity))
        {
            auto& s = sArray.GetData(entity);
            if (s.texture)
            {
                spriteSize = s.texture->GetNativeSize();
            }
        }

        for (const auto& colliderShape : c.shapes)
        {
            if (!colliderShape.isActive)
            {
                c.bounds.push_back(BoundingBox{});
                continue;
            }

            Vec2 effectiveSize;
            if (colliderShape.autoFitToSprite && sArray.Has(entity) && sArray.GetData(entity).texture)
            {
                auto& s = sArray.GetData(entity);
                effectiveSize = s.texture->GetNativeSize();
            }
            else
            {
                effectiveSize = colliderShape.size;
            }

            Vec2 scaledSize = Vec2(
                effectiveSize.x * tf.scale.x,
                effectiveSize.y * tf.scale.y
            );

            Vec2 worldOffset = Vec2(
                colliderShape.offset.x * tf.scale.x,
                colliderShape.offset.y * tf.scale.y
            );
            Vec2 worldPosition = Vec2(tf.position.x, tf.position.y) + worldOffset;

            BoundingBox newBound;
            newBound.min = worldPosition - scaledSize * 0.5f;
            newBound.max = worldPosition + scaledSize * 0.5f;
            c.bounds.push_back(newBound);
        }
    }
}

void Uma_ECS::CollisionSystem::UpdateCollision(float dt)
{
    if (aEntities.empty()) return;

    auto& cArray = gCoordinator->GetComponentArray<Collider>();
    auto& tfArray = gCoordinator->GetComponentArray<Transform>();
    auto& rbArray = gCoordinator->GetComponentArray<RigidBody>();

    // Build spatial grid
    std::unordered_map<Cell, std::vector<Entity>, CellHash> grid;

    for (const auto& entity : aEntities)
    {
        auto& c = cArray.GetData(entity);
        if (!c.shapes.empty() && c.shapes[0].isActive)
        {
            InsertIntoGrid(grid, entity, c.bounds[0]);
        }
    }

    // Check collisions within each cell
    for (auto const& [cell, entities] : grid)
    {
        // Check all pairs in this cell
        for (size_t i = 0; i < entities.size(); ++i)
        {
            for (size_t j = i + 1; j < entities.size(); ++j)
            {
                Entity e1 = entities[i];
                Entity e2 = entities[j];

                auto& tf1 = tfArray.GetData(e1);
                auto& tf2 = tfArray.GetData(e2);
                auto& c1 = cArray.GetData(e1);
                auto& c2 = cArray.GetData(e2);

                // Check if entities have RigidBody
                bool e1HasRb = rbArray.Has(e1);
                bool e2HasRb = rbArray.Has(e2);

                // Skip if no rigidbodies (both static)
                if (!rbArray.Has(e1) && !rbArray.Has(e2))
                    continue;

                // Broad phase: check primary bounds first
                if (!CollisionIntersection_RectRect_Static(c1.bounds[0], c2.bounds[0]))
                    continue;

                RigidBody* rb1 = e1HasRb ? &rbArray.GetData(e1) : nullptr;
                RigidBody* rb2 = e2HasRb ? &rbArray.GetData(e2) : nullptr;

                // Narrow phase: check all shape pairs
                for (size_t si = 0; si < c1.shapes.size(); ++si)
                {
                    const auto& shape1 = c1.shapes[si];
                    if (!shape1.isActive) continue;

                    for (size_t sj = 0; sj < c2.shapes.size(); ++sj)
                    {
                        const auto& shape2 = c2.shapes[sj];
                        if (!shape2.isActive) continue;

                        // Layer filtering
                        LayerMask layer1 = c1.GetEffectiveLayer(si);
                        LayerMask mask1 = c1.GetEffectiveMask(si);
                        LayerMask layer2 = c2.GetEffectiveLayer(sj);
                        LayerMask mask2 = c2.GetEffectiveMask(sj);

                        if (!((layer1 & mask2) && (mask1 & layer2)))
                            continue;

                        if (!ShouldShapesCollide(shape1.purpose, shape2.purpose))
                            continue;

                        // Collision test
                        float firstTimeOfCollision;
                        if (CollisionIntersection_RectRect(
                            c1.bounds[si], rb1->velocity,
                            c2.bounds[sj], rb2->velocity,
                            firstTimeOfCollision, dt))
                        {
                            // Resolve using your existing method
                           // Handle collision with physics and purpose awareness
                            HandleShapeCollision(
                                e1, e2,
                                tf1, tf2,
                                rb1, rb2,
                                c1.bounds[si], c2.bounds[sj],
                                shape1.purpose, shape2.purpose
                            );
                        }
                    }
                }
            }
        }
    }
}

bool Uma_ECS::CollisionSystem::ShouldShapesCollide(
    const ColliderPurpose& shape1Purpose,
    const ColliderPurpose& shape2Purpose)
{
    // Triggers always detect but never block
    if (shape1Purpose == ColliderPurpose::Trigger || shape2Purpose == ColliderPurpose::Trigger)
        return true;

    // Physics collides with Physics and Environment
    if (shape1Purpose == ColliderPurpose::Physics)
        return (shape2Purpose == ColliderPurpose::Physics || shape2Purpose == ColliderPurpose::Environment);

    if (shape2Purpose == ColliderPurpose::Physics)
        return (shape1Purpose == ColliderPurpose::Physics || shape1Purpose == ColliderPurpose::Environment);

    // Environment doesn't collide with Environment
    return false;
}

void Uma_ECS::CollisionSystem::HandleShapeCollision(
    Entity e1, Entity e2,
    Transform& tf1, Transform& tf2,
    RigidBody* rb1, RigidBody* rb2,
    const BoundingBox& box1, const BoundingBox& box2,
    ColliderPurpose purpose1, ColliderPurpose purpose2)
{
    // Handle triggers (no physics resolution)
    if (purpose1 == ColliderPurpose::Trigger || purpose2 == ColliderPurpose::Trigger)
    {
        // TODO: Emit trigger event when you add event system
        // Example: pEventSystem->Emit<TriggerEnterEvent>(e1, e2);
        return; // No physics resolution for triggers
    }

    // Determine if entities can move
    bool e1CanMove = rb1 != nullptr;
    bool e2CanMove = rb2 != nullptr;

    // Determine if we should resolve based on purposes
    bool shouldResolve = false;

    if (purpose1 == ColliderPurpose::Physics && purpose2 == ColliderPurpose::Physics)
    {
        // Physics vs Physics - resolve if at least one can move
        shouldResolve = e1CanMove || e2CanMove;
    }
    else if (purpose1 == ColliderPurpose::Physics && purpose2 == ColliderPurpose::Environment)
    {
        // Physics vs Environment (wall) - resolve if physics object can move
        shouldResolve = e1CanMove;
        e2CanMove = false; // wall
    }
    else if (purpose1 == ColliderPurpose::Environment && purpose2 == ColliderPurpose::Physics)
    {
        // Environment vs Physics - resolve if physics object can move
        shouldResolve = e2CanMove;
        e1CanMove = false; // wall
    }

    if (!shouldResolve)
        return;

    // Resolve position overlap
    ResolveAABBCollision(tf1, tf2, box1, box2, e1CanMove, e2CanMove);

    // Apply velocity changes based on collision type
    if (e1CanMove && e2CanMove)
    {
        // Both can move - apply bounce physics
        Vec2 relativeVel = rb1->velocity - rb2->velocity;
        Vec2 normal = GetCollisionNormal(box1, box2);

        float restitution = 0.3f; // Bounciness (0 = no bounce, 1 = perfect bounce)
        float impulse = -(1.0f + restitution) * dot(relativeVel, normal);
        impulse /= 2.0f; // Assuming equal mass

        rb1->velocity += normal * impulse;
        rb2->velocity -= normal * impulse;
    }
    else if (e1CanMove)
    {
        // Only e1 can move - stop sliding into wall
        Vec2 normal = GetCollisionNormal(box1, box2);
        float velAlongNormal = dot(rb1->velocity, normal);

        if (velAlongNormal < 0) // Moving into the wall
        {
            rb1->velocity -= normal * velAlongNormal;
        }
    }
    else if (e2CanMove)
    {
        // Only e2 can move - stop sliding into wall
        Vec2 normal = GetCollisionNormal(box2, box1);
        float velAlongNormal = dot(rb2->velocity, normal);

        if (velAlongNormal < 0) // Moving into the wall
        {
            rb2->velocity -= normal * velAlongNormal;
        }
    }
}

Vec2 Uma_ECS::CollisionSystem::GetCollisionNormal(
    const BoundingBox& box1,
    const BoundingBox& box2)
{
    Vec2 center1 = (box1.min + box1.max) * 0.5f;
    Vec2 center2 = (box2.min + box2.max) * 0.5f;
    Vec2 delta = center1 - center2;

    // Normalize
    float length = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    if (length > 0.0001f)
    {
        delta.x /= length;
        delta.y /= length;
    }
    else
    {
        // Collision at exact same position, default to up
        delta = Vec2(0, 1);
    }

    return delta;
}

void Uma_ECS::CollisionSystem::ResolveAABBCollision(
    Transform& tf1, Transform& tf2,
    const BoundingBox& box1, const BoundingBox& box2,
    bool e1CanMove, bool e2CanMove)
{
    // Calculate centers and half-extents
    Vec2 center1 = (box1.min + box1.max) * 0.5f;
    Vec2 center2 = (box2.min + box2.max) * 0.5f;
    Vec2 halfSize1 = (box1.max - box1.min) * 0.5f;
    Vec2 halfSize2 = (box2.max - box2.min) * 0.5f;

    // Calculate overlap on both axes
    Vec2 delta = center1 - center2;
    Vec2 overlap = Vec2{
        halfSize1.x + halfSize2.x - std::abs(delta.x),
        halfSize1.y + halfSize2.y - std::abs(delta.y)
    };

    // Only resolve if actually overlapping
    if (overlap.x <= 0 || overlap.y <= 0)
        return;

    // Find minimum penetration axis (MTV - Minimum Translation Vector)
    Vec2 separation{ 0, 0 };
    if (overlap.x < overlap.y)
    {
        // Separate on X axis
        separation.x = (delta.x > 0) ? overlap.x : -overlap.x;
    }
    else
    {
        // Separate on Y axis
        separation.y = (delta.y > 0) ? overlap.y : -overlap.y;
    }

    // Apply separation based on movement capability
    if (e1CanMove && e2CanMove)
    {
        // Both can move - split separation 50/50
        tf1.position += separation * 0.5f;
        tf2.position -= separation * 0.5f;
    }
    else if (e1CanMove)
    {
        // Only e1 moves (e2 is wall/static)
        tf1.position += separation;
    }
    else if (e2CanMove)
    {
        // Only e2 moves (e1 is wall/static)
        tf2.position -= separation;
    }
    // If both can't move, do nothing (shouldn't happen due to earlier check)
}

void Uma_ECS::CollisionSystem::InsertIntoGrid(std::unordered_map<Cell, std::vector<Entity>, CellHash>& grid, Entity e, const BoundingBox& box)
{
    int minX = WorldToCell(box.min.x);
    int minY = WorldToCell(box.min.y);
    int maxX = WorldToCell(box.max.x);
    int maxY = WorldToCell(box.max.y);

    for (int x = minX; x <= maxX; ++x) {
        for (int y = minY; y <= maxY; ++y) {
            grid[{x, y}].push_back(e);
        }
    }
}

bool Uma_ECS::CollisionSystem::CollisionIntersection_RectRect_Static(const BoundingBox& lhs, const BoundingBox& rhs)
{
    // Assume collision is true initially
    bool isCollided = true;

    // Check for non-overlapping cases on the x and y axes
    if (lhs.max.x < rhs.min.x) // AABB 1 is completely left of AABB 2
    {
        isCollided = false;
    }
    if (lhs.min.x > rhs.max.x) // AABB 1 is completely right of AABB 2
    {
        isCollided = false;
    }
    if (lhs.max.y < rhs.min.y) // AABB 1 is completely below AABB 2
    {
        isCollided = false;
    }
    if (lhs.min.y > rhs.max.y) // AABB 1 is completely above AABB 2
    {
        isCollided = false;
    }

    return isCollided;
}

bool Uma_ECS::CollisionSystem::CollisionIntersection_RectRect_Dynamic(const BoundingBox& lhs, const Vec2& vel1, const BoundingBox& rhs, const Vec2& vel2, float& firstTimeOfCollision,
    float dt)
{
    float g_dt = dt;

    // Calculate relative velocity between the two AABBs
    Vec2 relVel;
    relVel.x = vel2.x - vel1.x;
    relVel.y = vel2.y - vel1.y;

    // Initialize collision time range
    float tFirst = 0;
    float tLast = g_dt;

    // ===========================
    // X-Axis Collision Check
    // ===========================
    if (relVel.x < 0.f) // AABB 2 moving left relative to AABB 1
    {
        if (lhs.min.x > rhs.max.x) // No collision, moving away
        {
            return false;
        }
        if (lhs.max.x < rhs.min.x) // Possible first contact
        {
            tFirst = max((lhs.max.x - rhs.min.x) / relVel.x, tFirst);
        }
        if (lhs.min.x < rhs.max.x)
        {
            tLast = min((lhs.min.x - rhs.max.x) / relVel.x, tLast);
        }
    }
    else if (relVel.x > 0.f) // AABB 2 moving right relative to AABB 1
    {
        if (lhs.min.x > rhs.max.x) // Moving towards AABB 1
        {
            float firstDist = lhs.min.x - rhs.max.x;
            tFirst = max(firstDist / relVel.x, tFirst);
        }
        if (lhs.max.x > rhs.min.x) // Moving towards AABB 1
        {
            float lastDist = lhs.max.x - rhs.min.x;
            tLast = min(lastDist / relVel.x, tLast);
        }
        if (lhs.max.x < rhs.min.x) // No collision, moving away
        {
            return false;
        }
    }
    else // Objects moving parallel (no relative motion on x-axis)
    {
        if (lhs.max.x < rhs.min.x || lhs.min.x > rhs.max.x)
        {
            return false;
        }
    }

    // Check if there is no valid collision time range
    if (tFirst > tLast)
    {
        firstTimeOfCollision = tFirst;
        return false;
    }

    // ===========================
    // Y-Axis Collision Check
    // ===========================
    if (relVel.y < 0.f) // AABB 2 moving down relative to AABB 1
    {
        if (lhs.min.y > rhs.max.y) // No collision, moving away
        {
            return false;
        }
        if (lhs.max.y < rhs.min.y) // Possible first contact
        {
            tFirst = max((lhs.max.y - rhs.min.y) / relVel.y, tFirst);
        }
        if (lhs.min.y < rhs.max.y)
        {
            tLast = min((lhs.min.y - rhs.max.y) / relVel.y, tLast);
        }
    }
    else if (relVel.y > 0.f) // AABB 2 moving up relative to AABB 1
    {
        if (lhs.min.y > rhs.max.y) // Moving towards AABB 1
        {
            float firstDist = lhs.min.y - rhs.max.y;
            tFirst = max(firstDist / relVel.y, tFirst);
        }
        if (lhs.max.y > rhs.min.y) // Moving towards AABB 1
        {
            float lastDist = lhs.max.y - rhs.min.y;
            tLast = min(lastDist / relVel.y, tLast);
        }
        if (lhs.max.y < rhs.min.y) // No collision, moving away
        {
            return false;
        }
    }
    else // Objects moving parallel (no relative motion on y-axis)
    {
        if (lhs.max.y < rhs.min.y || lhs.min.y > rhs.max.y)
        {
            return false;
        }
    }

    // Final collision check
    if (tFirst > tLast)
    {
        firstTimeOfCollision = tFirst;
        return false;
    }

    firstTimeOfCollision = tFirst;
    return true;
}

// Change signature to accept specific bounding boxes instead of whole colliders
//void Uma_ECS::CollisionSystem::ResolveAABBCollision(
//    Transform& lhsTransform,
//    Transform& rhsTransform,
//    const BoundingBox& lhsBox,
//    const BoundingBox& rhsBox)
//{
//    // Calculate the centers and half-extents of the SPECIFIC shapes
//    Vec2 lhsCenter = Vec2(
//        (lhsBox.min.x + lhsBox.max.x) * 0.5f,
//        (lhsBox.min.y + lhsBox.max.y) * 0.5f
//    );
//    Vec2 rhsCenter = Vec2(
//        (rhsBox.min.x + rhsBox.max.x) * 0.5f,
//        (rhsBox.min.y + rhsBox.max.y) * 0.5f
//    );
//
//    Vec2 lhsHalfExtents = Vec2(
//        (lhsBox.max.x - lhsBox.min.x) * 0.5f,
//        (lhsBox.max.y - lhsBox.min.y) * 0.5f
//    );
//    Vec2 rhsHalfExtents = Vec2(
//        (rhsBox.max.x - rhsBox.min.x) * 0.5f,
//        (rhsBox.max.y - rhsBox.min.y) * 0.5f
//    );
//
//    // Calculate overlap on both axes
//    float overlapX = (lhsHalfExtents.x + rhsHalfExtents.x) - std::abs(lhsCenter.x - rhsCenter.x);
//    float overlapY = (lhsHalfExtents.y + rhsHalfExtents.y) - std::abs(lhsCenter.y - rhsCenter.y);
//
//    // Only resolve if actually overlapping
//    if (overlapX > 0 && overlapY > 0) {
//        // Resolve along the axis with the smallest overlap (minimum translation vector)
//        if (overlapX < overlapY) {
//            // Resolve along X axis
//            if (lhsCenter.x < rhsCenter.x) {
//                lhsTransform.position.x -= overlapX / 2;
//                rhsTransform.position.x += overlapX / 2;
//            }
//            else {
//                lhsTransform.position.x += overlapX / 2;
//                rhsTransform.position.x -= overlapX / 2;
//            }
//        }
//        else {
//            // Resolve along Y axis
//            if (lhsCenter.y < rhsCenter.y) {
//                lhsTransform.position.y -= overlapY / 2;
//                rhsTransform.position.y += overlapY / 2;
//            }
//            else {
//                lhsTransform.position.y += overlapY / 2;
//                rhsTransform.position.y -= overlapY / 2;
//            }
//        }
//    }
//}