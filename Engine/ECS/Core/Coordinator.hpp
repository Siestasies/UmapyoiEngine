/*!
\file   Coordinator.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Central facade class that unifies EntityManager, ComponentManager, and SystemManager into a single interface.

Provides the primary API for ECS operations: entity creation/destruction, component registration/manipulation,
and system registration with signature-based filtering.
Template methods handle component and system operations with automatic signature updates and system membership
recalculation. Implements ISerializer for JSON-based scene serialization with RapidJSON. Integrates with
Uma_Engine::EventSystem to emit entity lifecycle events for external observers.


All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include <memory>

#include "Types.hpp"
#include "ComponentManager.hpp"
#include "EntityManager.hpp"
#include "SystemManager.hpp"

#include "Core/BaseSerializer.h"

// Event system
#include "Core/EventSystem.h"
#include "Events/ECSEvents.h"

#include "../Components/Transform.h"
#include "../Components/RigidBody.h"
#include "../Components/Sprite.h"
#include "../Components/Collider.h"
#include "../Components/Camera.h"
#include "../Components/Player.h"
#include "../Components/Enemy.h"
#include "../Components/Animator.h"
#include "../Components/LuaScript.h"
#include "../Components/AudioComponent.h"
#include "../Components/AudioListener.h"
#include "../Components/PathFinding.h"
#include "../Components/ParticleEmitter.h"
#include "../Components/Prefab.h"
#include "../Components/Projectile.h"
#include "../Components/Tilemap.h"
#include "UI/Components/RectTransform.h"
#include "UI/Components/Image.h"
#include "UI/Components/Button.h"
#include "UI/Components/Slider.h"
#include "UI/Components/Checkbox.h"
#include "UI/Components/Canvas.h"
#include "UI/Components/Text.h"
#include "UI/Components/Effects.h"
#include "UI/Components/Dialogue.h"
#include "../Components/FSM.h"
#include "../Components/SpriteMaterial.h"
#include "../Components/Cutscene.h"

#include <unordered_set>


namespace Uma_ECS
{
    // this is to store the cache data for these 2 manager
    // so when editor enters / exits game mode, it can cache the manager and load it
    struct StateCache
    {
        std::unique_ptr<EntityManager> cachedEntityManager;
        std::unique_ptr<ComponentManager> cachedComponentManager;
        std::vector<Entity> cachedHierarchyOrder;
    };

    // this whole Corrdinator context is about combining:
    // Entity Manager, System Manager and Entity Manager 
    // into a single coordinator that can handles everything 
    // within this class
    class Coordinator : public Uma_Engine::ISerializer
    {
    public:
        /*!
        \brief Initializes the Coordinator by creating all internal managers and registering all component types.
        \param eventSystem Pointer to the EventSystem for emitting entity lifecycle events.
        */
        void Init(Uma_Engine::EventSystem* eventSystem);


        //------------------------------------------+
        //          Entity functions                |
        //------------------------------------------+

        /*!
        \brief Creates a new entity and emits an EntityCreated event.
        \return The newly created Entity ID.
        */
        Entity CreateEntity();

        /*!
        \brief Queues an entity for deferred destruction at the end of the frame.
        \param entity The Entity ID to destroy.
        */
        void DestroyEntity(Entity entity);

        /*!
        \brief Checks whether the given entity is currently active.
        \param entity The Entity ID to check.
        \return True if the entity is active.
        */
        bool HasActiveEntity(Entity entity) const;

        /*!
        \brief Returns the component signature bitset for the given entity.
        \param entity The Entity ID to query.
        \return The entity's current Signature.
        */
        Signature GetEntitySignature(Entity entity);

        /*!
        \brief Returns the number of currently active entities.
        \return The active entity count.
        */
        int GetEntityCount() const;

        /*!
        \brief Destroys all entities and clears all systems.
        */
        void DestroyAllEntities();

        /*!
        \brief Returns all entities that have a component of the given type.
        \return A vector of Entity IDs with component T.
        */
        template<typename T>
        std::vector<Entity> GetEntitiesByComponent()
        {
            std::vector<Entity> result;

            ComponentArray<T>& arr = aComponentManager->GetComponentArray<T>();

            for (size_t i = 0; i < arr.Size(); ++i)
            {
                result.push_back(arr.GetEntity(i));
            }

            return result;
        }

        /*!
        \brief Returns the first entity that has a component of the given type.
        \return The Entity ID, or an invalid entity if none found.
        */
        template<typename T>
        Entity GetEntityByComponent()
        {
            ComponentArray<T>& arr = aComponentManager->GetComponentArray<T>();
            if (arr.Size() > 0)
            {
                return arr.GetEntity(0);
            }

            return static_cast<Entity>(-1); // Invalid entity
        }

        /*!
        \brief Finds all entities with a component matching the given friendly name string (for Lua).
        \param componentName The friendly name of the component type.
        \return A vector of Entity IDs.
        */
        std::vector<Entity> FindEntitiesWithComponentByName(const std::string& componentName);

        /*!
        \brief Finds the first entity with a component matching the given friendly name string (for Lua).
        \param componentName The friendly name of the component type.
        \return The Entity ID, or an invalid entity if none found.
        */
        Entity FindEntityWithComponentByName(const std::string& componentName);

        /*!
        \brief Sets a parent-child relationship between two entities.
        \param child The child Entity ID.
        \param parent The parent Entity ID.
        */
        void SetParent(Entity child, Entity parent);

        /*!
        \brief Removes the parent relationship from a child entity, making it a root entity.
        \param child The child Entity ID to unparent.
        */
        void RemoveParent(Entity child);

        /*!
        \brief Returns the parent entity of the given entity, if one exists.
        \param entity The Entity ID to query.
        \return An optional containing the parent Entity ID, or std::nullopt if no parent.
        */
        std::optional<Entity> GetParent(Entity entity);

        /*!
        \brief Returns a list of all direct children of the given entity.
        \param entity The parent Entity ID.
        \return A vector of child Entity IDs.
        */
        std::vector<Entity> GetChildrenList(Entity entity);

        /*!
        \brief Returns the child entity at the specified index.
        \param entity The parent Entity ID.
        \param index The child index.
        \return The child Entity ID.
        */
        Entity GetChildren(Entity entity, int index);

        /*!
        \brief Recursively destroys an entity and all of its children.
        \param entity The root Entity ID to destroy.
        */
        void DestroyEntityAndChildren(Entity entity);

        /*!
        \brief Sets the active state of an entity (Unity-like SetActive).
        \param entity The Entity ID to update.
        \param active True to activate, false to deactivate.
        */
        void SetActive(Entity entity, bool active);

        /*!
        \brief Checks whether the entity itself is set to active.
        \param entity The Entity ID to check.
        \return True if the entity's own active flag is set.
        */
        bool IsActiveSelf(Entity entity) const;

        /*!
        \brief Checks whether the entity is active considering its parent hierarchy.
        \param entity The Entity ID to check.
        \return True if the entity and all ancestors are active.
        */
        bool IsActiveInHierarchy(Entity entity) const;

        /*!
        \brief Filters and returns only the active entities from a given list.
        \param entities The input vector of Entity IDs to filter.
        \return A vector containing only the active Entity IDs.
        */
        std::vector<Entity> GetActiveEntities(const std::vector<Entity>& entities) const;

        /*!
        \brief Processes the deferred deletion queue, destroying all queued entities. Call once per frame.
        */
        void ProcessDeletionQueue();

        /*!
        \brief Recursively collects an entity and its children in hierarchy order.
        \param root The root Entity ID to start from.
        \param outEntities Output vector to append entities to.
        */
        void CollectHierarchy(Entity root, std::vector<Entity>& outEntities);

        /*!
        \brief Moves an entity to a specific index in the global hierarchy order.
        \param entity The Entity ID to move.
        \param newIndex The target index in the hierarchy.
        */
        void MoveEntityInHierarchy(Entity entity, int newIndex);

        /*!
        \brief Moves an entity to a specific index within its parent's children list.
        \param entity The Entity ID to move.
        \param newIndex The target index within the parent's children.
        */
        void MoveChildInParent(Entity entity, int newIndex);

        /*!
        \brief Moves an entity one position up in the hierarchy order.
        \param entity The Entity ID to move up.
        */
        void MoveEntityUp(Entity entity);

        /*!
        \brief Moves an entity one position down in the hierarchy order.
        \param entity The Entity ID to move down.
        */
        void MoveEntityDown(Entity entity);

        /*!
        \brief Returns the current index of an entity in the global hierarchy order.
        \param entity The Entity ID to query.
        \return The hierarchy index, or -1 if not found.
        */
        int GetHierarchyIndex(Entity entity) const;

        /*!
        \brief Returns a const reference to the global hierarchy order for rendering/inspector use.
        \return A const reference to the hierarchy order vector.
        */
        const std::vector<Entity>& GetHierarchyOrder() const;

        /*!
        \brief Resource paths collected from a prefab entity hierarchy.
        */
        struct PrefabResources
        {
            std::unordered_set<std::string> textures;
            std::unordered_set<std::string> sounds;
            std::unordered_set<std::string> fonts;
        };

        /*!
        \brief Recursively collects all resource paths (textures, sounds, fonts) used by a prefab hierarchy.
        \param root The root Entity ID of the prefab.
        \param outResources Output struct to collect resource paths into.
        */
        void CollectPrefabResources(Entity root, PrefabResources& outResources);

        //------------------------------------------+
        //          Components functions            |
        //------------------------------------------+

        /*!
        \brief Registers a new component type with the ComponentManager.
        */
        template<typename T>
        void RegisterComponent()
        {
            aComponentManager->RegisterComponent<T>();
        }

        /*!
        \brief Adds a component to an entity and updates its signature across all managers.
        \param entity The Entity ID to add the component to.
        \param component The component data to add.
        */
        template<typename T>
        void AddComponent(Entity entity, const T& component)
        {
            // add component
            aComponentManager->AddComponent<T>(entity, component);

            // get curr signature of the entity 
            // set the bitset of the component to true
            // update the entity manager and system manager
            auto signature = aEntityManager->GetSignature(entity);
            signature.set(aComponentManager->GetComponentType<T>(), true);

            aEntityManager->SetSignature(entity, signature);

            aSystemManager->EntitySignatureChanged(entity, signature);
        }

        /*!
        \brief Removes a component from an entity and updates its signature across all managers.
        \param entity The Entity ID to remove the component from.
        */
        template<typename T>
        void RemoveComponent(Entity entity)
        {
            // remove component
            aComponentManager->RemoveComponent<T>(entity);

            // get curr signature of the entity 
            // set the bitset of the component to false
            // update the entity manager and system manager
            auto signature = aEntityManager->GetSignature(entity);
            signature.set(aComponentManager->GetComponentType<T>(), false);

            aEntityManager->SetSignature(entity, signature);

            aSystemManager->EntitySignatureChanged(entity, signature);
        }

        /*!
        \brief Retrieves a reference to the component data for the given entity.
        \param entity The Entity ID to get the component from.
        \return A reference to the component data.
        */
        template<typename T>
        T& GetComponent(Entity entity)
        {
            return aComponentManager->GetComponent<T>(entity);
        }

        /*!
        \brief Checks whether the given entity has a component of the specified type.
        \param entity The Entity ID to check.
        \return True if the entity has the component.
        */
        template<typename T>
        bool HasComponent(Entity entity)
        {
            return aComponentManager->HasComponent<T>(entity);
        }

        /*!
        \brief Returns a reference to the typed ComponentArray for the given component type.
        \return A reference to the ComponentArray of type T.
        */
        template<typename T>
        ComponentArray<T>& GetComponentArray()
        {
            return aComponentManager->GetComponentArray<T>();
        }

        /*!
        \brief Returns the unique ComponentType ID for the given component type.
        \return The ComponentType identifier.
        */
        template<typename T>
        ComponentType GetComponentType()
        {
            return aComponentManager->GetComponentType<T>();
        }

        /*!
        \brief Iterates over all registered component types that the entity has, invoking a callback for each.
        \param entity The Entity ID to iterate components for.
        \param func A callable that receives a ComponentType ID for each present component.
        */
        template<typename Func>
        void ForEachComponent(Entity& entity, Func&& func)
        {
            Signature sig = GetEntitySignature(entity);
            // Check each registered component type
#define CHECK_COMPONENT(ComponentType) \
            if (sig.test(GetComponentType<ComponentType>())) \
            { \
            func(GetComponentType<ComponentType>()); \
            }

            CHECK_COMPONENT(Transform)
            CHECK_COMPONENT(RigidBody)
            CHECK_COMPONENT(Sprite)
            CHECK_COMPONENT(Collider)
            CHECK_COMPONENT(Camera)
            CHECK_COMPONENT(Player)
            CHECK_COMPONENT(Enemy)
            CHECK_COMPONENT(Animator)
            CHECK_COMPONENT(LuaScript)
            CHECK_COMPONENT(AudioComponent)
            CHECK_COMPONENT(AudioListener)
            CHECK_COMPONENT(PathFinding)
            CHECK_COMPONENT(ParticleEmitter)
            CHECK_COMPONENT(Prefab)
            CHECK_COMPONENT(Projectile)
            CHECK_COMPONENT(Tilemap)
            CHECK_COMPONENT(FSM)
            CHECK_COMPONENT(Uma_UI::RectTransform)
            CHECK_COMPONENT(Uma_UI::Image)
            CHECK_COMPONENT(Uma_UI::Button)
            CHECK_COMPONENT(Uma_UI::Slider)
            CHECK_COMPONENT(Uma_UI::Checkbox)
            CHECK_COMPONENT(Uma_UI::Canvas)
            CHECK_COMPONENT(Uma_UI::Text)
            CHECK_COMPONENT(Uma_UI::Effects)
            CHECK_COMPONENT(SpriteMaterial)
            CHECK_COMPONENT(Uma_UI::Dialogue)
            CHECK_COMPONENT(Cutscene)
#undef CHECK_COMPONENT
        }

        //------------------------------------------+
        //          System functions                |
        //------------------------------------------+

        /*!
        \brief Registers a new ECS system of the given type.
        \return A shared pointer to the newly registered system instance.
        */
        template<typename T>
        std::shared_ptr<T> RegisterSystem()
        {
            return aSystemManager->RegisterSystem<T>();
        }

        /*!
        \brief Retrieves the registered system of the given type.
        \return A shared pointer to the system, or nullptr if not found.
        */
        template<typename T>
        std::shared_ptr<T> GetSystem()
        {
            return aSystemManager->GetSystem<T>();
        }

        /*!
        \brief Sets the component signature for a system, defining which components it requires.
        \param signature The Signature bitset specifying required components.
        */
        template<typename T>
        void SetSystemSignature(Signature signature)
        {
            aSystemManager->SetSignature<T>(signature);
        }

        /*!
        \brief Duplicates an entity including all its components.
        \param src The source Entity ID to duplicate.
        \return The newly created duplicate Entity ID.
        */
        Entity DuplicateEntity(Entity src);

        /*!
        \brief Recursively duplicates an entity and its entire child hierarchy.
        \param src The source root Entity ID to duplicate.
        \param oldToNewMap Output map from original Entity IDs to their duplicated counterparts.
        \return The newly created root Entity ID of the duplicated hierarchy.
        */
        Entity DuplicateEntityHierarchy(Entity src, std::unordered_map<Entity, Entity>& oldToNewMap);

        //------------------------------------------+
        //          Serialization                   |
        //------------------------------------------+

        /*! \brief Returns the JSON section name used for serialization. */
        const char* GetSectionName() const override { return "entities"; };

        /*! \brief Returns a human-readable name for this serializer. */
        std::string GetSerializerName() const override { return "coordinator"; };

        /*!
        \brief Serializes all entities and their components into a RapidJSON value.
        \param out The RapidJSON value to write into.
        \param allocator The RapidJSON allocator.
        */
        void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) override;

        /*!
        \brief Deserializes all entities and their components from a RapidJSON value.
        \param in The RapidJSON value containing serialized scene data.
        */
        void Deserialize(const rapidjson::Value& in) override;

        /*!
        \brief Serializes a single entity and its children as a prefab into a RapidJSON value.
        \param entity The root Entity ID of the prefab.
        \param out The RapidJSON value to write into.
        \param allocator The RapidJSON allocator.
        */
        void SerializePrefab(Entity entity, rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) override;

        /*!
        \brief Deserializes a prefab from a RapidJSON value, creating new entities.
        \param in The RapidJSON value containing prefab data.
        \return The root Entity ID of the deserialized prefab.
        */
        Entity DeserializePrefab(const rapidjson::Value& in) override;

        /*!
        \brief Serializes a single entity's components into a RapidJSON value.
        \param entity The Entity ID to serialize.
        \param comps The RapidJSON value to write component data into.
        \param allocator The RapidJSON allocator.
        */
        void SerializeEntity(Entity entity, rapidjson::Value& comps, rapidjson::Document::AllocatorType& allocator);

        /*!
        \brief Deserializes components for a single entity from a RapidJSON value.
        \param entity The Entity ID to deserialize components for.
        \param comps The RapidJSON value containing component data.
        */
        void DeserializeEntity(Entity entity, const rapidjson::Value& comps);

        /*!
        \brief Loads a prefab instance from a file, applying a transform override to the root entity.
        \param prefabPath The file path to the prefab JSON.
        \param rootEntityID The Entity ID to use as the prefab root.
        \param transformOverride The RapidJSON value containing transform overrides.
        \return A map from original prefab Entity IDs to the newly instantiated Entity IDs.
        */
        std::unordered_map<Entity, Entity> LoadPrefabInstance(
            const std::string& prefabPath,
            Entity rootEntityID,
            const rapidjson::Value& transformOverride);


        //------------------------------------------+
        //          State cache                     |
        //------------------------------------------+

        /*!
        \brief Caches the current entity and component state for editor play mode transitions.
        */
        void CacheState();

        /*!
        \brief Restores the previously cached entity and component state when exiting play mode.
        */
        void RestoreState();

        //------------------------------------------+
        //          Helper Func                     |
        //------------------------------------------+

        /*!
        \brief Returns a pointer to the EventSystem used for entity lifecycle events.
        \return Pointer to the EventSystem.
        */
        Uma_Engine::EventSystem* GetEventSystem();

        /*!
        \brief Shuts down the Coordinator, destroying all entities and releasing cached state.
        */
        void ShutDown()
        {
            // DestroyAllEntities() now actually destroys and clears the queue
            DestroyAllEntities();

            // Defensive: Ensure deletion queue is empty before managers are released
            mEntitiesToDestroy.clear();

            mStateCache.cachedComponentManager.release();
            mStateCache.cachedEntityManager.release();
        }

    private:
        std::unique_ptr<ComponentManager> aComponentManager;
        std::unique_ptr<EntityManager> aEntityManager;

        // we keep track of all entity systems here
        // so that we can update all systems when 
        // thr are changes for any entities
        std::unique_ptr<SystemManager> aSystemManager;

        StateCache mStateCache;

        Uma_Engine::EventSystem* pEventSystem = nullptr;

        // gameobject deletion queue
        std::unordered_set<Entity> mEntitiesToDestroy;
        bool mIsProcessingDeletions = false;

        // hierarchy 
        std::vector<Entity> aHierarchyOrder;
    };
}





