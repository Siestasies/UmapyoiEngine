#include "NavMesh.hpp"

namespace Uma_Navigation {
    //constructor
	NavMesh::NavMesh(const Rect& worldBounds) : worldBounds(worldBounds) {
        //empty on purpose
    }

    void NavMesh::Generate(const std::vector<Tile>& obstacles) {
        //clears the vector of walkable regions
        v_regions.clear();

        //creates a rect the size of world bound
        std::vector<Rect> walkableRects = { worldBounds };

        //loops thru all the obstacles and splits the rect into walkable areas
        for (const auto& tile : obstacles) {
            Rect obstacle = tile.ToRect();
            std::vector<Rect> newWalkable;

            for (const auto& walkable : walkableRects) {
                //if the walkable does not intersects an obstacle insert into the new walkable vector
                if (!walkable.Intersects(obstacle)) {
                    newWalkable.push_back(walkable);
                }
                else {
                    //splits the intersecting rect into parts avoiding the obstacle and 
                    //insert those into new walkable vector
                    auto splits = SplitRectangle(walkable, obstacle);
                    newWalkable.insert(newWalkable.end(), splits.begin(), splits.end());
                }
            }

            walkableRects = newWalkable;
        }

        int regionId = 0;
        for (const auto& rect : walkableRects) {
            //adds the walkable rects into vector of regions
            v_regions.emplace_back(regionId++, rect);
        }

        //creates connections between reegions
        BuildNeighborGraph();
        std::cout << "[NavMesh] Created " << v_regions.size() << " walkable regions from "
            << obstacles.size() << " obstacles" << std::endl;
    }

    std::vector<Vec2> NavMesh::FindPath(const Vec2& start, const Vec2& goal) {
        OptionalRegionRef startRegion = FindRegion(start);
        OptionalRegionRef goalRegion = FindRegion(goal);

        // Debug: Check if positions are in regions
        if (!startRegion.has_value()) {
            std::cout << "[NavMesh] Start position (" << start.x << "," << start.y << ") not in any region!" << std::endl;
            return {};
        }
        if (!goalRegion.has_value()) {
            std::cout << "[NavMesh] Goal position (" << goal.x << "," << goal.y << ") not in any region!" << std::endl;
            return {};
        }

        std::cout << "[NavMesh] Start in region " << startRegion.value().get().id
            << ", Goal in region " << goalRegion.value().get().id << std::endl;

        // Check if same region (commented out for now)
        /*if (&startRegion.value().get() == &goalRegion.value().get()) {
            if (HasLineOfSight(start, goal)) {
                return { start, goal };
            }
        }*/

        // A* search between regions
        std::vector<WalkableRegion*> regionPath = AStarSearch(&startRegion.value().get(), &goalRegion.value().get());

        if (regionPath.empty()) {
            std::cout << "[NavMesh] A* failed to find path between regions!" << std::endl;
            return {};
        }

        std::cout << "[NavMesh] Found path through " << regionPath.size() << " regions" << std::endl;

        std::vector<Vec2> path;
        path.push_back(start);

        for (size_t i = 1; i < regionPath.size(); i++) {
            path.push_back(regionPath[i]->Center());
        }

        path.push_back(goal);

        SmoothPath(path);

        return path;
    }


    std::vector<Rect> NavMesh::SplitRectangle(const Rect& walkable, const Rect& obstacle) {
        std::vector<Rect> result;

        //this gets the where in the walkable rect the obstacle is intersecting 
        float intersectX = std::max(walkable.x, obstacle.x);
        float intersectY = std::max(walkable.y, obstacle.y);
        //gets the width and height of the intersection
        float intersectW = std::min(walkable.x + walkable.width, obstacle.x + obstacle.width) - intersectX;
        float intersectH = std::min(walkable.y + walkable.height, obstacle.y + obstacle.height) - intersectY;

        //if obstacle does not intersect return
        if (intersectW <= 0 || intersectH <= 0) {
            result.push_back(walkable);
            return result;
        }

        // Left rectangle
        if (intersectX > walkable.x) {
            result.push_back(Rect(walkable.x, walkable.y, intersectX - walkable.x, walkable.height));
        }

        // Right rectangle
        if (intersectX + intersectW < walkable.x + walkable.width) {
            result.push_back(Rect(intersectX + intersectW, walkable.y, walkable.x + walkable.width -
                (intersectX + intersectW), walkable.height));
        }

        // Bottom rectangle
        if (intersectY > walkable.y) {
            result.push_back(Rect(intersectX, walkable.y, intersectW, intersectY - walkable.y));
        }

        // Top rectangle
        if (intersectY + intersectH < walkable.y + walkable.height) {
            result.push_back(Rect(intersectX, intersectY + intersectH, intersectW,
                walkable.y + walkable.height - (intersectY + intersectH)));
        }

        return result;
    }

