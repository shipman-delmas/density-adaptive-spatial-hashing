#include "Benchmark.h"
#include "AdaptiveSpatialHash.h"
#include "SpatialHash.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace
{
    using Clock = std::chrono::steady_clock;

    struct Query
    {
        double x;
        double y;
        double radius;
    };

    // Sort point records by ID and coordinates for consistent comparisons.
    void sortPoints(std::vector<SpatialPoint>& points)
    {
        std::sort(points.begin(), points.end(), [](const SpatialPoint& a, const SpatialPoint& b)
        {
            return std::tie(a.id, a.x, a.y) < std::tie(b.id, b.x, b.y);
        });
    }

    // Compare sorted results without losing duplicate point records.
    bool equalPoints(const std::vector<SpatialPoint>& a, const std::vector<SpatialPoint>& b)
    {
        return a.size() == b.size() &&
            std::equal(a.begin(), a.end(), b.begin(), [](const SpatialPoint& p, const SpatialPoint& q)
            {
                return p.id == q.id && p.x == q.x && p.y == q.y;
            });
    }

    // Generate repeatable queries across the domain and at data points.
    std::vector<Query> makeQueries(const std::vector<SpatialPoint>& points)
    {
        constexpr std::size_t queryCount = 250;
        constexpr double queryRadius = 5.0;

        std::mt19937 random(12345);
        std::uniform_real_distribution<double> location(0.0, 100.0);
        std::vector<Query> queries;
        queries.reserve(queryCount);

        for (std::size_t i = 0; i < queryCount; ++i)
        {
            if (i % 2 == 1 && !points.empty())
            {
                const SpatialPoint& point = points[i % points.size()];
                queries.push_back({point.x, point.y, queryRadius});
            }
            else
            {
                queries.push_back({location(random), location(random), queryRadius});
            }
        }

        return queries;
    }

    // Find expected results directly without using either spatial index.
    std::vector<std::vector<SpatialPoint>> expectedResults(
        const std::vector<SpatialPoint>& points, const std::vector<Query>& queries)
    {
        std::vector<std::vector<SpatialPoint>> expected;
        expected.reserve(queries.size());

        for (const Query& query : queries)
        {
            std::vector<SpatialPoint> found;

            for (const SpatialPoint& point : points)
            {
                if (std::hypot(point.x - query.x, point.y - query.y) <= query.radius)
                {
                    found.push_back(point);
                }
            }

            sortPoints(found);
            expected.push_back(std::move(found));
        }

        return expected;
    }

    // Build an index, verify its answers, and measure query performance.
    template <typename Index, typename... ConstructorArgs>
    void benchmarkIndex(
        const char* name,
        const std::vector<SpatialPoint>& points,
        const std::vector<Query>& queries,
        const std::vector<std::vector<SpatialPoint>>& expected,
        ConstructorArgs&&... args)
    {
        const auto buildStart = Clock::now();
        Index index(std::forward<ConstructorArgs>(args)...);

        for (const SpatialPoint& point : points)
        {
            index.insert(point);
        }

        const auto buildEnd = Clock::now();

        if (index.size() != points.size())
        {
            throw std::runtime_error(std::string(name) + " did not retain all points");
        }

        // Check full records, including duplicates, outside the timed section.
        std::size_t expectedMatchCount = 0;

        for (std::size_t i = 0; i < queries.size(); ++i)
        {
            std::vector<SpatialPoint> actual =
                index.radiusQuery(queries[i].x, queries[i].y, queries[i].radius);
            sortPoints(actual);

            if (!equalPoints(actual, expected[i]))
            {
                throw std::runtime_error(std::string(name) +
                    " returned incorrect records for query " + std::to_string(i));
            }

            expectedMatchCount += expected[i].size();
        }

        // Repeat the same queries to reduce timing noise.
        constexpr std::size_t repetitions = 5;
        std::size_t totalMatches = 0;
        const auto queryStart = Clock::now();

        for (std::size_t iteration = 0; iteration < repetitions; ++iteration)
        {
            for (const Query& query : queries)
            {
                totalMatches += index.radiusQuery(query.x, query.y, query.radius).size();
            }
        }

        const auto queryEnd = Clock::now();

        if (totalMatches != expectedMatchCount * repetitions)
        {
            throw std::runtime_error(std::string(name) + " produced inconsistent timed results");
        }

        const double buildMs =
            std::chrono::duration<double, std::milli>(buildEnd - buildStart).count();
        const double queryUs =
            std::chrono::duration<double, std::micro>(queryEnd - queryStart).count() /
            static_cast<double>(queries.size() * repetitions);

        const auto oldFlags = std::cout.flags();
        const auto oldPrecision = std::cout.precision();

        std::cout << std::fixed << std::setprecision(3)
                  << name << ": build " << buildMs << " ms; query "
                  << queryUs << " us/query; verified " << queries.size()
                  << " queries\n";

        std::cout.flags(oldFlags);
        std::cout.precision(oldPrecision);
    }
}

// Run both spatial hashing approaches on the same verified query workload.
void Benchmark::run(const std::vector<SpatialPoint>& points)
{
    for (const SpatialPoint& point : points)
    {
        if (!std::isfinite(point.x) || !std::isfinite(point.y))
        {
            throw std::invalid_argument("Dataset coordinates must be finite");
        }
    }

    const auto queries = makeQueries(points);
    const auto expected = expectedResults(points, queries);

    constexpr double cellSize = 10.0;
    constexpr std::size_t densityThreshold = 16;
    constexpr int maxDepth = 6;

    benchmarkIndex<SpatialHash>("fixed", points, queries, expected, cellSize);
    benchmarkIndex<AdaptiveSpatialHash>(
        "adaptive", points, queries, expected, cellSize, densityThreshold, maxDepth);
}
