#pragma once

#include "SpatialHash.h"
#include "SpatialPoint.h"

#include <array>
#include <cstddef>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

// Adaptive spatial hash that subdivides crowded root cells into quadrants.
class AdaptiveSpatialHash
{
public:
    // Initialize the root cell size, density threshold, and maximum depth.
    AdaptiveSpatialHash(double initialCellSize, std::size_t densityThreshold, int maxDepth);

    // Add a point to its root cell or an existing child quadrant.
    void insert(const SpatialPoint& point);

    // Find all points within a given radius of the query center.
    std::vector<SpatialPoint> radiusQuery(double x, double y, double radius) const;

    // Find nearby points and optionally report how many candidates were examined.
    std::vector<SpatialPoint> radiusQuery(
        double x, double y, double radius, std::size_t* candidatesExamined) const;

    // Return the number of inserted point records.
    std::size_t size() const noexcept
    {
        return pointCount_;
    }

    // Estimate memory occupied by the spatial hash structure.
    std::size_t estimatedMemoryBytes() const noexcept;

private:
    using Cell = std::pair<int, int>;

    // Store a quadrant's bounds, records, and up to four child quadrants.
    struct Node
    {
        // Initialize this node's bounding rectangle and depth.
        Node(double minX, double minY, double maxX, double maxY, int depth);

        double minX;
        double minY;
        double maxX;
        double maxY;
        double splitX{0.0};
        double splitY{0.0};
        int depth;
        bool divided{false};
        std::vector<SpatialPoint> points;
        std::array<std::unique_ptr<Node>, 4> children{};
    };

    // Convert an original coordinate pair into a root-grid cell.
    Cell cellFor(double x, double y) const;

    // Select a child quadrant based on its split coordinates.
    static std::size_t childIndex(const Node& node, double x, double y) noexcept;

    // Create a child node for one quadrant of a parent.
    static std::unique_ptr<Node> makeChild(const Node& node, std::size_t index);

    // Subdivide a leaf when its depth and coordinate precision allow it.
    bool subdivide(Node& node);

    // Insert a record into a node or one of its descendants.
    void insertInto(Node& node, const SpatialPoint& point);

    // Search the quadrants intersecting a radius query.
    static void queryNode(
        const Node& node, double x, double y, double radius,
        std::vector<SpatialPoint>& matches, std::size_t* candidatesExamined);

    // Estimate memory used by a node's dynamically allocated contents.
    static std::size_t nodeDynamicBytes(const Node& node) noexcept;

    double initialCellSize_;
    std::size_t densityThreshold_;
    int maxDepth_;
    std::size_t pointCount_{0};
    std::unordered_map<Cell, Node, CellHash> roots_;
};
