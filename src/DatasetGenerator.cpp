
/*
 * DatasetGenerator.cpp
 *
 * Implements spatial point dataset generation for evaluating
 * fixed-grid and adaptive spatial hashing algorithms.
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


#include "DatasetGenerator.h"
#include <random>

//Produces a uniform data set
std::vector<SpatialPoint> DatasetGenerator::uniform(std::size_t count, std::uint32_t seed) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> coord(0.0, 100.0);
    std::vector<SpatialPoint> points;
    points.reserve(count);
    for (std::size_t i = 0; i < count; ++i)
        points.push_back({i, coord(rng), coord(rng)});
    return points;
}

//Produces a clustered data set
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
    return points;
}

//Produces a skewed data set
std::vector<SpatialPoint> DatasetGenerator::skewed(
    std::size_t count, std::uint32_t seed
) {
    //Initialize our random number generator using the supplied seed.
    std::mt19937 rng(seed);

    //Generate random values between 0 and 1.
    std::uniform_real_distribution<double> coord(0.0, 1.0);

    //Create a vector to store our generated spatial points.
    std::vector<SpatialPoint> points;
    points.reserve(count);

    //Generate the requested number of points.
    for (std::size_t i = 0; i < count; ++i) {

        //Generate two random coordinates between 0 and 1.
        const double u = coord(rng);
        const double v = coord(rng);

        //Squaring u concentrates more X coordinates near zero. This helps skew the data set.
        //Y remains uniformly distributed across the grid.
        const double x = 100.0 * u * u;
        const double y = 100.0 * v;

        //Store each generated point with a unique ID.
        points.push_back({i, x, y});
    }

    //Return the completed dataset.
    return points;
}

//produces a data set consisting of hotspot point distribution
std::vector<SpatialPoint> DatasetGenerator::hotspot(
    std::size_t count, std::uint32_t seed
) {
    //Initialize our random number generator using the supplied seed.
    std::mt19937 rng(seed);

    //Generate background points across the full 100 x 100 domain.
    std::uniform_real_distribution<double> background(0.0, 100.0);

    //Generate hotspot points within a smaller 10 x 10 region.
    std::uniform_real_distribution<double> hotspotCoord(45.0, 55.0);

    //Create a vector to store our generated spatial points.
    std::vector<SpatialPoint> points;
    points.reserve(count);

    //Calculate how many points belong to the hotspot.
    //Using integer division gives us approximately 90%.
    const std::size_t hotspotCount =
        (count / 10) * 9 + ((count % 10) * 9) / 10;

    //Generate the requested number of points.
    for (std::size_t i = 0; i < count; ++i) {
        double x;
        double y;

        //Place approximately 90% of the points inside the hotspot.
        if (i < hotspotCount) {
            x = hotspotCoord(rng);
            y = hotspotCoord(rng);
        }
        //Distribute the remaining points across the full domain.
        else {
            x = background(rng);
            y = background(rng);
        }

        //Store each point with a unique ID and its X/Y coordinates.
        points.push_back({i, x, y});
    }

    //Return the completed dataset.
    return points;
}
