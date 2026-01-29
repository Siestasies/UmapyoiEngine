#pragma once

#include "../Core/SystemType.h"

#include "../ECS/Core/Types.hpp"
#include "../ECS/Components/Tilemap.h"
#include "../ECS/Components/Transform.h"

#include "../Systems/ResourcesTypes.hpp"
#include "../Systems/Graphics.hpp"
#include "../Systems/ResourcesManager.hpp"
#include "EventSystem.h"

#include "../ECS/Core/Coordinator.hpp"


#include <imgui.h>
#include <memory>

namespace Uma_Engine
{
    class TilemapEditorManager : public ISystem
    {
    public:


        void Init() override;

        void Update(float dt) override;

        void Shutdown() override;

        void OpenEditor(Entity entity);
        void CloseEditor();

        bool IsEditing() const;

        void HandlesSceneInput(const ImVec2& mouseWorldPos);
        void RenderSceneOverlay(ImDrawList* drawList, const Uma_ECS::Transform& transform, const ImVec2& imagePos);

        int GetSelectedTileIndex() const;
        bool ShouldShowGrid() const;

    private:

        // in future we will make tileset a individual resource type by its own
        //void LoadTileset(const std::string& path); 
        void RenderPaletteWindow();
        void RenderLayersWindow();

        void PlaceTile(int x, int y, int tileIndex);
        void EraseTile(int x, int y);
        void WorldToTileCoords(const ImVec2& worldPos, int& tileX, int& tileY);
        void DrawGridOverlay(ImDrawList* drawList, const Uma_ECS::Transform& transform, const ImVec2& imagePos);
        void DrawTileHighlight(ImDrawList* drawList, const Uma_ECS::Transform& transform, const ImVec2& imagePos);

        Uma_ECS::Entity currEntity;
        Uma_ECS::Tilemap* currTilemap = nullptr;

        // Editor state
        int selectedTileIndex = 0;
        bool showGrid = true;

        // Editor windows visibility
        bool showPalette = false;
        bool showLayers = false;

        // Tileset data
        Uma_ECS::Tileset currentTileset;

        // Systems
        Uma_ECS::Coordinator* pCoordinator = nullptr;
        ResourcesManager* pResourcesManager = nullptr;
        Graphics* pGraphics = nullptr;
        EventSystem* pEventSystem = nullptr;
    };
}