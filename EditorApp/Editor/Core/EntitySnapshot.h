/*!
\file   EntitySnapshot.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines the EntitySnapshot struct, a serializable container used to store
an entity’s full state for undo/redo operations. Captures component data,
hierarchy relationships, parent ID, and list of children.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "ECS/Core/Types.hpp"
#include "RapidJSON/document.h"

#include <optional>
#include <vector>

namespace Uma_Editor
{
    /**
     * \struct EntitySnapshot
     * \brief Stores all data necessary to restore an entity.
     *
     * This struct captures:
     * - The entity ID
     * - Serialized component data (RapidJSON)
     * - Parent entity (if any)
     * - List of children entities
     *
     * Used by EntitySnapshotCmd for reversible editor operations.
     */
    struct EntitySnapshot
    {
        Uma_ECS::Entity entityID;                 //!< ID of the stored entity
        rapidjson::Document componentData;        //!< Serialized components

        std::optional<Uma_ECS::Entity> parentID;  //!< Parent entity, if any
        std::vector<Uma_ECS::Entity> childrenIDs; //!< Entity's children
    };
}
