#pragma once
#include <cstddef>

// Shared 2D point representation used by both hashing approaches.
struct SpatialPoint {
    std::size_t id{};
    double x{};
    double y{};
};
