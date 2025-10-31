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
Implements Unity-style collision detection and resolution using spatial hashing and contact normals.

Updates axis-aligned bounding boxes from transform and collider data, then performs collision tests
using spatial grid partitioning. Resolves collisions with velocity projection for smooth wall sliding.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "CollisionSystem.hpp"

#include "../Core/Coordinator.hpp"
#include "../Components/Collider.h"
#include "../Components/Transform.h"
#include "../Components/RigidBody.h"
#include "../Components/Sprite.h"

#include "Events/CollisionEvent.h"

#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <cmath>

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

        // Ensure bounds array matches shapes array
        if (c.bounds.size() != c.shapes.size())
        {
            c.bounds.resize(c.shapes.size());
        }

        // Get sprite size if available
        Vec2 spriteSize{ 1.0f, 1.0f };
        if (sArray.Has(entity))
        {
            auto& s = sArray.GetData(entity);
            if (s.texture)
            {
                spriteSize = s.texture->GetNativeSize();
            }
        }

        // Update each shape's bounding box
        for (size_t i = 0; i < c.shapes.size(); ++i)
        {
            const auto& shape = c.shapes[i];
            if (!shape.isActive) continue;

            // Determine actual size based on autoFitToSprite flag
            Vec2 effectiveSize;
            if (shape.autoFitToSprite)
            {
                effectiveSize = spriteSize;
            }
            else
            {
                effectiveSize = shape.size;
            }

            // Scale by transform
            Vec2 scaledSize = Vec2{
                effectiveSize.x * tf.scale.x,
                effectiveSize.y * tf.scale.y
            };

            // Calculate world position with offset
            Vec2 worldOffset = Vec2{
                shape.offset.x * tf.scale.x,
                shape.offset.y * tf.scale.y
            };

            Vec2 currentWorldPos = tf.position + worldOffset;
            Vec2 halfSize = scaledSize * 0.5f;

            // Simple, tight bounds
            c.bounds[i].min = Vec2{
                currentWorldPos.x - halfSize.x,
                currentWorldPos.y - halfSize.y
            };
            c.bounds[i].max = Vec2{
                currentWorldPos.x + halfSize.x,
                currentWorldPos.y + halfSize.y
            };
        }
    }
}

void Uma_ECS::CollisionSystem::UpdateCollision(float dt)
{
    if (aEntities.empty()) return;

    auto& tfArray = gCoordinator->GetComponentArray<Transform>();
    auto& cArray = gCoordinator->GetComponentArray<Collider>();
    auto& rbArray = gCoordinator->GetComponentArray<RigidBody>();

    previousCollisions = std::move(currentCollisions);
    currentCollisions.clear();

    // Build spatial grid for broad phase
    std::unordered_map<Cell, std::vector<Entity>, CellHash> grid;

    for (const auto& entity : aEntities)
    {
        auto& collider = cArray.GetData(entity);

        // Insert entity into grid based on ALL active shapes, not just shape[0]
        for (size_t i = 0; i < collider.shapes.size(); ++i)
        {
            if (collider.shapes[i].isActive)
            {
                InsertIntoGrid(grid, entity, collider.bounds[i]);
            }
        }
    }

    // Check collisions within each cell
    for (auto const& [cell, entities] : grid)
    {
        for (size_t i = 0; i < entities.size(); ++i)
        {
            for (size_t j = i + 1; j < entities.size(); ++j)
            {
                Entity e1 = entities[i];
                Entity e2 = entities[j];

                CheckEntityPairCollision(e1, e2, tfArray, cArray, rbArray, dt);
            }
        }
    }

    // update the collsion pair that has exited
    for (const auto& pair : previousCollisions)
    {
        // means has ended
        if (currentCollisions.find(pair) == currentCollisions.end())
        {
            // Collision ended - emit exit event
            pEventSystem->Emit<Uma_Engine::OnCollisionExitEvent>(
                pair.entityA, pair.entityB);
        }
    }
}

