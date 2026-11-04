#include "Benchmark.h"
#include "DatasetGenerator.h"
#include <iostream>

int main() {
    try {
        constexpr std::size_t sampleSize = 1000;
        auto uniformPoints = DatasetGenerator::uniform(sampleSize);
        auto clusteredPoints = DatasetGenerator::clustered(sampleSize);
        std::cout << "=== Spatial Hashing Research Starter ===\n";
        std::cout << "Uniform dataset:\n";
        Benchmark::run(uniformPoints);
        std::cout << "Clustered dataset:\n";
        Benchmark::run(clusteredPoints);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
