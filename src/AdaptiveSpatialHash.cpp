#include "AdaptiveSpatialHash.h"

#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

// Initialize a subdivision node with its bounds and depth.
AdaptiveSpatialHash::Node::Node(double minX, double minY, double maxX, double maxY, int depth)
    : minX(minX), minY(minY), maxX(maxX), maxY(maxY), depth(depth)
{
}

// Set the root cell size, split threshold, and maximum subdivision depth.
AdaptiveSpatialHash::AdaptiveSpatialHash(
    double initialCellSize, std::size_t densityThreshold, int maxDepth)
    : initialCellSize_(initialCellSize),
      densityThreshold_(densityThreshold),
      maxDepth_(maxDepth)
{
    if (!(initialCellSize > 0.0) || !std::isfinite(initialCellSize))
    {
        throw std::invalid_argument("Cell size must be finite and positive");
    }

    if (densityThreshold == 0 || maxDepth < 0)
    {
        throw std::invalid_argument("Density threshold must be positive and depth nonnegative");
    }
}

// Convert a point's coordinates to root-grid cell coordinates.
AdaptiveSpatialHash::Cell AdaptiveSpatialHash::cellFor(double x, double y) const
{
    if (!std::isfinite(x) || !std::isfinite(y))
    {
        throw std::invalid_argument("Point coordinates must be finite");
    }

    const double cx = std::floor(x / initialCellSize_);
    const double cy = std::floor(y / initialCellSize_);
    const double minCell = static_cast<double>(std::numeric_limits<int>::min());
    const double maxCell = static_cast<double>(std::numeric_limits<int>::max());

    // Match the fixed grid's supported cell range (exclusive upper bound).
    if (cx < minCell || cx >= maxCell || cy < minCell || cy >= maxCell)
    {
        throw std::out_of_range("Grid coordinates exceed supported range");
    }

    return {static_cast<int>(cx), static_cast<int>(cy)};
}

// Choose the quadrant for a point, placing split-boundary points right or top.
std::size_t AdaptiveSpatialHash::childIndex(const Node& node, double x, double y) noexcept
{
    return (x >= node.splitX ? 1U : 0U) + (y >= node.splitY ? 2U : 0U);
}

// Allocate a child quadrant within its parent's bounds.
std::unique_ptr<AdaptiveSpatialHash::Node> AdaptiveSpatialHash::makeChild(
    const Node& node, std::size_t index)
{
    const bool right = (index & 1U) != 0;
    const bool top = (index & 2U) != 0;

    return std::make_unique<Node>(
        right ? node.splitX : node.minX,
        top ? node.splitY : node.minY,
        right ? node.maxX : node.splitX,
        top ? node.maxY : node.splitY,
        node.depth + 1);
}

// Divide a full leaf into quadrants when depth and precision allow it.
bool AdaptiveSpatialHash::subdivide(Node& node)
{
    if (node.depth >= maxDepth_)
    {
        return false;
    }

    // Avoid subdivisions that produce zero-width children after rounding.
    const double midX = node.minX * 0.5 + node.maxX * 0.5;
    const double midY = node.minY * 0.5 + node.maxY * 0.5;

    if (!std::isfinite(midX) || !std::isfinite(midY) ||
        !(node.minX < midX && midX < node.maxX) ||
        !(node.minY < midY && midY < node.maxY))
    {
        return false;
    }

    node.splitX = midX;
    node.splitY = midY;
    std::array<std::unique_ptr<Node>, 4> replacement{};

    // Redistribute records into new children before committing the split.
    for (const SpatialPoint& point : node.points)
    {
        const auto index = childIndex(node, point.x, point.y);

        if (!replacement[index])
        {
            replacement[index] = makeChild(node, index);
        }

        insertInto(*replacement[index], point);
    }

    node.children = std::move(replacement);
    node.divided = true;
    std::vector<SpatialPoint>().swap(node.points);
    return true;
}

// Insert a record into the appropriate leaf, subdividing when necessary.
void AdaptiveSpatialHash::insertInto(Node& node, const SpatialPoint& point)
{
    // Leaves can exceed the threshold at the depth or precision limit.
    if (!node.divided && node.points.size() >= densityThreshold_)
    {
        subdivide(node);
    }

    if (!node.divided)
    {
        node.points.push_back(point);
        return;
    }

    const auto index = childIndex(node, point.x, point.y);

    if (!node.children[index])
    {
        node.children[index] = makeChild(node, index);
    }

    insertInto(*node.children[index], point);
}

