#pragma once
#include "NavMeshTypes.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <cmath>
#include <algorithm>
#include <optional>

namespace Uma_Navigation {
    using OptionalRegionRef = std::optional<std::reference_wrapper<WalkableRegion>>;

    //builds navigation mesh based on walkable regions and obstacles then uses the walkable to to create a path
	class NavMesh {
	public:
        NavMesh(const Rect& worldBounds);

        /*Starting with the full worldBounds as one large walkable area
        Iteratively splitting rectangles when they intersect obstacles using SplitRectangle()
        Creating WalkableRegion objects for each non - overlapping walkable area
        Calling BuildNeighborGraph() to establish connectivity between adjacent regions*/
		void Generate(const std::vector<Tile>& obstacles);

        /*Uses FindRegion() to locate which walkable regions contain the start and goal points
        Calls AStarSearch() to find the optimal sequence of regions to traverse
        Converts the region path into waypoints(typically region centers)
        Applies SmoothPath() to remove unnecessary waypoints using line - of - sight optimization
        Returns the final waypoint list*/
		std::vector<Vec2> FindPath(const Vec2& start, const Vec2& goal);

		//NavMesh info getters
		int GetRegionCount() const { return static_cast<int>(v_regions.size()); }
		const std::vector<WalkableRegion>& GetRegions() const { return v_regions; }
	private:
		std::vector<WalkableRegion> v_regions;
		Rect worldBounds;

		struct AStarNode {
			WalkableRegion* region;
			float gCost;  // Cost from start
			float fCost;  // gCost + heuristic
			AStarNode* parent;

			bool operator>(const AStarNode& other) const {
				return fCost > other.fCost;
			}
		};

        //When a walkable rectangle intersects an obstacle, it cuts up the area into smaller rects
        //to avoid the obstacle
		std::vector<Rect> SplitRectangle(const Rect& walkable, const Rect& obstacle);

        //checks if regions share an edge if yes create a connection between them
		void BuildNeighborGraph();

        //looks for region that contains this point return null if it cant be found
        OptionalRegionRef FindRegion(const Vec2& point);

        //implementation of A star
		std::vector<WalkableRegion*> AStarSearch(WalkableRegion* start, WalkableRegion* goal);

        //check if the line between 2 points has an object in between them
		bool HasLineOfSight(const Vec2& a, const Vec2& b);

        //makes the path found smoother by cutting out unneeded waypoints 
		void SmoothPath(std::vector<Vec2>& path);
	};

    //Fast look up for relevant tiles via partitioning and caching them instead of looking up all tiles
    class SpatialHash {
    private:
        //size of the grid 
        float cellSize;

        //stores all the tiles within that grid cell
        //the use of unordered map because of look up time
        std::unordered_map<int64_t, std::vector<Tile*>> grid;

        //to convert the position into a hashkey
        int64_t GetKey(int x, int y) const;

        //Converts world-space positions to grid coordinates
        void GetCellCoords(const Vec2& pos, int& x, int& y) const;
    public:
        explicit SpatialHash(float cellSize = 50.0f);

        //clears all tile references
        void Clear();

        //inserts the tile into the cells
        void Insert(Tile* tile);

        //gets all the tiles in an area defined by bounds
        std::vector<Tile*> Query(const Rect& bounds);
    };

    //creates and maintains a NavMesh around entities of interest instead of making a NavMesh for an entire map
    class DynamicNavMesh {
    private:
        //actual navigation mesh around the player used to path find
        NavMesh navmesh;
        //uses the spacial hash to look for tiles near the player
        SpatialHash spatialHash;

        //Keeps track of the player's position and only updates the 
        //navmesh after they pass the quantization threshold.
        Vec2 currentCenter;
        Vec2 quantizedCenter;
        float quantizationSize;
        //snapping of the position to quantization grid 
        Vec2 Quantize(const Vec2& pos) const;

        //the size of the active region
        float radius;

        //keeps track of the current bounds of the dynamic mesh
        Rect currentBounds;

        //boolean that signals that the mesh needs to be rebuilt
        bool needsRebuild;

        //stores all the active tiles for quick access instead of looking it up again
        std::vector<Tile> cachedTiles;
    public:
        explicit DynamicNavMesh(float radius = 150.0f, float quantSize = 20.0f);

        //insert all the tiles into the hash for look up
        void BuildSpatialHash(const std::vector<Tile>& allTiles);

        //checks if the player has moved far enough from previous location to trigger an update
        void Update(const Vec2& playerPos);

        //triggers the rebuild flag
        void MarkDirty();

        //calls NavMesh to call path finding 
        std::vector<Vec2> FindPath(const Vec2& start, const Vec2& goal);

        // Inline getters
        bool IsInBounds(const Vec2& pos) const { return currentBounds.Contains(pos); }
        int GetRegionCount() const { return navmesh.GetRegionCount(); }
        Rect GetBounds() const { return currentBounds; }
        const std::vector<Tile>& GetCachedTiles() const { return cachedTiles; }
        const std::vector<WalkableRegion>& GetRegions() const { return navmesh.GetRegions(); }

    };
}
