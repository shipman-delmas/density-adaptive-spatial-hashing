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

    constexpr std::size_t trialCount = 3;
    constexpr std::size_t queryCount = 250;
    constexpr std::size_t queryRepetitions = 5;
    constexpr double queryRadius = 5.0;
    constexpr double cellSize = 10.0;
    constexpr std::size_t densityThreshold = 16;
    constexpr int maxDepth = 6;

    struct Query
    {
        double x;
        double y;
        double radius;
    };

    struct TrialResult
    {
        double buildMs;
        double queryUs;
        double candidatesPerQuery;
        double estimatedMemoryBytes;
    };

    struct Statistics
    {
        double mean;
        double stddev;
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

    // Generate the same repeatable query workload for both algorithms.
    std::vector<Query> makeQueries(const std::vector<SpatialPoint>& points)
    {
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

    // Find reference answers by checking every point directly.
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

    // Build a fresh index, verify its answers, and collect one trial's measurements.
    template <typename Index, typename... ConstructorArgs>
    TrialResult benchmarkIndex(
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

        std::size_t expectedMatchCount = 0;
        std::size_t totalCandidates = 0;

        // Verification and candidate counting are outside the performance timers.
        // This pass also warms the same query workload for both algorithms.
        for (std::size_t i = 0; i < queries.size(); ++i)
        {
            std::size_t candidates = 0;
            std::vector<SpatialPoint> actual = index.radiusQuery(
                queries[i].x, queries[i].y, queries[i].radius, &candidates);
            sortPoints(actual);

            if (!equalPoints(actual, expected[i]))
            {
                throw std::runtime_error(std::string(name) +
                    " returned incorrect records for query " + std::to_string(i));
            }

            totalCandidates += candidates;
            expectedMatchCount += expected[i].size();
        }

        std::size_t totalMatches = 0;
        const auto queryStart = Clock::now();

        for (std::size_t iteration = 0; iteration < queryRepetitions; ++iteration)
        {
            for (const Query& query : queries)
            {
                totalMatches += index.radiusQuery(query.x, query.y, query.radius).size();
            }
        }

        const auto queryEnd = Clock::now();

        if (totalMatches != expectedMatchCount * queryRepetitions)
        {
            throw std::runtime_error(std::string(name) + " produced inconsistent timed results");
        }

        return {
            std::chrono::duration<double, std::milli>(buildEnd - buildStart).count(),
            std::chrono::duration<double, std::micro>(queryEnd - queryStart).count() /
                static_cast<double>(queries.size() * queryRepetitions),
            static_cast<double>(totalCandidates) / static_cast<double>(queries.size()),
            static_cast<double>(index.estimatedMemoryBytes())
        };
    }

    // Calculate the mean and sample standard deviation across rebuilt trials.
    Statistics statistics(const std::vector<double>& values)
    {
        double mean = 0.0;
        double sumSquaredDifferences = 0.0;
        std::size_t count = 0;

        for (const double value : values)
        {
            ++count;
            const double difference = value - mean;
            mean += difference / static_cast<double>(count);
            sumSquaredDifferences += difference * (value - mean);
        }

        const double stddev = count > 1
            ? std::sqrt(std::max(0.0, sumSquaredDifferences / static_cast<double>(count - 1)))
            : 0.0;

        return {mean, stddev};
    }

    // Print the four required measurements as mean and sample standard deviation.
    void printResults(const char* name, const std::vector<TrialResult>& trials)
    {
        std::vector<double> build, query, candidates, memory;

        for (const TrialResult& trial : trials)
        {
            build.push_back(trial.buildMs);
            query.push_back(trial.queryUs);
            candidates.push_back(trial.candidatesPerQuery);
            memory.push_back(trial.estimatedMemoryBytes);
        }

        const Statistics buildStats = statistics(build);
        const Statistics queryStats = statistics(query);
        const Statistics candidateStats = statistics(candidates);
        const Statistics memoryStats = statistics(memory);
        const auto oldFlags = std::cout.flags();
        const auto oldPrecision = std::cout.precision();

        std::cout << std::fixed << std::setprecision(3)
                  << name << ": build " << buildStats.mean << " +/- " << buildStats.stddev << " ms; "
                  << "query " << queryStats.mean << " +/- " << queryStats.stddev << " us/query; "
                  << "candidates " << candidateStats.mean << " +/- " << candidateStats.stddev << " per query; "
                  << "estimated memory " << memoryStats.mean << " +/- " << memoryStats.stddev << " bytes\n";

        std::cout.flags(oldFlags);
        std::cout.precision(oldPrecision);
    }
}

// Compare both algorithms over repeated builds of the same dataset and queries.
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
    std::vector<TrialResult> fixedTrials, adaptiveTrials;
    fixedTrials.reserve(trialCount);
    adaptiveTrials.reserve(trialCount);

    // Each trial rebuilds each index. Alternate execution order to reduce bias.
    for (std::size_t trial = 0; trial < trialCount; ++trial)
    {
        if (trial % 2 == 0)
        {
            fixedTrials.push_back(benchmarkIndex<SpatialHash>(
                "fixed", points, queries, expected, cellSize));
            adaptiveTrials.push_back(benchmarkIndex<AdaptiveSpatialHash>(
                "adaptive", points, queries, expected, cellSize, densityThreshold, maxDepth));
        }
        else
        {
            adaptiveTrials.push_back(benchmarkIndex<AdaptiveSpatialHash>(
                "adaptive", points, queries, expected, cellSize, densityThreshold, maxDepth));
            fixedTrials.push_back(benchmarkIndex<SpatialHash>(
                "fixed", points, queries, expected, cellSize));
        }
    }

    // Memory is a structure estimate, excluding allocator and query temporaries.
    // Standard deviations describe trial averages, not individual query latencies.
    std::cout << "Trials: " << trialCount << "; queries: " << queryCount
              << "; query repetitions: " << queryRepetitions << "; query seed: 12345"
              << "; radius: " << queryRadius << "; cell size: " << cellSize
              << "; threshold: " << densityThreshold << "; max depth: " << maxDepth << '\n';

    printResults("fixed", fixedTrials);
    printResults("adaptive", adaptiveTrials);
}
