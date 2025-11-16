#include "GridPathfinder.hpp"
#include <iostream>
#include <algorithm>

namespace Uma_Navigation {

    GridPathfinder::GridPathfinder(float cellSize)
        : cellSize(cellSize) {
    }

    GridCell GridPathfinder::WorldToGrid(const Vec2& worldPos) const {
        return GridCell{
            static_cast<int>(std::floor(worldPos.x / cellSize)),
            static_cast<int>(std::floor(worldPos.y / cellSize))
        };
    }

    Vec2 GridPathfinder::GridToWorld(const GridCell& cell) const {
        return Vec2(
            (cell.x + 0.5f) * cellSize,
            (cell.y + 0.5f) * cellSize
        );
    }

    bool GridPathfinder::IsCellBlocked(const GridCell& cell) const {
        return blockedCells.count(cell) > 0;
    }

    float GridPathfinder::Heuristic(const GridCell& a, const GridCell& b) const {
        // Euclidean distance for better diagonal handling
        int dx = std::abs(a.x - b.x);
        int dy = std::abs(a.y - b.y);
        return std::sqrt(static_cast<float>(dx * dx + dy * dy));
    }

    std::vector<GridCell> GridPathfinder::GetNeighbors(const GridCell& cell) const {
        // 8-directional movement
        return {
            {cell.x - 1, cell.y},     // Left
            {cell.x + 1, cell.y},     // Right
            {cell.x, cell.y - 1},     // Down
            {cell.x, cell.y + 1},     // Up
            {cell.x - 1, cell.y - 1}, // Bottom-left
            {cell.x + 1, cell.y - 1}, // Bottom-right
            {cell.x - 1, cell.y + 1}, // Top-left
            {cell.x + 1, cell.y + 1}  // Top-right
        };
    }

    std::vector<Vec2> GridPathfinder::ReconstructPath(
        const std::unordered_map<GridCell, GridCell, GridCellHash>& cameFrom,
        GridCell current, const Vec2& start, const Vec2& goal) const
    {
        std::vector<Vec2> path;
        path.push_back(goal);

        while (cameFrom.count(current)) {
            current = cameFrom.at(current);
            path.push_back(GridToWorld(current));
        }

        path.push_back(start);
        std::reverse(path.begin(), path.end());

        return path;
    }

    void GridPathfinder::SetBlockedCells(const std::unordered_set<GridCell, GridCellHash>& blocked) {
        blockedCells = blocked;
    }

    void GridPathfinder::ClearBlockedCells() {
        blockedCells.clear();
    }

    std::vector<Vec2> GridPathfinder::FindPath(const Vec2& start, const Vec2& goal) {
        GridCell startCell = WorldToGrid(start);
        GridCell goalCell = WorldToGrid(goal);

        // Check if start or goal is blocked
        if (IsCellBlocked(startCell)) {
            std::cout << "[GridPathfinder] Start is blocked!" << std::endl;
            return {};
        }
        if (IsCellBlocked(goalCell)) {
            std::cout << "[GridPathfinder] Goal is blocked!" << std::endl;
            return {};
        }

        std::priority_queue<PathNode, std::vector<PathNode>, std::greater<PathNode>> openSet;
        std::unordered_map<GridCell, float, GridCellHash> gScore;
        std::unordered_map<GridCell, GridCell, GridCellHash> cameFrom;
        std::unordered_set<GridCell, GridCellHash> closedSet;

        gScore[startCell] = 0;
        openSet.push({ startCell, 0, Heuristic(startCell, goalCell) });

        int iterations = 0;
        const int maxIterations = 10000;

        while (!openSet.empty() && iterations++ < maxIterations) {
            PathNode current = openSet.top();
            openSet.pop();

            if (current.cell == goalCell) {
                std::cout << "[GridPathfinder] Path found (" << iterations << " iterations)" << std::endl;
                return ReconstructPath(cameFrom, current.cell, start, goal);
            }

            if (closedSet.count(current.cell)) continue;
            closedSet.insert(current.cell);

            for (const GridCell& neighbor : GetNeighbors(current.cell)) {
                if (IsCellBlocked(neighbor)) continue;
                if (closedSet.count(neighbor)) continue;

                // Cost: 1 for orthogonal, 1.414 for diagonal
                float moveCost = (std::abs(neighbor.x - current.cell.x) + std::abs(neighbor.y - current.cell.y) == 1)
                    ? 1.0f : 1.414f;
                float tentativeG = gScore[current.cell] + moveCost;

                if (!gScore.count(neighbor) || tentativeG < gScore[neighbor]) {
                    cameFrom[neighbor] = current.cell;
                    gScore[neighbor] = tentativeG;
                    float fScore = tentativeG + Heuristic(neighbor, goalCell);
                    openSet.push({ neighbor, tentativeG, fScore });
                }
            }
        }

        std::cout << "[GridPathfinder] No path found (" << iterations << " iterations)" << std::endl;
        return {};
    }

}
