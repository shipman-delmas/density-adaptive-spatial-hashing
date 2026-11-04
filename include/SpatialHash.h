#pragma once
#include "SpatialPoint.h"
#include <cstddef>
#include <vector>

// TEAM 1: Fixed-resolution spatial hashing baseline.
class SpatialHash {
public:
    explicit SpatialHash(double cellSize);
    void insert(const SpatialPoint& point);
    std::vector<SpatialPoint> radiusQuery(double x, double y, double radius) const;
    std::size_t size() const noexcept { return pointCount_; }
private:
    double cellSize_;
    std::size_t pointCount_{0};
    // TODO (Team 1): Define a hashable integer grid-cell key.
    // TODO (Team 1): Add the cell-to-points storage container.
    // TODO (Team 1): Decide how to handle negative coordinates.
};
