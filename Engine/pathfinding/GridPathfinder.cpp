/*!
\file   GridPathfinder.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Koh Kai Yang (100%)
\par    E-mail: k.kaiyang@digipen.edu
\par    DigiPen login: k.kaiyang

\brief
This implements the algorithm for entity pathfinding

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "GridPathfinder.hpp"
#include <iostream>
#include <algorithm>
#include <limits>

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

    std::vector<Vec2> GridPathfinder::ReconstructPath(const std::unordered_map<GridCell, GridCell, GridCellHash>& cameFrom,
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

    std::vector<Vec2> GridPathfinder::FindPath(const Vec2& start, const Vec2& goal, float agentRadius) {
        GridCell startCell = WorldToGrid(start);
        GridCell goalCell = WorldToGrid(goal);

        /*float startClearance = GetClearance(startCell);
        std::cout << "[DEBUG] Start cell " << startCell.x << "," << startCell.y
            << " clearance=" << startClearance
            << ", required agentRadius=" << agentRadius << std::endl;

        float goalClearance = GetClearance(goalCell);
        std::cout << "[DEBUG] Goal cell " << goalCell.x << "," << goalCell.y
            << " clearance=" << goalClearance
            << ", required agentRadius=" << agentRadius << std::endl;*/

        // Find nearest traversable cells for A* search
        GridCell validStart = FindNearestClearCell(start, agentRadius, /*maxSearchDist=*/50);
        /*if (!IsTraversable(validStart, agentRadius)) {
            std::cout << "[GridPathfinder] No clear cell near start!\n";
            return {};
        }*/
        GridCell validGoal = FindNearestClearCell(goal, agentRadius, /*maxSearchDist=*/50);
        /*if (!IsTraversable(validGoal, agentRadius)) {
            std::cout << "[GridPathfinder] No clear cell near goal!\n";
            return {};
        }*/

        std::priority_queue<PathNode, std::vector<PathNode>, std::greater<PathNode>> openSet;
        std::unordered_map<GridCell, float, GridCellHash> gScore;
        std::unordered_map<GridCell, GridCell, GridCellHash> cameFrom;
        std::unordered_set<GridCell, GridCellHash> closedSet;

        // Use validStart and validGoal here — not startCell/goalCell!
        gScore[validStart] = 0;
        openSet.push({ validStart, 0, Heuristic(validStart, validGoal) });

        int iterations = 0;
        const int maxIterations = 10000;

        while (!openSet.empty() && iterations++ < maxIterations) {
            PathNode current = openSet.top();
            openSet.pop();

            if (current.cell == validGoal) {
                //std::cout << "[GridPathfinder] Path found (" << iterations << " iterations)" << std::endl;

                auto path = ReconstructPath(cameFrom, current.cell, GridToWorld(validStart), GridToWorld(validGoal));
                return path;
            }


            if (closedSet.count(current.cell)) continue;
            closedSet.insert(current.cell);

            for (const GridCell& neighbor : GetNeighbors(current.cell)) {
                if (!IsTraversable(neighbor, agentRadius)) continue;
                if (closedSet.count(neighbor)) continue;

                // Diagonal corner-cutting prevention
                int dx = neighbor.x - current.cell.x;
                int dy = neighbor.y - current.cell.y;
                if (dx != 0 && dy != 0) {
                    GridCell cardinal1{ current.cell.x + dx, current.cell.y };
                    GridCell cardinal2{ current.cell.x, current.cell.y + dy };
                    if (!IsTraversable(cardinal1, agentRadius) || !IsTraversable(cardinal2, agentRadius)) {
                        continue;
                    }
                }

                float moveCost = (dx == 0 || dy == 0) ? 1.0f : 1.414f;
                float tentativeG = gScore[current.cell] + moveCost;

                if (!gScore.count(neighbor) || tentativeG < gScore[neighbor]) {
                    cameFrom[neighbor] = current.cell;
                    gScore[neighbor] = tentativeG;
                    float fScore = tentativeG + Heuristic(neighbor, validGoal);
                    openSet.push({ neighbor, tentativeG, fScore });
                }
            }
        }

        //std::cout << "[GridPathfinder] No path found (" << iterations << " iterations)" << std::endl;
        return {};
    }



    bool GridPathfinder::HasLineOfSight(const Vec2& start, const Vec2& end)
    {
        int x0 = static_cast<int>(std::floor(start.x / cellSize));
        int y0 = static_cast<int>(std::floor(start.y / cellSize));
        int x1 = static_cast<int>(std::floor(end.x / cellSize));
        int y1 = static_cast<int>(std::floor(end.y / cellSize));

        int dx = std::abs(x1 - x0);
        int dy = std::abs(y1 - y0);
        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;
        int err = dx - dy;

        while (true) {
            if (blockedCells.find({ x0, y0 }) != blockedCells.end()) {
                return false;
            }

            // Safety margin check
            if (blockedCells.find({ x0 + 1, y0 }) != blockedCells.end() ||
                blockedCells.find({ x0 - 1, y0 }) != blockedCells.end() ||
                blockedCells.find({ x0, y0 + 1 }) != blockedCells.end() ||
                blockedCells.find({ x0, y0 - 1 }) != blockedCells.end()) {
                return false;
            }

            if (x0 == x1 && y0 == y1) break;

            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x0 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y0 += sy;
            }
        }

        return true;
    }

    void GridPathfinder::ComputeClearances(int maxClearanceRadius) {
        clearanceValues.clear();

        std::queue<GridCell> frontier;
        std::unordered_map<GridCell, int, GridCellHash> distanceMap;

        // Initialize blocked cells
        for (const auto& cell : blockedCells) {
            distanceMap[cell] = 0;
            clearanceValues[cell] = 0.0f;
            frontier.push(cell);
        }

        // BFS with distance limit
        while (!frontier.empty()) {
            GridCell current = frontier.front();
            frontier.pop();
            int currentDist = distanceMap[current];

            // Stop expanding if we've reached the maximum clearance
            if (currentDist >= maxClearanceRadius) {
                continue;
            }

            for (const GridCell& neighbor : GetNeighbors(current)) {
                if (IsCellBlocked(neighbor)) continue;
                if (distanceMap.count(neighbor)) continue;

                int newDist = currentDist + 1;
                distanceMap[neighbor] = newDist;

                // Clamp clearance to maximum
                float clearance = std::min(newDist * cellSize,
                    maxClearanceRadius * cellSize);
                clearanceValues[neighbor] = clearance;

                frontier.push(neighbor);
            }
        }
    }



    // Returns the minimum distance (in world units) to any blocked cell
    float GridPathfinder::ComputeTrueClearance(const GridCell& cell, int maxRadius) const {
        // Agent requires a square of (2R+1) cells to be fully traversable
        int maxClear = maxRadius;
        for (int r = 1; r <= maxClear; ++r) {
            for (int dy = -r; dy <= r; ++dy) {
                for (int dx = -r; dx <= r; ++dx) {
                    // Only check the boundary of the current square expansion
                    if (std::abs(dx) != r && std::abs(dy) != r) continue;
                    GridCell check{ cell.x + dx, cell.y + dy };
                    if (IsCellBlocked(check)) {
                        // Return clearance in world units
                        return static_cast<float>(r) * cellSize;
                    }
                }
            }
        }
        return static_cast<float>(maxClear) * cellSize; // Max clearance in open space
    }

    GridCell GridPathfinder::FindNearestClearCell(const Vec2& pos, float agentRadius, int maxSearchDist)
    {
        GridCell center = WorldToGrid(pos);

        // Check center first
        if (IsTraversable(center, agentRadius)) {
            return center;
        }

        // BFS to find nearest traversable cell
        std::queue<GridCell> q;
        std::unordered_set<GridCell, GridCellHash> visited;

        q.push(center);
        visited.insert(center);

        while (!q.empty()) {
            GridCell current = q.front();
            q.pop();

            // Calculate distance from center
            int dx = current.x - center.x;
            int dy = current.y - center.y;
            int distSquared = dx * dx + dy * dy;

            // Skip if beyond search radius, but continue with other cells
            if (distSquared > maxSearchDist * maxSearchDist) {
                continue;
            }

            // Check if current cell is traversable (skip center since we checked it)
            if (current != center && IsTraversable(current, agentRadius)) {
                return current;  // Found a valid cell!
            }

            // Expand to neighbors
            for (const GridCell& neighbor : GetNeighbors(current)) {
                if (visited.count(neighbor)) continue;

                // Check if neighbor is within search radius before adding
                int ndx = neighbor.x - center.x;
                int ndy = neighbor.y - center.y;
                int nDistSquared = ndx * ndx + ndy * ndy;

                if (nDistSquared > maxSearchDist * maxSearchDist) continue;

                visited.insert(neighbor);
                q.push(neighbor);
            }
        }

        //PROBLEM: No valid cell found within radius
        // This means the search radius is too small or area is completely blocked
        /*std::cout << "[FindNearestClearCell] WARNING: No traversable cell found within radius "
            << maxSearchDist << " cells for agent radius " << agentRadius << std::endl;*/
        return center;  // Return center as last resort
    }

    //returns true if that cell can accomadate the agents size
    bool GridPathfinder::IsTraversable(const GridCell& cell, float agentRadius) const
    {
        return !IsCellBlocked(cell) && GetClearance(cell) >= agentRadius;
    }

    float GridPathfinder::GetClearance(const GridCell& cell) const
    {
        // If cell is explicitly blocked, return 0
        if (IsCellBlocked(cell)) {
            return 0.0f;
        }

        // Check if clearance was computed
        auto it = clearanceValues.find(cell);
        if (it != clearanceValues.end()) {
            return it->second;
        }

        // Cell is unblocked but not in map = far from obstacles = max clearance
        return std::numeric_limits<float>::infinity();
    }


    std::vector<Vec2> GridPathfinder::SmoothPath(const std::vector<Vec2>& path)
    {
        if (path.size() <= 2) return path; // Can't smooth paths with 2 or fewer points

        std::vector<Vec2> smoothed;
        smoothed.push_back(path.front()); // Always keep start

        size_t current = 0;

        // String pulling: try to skip as many waypoints as possible
        while (current < path.size() - 1) {
            size_t farthest = current + 1; // At minimum, move to next waypoint

            // Try to find the farthest waypoint we can reach directly
            for (size_t i = current + 2; i < path.size(); ++i) {
                if (HasLineOfSight(path[current], path[i])) {
                    farthest = i; // Can skip intermediate waypoints
                }
                else {
                    break; // Wall blocks, can't skip further
                }
            }

            smoothed.push_back(path[farthest]);
            current = farthest;
        }

        return smoothed;
    }

}


