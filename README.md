# Density-Adaptive Spatial Hashing — Team Starter

This is an intentionally **incomplete** C++17 project skeleton, not a finished research implementation.

## Build and run (CMake)
```bash
cmake -S . -B build
cmake --build build
./build/spatial_demo
ctest --test-dir build --output-on-failure
```
On Windows, the demo executable may be under `build/Debug/` depending on the generator.

## Work allocation (suggested)
- **Team 1 — Fixed grid:** `SpatialHash.h/.cpp`: cell key/hash, insertion, radius queries, unit tests.
- **Team 2 — Adaptive grid:** `AdaptiveSpatialHash.h/.cpp`: subdivisions, insertion, radius queries, tests.
- **Team 3 — Data:** `DatasetGenerator.h/.cpp`: skewed/clustered distributions, seeded experiments, dataset documentation.
- **Team 4 — Experiments:** `Benchmark.h/.cpp`: correctness, timings, candidate comparisons, memory estimates, CSV output.

## Shared interface agreement
- Coordinates are 2D doubles. Point IDs are `std::size_t`.
- `radiusQuery(x,y,r)` returns all points at Euclidean distance <= r, including boundary points.
- Both indices must use identical input and queries. Agree on duplicate-ID handling and ordering before implementation.
- Agree on baseline cell size, adaptive density threshold, max depth, and query radii before running comparisons.
- All performance measurements should be made in a release/optimized build and repeated across seeds.

## Current status
- **Implemented:** point type, parameter checks, seeded uniform and 3-cluster dataset generation, main driver, build system, scaffold tests.
- **Not implemented:** actual fixed/adaptive hashing, query logic, real benchmark metrics, CSV results.
- Unfinished hashing methods deliberately throw `std::logic_error` rather than pretending to work.
- Existing tests verify only the scaffold, not the algorithms.

## Integration checklist
- [ ] Agree on API and cell-boundary semantics.
- [ ] Implement fixed-grid index and test it.
- [ ] Implement adaptive index and test it.
- [ ] Add brute-force query oracle and compare returned point IDs.
- [ ] Extend data distributions and query workloads.
- [ ] Implement fair benchmarks and export CSV.
- [ ] Record configurations, results, limitations, and team contributions.
