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

//void Uma_ECS::CollisionSystem::UpdateBoundingBoxes()
//{
//    if (aEntities.empty()) return;
//
//    // Get dense component arrays once
//    auto& cArray = gCoordinator->GetComponentArray<Collider>();
//    auto& tfArray = gCoordinator->GetComponentArray<Transform>();
//
//    for (auto const& entity : aEntities)
//    {
//        auto& c = cArray.GetData(entity);
//        auto& tf = tfArray.GetData(entity);
//
//        // Skip entities on NONE collision layer
//        if ((c.layer & CollisionLayer::CL_NONE)) continue;
//
//        // Create UNIT-SIZED local corners (±0.5, not scaled yet)
//        std::array<Vec2, 4> localCorners = {
//            Vec2{-0.5f, -0.5f},  // Bottom-left
//            Vec2{ 0.5f, -0.5f},  // Bottom-right
//            Vec2{ 0.5f,  0.5f},  // Top-right
//            Vec2{-0.5f,  0.5f}   // Top-left
//        };
//
//        // Build transformation matrix: T * R * S (in reverse order for post-multiply)
//        Mat3 transform = Uma_Constants::IDENTITY_3X3_F;
//        transform = Translate(transform, tf.prevPos);
//        transform = Rotate(transform, tf.rotation.x);
//        transform = Scale(transform, tf.scale);
//
//        // Transform corners into world space
//        std::array<Vec2, 4> worldCorners;
//        for (int i = 0; i < 4; i++)
//        {
//            // Create homogeneous coordinate (x, y, 1)
//            Vec3 localHomogeneous(localCorners[i].x, localCorners[i].y, 1.0f);
//
//            // Transform to world space
//            Vec3 worldHomogeneous = transform * localHomogeneous;
//
//            // Extract 2D world position
//            worldCorners[i] = Vec2(worldHomogeneous.x, worldHomogeneous.y);
//        }
//
//        // Compute AABB from transformed corners
//        Vec2 bbMin = worldCorners[0];
//        Vec2 bbMax = worldCorners[0];
//
//        for (int i = 1; i < 4; i++)
//        {
//            bbMin.x = min(bbMin.x, worldCorners[i].x);
//            bbMin.y = min(bbMin.y, worldCorners[i].y);
//            bbMax.x = max(bbMax.x, worldCorners[i].x);
//            bbMax.y = max(bbMax.y, worldCorners[i].y);
//        }
//
//        // Store final bounding box
//        c.boundingBox.min = bbMin;
//        c.boundingBox.max = bbMax;
//    }
//}

void Uma_ECS::CollisionSystem::UpdateBoundingBoxes()
{
    if (aEntities.empty()) return;

    // Get dense component arrays once
    auto& cArray = gCoordinator->GetComponentArray<Collider>();
    auto& tfArray = gCoordinator->GetComponentArray<Transform>();
    auto& sArray = gCoordinator->GetComponentArray<Sprite>();

    for (auto const& entity : aEntities)
    {
        auto& c = cArray.GetData(entity);
        auto& tf = tfArray.GetData(entity);
        //auto& s = sArray.GetData(entity);

        // Skip entities on NONE collision layer
        if ((c.layer & CollisionLayer::CL_NONE)) continue;

        Vec2 effectiveSize;

        if (c.autoFitToSprite && sArray.Has(entity) && sArray.GetData(entity).texture)
        {
            auto& s = sArray.GetData(entity);
            effectiveSize = s.texture->GetNativeSize();
        }
        else
        {
            effectiveSize = c.size;
        }

        Vec2 scaledSize = Vec2(
            effectiveSize.x * tf.scale.x,
            effectiveSize.y * tf.scale.y
        );

        // Calculate world position with offset
        Vec2 worldOffset = Vec2(
            c.offset.x * tf.scale.x,
            c.offset.y * tf.scale.y
        );
        Vec2 worldPosition = Vec2(tf.position.x, tf.position.y) + worldOffset;

        // Update runtime bounding box
        c.boundingBox.min = worldPosition - scaledSize * 0.5f;
        c.boundingBox.max = worldPosition + scaledSize * 0.5f;
    }
}

