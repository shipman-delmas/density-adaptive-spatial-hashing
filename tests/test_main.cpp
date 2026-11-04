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
    bool rejected = false;
    try { SpatialHash invalid(0.0); } catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);
    rejected = false;
    try { AdaptiveSpatialHash invalid(1.0, 0, 4); } catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);
    // TODO (Teams 1/2): Add insert/query tests, boundary tests, and brute-force checks.
    std::cout << "Scaffold tests passed; hashing behavior is NOT tested yet.\n";
}
