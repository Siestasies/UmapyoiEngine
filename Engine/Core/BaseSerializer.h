/*!
\file   BaseSerializer.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines ISerializer interface for polymorphic serialization of engine subsystems to JSON format.

Requires derived classes to provide section name identifier and implement serialize/deserialize methods using RapidJSON.
Enables GameSerializer to orchestrate multi-system persistence by treating all subsystems uniformly through this interface.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "RapidJSON/document.h"

#include <string>

namespace Uma_Engine
{
    using Entity = unsigned int;

    /*!
     * \class ISerializer
     * \brief Interface for polymorphic JSON serialization of engine subsystems.
     */
    class ISerializer {
    public:
        /*!
         * \brief Returns the JSON section name used as a key during serialization.
         * \return Section name string (e.g. "entities", "resources").
         */
        virtual const char* GetSectionName() const = 0;

        /*!
         * \brief Returns a human-readable name identifying this serializer.
         * \return Serializer name string.
         */
        virtual std::string GetSerializerName() const = 0;

        /*!
         * \brief Serializes subsystem state to a JSON value.
         * \param out Output JSON value to populate.
         * \param allocator RapidJSON allocator for memory management.
         */
        virtual void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) = 0;

        /*!
         * \brief Deserializes subsystem state from a JSON value.
         * \param in Input JSON value to read from.
         */
        virtual void Deserialize(const rapidjson::Value& in) = 0;

        /*!
         * \brief Serializes a single entity as a prefab to a JSON value.
         * \param entity The entity to serialize.
         * \param out Output JSON value to populate.
         * \param allocator RapidJSON allocator for memory management.
         */
        virtual void SerializePrefab(Entity entity, rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) = 0;

        /*!
         * \brief Deserializes a prefab from a JSON value and creates an entity.
         * \param in Input JSON value containing prefab data.
         * \return The newly created entity ID.
         */
        virtual Entity DeserializePrefab(const rapidjson::Value& in) = 0;

        virtual ~ISerializer() = default;
    };
}