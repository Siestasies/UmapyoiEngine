#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <cmath>
#include <algorithm>
#include "Math/Math.h"



namespace Uma_Navigation{
    struct Rect {
        float x, y;      // Bottom-left corner
        float width, height;

        Rect(float x = 0, float y = 0, float w = 0, float h = 0)
            : x(x), y(y), width(w), height(h) {}

        Vec2 Center() const {
            return { x + width / 2.0f, y + height / 2.0f };
        }

        bool Contains(const Vec2& point) const {
            return point.x >= x && point.x <= x + width &&
                point.y >= y && point.y <= y + height;
        }

        bool Intersects(const Rect& other) const {
            return !(x + width < other.x || other.x + other.width < x ||
                y + height < other.y || other.y + other.height < y);
        }

        // Check if rectangles share an edge (adjacent)
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

    // Tile from your editor
    struct Tile {
        Vec2 position;
        Vec2 size;

        Rect ToRect() const {
            return Rect(position.x, position.y, size.x, size.y);
        }
    };

    struct WalkableRegion {
        int id;
        Rect bounds;
        std::vector<WalkableRegion*> neighbors;

        WalkableRegion(int id, const Rect& rect) : id(id), bounds(rect) {}

        Vec2 Center() const { return bounds.Center(); }

        bool Contains(const Vec2& point) const {
            return bounds.Contains(point);
        }
    };

    struct GridCell {
        int x, y;

        bool operator==(const GridCell& other) const {
            return x == other.x && y == other.y;
        }
    };

    struct GridCellHash {
        size_t operator()(const GridCell& cell) const {
            return std::hash<int>()(cell.x) ^ (std::hash<int>()(cell.y) << 1);
        }
    };

    struct PathNode {
        GridCell cell;
        float gCost;
        float fCost;

        bool operator>(const PathNode& other) const {
            return fCost > other.fCost;
        }
    };
}

