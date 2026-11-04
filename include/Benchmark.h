#pragma once
#include "SpatialPoint.h"
#include <vector>

// TEAM 4: Common experiment runner; do not report placeholder metrics.
class Benchmark {
public:
    static void run(const std::vector<SpatialPoint>& points);
};
