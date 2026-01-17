/*!
\file   RenderingSystem.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

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
        void Update(float dt);

        /*!
        \brief Toggles whether this system should update the internal Graphics camera.

        Useful for switching between the Game Camera (runtime) and Editor Camera (tooling).
        \param update True to control the camera, False to leave camera control to the Editor.
        */
        void SetUpdateCamera(bool update) { mUpdateCamera = update; }

    private:

        void RenderWorldPass(float dt);
        void RenderUIPass(float dt);

        void GatherWorldSprites(std::vector<LayeredSprite>& outSprites);
        void RenderWorldSprites(std::vector<LayeredSprite>& sprites);

        void GatherUIElements(std::vector<UIDrawCommand>& outSprites);
        void RenderUIElements(std::vector<UIDrawCommand>& sprites);

        void GetAllChildren(Entity parent, std::vector<Entity>& childrenList);

        Coordinator* pCoordinator = nullptr;
        Uma_Engine::Graphics* pGraphics = nullptr;
        Uma_Engine::ResourcesManager* pResourcesManager = nullptr;

        bool mUpdateCamera = true;
    };
}
