#pragma once
#include "SpatialPoint.h"
#include <cstddef>
#include <vector>

// TEAM 2: Adaptive spatial hash; subdivision policy is the research task.
class AdaptiveSpatialHash {
public:
    AdaptiveSpatialHash(double initialCellSize, std::size_t densityThreshold, int maxDepth);
    void insert(const SpatialPoint& point);
    std::vector<SpatialPoint> radiusQuery(double x, double y, double radius) const;
    std::size_t size() const noexcept { return pointCount_; }
private:
    double initialCellSize_;
    std::size_t densityThreshold_;
    int maxDepth_;
    std::size_t pointCount_{0};
    // TODO (Team 2): Define root-cell lookup and child-cell representation.
    // TODO (Team 2): Choose subdivision and redistribution rules.
    // TODO (Team 2): Specify duplicate-point and boundary behavior.
};
