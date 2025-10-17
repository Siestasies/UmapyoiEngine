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
        EntityManager();

        Entity CreateEntity();
        void DestroyEntity(Entity entity);

        bool HasActiveEntity(Entity entity) const;

        void SetSignature(Entity entity, const Signature& signature);
        Signature GetSignature(Entity entity) const;

        int GetEntityCount() const;

        std::vector<Entity> GetAllEntites() const;

        inline bool IsEntityActive(Entity en) { return aEntityActive[en]; }

        void DestroyAllEntities();

    private:

        // a queue of all the unused entities id
        std::queue<Entity> aAvailableEntities{};

        std::array<Signature, MAX_ENTITIES> aSignatures{};
        std::array<bool, MAX_ENTITIES> aEntityActive{};

        unsigned int mActiveEntityCnt{};
    };
}


