#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>

#include "Math/Math.h"
#include "NavMeshTypes.h"

namespace Uma_Navigation {
    class GridPathfinder {
    private:
        //set of blocked cells
        std::unordered_set<GridCell, GridCellHash> blockedCells;
        float cellSize;

        //helper functions
        GridCell WorldToGrid(const Vec2& worldPos) const;
        Vec2 GridToWorld(const GridCell& cell) const;
        bool IsCellBlocked(const GridCell& cell) const;
        std::vector<GridCell> GetNeighbors(const GridCell& cell) const;

        //a star 
        float Heuristic(const GridCell& a, const GridCell& b) const;
        std::vector<Vec2> ReconstructPath(const std::unordered_map<GridCell, GridCell, GridCellHash>& cameFrom,
            GridCell current, const Vec2& start, const Vec2& goal) const;
        std::unordered_map<GridCell, float, GridCellHash> clearanceValues;

    public:
        //constructor
        explicit GridPathfinder(float cellSize = 2.0f);

        //helper functions
        int GetBlockedCellCount() const { return static_cast<int>(blockedCells.size()); }
        float GetCellSize() const { return cellSize; }
        
        //getter and setter and clearing blocked cells
        const std::unordered_set<GridCell, GridCellHash>& GetBlockedCells() const { return blockedCells; }
        void SetBlockedCells(const std::unordered_set<GridCell, GridCellHash>& blocked);
        void ClearBlockedCells();

        //pathing algos
        std::vector<Vec2> FindPath(const Vec2& start, const Vec2& goal, float agentRadius);
        std::vector<Vec2> SmoothPath(const std::vector<Vec2>& path);
        bool HasLineOfSight(const Vec2& start, const Vec2& end);

        //using bushfire algorithm to calculate distance from obstacles to entities of different size can optimise
        //paths that i can actually cross and not clip walls.
        //doing this because entity is bigger than grid size
        void ComputeClearances(int maxClearanceRadius);
        bool IsTraversable(const GridCell& cell, float agentRadius) const;
        float GetClearance(const GridCell& cell) const;
        float ComputeTrueClearance(const GridCell& cell, int maxRadius) const;
        GridCell FindNearestClearCell(const Vec2& pos, float agentRadius, int maxSearchDist);
    };

}
