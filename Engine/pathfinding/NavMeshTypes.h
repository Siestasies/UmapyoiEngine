#pragma once
/*!
\file   NavMeshTypes.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Koh Kai Yang (100%)
\par    E-mail: k.kaiyang@digipen.edu
\par    DigiPen login: k.kaiyang

\brief
This files just provides structs for use of navigations

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <cmath>
#include <algorithm>
#include "Math/Math.h"



namespace Uma_Navigation {

    /**
    * @brief Axis-aligned rectangle in 2D space.
    *
    * Used as a basic geometric primitive for tiles, regions, and overlap tests.
    */
    struct Rect {
        /// X coordinate of the bottom-left corner.
        float x;
        /// Y coordinate of the bottom-left corner.
        float y;
        /// Rectangle width.
        float width;
        /// Rectangle height.
        float height;

        /**
        * @brief Construct a rectangle from position and size.
        * @param x Bottom-left x coordinate.
        * @param y Bottom-left y coordinate.
        * @param w Width of the rectangle.
        * @param h Height of the rectangle.
        */
        Rect(float x = 0, float y = 0, float w = 0, float h = 0)
            : x(x), y(y), width(w), height(h) {}

        /**
        * @brief Get the center point of this rectangle.
        * @return Center as a Vec2.
        */
        Vec2 Center() const {
            return { x + width / 2.0f, y + height / 2.0f };
        }

        /**
        * @brief Check if a point lies inside this rectangle.
        *
        * Edges are treated as inclusive.
        *
        * @param point Point in world space.
        * @return true if point is inside or on the edge, false otherwise.
        */
        bool Contains(const Vec2& point) const {
            return point.x >= x && point.x <= x + width &&
                point.y >= y && point.y <= y + height;
        }

        /**
        * @brief Check if this rectangle intersects another rectangle.
        * @param other Other rectangle to test against.
        * @return true if the rectangles overlap, false otherwise.
        */
        bool Intersects(const Rect& other) const {
            return !(x + width < other.x || other.x + other.width < x ||
                y + height < other.y || other.y + other.height < y);
        }

        /**
        * @brief Check if two rectangles share an edge (are adjacent).
        *
        * Uses a small epsilon to tolerate floating-point imprecision.
        *
        * @param other   Other rectangle to test against.
        * @param epsilon Tolerance for edge comparisons.
        * @return true if the rectangles touch along an edge, false otherwise.
        */
        bool SharesEdge(const Rect& other, float epsilon = 0.1f) const {
            // Check if they touch horizontally
            bool horizontalTouch = std::abs((x + width) - other.x) < epsilon ||
                std::abs(other.x + other.width - x) < epsilon;
            // Check if they overlap vertically
            bool verticalOverlap = !(y + height < other.y || other.y + other.height < y);

            // Check if they touch vertically
            bool verticalTouch = std::abs((y + height) - other.y) < epsilon ||
                std::abs(other.y + other.height - y) < epsilon;
            // Check if they overlap horizontally
            bool horizontalOverlap = !(x + width < other.x || other.x + other.width < x);

            return (horizontalTouch && verticalOverlap) || (verticalTouch && horizontalOverlap);
        }
    };

    /**
    * @brief Tile description from the level/editor.
    *
    * Stores a world-space position and size and can be converted to Rect.
    */
    struct Tile {
        /// Tile origin in world space.
        Vec2 position;
        /// Tile size in world space.
        Vec2 size;

        /**
        * @brief Convert this tile to an axis-aligned rectangle.
        * @return Rect covering the tile in world space.
        */
        Rect ToRect() const {
            return Rect(position.x, position.y, size.x, size.y);
        }
    };

    /**
    * @brief A contiguous walkable region with adjacency information.
    *
    * Represents a merged area of walkable tiles and stores neighbor links
    * for higher-level navigation (e.g., region graph / navmesh cells).
    */
    struct WalkableRegion {
        /// Unique region identifier.
        int id;
        /// Bounding rectangle of this region in world space.
        Rect bounds;
        /// Neighboring regions that share an edge with this region.
        std::vector<WalkableRegion*> neighbors;

        /**
        * @brief Construct a walkable region from an id and bounds.
        * @param id   Region identifier.
        * @param rect Bounding rectangle of the region.
        */
        WalkableRegion(int id, const Rect& rect) : id(id), bounds(rect) {}

        /**
        * @brief Get the center of this region.
        * @return Center point of the region bounds.
        */
        Vec2 Center() const { return bounds.Center(); }

        /**
        * @brief Check if a point lies inside this region.
        * @param point World-space point.
        * @return true if the point is inside the region bounds, false otherwise.
        */
        bool Contains(const Vec2& point) const {
            return bounds.Contains(point);
        }
    };

    /**
    * @brief Integer grid coordinate used for grid-based pathfinding.
    */
    struct GridCell {
        /// X coordinate in grid space.
        int x;
        /// Y coordinate in grid space.
        int y;

        /**
        * @brief Equality comparison for grid cells.
        * @param other Other cell to compare with.
        * @return true if both x and y match, false otherwise.
        */
        bool operator==(const GridCell& other) const {
            return x == other.x && y == other.y;
        }
    };

    /**
    * @brief Hash functor for GridCell to use in unordered containers.
    */
    struct GridCellHash {
        /**
        * @brief Compute hash value for a GridCell.
        * @param cell Cell to hash.
        * @return Hash value combining x and y.
        */
        size_t operator()(const GridCell& cell) const {
            return std::hash<int>()(cell.x) ^ (std::hash<int>()(cell.y) << 1);
        }
    };

    /**
    * @brief Node used by A* or similar grid search algorithms.
    *
    * Stores the grid cell and the g/f costs used for priority queue ordering.
    */
    struct PathNode {
        /// Grid cell position.
        GridCell cell;
        /// Cost from start to this node.
        float gCost;
        /// Estimated total cost (g + heuristic) to the goal.
        float fCost;

        /**
        * @brief Comparison operator for priority queues (min-heap by fCost).
        * @param other Other node to compare with.
        * @return true if this node has a higher fCost than other.
        */
        bool operator>(const PathNode& other) const {
            return fCost > other.fCost;
        }
    };
}

