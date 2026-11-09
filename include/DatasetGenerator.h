/*
* DatasetGenerator.h
 *
 * Defines the interface for generating spatial point datasets
 * used to evaluate fixed-grid and adaptive spatial hashing algorithms.
 *
 * Initial implementation generated with assistance from OpenAI ChatGPT.
 * The implementation was subsequently modified, reviewed, documented, and tested by the authors.
 *
 * Dataset generation uses seeded pseudorandom number generators
 * to produce repeatable experimental datasets.
 *
 * Supported distributions include uniform, clustered, skewed,
 * and hotspot datasets for evaluating algorithm performance
 * under different spatial density conditions.
 *
 * References:
 * C++ Standard Library: <random>
 * Mersenne Twister pseudorandom number generator (std::mt19937).
 */

#pragma once
#include "SpatialPoint.h"
#include <cstddef>
#include <cstdint>
#include <vector>

//This class defines the methods used to generate different spatial
//point distributions for testing and benchmarking our algorithms.
class DatasetGenerator {
public:
    //Generates a dataset with points distributed uniformly across the grid.
    static std::vector<SpatialPoint> uniform(std::size_t count, std::uint32_t seed = 42);

    //Generates a dataset where points are distributed in clusters across the grid.
    static std::vector<SpatialPoint> clustered(std::size_t count, std::uint32_t seed = 42);

    //Generate a dataset with points concentrated toward one side of the grid.
    //The seed allows us to reproduce the same dataset for benchmarking.
    static std::vector<SpatialPoint> skewed(std::size_t count, std::uint32_t seed = 42);

    //Generate a dataset where most points are concentrated in a small area.
    //The remaining points are distributed throughout the grid.
    //The seed allows us to reproduce the same dataset for benchmarking.
    static std::vector<SpatialPoint> hotspot(std::size_t count, std::uint32_t seed = 42);
};
