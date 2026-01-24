#pragma once

#include "../Core/SystemType.h"

#include "../ECS/Core/Types.hpp"
#include "../ECS/Components/Tilemap.h"
#include "../ECS/Components/Sprite.h"

#include "../Systems/ResourcesTypes.hpp"
#include "../Systems/Graphics.hpp"
#include "../Systems/ResourcesManager.hpp"

#include "../ECS/Core/Coordinator.hpp"


#include <imgui.h>
#include <memory>

namespace Uma_Engine
{
    class tilemapEditorManager : public ISystem
    {
    public:

        void OpenEditor(Entity entity);
        void CloseEditor();

        void IsEditing() const;

        void Init() override;

        void Update(float dt) override;

        void Shutdown() override;


    private:
        Uma_ECS::Entity currEntity;
        Uma_ECS::Tilemap* currTilemap = nullptr;

        // Editor state
        int selectedTileIndex = 0;
        float zoom = 1.0f;
        ImVec2 cameraOffset = { 0, 0 };
        bool showGrid = true;

        // Editor windows visibility
        bool showCanvas = false;
        bool showPalette = false;
        bool showLayers = false;

        // Tileset data
        //Tileset currentTileset;

        // Systems
        Uma_ECS::Coordinator* pCoordinator;
        ResourcesManager* pResourcesManager;
        Graphics* pGraphics;
    };
}