void Uma_ECS::CollisionSystem::CheckEntityPairCollision(
    Entity e1, Entity e2,
    ComponentArray<Transform>& tfArray,
    ComponentArray<Collider>& cArray,
    ComponentArray<RigidBody>& rbArray,
    float dt)
{

    auto& c1 = cArray.GetData(e1);
    auto& c2 = cArray.GetData(e2);

    // Validate shapes exist
    if (c1.shapes.empty() || c2.shapes.empty()) return;
    if (!c1.shapes[0].isActive || !c2.shapes[0].isActive) return;

    // Check if entities have RigidBody (dynamic vs static)
    bool e1HasRb = rbArray.Has(e1);
    bool e2HasRb = rbArray.Has(e2);

    // Skip if both static (optimization)
    if (!e1HasRb && !e2HasRb)
        return;

    // Broad phase: check primary bounds
    /*if (!CollisionIntersection_RectRect_Static(c1.bounds[0], c2.bounds[0]))
        return;*/

    // Get components
    auto& tf1 = tfArray.GetData(e1);
    auto& tf2 = tfArray.GetData(e2);

    RigidBody* rb1 = e1HasRb ? &rbArray.GetData(e1) : nullptr;
    RigidBody* rb2 = e2HasRb ? &rbArray.GetData(e2) : nullptr;

    // Narrow phase: check all shape pairs
    for (size_t i = 0; i < c1.shapes.size(); ++i)
    {
        const auto& shape1 = c1.shapes[i];
        if (!shape1.isActive) continue;

        for (size_t j = 0; j < c2.shapes.size(); ++j)
        {
            const auto& shape2 = c2.shapes[j];
            if (!shape2.isActive) continue;

            // Layer filtering
            LayerMask layer1 = c1.GetEffectiveLayer(i);
            LayerMask mask1 = c1.GetEffectiveMask(i);
            LayerMask layer2 = c2.GetEffectiveLayer(j);
            LayerMask mask2 = c2.GetEffectiveMask(j);

            // checking if these 2 mask shd collide with each other
            if (!((layer1 & mask2) && (mask1 & layer2)))
                continue;

            // Purpose filtering
            // checking whether these 2 purpose shd collide with each other
            if (!ShouldPurposesCollide(shape1.purpose, shape2.purpose))
                continue;

            // Collision test
            if (CollisionIntersection_RectRect_Static(c1.bounds[i], c2.bounds[j]))
            {
                // handle the collision
                HandleShapeCollision(
                    e1, e2,
                    tf1, tf2,
                    rb1, rb2,
                    c1.bounds[i], c2.bounds[j],
                    shape1.purpose, shape2.purpose
                );
            }
        }
    }
}

bool Uma_ECS::CollisionSystem::ShouldPurposesCollide(
    ColliderPurpose p1,
    ColliderPurpose p2)
{
    // Triggers always detect but never block
    if (p1 == ColliderPurpose::Trigger || p2 == ColliderPurpose::Trigger)
        return true;

    // Physics collides with Physics and Environment
    if (p1 == ColliderPurpose::Physics)
        return (p2 == ColliderPurpose::Physics || p2 == ColliderPurpose::Environment);

    if (p2 == ColliderPurpose::Physics)
        return (p1 == ColliderPurpose::Physics || p1 == ColliderPurpose::Environment);

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
    EntityPair pair(e1, e2);

    // Handle triggers (no physics resolution)
    if (purpose1 == ColliderPurpose::Trigger || purpose2 == ColliderPurpose::Trigger)
    {
        // TODO: Emit trigger event when event system is available
         // Check if this is a new trigger interaction
        bool wasColliding = previousCollisions.find(pair) != previousCollisions.end();
        currentCollisions.insert(pair);

        if (!wasColliding)
        {
            // New trigger - emit enter event
            pEventSystem->Emit<Uma_Engine::OnTriggerEnterEvent>(e1, e2);
        }
        else
        {
            // Ongoing trigger - emit stay event
            pEventSystem->Emit<Uma_Engine::OnTriggerEvent>(e1, e2);
        }

        return;
    }

    // track collision for physics colliders
    bool wasColliding = previousCollisions.find(pair) != previousCollisions.end();
    currentCollisions.insert(pair);

    // Emit appropriate collision event
    if (!wasColliding)
    {
        // New collision - emit enter event
        pEventSystem->Emit<Uma_Engine::OnCollisionEnterEvent>(e1, e2);
    }
    else
    {
        // Ongoing collision - emit stay event
        pEventSystem->Emit<Uma_Engine::OnCollisionEvent>(e1, e2);
    }

    // Determine if entities can move
    bool e1CanMove = rb1 != nullptr;
    bool e2CanMove = rb2 != nullptr;

    // Override: Environment purpose is ALWAYS static
    if (purpose1 == ColliderPurpose::Environment)
        e1CanMove = false;  // Force e1 to be static

    if (purpose2 == ColliderPurpose::Environment)
        e2CanMove = false;  // Force e2 to be static

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
    }
    else if (purpose1 == ColliderPurpose::Environment && purpose2 == ColliderPurpose::Physics)
    {
        // Environment vs Physics - resolve if physics object can move
        shouldResolve = e2CanMove;
    }

    if (shouldResolve)
    {
        ResolveAABBCollision(tf1, tf2, box1, box2, e1CanMove, e2CanMove, rb1, rb2);
    }
}

