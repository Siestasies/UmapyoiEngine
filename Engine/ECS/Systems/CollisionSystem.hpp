/*!
\file   CollisionSystem.hpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines collision detection and resolution system using spatial hashing with AABB primitives.

Unity-inspired approach with contact normals, velocity projection, and purpose-based resolution.
Provides layer-based collision filtering through bitmask operations on Collider components.
Cell struct and CellHash functor enable grid-based spatial partitioning with configurable cellSize constant.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "Core/System.hpp"
#include "Core/Coordinator.hpp"
#include "Components/Collider.h"
#include "Core/EventSystem.h"
#include "../Systems/Graphics.hpp"

#include <unordered_set>

const int MAX_CELL_COORD = 1000;    // so 1 axis is -1000 to 1000

namespace Uma_ECS
{
    struct Cell
    {
        int x, y;

        bool operator==(const Cell& other) const
        {
            return x == other.x && y == other.y;
        }
    };

    // Hash function for spatial grid
    struct CellHash
    {
        std::size_t operator()(const Cell& c) const
        {
            return std::hash<int>()(c.x) * 73856093 ^ std::hash<int>()(c.y) * 19349663;
        }
    };

    // Tracking entity pairs currently colliding
    struct EntityPair
    {
        Entity entityA;
        Entity entityB;

        EntityPair(Entity a, Entity b)
            : entityA((a < b) ? a : b)
            , entityB((a < b) ? b : a)
        {
        }

        bool operator==(const EntityPair& other) const
        {
            return entityA == other.entityA && entityB == other.entityB;
        }
    };

    struct EntityPairHash
    {
        std::size_t operator()(const EntityPair& p) const
        {
            return std::hash<Uma_ECS::Entity>()(p.entityA) ^
                (std::hash<Uma_ECS::Entity>()(p.entityB) << 1);
        }
    };

    // Tracking trigger pairs with trigger owner to allow both directions
    struct TriggerPair
    {
        Entity entityA;
        Entity entityB;
        Entity triggerOwner;

        TriggerPair(Entity a, Entity b, Entity owner)
            : entityA((a < b) ? a : b)
            , entityB((a < b) ? b : a)
            , triggerOwner(owner)
        {
        }

        bool operator==(const TriggerPair& other) const
        {
            return entityA == other.entityA && entityB == other.entityB
                && triggerOwner == other.triggerOwner;
        }
    };

    struct TriggerPairHash
    {
        std::size_t operator()(const TriggerPair& p) const
        {
            return std::hash<Uma_ECS::Entity>()(p.entityA) ^
                (std::hash<Uma_ECS::Entity>()(p.entityB) << 1) ^
                (std::hash<Uma_ECS::Entity>()(p.triggerOwner) << 2);
        }
    };

    struct Transform;
    struct Collider;
    struct RigidBody;

    class CollisionSystem : public ECSSystem
    {
    public:
        /**
         * \brief Initializes the CollisionSystem with necessary subsystem pointers
         * \param c Pointer to the ECS Coordinator
         * \param e Pointer to the Event System
         * \param g Pointer to the Graphics System (for debug rendering)
         */
        inline void Init(Coordinator* c, Uma_Engine::EventSystem* e, Uma_Engine::Graphics* g)
        {
            pCoordinator = c;
            pEventSystem = e;
            pGraphics = g;
        }

        /**
         * \brief Updates the collision system logic each frame
         * \param dt Delta time between frames
         */
        void Update(float dt);

        /**
         * \brief Renders collision debug visuals such as bounding boxes or grid cells
         */
        void DebugRender();

        /*!
         * \brief Retrieves all entities whose bounding boxes overlap a rectangular area.
         * \param min Minimum corner of the query rectangle in world space.
         * \param max Maximum corner of the query rectangle in world space.
         * \return Set of entities found within the specified area.
         */
        std::unordered_set<Entity> GetEntitiesInArea(Vec2 min, Vec2 max);

    private:

        /**
         * \brief Updates all entities� bounding boxes based on their transforms
         */
        void UpdateBoundingBoxes();

        /**
         * \brief Performs collision detection and resolution for all entities
         * \param dt Delta time between frames
         */
        void UpdateCollision(float dt);

        /**
         * \brief Gets the physics-enabled entity associated with a given entity
         * \param entity Entity to check
         * \param tfArray Reference to the Transform component array
         * \param rbArray Reference to the RigidBody component array
         * \return The entity that represents a valid physics object
         */
        Entity GetPhysicsEntity(
            Entity entity,
            ComponentArray<Transform>& tfArray,
            ComponentArray<RigidBody>& rbArray);

        /**
         * \brief Checks for collision between two entities and resolves them if needed
         * \param e1 First entity
         * \param e2 Second entity
         * \param tfArray Reference to Transform component array
         * \param cArray Reference to Collider component array
         * \param rbArray Reference to RigidBody component array
         * \param dt Delta time between frames
         */
        void CheckEntityPairCollision(
            Entity e1, Entity e2,
            ComponentArray<Transform>& tfArray,
            ComponentArray<Collider>& cArray,
            ComponentArray<RigidBody>& rbArray,
            float dt);

        /**
         * \brief Determines if two collider purposes should interact based on filtering rules
         * \param p1 Purpose of the first collider
         * \param p2 Purpose of the second collider
         * \return True if the purposes should collide, false otherwise
         */
        bool ShouldPurposesCollide(ColliderPurpose p1, ColliderPurpose p2);

        /**
         * \brief Handles collision logic between two entities� collider shapes
         * \param e1 First entity
         * \param e2 Second entity
         * \param tf1 Transform of the first entity
         * \param tf2 Transform of the second entity
         * \param rb1 Pointer to the first entity�s rigid body (nullable)
         * \param rb2 Pointer to the second entity�s rigid body (nullable)
         * \param box1 Bounding box of the first entity
         * \param box2 Bounding box of the second entity
         * \param purpose1 Collision purpose of the first collider
         * \param purpose2 Collision purpose of the second collider
         */
        void HandleShapeCollision(
            Entity colliderEntity1, Entity colliderEntity2,
            Entity physicsEntity1, Entity physicsEntity2,
            Transform& tf1, Transform& tf2,
            RigidBody* rb1, RigidBody* rb2,
            const BoundingBox& box1, const BoundingBox& box2,
            ColliderPurpose purpose1, ColliderPurpose purpose2);

        /**
         * \brief Resolves collision between two AABBs using contact normals and velocity projection
         * \param tf1 Transform of the first entity
         * \param tf2 Transform of the second entity
         * \param box1 Bounding box of the first entity
         * \param box2 Bounding box of the second entity
         * \param e1CanMove Whether the first entity is movable
         * \param e2CanMove Whether the second entity is movable
         * \param rb1 Pointer to the first entity�s rigid body (nullable)
         * \param rb2 Pointer to the second entity�s rigid body (nullable)
         */
        void ResolveAABBCollision(
            Transform& tf1, Transform& tf2,
            const BoundingBox& box1, const BoundingBox& box2,
            bool e1CanMove, bool e2CanMove,
            RigidBody* rb1, RigidBody* rb2);

        /**
         * \brief Calculates the collision normal vector between two AABBs
         * \param box1 Bounding box of the first entity
         * \param box2 Bounding box of the second entity
         * \return Collision normal direction as a 2D vector
         */
        Vec2 GetCollisionNormal(const BoundingBox& box1, const BoundingBox& box2);

        /**
         * \brief Converts a world-space coordinate to a grid cell index
         * \param coord World coordinate (X or Y)
         * \return Corresponding grid cell index
         */
        inline int WorldToCell(float coord)
        {
            return static_cast<int>(std::floor(coord * invCellSize));
        }

        /**
         * \brief Inserts an entity into the spatial grid based on its bounding box
         * \param grid Reference to the spatial hash grid
         * \param e Entity to insert
         * \param box Bounding box of the entity
         */
        void InsertIntoGrid(
            std::unordered_map<Cell, std::vector<Entity>, CellHash>& grid,
            Entity e,
            const BoundingBox& box);

        /**
         * \brief Checks for intersection between two static axis-aligned bounding boxes
         * \param lhs First bounding box
         * \param rhs Second bounding box
         * \return True if the bounding boxes intersect, false otherwise
         */
        bool CollisionIntersection_RectRect_Static(
            const BoundingBox& lhs,
            const BoundingBox& rhs);

        /*!
         * \brief Computes the current world-space bounding box for a specific collider shape on an entity.
         * \param entity Entity owning the collider.
         * \param shapeIndex Index of the shape within the entity's collider component.
         * \return Computed bounding box in world space.
         */
        BoundingBox ComputeCurrentBounds(Entity entity, size_t shapeIndex);

        float cellSize = 300.f;      // Tune based on your game world
        float invCellSize = 1.f / cellSize;

        // Member variables
        Coordinator* pCoordinator = nullptr;             ///< Pointer to ECS coordinator
        Uma_Engine::Graphics* pGraphics = nullptr;       ///< Pointer to graphics system (for debug)
        Uma_Engine::EventSystem* pEventSystem = nullptr; ///< Pointer to event system

        std::unordered_map<Cell, std::vector<Entity>, CellHash> persistentGrid;

        std::unordered_set<EntityPair, EntityPairHash> currentCollisions;  ///< Collisions detected this frame
        std::unordered_set<EntityPair, EntityPairHash> previousCollisions; ///< Collisions detected last frame

        std::unordered_set<TriggerPair, TriggerPairHash> currentTriggers;  ///< Triggers detected this frame
        std::unordered_set<TriggerPair, TriggerPairHash> previousTriggers; ///< Triggers detected last frame
    };
}
