#pragma once

#include "Types.h"
#include "System.h"

#include <string>
#include <unordered_map>
#include <memory>

namespace Uma_ECS
{
    class SystemManager
    {
    public:

        template<typename T>
        std::shared_ptr<T> RegisterSystem();
        
        template<typename T> 
        void SetSignature(Signature signature);

        void EntityDestroyed(Entity entity);

        void EntitySignatureChanged(Entity entity, Signature entitySiganture);

    private:

        std::unordered_map<std::string, Signature> aSignatures{};

        std::unordered_map<std::string, std::shared_ptr<ISystem>> aSystems{};

    };
}


