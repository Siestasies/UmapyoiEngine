/*!
\file   RenderingSystem.hpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines rendering system that orchestrates sprite drawing through graphics API with texture batching optimization.

Operates on entities with SpriteRenderer and Transform components to extract sprite data and world positions.
Requires initialization with Graphics renderer, ResourcesManager for texture loading, and Coordinator for component queries.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"

#include "../Systems/Graphics.hpp"
#include "../Systems/ResourcesManager.hpp"
#include "../Components/SpriteMaterial.h"


namespace Uma_ECS
{
    // Structure to hold sprite info WITH layer information
    struct LayeredSprite
    {
        Uma_Engine::Sprite_Info info;
        LayerMask layer;
        int order;
        int hierarchyOrder;
        unsigned int texId;
        Entity entityId;

        // Material override (optional)
        unsigned int shaderId{ 0 };
        const std::unordered_map<std::string, Uma_ECS::MaterialValue>* properties = nullptr;
        const std::vector<Uma_Engine::UniformInfo>* uniforms = nullptr;
    };

    struct tilesetLayerInfo
    {
        Entity tilemapId;
        int index;
        int sortingOrder;
        int hierarchyOrder;
    };

    struct UIDrawCommand
    {
        enum Type
        {
            UI_TEXT,
            UI_IMAGE
        };

        Type type;
        LayerMask layer;                // layer
        int order;
        int hierarchyOrder;
        Entity entity;

        // For images
        Uma_Engine::Sprite_Info spriteInfo;

        // For text
        std::string text;
        Uma_Engine::FontData* font;
        float fontSize;
        Uma_UI::TextAlignment alignment;
        Uma_UI::Color textColor;
    };

    class RenderingSystem : public ECSSystem
    {
    public:

        /*!
        \brief Initializes the rendering system with required external dependencies.
        \param g Pointer to the low-level Graphics engine.
        \param rm Pointer to the ResourcesManager for asset retrieval.
        \param c Pointer to the ECS Coordinator.
        */
        void Init(Uma_Engine::Graphics* g, Uma_Engine::ResourcesManager* rm, Coordinator* c);

        /*!
        \brief Executes the rendering pipeline for the current frame.

        Collects sprite data, applies culling/checks, sorts by layer/texture, and issues draw calls.
        \param dt Delta time (unused for rendering, but required by ISystem interface).
        */
        void Update(float dt, bool enableCullMode = false);

        /*!
        \brief Toggles whether this system should update the internal Graphics camera.

        Useful for switching between the Game Camera (runtime) and Editor Camera (tooling).
        \param update True to control the camera, False to leave camera control to the Editor.
        */
        void SetUpdateCamera(bool update) { mUpdateCamera = update; }

    private:

        /*!
        \brief Renders all world-space sprites for the current frame.
        \param dt Delta time in seconds.
        */
        void RenderWorldPass(float dt);

        /*!
        \brief Renders all UI elements for the current frame.
        \param dt Delta time in seconds.
        */
        void RenderUIPass(float dt);

        /*!
        \brief Collects all visible world-space sprites from entities into a list.
        \param outSprites Output vector to populate with layered sprite data.
        */
        void GatherWorldSprites(std::vector<LayeredSprite>& outSprites);

        /*!
        \brief Sorts and draws the collected world-space sprites.
        \param sprites Vector of layered sprites to render.
        */
        void RenderWorldSprites(std::vector<LayeredSprite>& sprites);

        /*!
        \brief Collects all UI draw commands from UI entities into a list.
        \param outSprites Output vector to populate with UI draw commands.
        */
        void GatherUIElements(std::vector<UIDrawCommand>& outSprites);

        /*!
        \brief Sorts and draws the collected UI elements.
        \param sprites Vector of UI draw commands to render.
        */
        void RenderUIElements(std::vector<UIDrawCommand>& sprites);

        /*!
        \brief Recursively collects all child entities of a parent entity.
        \param parent Parent entity to query.
        \param childrenList Output vector to populate with child entity IDs.
        */
        void GetAllChildren(Entity parent, std::vector<Entity>& childrenList);

        /*!
        \brief Determines whether a sprite is within the camera's visible bounds.
        \param spritePos World-space position of the sprite.
        \param spriteScale Scale of the sprite.
        \param camMin Minimum corner of the camera's visible area.
        \param camMax Maximum corner of the camera's visible area.
        \return True if the sprite is at least partially visible.
        */
        bool IsSpriteVisible(const Vec2& spritePos, const Vec2& spriteScale,
                             const Vec2& camMin, const Vec2& camMax);

        Coordinator* pCoordinator = nullptr;
        Uma_Engine::Graphics* pGraphics = nullptr;
        Uma_Engine::ResourcesManager* pResourcesManager = nullptr;

        bool mUpdateCamera = false;

        bool isCullMode = false;
    };
}
