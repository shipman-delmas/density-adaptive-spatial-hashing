#pragma once
#include "SpatialPoint.h"
#include <cstddef>
#include <cstdint>
#include <vector>

// TEAM 3: Extend datasets, but reproducible uniform/clustered samples work now.
class DatasetGenerator {
public:
    static std::vector<SpatialPoint> uniform(std::size_t count, std::uint32_t seed = 42);
    static std::vector<SpatialPoint> clustered(std::size_t count, std::uint32_t seed = 42);
};
