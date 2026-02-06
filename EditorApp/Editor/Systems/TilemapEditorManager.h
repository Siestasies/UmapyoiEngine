/*!
\file   TilemapEditorManager.h
\par    Project: GAM250
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
 * Provides a complete in-editor tilemap authoring tool that allows designers
 * to paint tiles directly in the scene view. Features include:
 * - Tile palette selection with visual preview
 * - Multi-layer editing support
 * - Grid overlay and tile highlighting
 * - Real-time scene interaction
 * - Layer visibility and management controls


All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

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
    /**
     * @class TilemapEditorManager
     * @brief System that manages the in-editor tilemap painting and editing workflow
     *
     * This system provides an interactive tilemap editor that operates within the
     * main engine editor. It handles:
     * - Opening/closing the editor for specific tilemap entities
     * - Rendering the tile palette UI
     * - Processing mouse input for tile placement/erasure
     * - Drawing grid overlays and tile highlights in the scene view
     * - Managing layer selection and visibility
     *
     * The editor maintains a reference to the currently edited tilemap and provides
     * visual feedback through ImGui windows and scene overlays.
     */
    class TilemapEditorManager : public ISystem
    {
    public:

        /**
         * @brief Initialize the tilemap editor system
         *
         * Acquires references to required systems (Coordinator, ResourcesManager,
         * Graphics, EventSystem) and prepares the editor for use.
         */
        void Init() override;

        /**
         * @brief Update the tilemap editor each frame
         *
         * Processes input, updates UI windows, and handles tile painting operations
         * if the editor is currently active.
         *
         * @param dt Delta time in seconds since last frame
         */
        void Update(float dt) override;

        /**
         * @brief Clean up editor resources on shutdown
         *
         * Releases any allocated resources and resets editor state.
         */
        void Shutdown() override;

        /**
         * @brief Open the tilemap editor for a specific entity
         *
         * Activates the editor and binds it to the Tilemap component of the given
         * entity. Opens associated UI windows (palette, layers) and enables
         * scene interaction.
         *
         * @param entity The entity with a Tilemap component to edit
         */
        void OpenEditor(Entity entity);

        /**
         * @brief Close the tilemap editor
         *
         * Deactivates the editor, closes all editor windows, and clears the
         * currently edited tilemap reference. Changes are automatically saved
         * to the component.
         */
        void CloseEditor();

        /**
         * @brief Check if the editor is currently active
         *
         * @return true if editor is open and editing a tilemap, false otherwise
         */
        bool IsEditing() const;

        /**
         * @brief Process mouse input in the scene view for tile painting
         *
         * Handles left-click to place tiles and right-click to erase tiles.
         * Converts mouse world position to tile coordinates and applies changes
         * to the active layer.
         *
         * @param mouseWorldPos Mouse position in world space coordinates
         */
        void HandlesSceneInput(const ImVec2& mouseWorldPos);

        /**
         * @brief Render visual overlays in the scene view
         *
         * Draws grid lines and tile highlights over the tilemap to provide
         * visual feedback during editing. Called by the scene renderer.
         *
         * @param drawList ImGui draw list for rendering overlay graphics
         * @param transform Transform component of the tilemap entity (for position/scale)
         * @param imagePos Screen-space position of the scene view image
         */
        void RenderSceneOverlay(ImDrawList* drawList, const Uma_ECS::Transform& transform, const ImVec2& imagePos);

        /**
         * @brief Get the currently selected tile index from the palette
         *
         * @return Tile index in the tileset (0-based), or -1 if none selected
         */
        int GetSelectedTileIndex() const;

        /**
         * @brief Check if grid overlay is currently enabled
         *
         * @return true if grid should be drawn, false otherwise
         */
        bool ShouldShowGrid() const;

    private:
        /**
         * @brief Render the tile palette window
         *
         * Displays all available tiles from the current tileset as a clickable
         * grid. Users can select tiles to paint with. Updates selectedTileIndex
         * when a tile is clicked.
         */
        // in future we will make tileset a individual resource type by its own
        //void LoadTileset(const std::string& path); 
        void RenderPaletteWindow();

        /**
         * @brief Render the layers management window
         *
         * Shows all layers in the tilemap with controls for:
         * - Selecting the active layer for editing
         * - Toggling layer visibility
         * - Creating/deleting layers
         * - Adjusting render order
         */
        void RenderLayersWindow();

        /**
         * @brief Place a tile at the specified grid coordinates
         *
         * Sets the tile at (x, y) in the active layer to the given tile index.
         * Automatically clamps coordinates to valid tilemap bounds.
         *
         * @param x Tile X coordinate in grid space
         * @param y Tile Y coordinate in grid space
         * @param tileIndex Tile index from tileset to place
         */
        void PlaceTile(int x, int y, int tileIndex);

        /**
         * @brief Erase the tile at the specified grid coordinates
         *
         * Sets the tile at (x, y) in the active layer to -1 (empty).
         * Automatically clamps coordinates to valid tilemap bounds.
         *
         * @param x Tile X coordinate in grid space
         * @param y Tile Y coordinate in grid space
         */
        void EraseTile(int x, int y);

        /**
         * @brief Convert world-space position to tile grid coordinates
         *
         * Transforms a world position (from mouse input) into discrete tile
         * coordinates based on the tilemap's transform and tile size.
         *
         * @param worldPos Position in world space
         * @param tileX Output: Tile X coordinate
         * @param tileY Output: Tile Y coordinate
         */
        void WorldToTileCoords(const ImVec2& worldPos, int& tileX, int& tileY);

        /**
         * @brief Draw grid lines over the tilemap
         *
         * Renders a visual grid showing tile boundaries to aid in precise editing.
         * Grid visibility is controlled by the showGrid flag.
         *
         * @param drawList ImGui draw list for rendering
         * @param transform Tilemap entity's transform (position, scale, rotation)
         * @param imagePos Screen-space position of scene view
         */
        void DrawGridOverlay(ImDrawList* drawList, const Uma_ECS::Transform& transform, const ImVec2& imagePos);

        /**
         * @brief Draw highlight box over the tile under mouse cursor
         *
         * Provides visual feedback showing which tile will be affected by
         * the next paint/erase operation.
         *
         * @param drawList ImGui draw list for rendering
         * @param transform Tilemap entity's transform (position, scale, rotation)
         * @param imagePos Screen-space position of scene view
         */
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