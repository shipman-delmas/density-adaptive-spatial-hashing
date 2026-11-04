#include "DatasetGenerator.h"
#include <random>

std::vector<SpatialPoint> DatasetGenerator::uniform(std::size_t count, std::uint32_t seed) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> coord(0.0, 100.0);
    std::vector<SpatialPoint> points;
    points.reserve(count);
    for (std::size_t i = 0; i < count; ++i)
        points.push_back({i, coord(rng), coord(rng)});
    return points;
}

std::vector<SpatialPoint> DatasetGenerator::clustered(std::size_t count, std::uint32_t seed) {
    std::mt19937 rng(seed);
    std::normal_distribution<double> offset(0.0, 5.0);
    const double centers[][2] = {{25.0, 25.0}, {75.0, 75.0}, {25.0, 75.0}};
    std::vector<SpatialPoint> points;
    points.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        const auto& center = centers[i % 3];
        points.push_back({i, center[0] + offset(rng), center[1] + offset(rng)});
    }
    // TODO (Team 3): Add skewed/hotspot datasets and document distributions.
    // TODO (Team 3): Decide whether to clamp coordinates to a common domain.
    return points;
}