    void NavMesh::BuildNeighborGraph() {
        for (size_t i = 0; i < v_regions.size(); i++) {
            for (size_t j = i + 1; j < v_regions.size(); j++) {
                if (v_regions[i].bounds.SharesEdge(v_regions[j].bounds)) {
                    v_regions[i].neighbors.push_back(&v_regions[j]);
                    v_regions[j].neighbors.push_back(&v_regions[i]);
                }
            }
        }

        // Debug: Print region details
        std::cout << "[NavMesh] Region details:" << std::endl;
        for (size_t i = 0; i < v_regions.size(); ++i) {
            const auto& r = v_regions[i];
            std::cout << "  Region " << r.id << ": (" << r.bounds.x << "," << r.bounds.y
                << ") size(" << r.bounds.width << "x" << r.bounds.height
                << ") neighbors:" << r.neighbors.size() << std::endl;
        }
    }


    OptionalRegionRef NavMesh::FindRegion(const Vec2& point) {
        for (auto& region : v_regions) {
            if (region.Contains(point)) {
                return region;
            }
        }
        return std::nullopt;
    }

    std::vector<WalkableRegion*> NavMesh::AStarSearch(WalkableRegion* start, WalkableRegion* goal)
    {
        //setting up the open and closed list for a star searches
        std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> openList;
        std::unordered_set<int> closedSet;
        //this is used for quick loop up and keep track of all the nodes created
        std::unordered_map<int, AStarNode> allNodes;

        float heuristic = Uma_Math::distance(start->Center(), goal->Center());
        AStarNode startNode{ start, 0.0f, heuristic, nullptr };
        openList.push(startNode);
        allNodes[start->id] = startNode;

        //checks all connected nodes and determines the path based on cost
        while (!openList.empty()) {
            AStarNode current = openList.top();
            openList.pop();

            if (current.region == goal) {
                std::vector<WalkableRegion*> path;
                AStarNode* node = &allNodes[goal->id];
                while (node != nullptr) {
                    path.push_back(node->region);
                    node = node->parent;
                }
                std::reverse(path.begin(), path.end());
                return path;
            }

            closedSet.insert(current.region->id);

            for (WalkableRegion* neighbor : current.region->neighbors) {
                if (closedSet.count(neighbor->id)) continue;

                float edgeCost = Uma_Math::distance(current.region->Center(), neighbor->Center());
                float newGCost = current.gCost + edgeCost;

                auto it = allNodes.find(neighbor->id);
                if (it == allNodes.end() || newGCost < it->second.gCost) {
                    float hCost = Uma_Math::distance(neighbor->Center(),goal->Center());
                    AStarNode neighborNode{
                        neighbor,
                        newGCost,
                        newGCost + hCost,
                        &allNodes[current.region->id]
                    };
                    allNodes[neighbor->id] = neighborNode;
                    openList.push(neighborNode);
                }
            }
        }

        return {};
    }

    //checks line of sight between 2 points returns true if there is something inbetween the 2 points
    bool NavMesh::HasLineOfSight(const Vec2& a, const Vec2& b) {
        //the distance before every check
        //bigger means less accurate but less costly
        const int samples = 10;
        for (int i = 0; i <= samples; i++) {
            float t = static_cast<float>(i) / samples;
            Vec2 point = a + (b - a) * t;
            if (!FindRegion(point)) {
                return false;
            }
        }
        return true;
    }

    void NavMesh::SmoothPath(std::vector<Vec2>& path) {
        //if there is 2 or less waypoints it optimised 
        if (path.size() <= 2) return;

        std::vector<Vec2> smoothed;
        smoothed.push_back(path[0]);

        size_t current = 0;
        while (current < path.size() - 1) {
            size_t farthest = current + 1;

            //it is +2 because there is no intermediate steps with anything less than +2
            for (size_t i = current + 2; i < path.size(); i++) {
                //check for the furthest possible waypoint that still maintains line of sight
                //so it cuts out all waypoints inbetween
                if (HasLineOfSight(path[current], path[i])) {
                    farthest = i;
                }
                else {
                    break;
                }
            }
            //adds waypoint to the new vector
            smoothed.push_back(path[farthest]);
            current = farthest;
        }

        path = smoothed;
    }

