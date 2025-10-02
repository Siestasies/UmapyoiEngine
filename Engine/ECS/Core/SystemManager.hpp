#pragma once

#include "Types.hpp"
#include "System.hpp"

#include <string>
#include <unordered_map>
#include <memory>
#include <cassert>

namespace Uma_ECS
{
    //class ECSSystem;  // forward declaration instead of #include "System.h"

    class SystemManager
    {
    public:

        template<typename T>
        std::shared_ptr<T> RegisterSystem()
        {
            std::string type_name = std::string(typeid(T).name());

            assert(aSystems.find(type_name) == aSystems.end() && "Error : Registering the system more than once.");

            // Create a pointer to the system and return it, so that it can be used externally
            auto system = std::make_shared<T>();
            aSystems.insert({ type_name, system });
            return system;
        }
        
        template<typename T> 
        void SetSignature(Signature signature)
        {
            std::string type_name = std::string(typeid(T).name());

            assert(aSystems.find(type_name) != aSystems.end() && "Error : setting the signature of the system before registering.");

            aSignatures.insert({ type_name, signature });
        }

        void EntityDestroyed(Entity entity);

        void EntitySignatureChanged(Entity entity, Signature entitySiganture);

    private:

        std::unordered_map<std::string, Signature> aSignatures{};

        std::unordered_map<std::string, std::shared_ptr<ECSSystem>> aSystems{};

    };
}


