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
Implements AABB collision detection and resolution using spatial hashing for broadphase optimization.

Features swept bounding boxes covering entity movement paths to prevent tunneling through objects.
Uses axis-of-minimum-penetration method for accurate collision normal calculation and Baumgarte
stabilization for gradual position correction. Supports three collision purposes: Physics (entity-entity),
Environment (static walls), and Trigger (non-blocking detection zones). Resolves collisions through
velocity projection with separate handling for normal (penetrating) and tangent (sliding) components.
Implements layer-based filtering via bitmask operations and tracks collision state transitions to emit
OnCollisionEnter, OnCollision, OnCollisionExit, OnTriggerEnter, OnTrigger, and OnTriggerExit events.
Spatial grid partitioning reduces collision checks from O(n²) to near O(n) for sparse entity distributions.

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

    auto& cArray = pCoordinator->GetComponentArray<Collider>();
    auto& tfArray = pCoordinator->GetComponentArray<Transform>();
    auto& sArray = pCoordinator->GetComponentArray<Sprite>();

    for (auto const& entity : aEntities)
    {
        if (!pCoordinator->IsActiveInHierarchy(entity))
            continue;

        auto& c = cArray.GetData(entity);
        auto& tf = tfArray.GetData(entity);

        if (c.shapes.empty()) continue;

        if (c.bounds.size() != c.shapes.size())
        {
            c.bounds.resize(c.shapes.size());
        }

        Vec2 spriteSize{ 1.0f, 1.0f };
        if (sArray.Has(entity))
        {
            auto& s = sArray.GetData(entity);
            if (s.texture)
            {
                spriteSize = s.texture->GetNativeSize();
            }
        }

        for (size_t i = 0; i < c.shapes.size(); ++i)
        {
            const auto& shape = c.shapes[i];
            if (!shape.isActive) continue;

            Vec2 effectiveSize = shape.autoFitToSprite ? spriteSize : shape.size;
            Vec2 scaledSize = Vec2{
                effectiveSize.x * tf.worldScale.x,
                effectiveSize.y * tf.worldScale.y
            };

            Vec2 worldOffset = Vec2{
                shape.offset.x * tf.worldScale.x,
                shape.offset.y * tf.worldScale.y
            };

            Vec2 halfSize = scaledSize * 0.5f;

            Vec2 currentWorldPos = tf.worldPosition + worldOffset;

            // Calculate bounds at current position
            Vec2 currentMin = Vec2{
                currentWorldPos.x - halfSize.x,
                currentWorldPos.y - halfSize.y
            };
            Vec2 currentMax = Vec2{
                currentWorldPos.x + halfSize.x,
                currentWorldPos.y + halfSize.y
            };

            // For Triggers: use current position only (precise detection)
            // For Physics/Environment: use swept AABB (prevents tunneling)
            if (shape.purpose == ColliderPurpose::Trigger)
            {
                // Triggers use current position only - no swept bounds
                c.bounds[i].min = currentMin;
                c.bounds[i].max = currentMax;
            }
            else
            {
                // SWEPT: Cover both current AND previous position for physics
                Vec2 prevWorldPos = tf.prevWorldPos + worldOffset;

                // Calculate bounds at previous position
                Vec2 prevMin = Vec2{
                    prevWorldPos.x - halfSize.x,
                    prevWorldPos.y - halfSize.y
                };
                Vec2 prevMax = Vec2{
                    prevWorldPos.x + halfSize.x,
                    prevWorldPos.y + halfSize.y
                };

                // Combine to create swept AABB
                c.bounds[i].min = Vec2{
                    std::min(currentMin.x, prevMin.x),
                    std::min(currentMin.y, prevMin.y)
                };
                c.bounds[i].max = Vec2{
                    std::max(currentMax.x, prevMax.x),
                    std::max(currentMax.y, prevMax.y)
                };
            }

            /*if (tf.parent.has_value())
            {
                std::cout << "bound : " << c.bounds[i].min << " | " << c.bounds[i].max << std::endl;
            }*/
        }
    }
}