// Insert a point into its root cell and update the stored point count.
void AdaptiveSpatialHash::insert(const SpatialPoint& point)
{
    const auto cell = cellFor(point.x, point.y);
    const double cx = static_cast<double>(cell.first);
    const double cy = static_cast<double>(cell.second);

    auto root = roots_.try_emplace(
        cell,
        cx * initialCellSize_, cy * initialCellSize_,
        (cx + 1.0) * initialCellSize_, (cy + 1.0) * initialCellSize_,
        0);

    insertInto(root.first->second, point);
    ++pointCount_;
}

// Search matching leaf quadrants and count inspected points if requested.
void AdaptiveSpatialHash::queryNode(
    const Node& node, double x, double y, double radius,
    std::vector<SpatialPoint>& matches, std::size_t* candidatesExamined)
{
    if (!node.divided)
    {
        if (candidatesExamined)
        {
            *candidatesExamined += node.points.size();
        }

        for (const SpatialPoint& point : node.points)
        {
            if (std::hypot(point.x - x, point.y - y) <= radius)
            {
                matches.push_back(point);
            }
        }

        return;
    }

    for (std::size_t i = 0; i < node.children.size(); ++i)
    {
        if (!node.children[i])
        {
            continue;
        }

        const bool right = (i & 1U) != 0;
        const bool top = (i & 2U) != 0;

        // Skip quadrants that cannot intersect the query circle.
        if ((right ? node.splitX - x : x - node.splitX) > radius)
        {
            continue;
        }

        if ((top ? node.splitY - y : y - node.splitY) > radius)
        {
            continue;
        }

        queryNode(*node.children[i], x, y, radius, matches, candidatesExamined);
    }
}

// Return stored points within the radius using the original public interface.
std::vector<SpatialPoint> AdaptiveSpatialHash::radiusQuery(
    double x, double y, double radius) const
{
    return radiusQuery(x, y, radius, nullptr);
}

// Return nearby points and optionally count examined candidate records.
std::vector<SpatialPoint> AdaptiveSpatialHash::radiusQuery(
    double x, double y, double radius, std::size_t* candidatesExamined) const
{
    if (candidatesExamined)
    {
        *candidatesExamined = 0;
    }

    if (!std::isfinite(radius) || radius < 0.0)
    {
        throw std::invalid_argument("Radius must be finite and nonnegative");
    }

    // Match the existing fixed grid's inclusive root-cell query range.
    const Cell low = cellFor(x - radius, y - radius);
    const Cell high = cellFor(x + radius, y + radius);
    std::vector<SpatialPoint> matches;

    if (roots_.empty())
    {
        return matches;
    }

    for (long long cx = low.first; cx <= high.first; ++cx)
    {
        for (long long cy = low.second; cy <= high.second; ++cy)
        {
            const auto it = roots_.find({static_cast<int>(cx), static_cast<int>(cy)});

            if (it != roots_.end())
            {
                queryNode(it->second, x, y, radius, matches, candidatesExamined);
            }
        }
    }

    return matches;
}

// Estimate memory held by a node's point storage and allocated children.
std::size_t AdaptiveSpatialHash::nodeDynamicBytes(const Node& node) noexcept
{
    std::size_t bytes = node.points.capacity() * sizeof(SpatialPoint);

    for (const auto& child : node.children)
    {
        if (child)
        {
            bytes += sizeof(Node) + nodeDynamicBytes(*child);
        }
    }

    return bytes;
}

// Estimate total memory held by root cells, child nodes, and stored records.
std::size_t AdaptiveSpatialHash::estimatedMemoryBytes() const noexcept
{
    // Includes the object, hash table buckets and entries, child nodes, and
    // point-vector capacities, but excludes allocator and temporary overhead.
    std::size_t bytes = sizeof(*this) + roots_.bucket_count() * sizeof(void*);
    bytes += roots_.size() * (sizeof(decltype(roots_)::value_type) + sizeof(void*));

    for (const auto& entry : roots_)
    {
        bytes += nodeDynamicBytes(entry.second);
    }

    return bytes;
}