void Uma_ECS::CollisionSystem::ResolveAABBCollision(
    Transform& tf1, Transform& tf2,
    const BoundingBox& box1, const BoundingBox& box2,
    bool e1CanMove, bool e2CanMove,
    RigidBody* rb1, RigidBody* rb2)
{
    // Collision resolution constants
    const float POSITION_CORRECTION = 0.4f;  // 40% position correction per frame
    const float PENETRATION_SLOP = 0.01f;    // Allow small overlap to prevent jitter
    const float WALL_FRICTION = 0.98f;        // Sliding friction (98% retention)
    const float RESTITUTION = 0.0f;           // Bounciness (0 = no bounce for top-down)

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

    // Early exit if not overlapping
    if (overlap.x <= 0 || overlap.y <= 0)
        return;

    // ═══════════════════════════════════════════════════════════
    // Determine collision normal and penetration depth
    // ═══════════════════════════════════════════════════════════
    Vec2 normal{ 0, 0 };
    float penetration = 0;

    if (overlap.x < overlap.y)
    {
        // Collision on X axis (vertical wall)
        normal.x = (delta.x > 0) ? 1.0f : -1.0f;
        normal.y = 0;
        penetration = overlap.x;
    }
    else
    {
        // Collision on Y axis (horizontal wall)
        normal.x = 0;
        normal.y = (delta.y > 0) ? 1.0f : -1.0f;
        penetration = overlap.y;
    }

    // ═══════════════════════════════════════════════════════════
    // Case 1: Both entities can move (dynamic vs dynamic)
    // ═══════════════════════════════════════════════════════════
    if (e1CanMove && e2CanMove)
    {
        // Apply position correction with slop
        float correctionAmount = max(penetration - PENETRATION_SLOP, 0.0f) * POSITION_CORRECTION;
        tf1.position += normal * (correctionAmount * 0.5f);
        tf2.position -= normal * (correctionAmount * 0.5f);

        if (rb1 && rb2)
        {
            // Calculate relative velocity
            Vec2 relVel = rb1->velocity - rb2->velocity;
            float velAlongNormal = relVel.x * normal.x + relVel.y * normal.y;

            // Only resolve if moving towards each other
            if (velAlongNormal < 0)
            {
                // Calculate impulse (simple equal mass assumption)
                float impulse = -(1.0f + RESTITUTION) * velAlongNormal * 0.5f;

                // Apply impulse to both objects
                rb1->velocity.x += normal.x * impulse;
                rb1->velocity.y += normal.y * impulse;
                rb2->velocity.x -= normal.x * impulse;
                rb2->velocity.y -= normal.y * impulse;

                // Apply friction to tangential velocity
                Vec2 tangent{ -normal.y, normal.x };
                float tangentVel1 = rb1->velocity.x * tangent.x + rb1->velocity.y * tangent.y;
                float tangentVel2 = rb2->velocity.x * tangent.x + rb2->velocity.y * tangent.y;

                rb1->velocity.x = normal.x * (rb1->velocity.x * normal.x + rb1->velocity.y * normal.y) +
                    tangent.x * tangentVel1 * WALL_FRICTION;
                rb1->velocity.y = normal.y * (rb1->velocity.x * normal.x + rb1->velocity.y * normal.y) +
                    tangent.y * tangentVel1 * WALL_FRICTION;

                rb2->velocity.x = normal.x * (rb2->velocity.x * normal.x + rb2->velocity.y * normal.y) +
                    tangent.x * tangentVel2 * WALL_FRICTION;
                rb2->velocity.y = normal.y * (rb2->velocity.x * normal.x + rb2->velocity.y * normal.y) +
                    tangent.y * tangentVel2 * WALL_FRICTION;
            }
        }
    }
    // ═══════════════════════════════════════════════════════════
    // Case 2: Entity 1 can move, Entity 2 is static (vs wall/environment)
    // ═══════════════════════════════════════════════════════════
    else if (e1CanMove && rb1)
    {
        // Apply gradual position correction
        float correctionAmount = max(penetration - PENETRATION_SLOP, 0.0f) * POSITION_CORRECTION;
        tf1.position += normal * correctionAmount;

        // Calculate velocity along collision normal
        float velAlongNormal = rb1->velocity.x * normal.x + rb1->velocity.y * normal.y;

        // Only resolve if moving INTO the wall
        if (velAlongNormal < 0)
        {
            // Remove velocity component going into wall
            rb1->velocity.x -= normal.x * velAlongNormal;
            rb1->velocity.y -= normal.y * velAlongNormal;

            // Apply friction to sliding velocity (perpendicular to normal)
            Vec2 tangent{ -normal.y, normal.x };  // Perpendicular vector
            float velAlongTangent = rb1->velocity.x * tangent.x + rb1->velocity.y * tangent.y;

            // Reconstruct velocity: 0 along normal + damped tangent
            rb1->velocity.x = tangent.x * velAlongTangent * WALL_FRICTION;
            rb1->velocity.y = tangent.y * velAlongTangent * WALL_FRICTION;

            // Handle acceleration (for continuous input like WASD)
            float accelAlongNormal = rb1->acceleration.x * normal.x + rb1->acceleration.y * normal.y;

            // Only zero acceleration if pushing into wall AND deeply penetrating
            if (accelAlongNormal < 0 && penetration > PENETRATION_SLOP * 2.0f)
            {
                rb1->acceleration.x -= normal.x * accelAlongNormal;
                rb1->acceleration.y -= normal.y * accelAlongNormal;
            }
        }
    }
    // ═══════════════════════════════════════════════════════════
    // Case 3: Entity 2 can move, Entity 1 is static
    // ═══════════════════════════════════════════════════════════
    else if (e2CanMove && rb2)
    {
        // Apply gradual position correction (opposite direction)
        float correctionAmount = max(penetration - PENETRATION_SLOP, 0.0f) * POSITION_CORRECTION;
        tf2.position -= normal * correctionAmount;

        // Calculate velocity along collision normal (flip sign for entity 2)
        float velAlongNormal = rb2->velocity.x * (-normal.x) + rb2->velocity.y * (-normal.y);

        // Only resolve if moving INTO the wall
        if (velAlongNormal < 0)
        {
            // Remove velocity component going into wall
            rb2->velocity.x -= (-normal.x) * velAlongNormal;
            rb2->velocity.y -= (-normal.y) * velAlongNormal;

            // Apply friction to sliding velocity
            Vec2 tangent{ normal.y, -normal.x };  // Perpendicular (flipped for entity 2)
            float velAlongTangent = rb2->velocity.x * tangent.x + rb2->velocity.y * tangent.y;

            // Reconstruct velocity with friction
            rb2->velocity.x = tangent.x * velAlongTangent * WALL_FRICTION;
            rb2->velocity.y = tangent.y * velAlongTangent * WALL_FRICTION;

            // Handle acceleration
            float accelAlongNormal = rb2->acceleration.x * (-normal.x) + rb2->acceleration.y * (-normal.y);

            // Only zero acceleration if pushing into wall AND deeply penetrating
            if (accelAlongNormal < 0 && penetration > PENETRATION_SLOP * 2.0f)
            {
                rb2->acceleration.x -= (-normal.x) * accelAlongNormal;
                rb2->acceleration.y -= (-normal.y) * accelAlongNormal;
            }
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
        delta = Vec2{ 0, 1 }; // Default to up if at exact same position
    }

    return delta;
}

void Uma_ECS::CollisionSystem::InsertIntoGrid(
    std::unordered_map<Cell, std::vector<Entity>, CellHash>& grid,
    Entity entity,
    const BoundingBox& box)
{
    int minX = WorldToCell(box.min.x);
    int maxX = WorldToCell(box.max.x);
    int minY = WorldToCell(box.min.y);
    int maxY = WorldToCell(box.max.y);

    for (int x = minX; x <= maxX; ++x)
    {
        for (int y = minY; y <= maxY; ++y)
        {
            grid[Cell{ x, y }].push_back(entity);
        }
    }
}

bool Uma_ECS::CollisionSystem::CollisionIntersection_RectRect_Static(
    const BoundingBox& lhs,
    const BoundingBox& rhs)
{
    return !(lhs.max.x < rhs.min.x || // lhs is left of rhs
        lhs.min.x > rhs.max.x || // lhs is right of rhs
        lhs.max.y < rhs.min.y || // lhs is below rhs
        lhs.min.y > rhs.max.y);  // lhs is above rhs
}