    SpatialHash::SpatialHash(float cellSize) : cellSize(cellSize) {
        //empty on purpose
    }

    int64_t SpatialHash::GetKey(int x, int y) const
    {
        //shifts all the x coords to the first 32 bits and combines it with y coords
        // to be used as a key
        // 
        //reason being using unordered map has the search of O(1) so it is preferred over map
        return (static_cast<int64_t>(x) << 32) | static_cast<int64_t>(y);
    }

    void SpatialHash::GetCellCoords(const Vec2& pos, int& x, int& y) const {
        //gets the cell the world coords point to
        x = static_cast<int>(std::floor(pos.x / cellSize));
        y = static_cast<int>(std::floor(pos.y / cellSize));
    }

    void SpatialHash::Clear() {
        grid.clear();
    }

    void SpatialHash::Insert(Tile* tile) {
        Rect rect = tile->ToRect();

        int minX, minY, maxX, maxY;
        GetCellCoords(Vec2(rect.x, rect.y), minX, minY);
        GetCellCoords(Vec2(rect.x + rect.width, rect.y + rect.height), maxX, maxY);

        //insert the tile into every grid that contains this tile
        for (int x = minX; x <= maxX; x++) {
            for (int y = minY; y <= maxY; y++) {
                int64_t key = GetKey(x, y);
                grid[key].push_back(tile);
            }
        }
    }

    std::vector<Tile*> SpatialHash::Query(const Rect& bounds) {
        std::unordered_set<Tile*> results;

        //The tiles are not added just because they are in the relevant grid;
        //they are only added if they also intersect the query bounds

        int minX, minY, maxX, maxY;
        GetCellCoords(Vec2(bounds.x, bounds.y), minX, minY);
        GetCellCoords(Vec2(bounds.x + bounds.width, bounds.y + bounds.height), maxX, maxY);

        for (int x = minX; x <= maxX; x++) {
            for (int y = minY; y <= maxY; y++) {
                int64_t key = GetKey(x, y);
                auto it = grid.find(key);
                if (it != grid.end()) {
                    for (Tile* tile : it->second) {
                        if (bounds.Intersects(tile->ToRect())) {
                            results.insert(tile);
                        }
                    }
                }
            }
        }

        return std::vector<Tile*>(results.begin(), results.end());
    }

    DynamicNavMesh::DynamicNavMesh(float radius, float quantSize)
        : navmesh(Rect(0, 0, 0, 0)),
        spatialHash(radius / 3.0f),
        currentCenter(0, 0),
        quantizedCenter(0, 0),
        radius(radius),
        quantizationSize(quantSize),
        currentBounds(0, 0, 0, 0),
        needsRebuild(true) {
        //empty on purpose
    }

    Vec2 DynamicNavMesh::Quantize(const Vec2& pos) const {
        return Vec2(
            std::round(pos.x / quantizationSize) * quantizationSize,
            std::round(pos.y / quantizationSize) * quantizationSize
        );
    }

    void DynamicNavMesh::BuildSpatialHash(const std::vector<Tile>& allTiles) {
        spatialHash.Clear();
        for (size_t i = 0; i < allTiles.size(); i++) {
            spatialHash.Insert(const_cast<Tile*>(&allTiles[i]));
        }
    }

    void DynamicNavMesh::Update(const Vec2& playerPos) {
        Vec2 newQuantized = Quantize(playerPos);

        float distMoved = Uma_Math::distance(newQuantized, quantizedCenter);

        //if navMesh doesnt need to be rebuild return
        if (distMoved < quantizationSize && !needsRebuild) {
            return;
        }

        quantizedCenter = newQuantized;
        currentCenter = playerPos;
        needsRebuild = false;

        Rect newBounds(
            currentCenter.x - radius,
            currentCenter.y - radius,
            radius * 2.0f,
            radius * 2.0f
        );

        std::vector<Tile*> tilePointers = spatialHash.Query(newBounds);

        cachedTiles.clear();
        for (Tile* tilePtr : tilePointers) {
            cachedTiles.push_back(*tilePtr);
        }

        currentBounds = newBounds;
        navmesh = NavMesh(newBounds);
        navmesh.Generate(cachedTiles);
    }

    void DynamicNavMesh::MarkDirty() {
        needsRebuild = true;
    }

    std::vector<Vec2> DynamicNavMesh::FindPath(const Vec2& start, const Vec2& goal) {
        if (!IsInBounds(start) || !IsInBounds(goal)) {
            return {};
        }

        return navmesh.FindPath(start, goal);
    }
}


