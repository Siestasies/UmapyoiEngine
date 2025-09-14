#pragma once

#include "Types.hpp"
#include <queue>
#include <array>

namespace Uma_ECS
{
    class EntityManager
    {
    public:
        EntityManager();

        Entity CreateEntity();
        void DestroyEntity(Entity entity);

        void SetSignature(Entity entity, Signature& const signature);
        Signature GetSignature(Entity entity) const;

    private:

        // a queue of all the unused entities id
        std::queue<Entity> aAvailableEntities{};

        std::array<Signature, MAX_ENTITIES> aSignatures{};
        std::array<bool, MAX_ENTITIES> aEntityActive{};

        unsigned int mActiveEntityCnt{};
    };
}


