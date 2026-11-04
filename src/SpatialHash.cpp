#include "SpatialHash.h"
#include <stdexcept>
#include <cmath>

SpatialHash::SpatialHash(double cellSize) : cellSize_(cellSize) {
    if (!(cellSize > 0.0) || !std::isfinite(cellSize)) {
        throw std::invalid_argument("cellSize must be positive and finite");
    }
}

void SpatialHash::insert(const SpatialPoint& point) {
    (void)point;
    // TODO (Team 1): Map (x,y) to integer grid coordinates using floor().
    // TODO (Team 1): Insert into the appropriate cell and increment pointCount_.
    throw std::logic_error("TODO: fixed-grid insertion not implemented");
}

std::vector<SpatialPoint> SpatialHash::radiusQuery(double x, double y, double radius) const {
    (void)x; (void)y; (void)radius;
    // TODO (Team 1): Visit every grid cell intersecting the query bounding box.
    // TODO (Team 1): Filter candidates using squared Euclidean distance.
    // TODO (Team 1): Count candidate checks for benchmarking (agree on interface).
    throw std::logic_error("TODO: fixed-grid radius query not implemented");
}