void Uma_ECS::CollisionSystem::UpdateCollision(float dt)
{
    if (aEntities.empty()) return;

    auto& tfArray = pCoordinator->GetComponentArray<Transform>();
    auto& cArray = pCoordinator->GetComponentArray<Collider>();
    auto& rbArray = pCoordinator->GetComponentArray<RigidBody>();

    previousCollisions = std::move(currentCollisions);
    currentCollisions.clear();

    previousTriggers = std::move(currentTriggers);
    currentTriggers.clear();

    if (persistentGrid.empty())
    {
        // reserve at least 512 grids
        persistentGrid.reserve(512);
    }

    // clean up the persistentGrid
    for (auto& [cell, entities] : persistentGrid)
    {
        entities.clear();
    }

    for (const auto& entity : aEntities)
    {
        if (!pCoordinator->IsActiveInHierarchy(entity))
            continue;

        auto& collider = cArray.GetData(entity);

        if (collider.shapes.empty()) continue;

        // Insert entity into grid based on ALL active shapes, not just shape[0]
        for (size_t i = 0; i < collider.shapes.size(); ++i)
        {
            if (collider.shapes[i].isActive)
            {
                InsertIntoGrid(persistentGrid, entity, collider.bounds[i]);
            }
        }
    }

    // Track which entity pairs have been checked this frame to avoid duplicates
    std::unordered_set<EntityPair, EntityPairHash> checkedPairs;
    checkedPairs.reserve(aEntities.size() * 2);

    // Check collisions within each cell
    for (auto const& [cell, entities] : persistentGrid)
    {
        for (size_t i = 0; i < entities.size(); ++i)
        {
            for (size_t j = i + 1; j < entities.size(); ++j)
            {
                Entity e1 = entities[i];
                Entity e2 = entities[j];

                // Skip if we already checked this pair this frame
                EntityPair pair(e1, e2);
                if (checkedPairs.find(pair) != checkedPairs.end())
                    continue;

                checkedPairs.insert(pair);
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

    // update the Trigger pair that has exited
    for (const auto& pair : previousTriggers)
    {
        // means has ended
        if (currentTriggers.find(pair) == currentTriggers.end())
        {
            // Trigger ended - emit exit event
            pEventSystem->Emit<Uma_Engine::OnTriggerExitEvent>(
                pair.entityA, pair.entityB);
        }
    }


}

Uma_ECS::Entity Uma_ECS::CollisionSystem::GetPhysicsEntity(Entity entity, ComponentArray<Transform>& tfArray, ComponentArray<RigidBody>& rbArray)
{
    Entity curr = entity;

    // Walk up to find root entity
    while (tfArray.Has(curr) && tfArray.GetData(curr).parent.has_value())
    {
        curr = tfArray.GetData(curr).parent.value();
    }

    // Only return root if it has RigidBody, otherwise return invalid
    if (rbArray.Has(curr))
        return curr;

    return static_cast<Entity>(-1);  // Return invalid if no physics
}

void Uma_ECS::CollisionSystem::CheckEntityPairCollision(
    Entity e1, Entity e2,
    ComponentArray<Transform>& tfArray,
    ComponentArray<Collider>& cArray,
    ComponentArray<RigidBody>& rbArray,
    float dt)
{
    (void)dt;
    auto& c1 = cArray.GetData(e1);
    auto& c2 = cArray.GetData(e2);

    // Validate shapes exist
    if (c1.shapes.empty() || c2.shapes.empty()) return;
    if (!c1.shapes[0].isActive || !c2.shapes[0].isActive) return;

    // Get physics entities FIRST
    Entity physicsEntity1 = GetPhysicsEntity(e1, tfArray, rbArray);
    Entity physicsEntity2 = GetPhysicsEntity(e2, tfArray, rbArray);

    // Check if PHYSICS entities have RigidBody
    bool e1HasRb = (physicsEntity1 != static_cast<Entity>(-1));
    bool e2HasRb = (physicsEntity2 != static_cast<Entity>(-1));

    // Skip if both static (optimization)
    if (!e1HasRb && !e2HasRb)
        return;

    // Get components from physics entities
    Transform* tf1 = e1HasRb ? &tfArray.GetData(physicsEntity1) : nullptr;
    Transform* tf2 = e2HasRb ? &tfArray.GetData(physicsEntity2) : nullptr;

    RigidBody* rb1 = e1HasRb ? &rbArray.GetData(physicsEntity1) : nullptr;
    RigidBody* rb2 = e2HasRb ? &rbArray.GetData(physicsEntity2) : nullptr;

    // Handle case where one entity doesn't have physics
    if (!tf1 || !tf2) return;

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

            if (!((layer1 & mask2) && (mask1 & layer2)))
                continue;

            // Purpose filtering
            if (!ShouldPurposesCollide(shape1.purpose, shape2.purpose))
                continue;

            // Collision test
            if (CollisionIntersection_RectRect_Static(c1.bounds[i], c2.bounds[j]))
            {
                // Pass physics entities and their transforms
                HandleShapeCollision(
                    physicsEntity1, physicsEntity2,
                    *tf1, *tf2,
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
        // Check if this is a new trigger interaction
        bool wasColliding = previousTriggers.find(pair) != previousTriggers.end();

        // Only emit events if this is the first shape collision for this pair this frame
        auto insertResult = currentTriggers.insert(pair);
        bool isFirstCollisionThisFrame = insertResult.second;

        if (isFirstCollisionThisFrame)
        {
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
        }

        return;
    }

    // track collision for physics colliders
    bool wasColliding = previousCollisions.find(pair) != previousCollisions.end();

    // Only emit events if this is the first shape collision for this pair this frame
    auto insertResult = currentCollisions.insert(pair);
    bool isFirstCollisionThisFrame = insertResult.second;

    if (isFirstCollisionThisFrame)
    {
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
    // Calculate penetration on both axes
    Vec2 center1 = (box1.min + box1.max) * 0.5f;
    Vec2 center2 = (box2.min + box2.max) * 0.5f;
    Vec2 halfSize1 = (box1.max - box1.min) * 0.5f;
    Vec2 halfSize2 = (box2.max - box2.min) * 0.5f;

    // Calculate delta between centers
    Vec2 delta = center1 - center2;

    // Calculate overlap on each axis (penetration depth)
    float overlapX = halfSize1.x + halfSize2.x - std::abs(delta.x);
    float overlapY = halfSize1.y + halfSize2.y - std::abs(delta.y);

    // Early exit if not overlapping
    if (overlapX <= 0 || overlapY <= 0)
        return;

    // Find axis of minimum penetration
    Vec2 normal{ 0, 0 };
    float penetration = 0;

    // Use the axis with SMALLER overlap (minimum penetration)
    if (overlapX < overlapY)
    {
        // Collision on X axis (vertical surface)
        normal.x = (delta.x > 0) ? 1.0f : -1.0f;
        normal.y = 0;
        penetration = overlapX;
    }
    else
    {
        // Collision on Y axis (horizontal surface)
        normal.x = 0;
        normal.y = (delta.y > 0) ? 1.0f : -1.0f;
        penetration = overlapY;
    }

    // Position correction with Baumgarte stabilization
    const float BAUMGARTE_COEFF = 0.2f;  // 20% correction (reduced from 40%)
    const float PENETRATION_SLOP = 0.01f; // Allow small overlap

    float correctionAmount = std::max(penetration - PENETRATION_SLOP, 0.0f) * BAUMGARTE_COEFF;

    if (e1CanMove && e2CanMove)
    {
        // dynamic
        tf1.position += normal * (correctionAmount * 0.5f);
        tf2.position -= normal * (correctionAmount * 0.5f);
    }
    else if (e1CanMove)
    {
        tf1.position += normal * correctionAmount;
    }
    else if (e2CanMove)
    {
        tf2.position -= normal * correctionAmount;
    }

    // Velocity-based impulse resolution

    if (e1CanMove && e2CanMove && rb1 && rb2)
    {
        // Dynamic vs Dynamic
        Vec2 relativeVel = rb1->velocity - rb2->velocity;
        float velAlongNormal = relativeVel.x * normal.x + relativeVel.y * normal.y;

        // Only resolve if moving towards each other
        if (velAlongNormal < 0)
        {
            const float RESTITUTION = 0.0f; // No bounce for top-down
            float impulse = -(1.0f + RESTITUTION) * velAlongNormal * 0.5f;

            Vec2 impulseVec = normal * impulse;
            rb1->velocity += impulseVec;
            rb2->velocity -= impulseVec;

            // Apply friction to tangent velocity
            const float FRICTION = 0.95f;
            Vec2 tangent{ -normal.y, normal.x };

            // Project velocity onto tangent and apply friction
            float tangentVel1 = rb1->velocity.x * tangent.x + rb1->velocity.y * tangent.y;
            float tangentVel2 = rb2->velocity.x * tangent.x + rb2->velocity.y * tangent.y;

            rb1->velocity -= tangent * (tangentVel1 * (1.0f - FRICTION));
            rb2->velocity -= tangent * (tangentVel2 * (1.0f - FRICTION));
        }
    }
    else if (e1CanMove && rb1)
    {
        // Dynamic vs Static
        float velAlongNormal = rb1->velocity.x * normal.x + rb1->velocity.y * normal.y;

        // Only resolve if moving into the wall
        if (velAlongNormal < 0)
        {
            // Remove normal component of velocity
            rb1->velocity.x -= normal.x * velAlongNormal;
            rb1->velocity.y -= normal.y * velAlongNormal;

            // Apply friction to sliding velocity
            const float WALL_FRICTION = 0.95f;
            Vec2 tangent{ -normal.y, normal.x };
            float tangentVel = rb1->velocity.x * tangent.x + rb1->velocity.y * tangent.y;

            // Reduce tangent velocity by friction
            rb1->velocity.x = tangent.x * tangentVel * WALL_FRICTION;
            rb1->velocity.y = tangent.y * tangentVel * WALL_FRICTION;

            // Handle acceleration more carefully
            float accelAlongNormal = rb1->acceleration.x * normal.x + rb1->acceleration.y * normal.y;

            // Only remove normal acceleration if pushing into wall
            if (accelAlongNormal < 0)
            {
                rb1->acceleration.x -= normal.x * accelAlongNormal;
                rb1->acceleration.y -= normal.y * accelAlongNormal;
            }
        }
    }
    else if (e2CanMove && rb2)
    {
        // Static vs Dynamic (same as above but flipped)
        Vec2 flippedNormal = normal * -1.0f;
        float velAlongNormal = rb2->velocity.x * flippedNormal.x + rb2->velocity.y * flippedNormal.y;

        if (velAlongNormal < 0)
        {
            rb2->velocity.x -= flippedNormal.x * velAlongNormal;
            rb2->velocity.y -= flippedNormal.y * velAlongNormal;

            const float WALL_FRICTION = 0.95f;
            Vec2 tangent{ -flippedNormal.y, flippedNormal.x };
            float tangentVel = rb2->velocity.x * tangent.x + rb2->velocity.y * tangent.y;

            rb2->velocity.x = tangent.x * tangentVel * WALL_FRICTION;
            rb2->velocity.y = tangent.y * tangentVel * WALL_FRICTION;

            float accelAlongNormal = rb2->acceleration.x * flippedNormal.x + rb2->acceleration.y * flippedNormal.y;

            if (accelAlongNormal < 0)
            {
                rb2->acceleration.x -= flippedNormal.x * accelAlongNormal;
                rb2->acceleration.y -= flippedNormal.y * accelAlongNormal;
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

void Uma_ECS::CollisionSystem::DebugRender()
{
    if (!pGraphics) return;

    auto& cArray = pCoordinator->GetComponentArray<Collider>();
    auto& tfArray = pCoordinator->GetComponentArray<Transform>();
    auto& sArray = pCoordinator->GetComponentArray<Sprite>();

    // Container to collect all debug lines
    std::vector<Uma_Engine::DebugLineInfo> debug_lines;

    for (const auto& entity : aEntities)
    {
        if (!pCoordinator->IsActiveInHierarchy(entity))
            continue;

        if (!cArray.Has(entity)) continue;

        auto& c = cArray.GetData(entity);
        auto& tf = tfArray.GetData(entity);

        if (!c.showBBox) continue;

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

        for (size_t i = 0; i < c.shapes.size(); ++i)
        {
            const auto& shape = c.shapes[i];
            if (!shape.isActive) continue;

            // Calculate bounds using current position
            Vec2 effectiveSize = shape.autoFitToSprite ? spriteSize : shape.size;

            Vec2 scaledSize = Vec2{
                effectiveSize.x * tf.scale.x,
                effectiveSize.y * tf.scale.y
            };

            Vec2 worldOffset = Vec2{
                shape.offset.x * tf.scale.x,
                shape.offset.y * tf.scale.y
            };

            Vec2 halfSize = scaledSize * 0.5f;

            // Use renderPos (interpolated) instead of position
            //Vec2 renderWorldPos = tf.prevWorldPos + worldOffset;

            // Use current position
            Vec2 renderWorldPos = tf.worldPosition + worldOffset;

            // Calculate bounds for visualization
            BoundingBox visualBounds;
            visualBounds.min = Vec2{
                renderWorldPos.x - halfSize.x,
                renderWorldPos.y - halfSize.y
            };
            visualBounds.max = Vec2{
                renderWorldPos.x + halfSize.x,
                renderWorldPos.y + halfSize.y
            };

            //LayerMask effectiveLayer = c.GetEffectiveLayer(i);
            LayerMask effectiveMask = c.GetEffectiveMask(i);

            // Determine color based on purpose
            float r = 1.f, g = 0.f, b = 0.f;

            if (shape.purpose == ColliderPurpose::Trigger)
            {
                // Triggers: Blue
                r = 0.f; g = 0.f; b = 1.f;
            }
            else if (shape.purpose == ColliderPurpose::Environment)
            {
                // Walls: Green
                r = 0.f; g = 1.f; b = 0.f;
            }
            else if (shape.purpose == ColliderPurpose::Physics)
            {
                // Check what it collides with
                if (effectiveMask & CL_WALL)
                {
                    // Feet (collides with walls): green
                    r = 0.f; g = 1.f; b = 0.f;
                }
                else if (effectiveMask & CL_ENEMY || effectiveMask & CL_PLAYER)
                {
                    // Body (collides with enemies / player): Red
                    r = 1.f; g = 0.f; b = 0.f;
                }
                else
                {
                    // Other physics: Purple
                    r = 1.f; g = 0.f; b = 1.f;
                }
            }

            // Add rectangle to batch render
            Uma_Engine::Graphics::AddDebugRect(visualBounds, r, g, b, debug_lines);
        }
    }

    // Draw all debug lines in one call
    if (!debug_lines.empty())
    {
        pGraphics->DrawDebugLinesInstanced(debug_lines);
    }
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

    minX = std::clamp(minX, -MAX_CELL_COORD, MAX_CELL_COORD);
    maxX = std::clamp(maxX, -MAX_CELL_COORD, MAX_CELL_COORD);
    minY = std::clamp(minY, -MAX_CELL_COORD, MAX_CELL_COORD);
    maxY = std::clamp(maxY, -MAX_CELL_COORD, MAX_CELL_COORD);

    for (int x = minX; x <= maxX; ++x)
    {
        for (int y = minY; y <= maxY; ++y)
        {
            auto& cellEntities = grid[Cell{ x, y }];

            if (cellEntities.capacity() == 0)
            {
                cellEntities.reserve(16);
            }

            cellEntities.push_back(entity);
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