void Uma_ECS::CollisionSystem::UpdateCollision(float dt)
{
    if (!aEntities.size()) return;

    // Get dense component arrays once
    auto& cArray = gCoordinator->GetComponentArray<Collider>();
    auto& tfArray = gCoordinator->GetComponentArray<Transform>();
    auto& rbArray = gCoordinator->GetComponentArray<RigidBody>();

    // split entities into list of entities in grid (spatial hash buckets)
    // this idea is to prevent situation that we are checking collison of 
    // all entities with one another at one shot, this is way too expensive and
    // not practical

    // so we split the world into cells and only check collision of the entites
    // that are inside the same cell
    // Broadphase storage (spatial hash buckets)
    std::unordered_map<Cell, std::vector<Entity>, CellHash> grid;

    // Insert all colliders into spatial grid
    for (size_t i = 0; i < cArray.Size(); ++i) {
        Entity e = cArray.GetEntity(i);
        if (!tfArray.Has(e)) continue;

        auto& c = cArray.GetComponentAt(i);

        // skip none-layer
        if (c.layer == CollisionLayer::CL_NONE) continue;

        InsertIntoGrid(grid, e, c.boundingBox);
    }


    // Check collisions inside each cell
    for (auto& [cell, entities] : grid) {
        for (size_t i = 0; i < entities.size(); ++i) {
            for (size_t j = i + 1; j < entities.size(); ++j) {
                Entity a = entities[i];
                Entity b = entities[j];

                auto& ca = cArray.GetData(a);
                auto& cb = cArray.GetData(b);

                // Layer mask filtering
                if (!(ca.colliderMask & cb.layer)) continue;
                if (!(cb.colliderMask & ca.layer)) continue;

                auto& rba = rbArray.GetData(a);
                auto& rbb = rbArray.GetData(b);

                float firstTimeCollide = 0.f;

                // Narrowphase AABB test
                if (CollisionIntersection_RectRect(ca.boundingBox, rba.velocity, cb.boundingBox, rbb.velocity, firstTimeCollide, dt)) 
                {
                    auto& tfa = tfArray.GetData(a);
                    auto& tfb = tfArray.GetData(b);

                    // handle collision
                    ResolveAABBCollision(tfa, ca, tfb, cb);

                    // shd dispatch event system for this
                }
            }
        }
    }
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

void Uma_ECS::CollisionSystem::ResolveAABBCollision(Transform& lhsTransform, Collider& lhsCollider,
    Transform& rhsTransform, Collider& rhsCollider)
{
    // Use the already-calculated bounding boxes from the colliders
    const BoundingBox& lhsBox = lhsCollider.boundingBox;
    const BoundingBox& rhsBox = rhsCollider.boundingBox;

    // Calculate the centers and half-extents
    Vec2 lhsCenter = Vec2(
        (lhsBox.min.x + lhsBox.max.x) * 0.5f,
        (lhsBox.min.y + lhsBox.max.y) * 0.5f
    );
    Vec2 rhsCenter = Vec2(
        (rhsBox.min.x + rhsBox.max.x) * 0.5f,
        (rhsBox.min.y + rhsBox.max.y) * 0.5f
    );

    Vec2 lhsHalfExtents = Vec2(
        (lhsBox.max.x - lhsBox.min.x) * 0.5f,
        (lhsBox.max.y - lhsBox.min.y) * 0.5f
    );
    Vec2 rhsHalfExtents = Vec2(
        (rhsBox.max.x - rhsBox.min.x) * 0.5f,
        (rhsBox.max.y - rhsBox.min.y) * 0.5f
    );

    // Calculate overlap on both axes
    float overlapX = (lhsHalfExtents.x + rhsHalfExtents.x) - std::abs(lhsCenter.x - rhsCenter.x);
    float overlapY = (lhsHalfExtents.y + rhsHalfExtents.y) - std::abs(lhsCenter.y - rhsCenter.y);

    // Only resolve if actually overlapping
    if (overlapX > 0 && overlapY > 0) {
        // Resolve along the axis with the smallest overlap (minimum translation vector)
        if (overlapX < overlapY) {
            // Resolve along X axis
            if (lhsCenter.x < rhsCenter.x) {
                lhsTransform.position.x -= overlapX / 2;
                rhsTransform.position.x += overlapX / 2;
            }
            else {
                lhsTransform.position.x += overlapX / 2;
                rhsTransform.position.x -= overlapX / 2;
            }
        }
        else {
            // Resolve along Y axis
            if (lhsCenter.y < rhsCenter.y) {
                lhsTransform.position.y -= overlapY / 2;
                rhsTransform.position.y += overlapY / 2;
            }
            else {
                lhsTransform.position.y += overlapY / 2;
                rhsTransform.position.y -= overlapY / 2;
            }
        }
    }
}
