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
        std::unordered_set<GridCell, GridCellHash> blockedCells;
        float cellSize;

        GridCell WorldToGrid(const Vec2& worldPos) const;
        Vec2 GridToWorld(const GridCell& cell) const;
        bool IsCellBlocked(const GridCell& cell) const;
        float Heuristic(const GridCell& a, const GridCell& b) const;
        std::vector<GridCell> GetNeighbors(const GridCell& cell) const;
        std::vector<Vec2> ReconstructPath(
            const std::unordered_map<GridCell, GridCell, GridCellHash>& cameFrom,
            GridCell current, const Vec2& start, const Vec2& goal) const;

    public:
        explicit GridPathfinder(float cellSize = 2.0f);

        void SetBlockedCells(const std::unordered_set<GridCell, GridCellHash>& blocked);
        void ClearBlockedCells();

        std::vector<Vec2> FindPath(const Vec2& start, const Vec2& goal);

        int GetBlockedCellCount() const { return static_cast<int>(blockedCells.size()); }
        float GetCellSize() const { return cellSize; }

        const std::unordered_set<GridCell, GridCellHash>& GetBlockedCells() const { return blockedCells; }
    };

}
