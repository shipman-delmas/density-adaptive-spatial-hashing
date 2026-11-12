#include "Benchmark.h"
#include "DatasetGenerator.h"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>

// Run each distribution at three sizes and print the research measurements.
int main()
{
    try
    {
        const std::size_t sizes[] = {1000, 10000, 100000};
        constexpr std::uint32_t seed = 42;

        for (const std::size_t size : sizes)
        {
            std::cout << "\nPoints: " << size << "; dataset seed: " << seed << '\n';

            std::cout << "Uniform\n";
            Benchmark::run(DatasetGenerator::uniform(size, seed));

            std::cout << "Clustered\n";
            Benchmark::run(DatasetGenerator::clustered(size, seed));

            std::cout << "Skewed\n";
            Benchmark::run(DatasetGenerator::skewed(size, seed));

            std::cout << "Hotspot\n";
            Benchmark::run(DatasetGenerator::hotspot(size, seed));
        }

        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Error: " << error.what() << '\n';

        return 1;
    }
}
