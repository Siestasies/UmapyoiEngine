#pragma once
/*!
\file   GridPathfinder.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Koh Kai Yang (100%)
\par    E-mail: k.kaiyang@digipen.edu
\par    DigiPen login: k.kaiyang

\brief
This defines the algorithm for entity pathfinding

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>

#include "Math/Math.h"
#include "NavMeshTypes.h"

namespace Uma_Navigation {
    /**
    * @brief Grid-based pathfinding helper using A* and clearance values.
    *
    * Maintains a set of blocked grid cells, converts between world and grid
    * space, runs A* on the grid, and supports clearance-based traversability
    * checks for different agent radii.
    */
    class GridPathfinder {
    private:
        //set of blocked cells
        std::unordered_set<GridCell, GridCellHash> blockedCells;
        float cellSize;

        //helper functions
        /**
        * @brief Convert a world-space position to a grid cell coordinate.
        * @param worldPos World-space position.
        * @return Corresponding grid cell.
        */
        GridCell WorldToGrid(const Vec2& worldPos) const;

        /**
        * @brief Convert a grid cell coordinate to a world-space position.
        * @param cell Grid cell.
        * @return World-space position (typically cell center).
        */
        Vec2 GridToWorld(const GridCell& cell) const;

        /**
        * @brief Check if a given grid cell is blocked.
        * @param cell Grid cell to query.
        * @return true if the cell is blocked, false otherwise.
        */
        bool IsCellBlocked(const GridCell& cell) const;

        /**
        * @brief Get all neighboring cells of a given cell.
        * @param cell Grid cell to expand from.
        * @return List of neighboring cells (4- or 8-connected, depending on implementation).
        */
        std::vector<GridCell> GetNeighbors(const GridCell& cell) const;

        //a star 
        /**
        * @brief Heuristic cost estimate between two grid cells for A*.
        * @param a Start cell.
        * @param b Goal cell.
        * @return Heuristic distance between a and b.
        */
        float Heuristic(const GridCell& a, const GridCell& b) const;

        /**
        * @brief Reconstruct a world-space path from A* predecessor map.
        *
        * @param cameFrom Map from cell to its predecessor in the search.
        * @param current  Goal cell to start backtracking from.
        * @param start    Original world-space start position.
        * @param goal     Original world-space goal position.
        * @return Path as a sequence of world-space positions from start to goal.
        */
        std::vector<Vec2> ReconstructPath(const std::unordered_map<GridCell, GridCell, GridCellHash>& cameFrom,
            GridCell current, const Vec2& start, const Vec2& goal) const;

        // Precomputed clearance values (in cells or world units) for each grid cell.
        std::unordered_map<GridCell, float, GridCellHash> clearanceValues;

    public:
        //constructor
        /**
        * @brief Construct a GridPathfinder with a given cell size.
        * @param cellSize Size of each grid cell in world units (default 2.0f).
        */
        explicit GridPathfinder(float cellSize = 2.0f);

        //helper functions
        // @brief Get current number of blocked cells.
        int GetBlockedCellCount() const { return static_cast<int>(blockedCells.size()); }
        float GetCellSize() const { return cellSize; }
        
        //getter and setter and clearing blocked cells
        /**
        * @brief Get read-only access to the blocked cell set.
        * @return Const reference to the blocked cell set.
        */
        const std::unordered_set<GridCell, GridCellHash>& GetBlockedCells() const { return blockedCells; }

        /**
        * @brief Replace the current blocked cell set.
        * @param blocked New set of blocked grid cells.
        */
        void SetBlockedCells(const std::unordered_set<GridCell, GridCellHash>& blocked);

        /**
        * @brief Clear all blocked cells.
        */
        void ClearBlockedCells();

        //pathing algos
        /**
        * @brief Find a path between two world-space positions for an agent.
        *
        * Runs A* on the grid, taking into account blocked cells and clearance
        * for the given agent radius.
        *
        * @param start       World-space start position.
        * @param goal        World-space goal position.
        * @param agentRadius Agent radius in world units.
        * @return Vector of world-space waypoints from start to goal. Empty if no path.
        */
        std::vector<Vec2> FindPath(const Vec2& start, const Vec2& goal, float agentRadius);

        /**
        * @brief Post-process a raw path to reduce unnecessary waypoints.
        *
        * Typical smoothing may remove collinear points or use line-of-sight
        * checks to shortcut across multiple nodes.
        *
        * @param path Input path in world space.
        * @return Smoothed path in world space.
        */
        std::vector<Vec2> SmoothPath(const std::vector<Vec2>& path);

        /**
        * @brief Test unobstructed line of sight between two world positions.
        *
        * Uses the grid representation and blocked cells to determine whether
        * a straight segment from start to end crosses any obstacles.
        *
        * @param start World-space start position.
        * @param end   World-space end position.
        * @return true if the segment is unobstructed, false otherwise.
        */
        bool HasLineOfSight(const Vec2& start, const Vec2& end);

        //using bushfire algorithm to calculate distance from obstacles to entities of different size can optimise
        //paths that i can actually cross and not clip walls.
        //doing this because entity is bigger than grid size
        /**
        * @brief Precompute clearance values for each grid cell.
        *
        * Uses a bushfire / distance transform to compute minimum distance
        * from obstacles up to a given maximum radius, enabling fast
        * IsTraversable checks for different agent sizes.
        *
        * @param maxClearanceRadius Maximum clearance to compute, in cells or grid units.
        */
        void ComputeClearances(int maxClearanceRadius);

        /**
        * @brief Check if a cell is traversable for an agent of a given radius.
        *
        * Uses precomputed clearance values (if available) or falls back to
        * a direct clearance computation.
        *
        * @param cell        Grid cell to test.
        * @param agentRadius Agent radius in world units.
        * @return true if the agent can occupy this cell without colliding with obstacles.
        */
        bool IsTraversable(const GridCell& cell, float agentRadius) const;

        /**
        * @brief Get precomputed clearance value for a grid cell.
        * @param cell Grid cell to query.
        * @return Clearance value (0 if not found or blocked).
        */
        float GetClearance(const GridCell& cell) const;

        /**
        * @brief Compute exact clearance around a cell up to a max radius.
        *
        * Typically used internally to populate or refine clearanceValues.
        *
        * @param cell      Center grid cell.
        * @param maxRadius Maximum radius to check, in cells.
        * @return True clearance for the cell (in cells or world units, depending on implementation).
        */
        float ComputeTrueClearance(const GridCell& cell, int maxRadius) const;

        /**
        * @brief Find the nearest traversable cell near a world position.
        *
        * Searches outward from pos up to a given maximum search distance
        * for a cell that can be traversed by an agent of the given radius.
        *
        * @param pos            World-space query position.
        * @param agentRadius    Agent radius in world units.
        * @param maxSearchDist  Maximum search distance in cells.
        * @return Closest traversable cell, or some sentinel (e.g. invalid cell) if none found.
        */
        GridCell FindNearestClearCell(const Vec2& pos, float agentRadius, int maxSearchDist);
    };

}
