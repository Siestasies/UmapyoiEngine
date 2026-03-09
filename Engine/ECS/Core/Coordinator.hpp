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
        void Init(Uma_Engine::EventSystem* eventSystem);

        
        //------------------------------------------+
        //          Entity functions                |
        //------------------------------------------+

        Entity CreateEntity();

        void DestroyEntity(Entity entity);

        bool HasActiveEntity(Entity entity) const;

        Signature GetEntitySignature(Entity entity);

        int GetEntityCount() const;

        void DestroyAllEntities();

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

        // Find entities with a component by string name (for Lua)
        std::vector<Entity> FindEntitiesWithComponentByName(const std::string& componentName);

        // Find first entity with a component by string name (for Lua)
        Entity FindEntityWithComponentByName(const std::string& componentName);

        void SetParent(Entity child, Entity parent);
        void RemoveParent(Entity child);
        std::optional<Entity> GetParent(Entity entity);
        std::vector<Entity> GetChildrenList(Entity entity);
        Entity GetChildren(Entity entity, int index);
        void DestroyEntityAndChildren(Entity entity);

        // Enable/Disable functionality (Unity-like SetActive)
        void SetActive(Entity entity, bool active);
        bool IsActiveSelf(Entity entity) const;
        bool IsActiveInHierarchy(Entity entity) const;

        // Helper method for systems to filter active entities
        std::vector<Entity> GetActiveEntities(const std::vector<Entity>& entities) const;

        void ProcessDeletionQueue();  // Call this once per frame

        // collect the entities in hierachy order
        void CollectHierarchy(Entity root, std::vector<Entity>& outEntities);

        // Move entity to a specific index in global hierarchy
        void MoveEntityInHierarchy(Entity entity, int newIndex);

        // Move entity to a specific index within its parent's children list
        void MoveChildInParent(Entity entity, int newIndex);

        // Move one position up (updates both parent's children and global hierarchy)
        void MoveEntityUp(Entity entity);

        // Move one position down (updates both parent's children and global hierarchy)
        void MoveEntityDown(Entity entity);

        int GetHierarchyIndex(Entity entity) const;

        // Get hierarchy order for rendering inspector
        const std::vector<Entity>& GetHierarchyOrder() const;

        // collect resources used by prefab hierarchy
        struct PrefabResources
        {
            std::unordered_set<std::string> textures;
            std::unordered_set<std::string> sounds;
            std::unordered_set<std::string> fonts;
        };
        void CollectPrefabResources(Entity root, PrefabResources& outResources);

        //------------------------------------------+
        //          Components functions            |
        //------------------------------------------+

        template<typename T>
        void RegisterComponent() 
        {
            aComponentManager->RegisterComponent<T>();
        }

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

        template<typename T>
        T& GetComponent(Entity entity)
        {
            return aComponentManager->GetComponent<T>(entity);
        }

        template<typename T>
        bool HasComponent(Entity entity)
        {
            return aComponentManager->HasComponent<T>(entity);
        }

        template<typename T>
        ComponentArray<T>& GetComponentArray()
        {
            return aComponentManager->GetComponentArray<T>();
        }

        template<typename T>
        ComponentType GetComponentType()
        {
            return aComponentManager->GetComponentType<T>();
        }

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

        template<typename T>
        std::shared_ptr<T> RegisterSystem()
        {
            return aSystemManager->RegisterSystem<T>();
        }

        template<typename T>
        std::shared_ptr<T> GetSystem()
        {
            return aSystemManager->GetSystem<T>();
        }

        template<typename T>
        void SetSystemSignature(Signature signature)
        {
            aSystemManager->SetSignature<T>(signature);
        }

        Entity DuplicateEntity(Entity src);

        Entity DuplicateEntityHierarchy(Entity src, std::unordered_map<Entity, Entity>& oldToNewMap);

        //------------------------------------------+
        //          Serialization                   |
        //------------------------------------------+

        const char* GetSectionName() const override { return "entities"; };  // e.g. "entities", "resources"
        std::string GetSerializerName() const override { return "coordinator"; };  // e.g. "coordinator", "resources_manager"
        void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) override;
        void Deserialize(const rapidjson::Value& in) override;
        void SerializePrefab(Entity entity, rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator) override;
        Entity DeserializePrefab(const rapidjson::Value& in) override;
        void SerializeEntity(Entity entity, rapidjson::Value& comps, rapidjson::Document::AllocatorType& allocator);
        void DeserializeEntity(Entity entity, const rapidjson::Value& comps);

        // Prefab instance loading
        std::unordered_map<Entity, Entity> LoadPrefabInstance(
            const std::string& prefabPath,
            Entity rootEntityID,
            const rapidjson::Value& transformOverride);


        //------------------------------------------+
        //          State cache                     |
        //------------------------------------------+

        void CacheState();
        void RestoreState();

        //------------------------------------------+
        //          Helper Func                     |
        //------------------------------------------+

        Uma_Engine::EventSystem* GetEventSystem();
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





