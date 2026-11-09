#include "DatasetGenerator.h"
#include "SpatialHash.h"
#include "AdaptiveSpatialHash.h"
#include <cassert>
#include <iostream>
#include <stdexcept>

int main() {
    const auto a = DatasetGenerator::uniform(100, 42);
    const auto b = DatasetGenerator::uniform(100, 42);
    assert(a.size() == 100 && b.size() == 100);
    for (std::size_t i = 0; i < a.size(); ++i)
        assert(a[i].id == b[i].id && a[i].x == b[i].x && a[i].y == b[i].y);
    assert(DatasetGenerator::clustered(50).size() == 50);

//Test the skewed dataset generator.
const auto skewed = DatasetGenerator::skewed(1000, 42);
const auto skewedRepeat = DatasetGenerator::skewed(1000, 42);

//Verify that both datasets contain the requested number of points.
assert(skewed.size() == 1000);
assert(skewedRepeat.size() == 1000);

std::size_t leftHalf = 0;

for (std::size_t i = 0; i < skewed.size(); ++i) {
    //Verify that the same seed produces identical points.
    assert(skewed[i].id == skewedRepeat[i].id);
    assert(skewed[i].x == skewedRepeat[i].x);
    assert(skewed[i].y == skewedRepeat[i].y);

    //Verify that coordinates remain within our 100 x 100 domain.
    assert(skewed[i].x >= 0.0 && skewed[i].x <= 100.0);
    assert(skewed[i].y >= 0.0 && skewed[i].y <= 100.0);

    //Count points located in the left half of the domain.
    if (skewed[i].x < 50.0) {
        ++leftHalf;
    }
}

//Verify that the skewed distribution favors the left half.
assert(leftHalf > 600);


//Test the hotspot dataset generator.
const auto hotspot = DatasetGenerator::hotspot(1000, 42);
const auto hotspotRepeat = DatasetGenerator::hotspot(1000, 42);

assert(hotspot.size() == 1000);
assert(hotspotRepeat.size() == 1000);

std::size_t hotspotPoints = 0;

for (std::size_t i = 0; i < hotspot.size(); ++i) {
    //Verify reproducibility.
    assert(hotspot[i].id == hotspotRepeat[i].id);
    assert(hotspot[i].x == hotspotRepeat[i].x);
    assert(hotspot[i].y == hotspotRepeat[i].y);

    //Verify that coordinates remain within the domain.
    assert(hotspot[i].x >= 0.0 && hotspot[i].x <= 100.0);
    assert(hotspot[i].y >= 0.0 && hotspot[i].y <= 100.0);

    //Count points physically located inside the hotspot region.
    if (hotspot[i].x >= 45.0 && hotspot[i].x <= 55.0 &&
        hotspot[i].y >= 45.0 && hotspot[i].y <= 55.0) {
        ++hotspotPoints;
    }
}

//At least 900 points should fall inside the hotspot.
assert(hotspotPoints >= 900);


//Verify that requesting zero points produces an empty dataset.
assert(DatasetGenerator::uniform(0).empty());
assert(DatasetGenerator::clustered(0).empty());
assert(DatasetGenerator::skewed(0).empty());
assert(DatasetGenerator::hotspot(0).empty());

std::cout << "DatasetGenerator tests passed.\n";

    bool rejected = false;
    try { SpatialHash invalid(0.0); } catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);
    rejected = false;
    try { AdaptiveSpatialHash invalid(1.0, 0, 4); } catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);
    // TODO (Teams 1/2): Add insert/query tests, boundary tests, and brute-force checks.
    std::cout << "Scaffold tests passed; hashing behavior is NOT tested yet.\n";
}
