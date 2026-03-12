/*!
\file   EntityManager.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Manages entity IDs, signatures, and lifecycle state using fixed-size arrays and a reuse queue.

Maintains a queue of available entity IDs (0 to MAX_ENTITIES-1) for efficient allocation and reuse.
Stores per-entity component signatures (bitsets) and active status flags for validation.
Provides entity creation with ID recycling, destruction with signature cleanup, signature queries,
and active entity enumeration. Supports up to MAX_ENTITIES (11,000) concurrent entities.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "Types.hpp"
#include <queue>
#include <array>
#include <vector>

namespace Uma_ECS
{
    class EntityManager
    {
    public:
        /*!
        \brief Default constructor. Initializes the available entity queue with all entity IDs.
        */
        EntityManager();

        /*!
        \brief Copy constructor. Deep copies all entity state arrays and the available entity queue.
        \param other The EntityManager instance to copy from.
        */
        EntityManager(const EntityManager& other) noexcept;

        /*!
        \brief Creates a new entity by dequeuing an available ID and marking it as active.
        \return The newly created Entity ID.
        */
        Entity CreateEntity();

        /*!
        \brief Destroys an entity by resetting its signature, marking it inactive, and returning its ID to the queue.
        \param entity The Entity ID to destroy.
        */
        void DestroyEntity(Entity entity);

        /*!
        \brief Checks whether the given entity is currently active (has been created and not destroyed).
        \param entity The Entity ID to check.
        \return True if the entity is active, false otherwise.
        */
        bool HasActiveEntity(Entity entity) const;

        /*!
        \brief Sets the component signature bitset for the specified entity.
        \param entity The Entity ID to update.
        \param signature The new Signature to assign.
        */
        void SetSignature(Entity entity, const Signature& signature);

        /*!
        \brief Retrieves the component signature bitset for the specified entity.
        \param entity The Entity ID to query.
        \return The entity's current Signature.
        */
        Signature GetSignature(Entity entity) const;

        /*!
        \brief Returns the number of currently active entities.
        \return The active entity count.
        */
        int GetEntityCount() const;

        /*!
        \brief Returns a vector containing all currently active entity IDs.
        \return A vector of active Entity IDs.
        */
        std::vector<Entity> GetAllEntites() const;

        /*!
        \brief Checks if an entity is in the active state.
        \param en The Entity ID to check.
        \return True if the entity is active.
        */
        inline bool IsEntityActive(Entity en) { return aEntityActive[en]; }

        /*!
        \brief Sets the enabled/disabled state for an entity (Unity-like SetActive).
        \param entity The Entity ID to update.
        \param enabled True to enable, false to disable.
        */
        void SetEntityEnabled(Entity entity, bool enabled);

        /*!
        \brief Checks whether the entity is currently enabled.
        \param entity The Entity ID to query.
        \return True if the entity is enabled.
        */
        bool IsEntityEnabled(Entity entity) const;

        /*!
        \brief Destroys all active entities, resetting the manager to its initial state.
        */
        void DestroyAllEntities();

    private:

        // a queue of all the unused entities id
        std::queue<Entity> aAvailableEntities{};

        std::array<Signature, MAX_ENTITIES> aSignatures{};
        std::array<bool, MAX_ENTITIES> aEntityActive{};
        std::array<bool, MAX_ENTITIES> aEntityEnabled{};  // Track enabled/disabled state (Unity-like)

        unsigned int mActiveEntityCnt{};
    };
}


