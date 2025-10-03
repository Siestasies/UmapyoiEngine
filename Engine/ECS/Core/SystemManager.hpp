/*!
\file   SystemManager.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Manages registration and lifecycle of all ECS systems with signature-based entity filtering.

Maps system type names to system instances (shared pointers) and their required component signatures.
Template methods provide type-safe system registration and signature configuration.
Automatically updates system membership when entities are created, destroyed, or have components added/removed
by comparing entity signatures with system signatures using bitwise AND operations. Systems only track entities
that contain all required components specified in their signature.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

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


