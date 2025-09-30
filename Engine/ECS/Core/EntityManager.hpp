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

        void SetSignature(Entity entity, Signature& const signature);
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


