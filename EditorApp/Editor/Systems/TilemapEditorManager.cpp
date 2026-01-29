#include "TilemapEditorManager.h"
#include "../Systems/SceneManager.h"

#include "../Core/FilePaths.h"

#include "Events/IMGUIEvents.h"

namespace Uma_Engine
{
    void TilemapEditorManager::OpenEditor(Entity entity)
    {
        // neeed to get the coordinator
        auto sceneManager = pSystemManager->GetSystem<SceneManager>();
        if (sceneManager)
        {
            pCoordinator = &sceneManager->GetActiveScene()->GetCoordinator();
        }

        currEntity = entity;
        currTilemap = &pCoordinator->GetComponent<Uma_ECS::Tilemap>(entity);

        currTilemap->isInEditMode = true;

        // open all editor window
        showLayers = true;
        showPalette = true;

        if (!currentTileset.IsLoaded())
        {
            currentTileset.Load(pResourcesManager);
        }
    }

    void TilemapEditorManager::CloseEditor()
    {
        currTilemap->isInEditMode = false;

        showPalette = false;
        showLayers = false;
        currTilemap = nullptr;
        pCoordinator = nullptr;
        //currEntity = static_cast<Entity>(-1);
    }

    bool TilemapEditorManager::IsEditing() const
    {
        return currTilemap != nullptr;
    }

