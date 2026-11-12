#include "Benchmark.h"
#include "DatasetGenerator.h"

#include <cstddef>
#include <exception>
#include <iostream>

// Generate repeatable datasets and compare the two spatial hashing approaches.
int main()
{
    try
    {
        constexpr std::size_t sampleSize = 1000;

        const auto uniformPoints = DatasetGenerator::uniform(sampleSize);
        const auto clusteredPoints = DatasetGenerator::clustered(sampleSize);
        const auto skewedPoints = DatasetGenerator::skewed(sampleSize);
        const auto hotspotPoints = DatasetGenerator::hotspot(sampleSize);

        std::cout << "=== Spatial Hashing Research Starter ===\n";

        std::cout << "\nUniform dataset:\n";
        Benchmark::run(uniformPoints);

        std::cout << "\nClustered dataset:\n";
        Benchmark::run(clusteredPoints);

        std::cout << "\nSkewed dataset:\n";
        Benchmark::run(skewedPoints);

        std::cout << "\nHotspot dataset:\n";
        Benchmark::run(hotspotPoints);

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