    void TilemapEditorManager::HandlesSceneInput(const ImVec2& mouseWorldPos)
    {
        if (!currTilemap) return;

        // Convert world position to tile coordinates
        int tileX, tileY;
        WorldToTileCoords(mouseWorldPos, tileX, tileY);

        if (tileX == -1 || tileY == -1) return;

        // Handle mouse input
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) 
        {
            PlaceTile(tileX, tileY, selectedTileIndex);
        }
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) 
        {
            EraseTile(tileX, tileY);
        }
    }

    void TilemapEditorManager::RenderSceneOverlay(ImDrawList* drawList, const Uma_ECS::Transform& transform, const ImVec2& imagePos)
    {
        if (!currTilemap || !showGrid) return;

        DrawGridOverlay(drawList, transform, imagePos);
        DrawTileHighlight(drawList, transform, imagePos);
    }

    int TilemapEditorManager::GetSelectedTileIndex() const
    {
        return selectedTileIndex;
    }

    bool TilemapEditorManager::ShouldShowGrid() const
    {
        return showGrid;
    }

    void TilemapEditorManager::Init()
    {
        pGraphics = pSystemManager->GetSystem<Graphics>();
        pResourcesManager = pSystemManager->GetSystem<ResourcesManager>();
        pEventSystem = pSystemManager->GetSystem<EventSystem>();

        pEventSystem->Subscribe<PlaySceneRequest, ImguiManager>([this](const PlaySceneRequest& e)
            {
                (void)e;

                //stop everything
                if (IsEditing())
                {
                    CloseEditor();
                }

            });
    }

    void TilemapEditorManager::Update(float dt)
    {
        if (!currTilemap) return;

        if (currEntity == static_cast<Entity>(-1))
        {
            CloseEditor();
            return;
        }

        if (showPalette) RenderPaletteWindow();
        if (showLayers) RenderLayersWindow();
    }

    void TilemapEditorManager::Shutdown()
    {
        pEventSystem->UnsubscribeSystem<TilemapEditorManager>();
    }

    void TilemapEditorManager::RenderPaletteWindow()
    {
        ImGui::Begin("Tile Palette", &showPalette);
        if (!currTilemap->tileset.IsLoaded())
        {
            ImGui::End();
            return;
        }

        // Display tileset info
        ImGui::Text("Tileset: %s", currTilemap->tileset.texturePath.c_str());
        ImGui::Text("Tile Size: %dx%d", currTilemap->tileSize, currTilemap->tileSize);
        ImGui::Separator();

        // Tile preview size slider
        static float previewSize = 32.0f;
        ImGui::SliderFloat("Preview Size", &previewSize, 16.0f, 64.0f);

        ImGui::BeginChild("TileGrid", ImVec2(0, 0), true);

        // **KEY FIX: Push style vars to remove padding/spacing for tiles**
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 startPos = ImGui::GetCursorScreenPos();
        float padding = 2.0f;

        int tilesPerRow = (int)((ImGui::GetContentRegionAvail().x - padding) / (previewSize + padding));
        if (tilesPerRow < 1) tilesPerRow = 1;

        int tileCount = currTilemap->tileset.GetTileCount();

        for (int i = 0; i < tileCount; i++) {
            int col = i % tilesPerRow;
            int row = i / tilesPerRow;

            ImVec2 tilePos = ImVec2(
                startPos.x + col * (previewSize + padding) + padding,
                startPos.y + row * (previewSize + padding) + padding
            );

            // Get UV coordinates
            float u0, v0, u1, v1;
            currTilemap->tileset.GetUVs(i, u0, v0, u1, v1);

            // Draw tile
            ImGui::SetCursorScreenPos(tilePos);
            ImGui::PushID(i);

            char buttonID[32];
            snprintf(buttonID, sizeof(buttonID), "tile_%d", i);

            if (ImGui::ImageButton(
                buttonID,
                currTilemap->tileset.texture->tex_id,
                ImVec2(previewSize, previewSize),
                ImVec2(u0, v0),
                ImVec2(u1, v1),
                ImVec4(0, 0, 0, 0),  // bg_col
                ImVec4(1, 1, 1, 1)   // tint_col
            )) {
                selectedTileIndex = i;
            }

            ImGui::PopID();

            // Highlight selected tile
            if (selectedTileIndex == i) {
                drawList->AddRect(
                    tilePos,
                    ImVec2(tilePos.x + previewSize, tilePos.y + previewSize),
                    IM_COL32(255, 255, 0, 255),
                    0.0f, 0, 3.0f
                );
            }

            // Tooltip
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Tile #%d\nClick to select", i);
            }
        }

        // **Don't forget to pop the style vars!**
        ImGui::PopStyleVar(2);

        ImGui::EndChild();
        ImGui::End();
    }

    void TilemapEditorManager::RenderLayersWindow()
    {
        ImGui::Begin("Layers", &showLayers);

        // Grid toggle
        if (currTilemap->activeLayerIndex < currTilemap->layerNames.size())
        {
            ImGui::Text("Curr layer: %s", currTilemap->layerNames[currTilemap->activeLayerIndex].c_str());
        }
        ImGui::Checkbox("Show Grid", &showGrid);

        ImGui::Separator();

        // Layer controls
        if (ImGui::Button("+ Add Layer")) {
            std::string name = "Layer " + std::to_string(currTilemap->layers.size() + 1);
            currTilemap->CreateLayer(name, currTilemap->mapWidth, currTilemap->mapHeight, 0);
        }
        ImGui::SameLine();
        if (ImGui::Button("- Remove")) {
            if (currTilemap->activeLayerIndex >= 0 && currTilemap->layers.size() > 1) {
                currTilemap->RemoveLayer(currTilemap->activeLayerIndex);

                if (currTilemap->activeLayerIndex - 1 >= 0)
                {
                    --currTilemap->activeLayerIndex;
                }
            }
        }

        ImGui::Separator();

        // List layers (reverse order, top layers first)
        for (int i = static_cast<int>(currTilemap->layers.size()) - 1; i >= 0; i--)
        {
            ImGui::PushID(i);

            bool isActive = (i == currTilemap->activeLayerIndex);

            // Visibility toggle FIRST
            bool visible = currTilemap->layerVisibility[i];
            if (ImGui::Checkbox("##vis", &visible)) {
                currTilemap->layerVisibility[i] = visible;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Toggle visibility");
            }

            //ImGui::SameLine();

            //// Lock toggle SECOND
            //bool locked = currTilemap->layers[i].locked;
            //if (ImGui::Checkbox("##lock", &locked)) 
            //{
            //    currTilemap->layers[i].locked = locked;
            //}
            //if (ImGui::IsItemHovered()) 
            //{
            //    ImGui::SetTooltip("Toggle lock");
            //}

            ImGui::SameLine();

            // Layer button LAST - takes remaining width
            if (isActive) 
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.65f, 0.2f, 0.5f));
            }

            ImGui::SameLine();
            // Calculate button width manually to leave space for the input and text
            float buttonWidth = ImGui::GetContentRegionAvail().x * 0.5f; // Adjust 150 based on your needs

            if (ImGui::Button(currTilemap->layerNames[i].c_str(), ImVec2(buttonWidth, 0)))
            {
                currTilemap->activeLayerIndex = i;
            }

            if (isActive)
            {
                ImGui::PopStyleColor();
            }

            ImGui::SameLine();
            ImGui::SetNextItemWidth(100.0f); // Set explicit width for InputInt
            ImGui::InputInt(
                "##sorting order",
                &currTilemap->layers[i].renderOrder
            );

            ImGui::SameLine();
            ImGui::Text("Render Order");

            ImGui::PopID();
        }

        ImGui::End();
    }

    void TilemapEditorManager::PlaceTile(int x, int y, int tileIndex)
    {
        Uma_ECS::TileLayer& currLayer = currTilemap->layers[currTilemap->activeLayerIndex];
        int tileToPlace = x + y * currLayer.width;

        currLayer.tiles[tileToPlace] = tileIndex;
    }

    void TilemapEditorManager::EraseTile(int x, int y)
    {
        Uma_ECS::TileLayer& currLayer = currTilemap->layers[currTilemap->activeLayerIndex];
        int tileToPlace = x + y * currLayer.width;

        currLayer.tiles[tileToPlace] = -1;
    }

    void TilemapEditorManager::WorldToTileCoords(const ImVec2& worldPos, int& tileX, int& tileY)
    {
        auto& transform = pCoordinator->GetComponent<Uma_ECS::Transform>(currEntity);

        float tile_size_x = currTilemap->tileSize * transform.scale.x;
        float tile_size_y = currTilemap->tileSize * transform.scale.y;
        float half_tile_size_x = tile_size_x * 0.5f;
        float half_tile_size_y = tile_size_y * 0.5f;

        // Calculate bounds
        float minX = transform.worldPosition.x - half_tile_size_x;
        float maxX = transform.worldPosition.x + (currTilemap->mapWidth * tile_size_x) + half_tile_size_x;
        float minY = transform.worldPosition.y - (currTilemap->mapHeight * tile_size_y) - half_tile_size_y;  // Y goes down, so min is negative
        float maxY = transform.worldPosition.y + half_tile_size_y;

        // Check if out of bounds
        if (worldPos.x < minX || worldPos.x > maxX ||
            worldPos.y < minY || worldPos.y > maxY)
        {
            // out of range
            tileX = -1;
            tileY = -1;

            return;
        }

        Vec2 rel_offset{worldPos.x - transform.worldPosition.x, worldPos.y - transform.worldPosition.y};

        rel_offset.y = std::abs(rel_offset.y);

        tileX = static_cast<int>(std::round(rel_offset.x / tile_size_x));
        tileY = static_cast<int>(std::round(rel_offset.y / tile_size_y));

        std::string log = "tilemap pos : " + std::to_string(tileX) + " " + std::to_string(tileY);
        Debugger::Log(WarningLevel::eInfo, log);
    }

    void TilemapEditorManager::DrawGridOverlay(ImDrawList* drawList, const Uma_ECS::Transform& transform, const ImVec2& imagePos)
    {
        float tile_size_x = currTilemap->tileSize * transform.scale.x;
        float tile_size_y = currTilemap->tileSize * transform.scale.y;
        float half_tile_size_x = tile_size_x * 0.5f;
        float half_tile_size_y = tile_size_y * 0.5f;

        float minX = transform.worldPosition.x - half_tile_size_x;
        float maxX = transform.worldPosition.x + (currTilemap->mapWidth - 1) * tile_size_x + half_tile_size_x;
        float minY = transform.worldPosition.y - (currTilemap->mapHeight - 1) * tile_size_y - half_tile_size_y;
        float maxY = transform.worldPosition.y + half_tile_size_y;

        Vec2 screenMin = pGraphics->WorldToScreen(Vec2{ minX, maxY });
        Vec2 screenMax = pGraphics->WorldToScreen(Vec2{ maxX, minY });

        float tile_screen_x = (screenMax.x - screenMin.x) / currTilemap->layers[currTilemap->activeLayerIndex].width;
        float tile_screen_y = (screenMax.y - screenMin.y) / currTilemap->layers[currTilemap->activeLayerIndex].height;

        // vert
        for (int i = 0; i <= currTilemap->layers[currTilemap->activeLayerIndex].width; i++)
        {
            drawList->AddLine(
                ImVec2{ screenMin.x + imagePos.x + (i * tile_screen_x), screenMin.y + imagePos.y },
                ImVec2{ screenMin.x + imagePos.x + (i * tile_screen_x), screenMax.y + imagePos.y },
                IM_COL32(255, 255, 255, 80), 1.f
            );
        }

        for (int i = 0; i <= currTilemap->layers[currTilemap->activeLayerIndex].height; i++)
        {
            drawList->AddLine(
                ImVec2{ screenMin.x + imagePos.x, screenMax.y + imagePos.y - (i * tile_screen_y)},
                ImVec2{ screenMax.x + imagePos.x, screenMax.y + imagePos.y - (i * tile_screen_y)},
                IM_COL32(255, 255, 255, 80), 1.f
            );
        }
    }

    void TilemapEditorManager::DrawTileHighlight(ImDrawList* drawList, const Uma_ECS::Transform& transform, const ImVec2& imagePos)
    {
        ImVec2 mousePos = ImGui::GetMousePos();
        float localX = mousePos.x - imagePos.x;
        float localY = mousePos.y - imagePos.y;

        Vec2 worldPos = pGraphics->ScreenToWorld(Vec2(localX, localY));
        int tileX, tileY;
        WorldToTileCoords(ImVec2(worldPos.x, worldPos.y), tileX, tileY);

        if (tileX == -1 || tileY == -1) return;

        float tile_size_x = currTilemap->tileSize * transform.scale.x;
        float tile_size_y = currTilemap->tileSize * transform.scale.y;

        Vec2 selectedTileWorldPosMin = { transform.worldPosition.x + (tileX * tile_size_x) - (tile_size_x * 0.5f), transform.worldPosition.y - (tileY * tile_size_y) + (tile_size_y * 0.5f) };
        Vec2 selectedTileWorldPosMax = { transform.worldPosition.x + (tileX * tile_size_x) + (tile_size_x * 0.5f), transform.worldPosition.y - (tileY * tile_size_y) - (tile_size_y * 0.5f) };
        Vec2 selectedTileScreenPosMin = pGraphics->WorldToScreen(selectedTileWorldPosMin);
        Vec2 selectedTileScreenPosMax = pGraphics->WorldToScreen(selectedTileWorldPosMax);

        // Get UV coordinates for the selected palette tile
        float u0, v0, u1, v1;
        currTilemap->tileset.GetUVs(selectedTileIndex, u0, v0, u1, v1);

        drawList->AddImage(
            currTilemap->tileset.texture->tex_id,
            ImVec2(selectedTileScreenPosMin.x + imagePos.x, selectedTileScreenPosMin.y + imagePos.y),
            ImVec2(selectedTileScreenPosMax.x + imagePos.x, selectedTileScreenPosMax.y + imagePos.y),
            ImVec2(u0, v0),
            ImVec2(u1, v1),
            IM_COL32(255, 255, 255, 180)
        );
    }
}
