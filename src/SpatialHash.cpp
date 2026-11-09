
/*
 * SpatialHash.cpp
 *
 * Implements a traditional fixed-grid spatial hashing algorithm.
 *
 * Initial implementation generated with assistance from OpenAI ChatGPT.
 * The implementation was subsequently modified, reviewed, documented, and tested by the authors.
 *
 * Algorithmic concepts are based on traditional spatial hashing
 * techniques, including mapping spatial coordinates to grid cells
 * and using hash tables for efficient spatial lookup.
 *
 * References:
 * Teschner et al. (2003). Optimized Spatial Hashing for Collision
 * Detection of Deformable Objects.
 *
 * Ericson, C. (2004). Real-Time Collision Detection.
 */

#include "SpatialHash.h"

#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>

//this is custom our hashing function
//cell contains 2 integers that represent the X and Y coordinates of our spatial grid where the values are paired ie (X, Y)
//std::hash<int> is the standard c++ hashing function
//h1 is where we generate the hash value for X and h2 is where we generate the has value for Y
//the combined has values are returned because C++'s unordered_map needs a single hash value to determine where to store or find a grid cell.
std::size_t CellHash::operator()(const std::pair<int, int>& cell) const noexcept {
    const std::size_t h1 = std::hash<int>{}(cell.first);
    const std::size_t h2 = std::hash<int>{}(cell.second);
    return h1 ^ (h2 + 0x9e3779b9U + (h1 << 6) + (h1 >> 2));
}

//This constructor takes in a double that represents the size of each grid cell
//cellSize_(cellSize) is the member initializer list. It is going to allow us to initialize the variable before the constructor body runs
//It checks the values to make sure the size is finite numeric value and greater and 0
//IF the check fails, it throws an invalid argument exception.
SpatialHash::SpatialHash(double cellSize) : cellSize_(cellSize) {
    if (!std::isfinite(cellSize) || cellSize <= 0.0) {
        throw std::invalid_argument("Cell size must be finite and positive");
    }
}

//This method takes in a (X, Y) from our original point coordinates.
//cellFor finds the cell that would contain the point passed in
SpatialHash::Cell SpatialHash::cellFor(double x, double y) const {
    //Once again, we check the values to make sure the x and y coordinates are finite numeric values
    if (!std::isfinite(x) || !std::isfinite(y)) {
        throw std::invalid_argument("Point coordinates must be finite");
    }

    //cellSize_ is initialized in our constructor.
    //we use the const key word so cx and cy are not modified
    //floor is important: negative coordinates must round DOWN.
    //this translates our passed in values into their coordinates in our fixed size grid
    const double cx = std::floor(x / cellSize_);
    const double cy = std::floor(y / cellSize_);

    //We grab the max and min integer values allowed in C++
    const double minCell = static_cast<double>(std::numeric_limits<int>::min());
    const double maxCell = static_cast<double>(std::numeric_limits<int>::max());

    //check the boundaries of the cells and compare it to our calculated max and min values and throw an exception if necessary
    //both values must pass the check
    if (cx < minCell || cx >= maxCell || cy < minCell || cy >= maxCell) {
        throw std::out_of_range("Grid coordinates exceed supported range");
    }

    //finally return the calculated grid cell coordinates as a pair of integers.
    return {static_cast<int>(cx), static_cast<int>(cy)};
}

//this method receives a spatial point by reference and inserts a copy into our fixed-size grid.
void SpatialHash::insert(const SpatialPoint& point) {
    //Here is where we start inserting points into our grid
    // cellFor(x, y) is called returning the calculated grid cell coordinates as a pair of integers.
    // grid_ is our unordered map
    //we either insert into an existing bucket, or create a new one using the [] operator
    //.push_back(point) is the line where we actually insert the point
    grid_[cellFor(point.x, point.y)].push_back(point);

    //we increment the point count by 1
    ++pointCount_;
}

//this method uses our grid to identify nearby cells, examines the points in those cells, and returns only those within the requested distance
std::vector<SpatialPoint> SpatialHash::radiusQuery(double x, double y, double radius) const {
    if (!std::isfinite(radius) || radius < 0.0) {
        //making sure the radius is a valid, finite number and isn't negative.
        throw std::invalid_argument("Radius must be finite and nonnegative");
    }

    // Check all grid cells intersecting the query's bounding square.
    //this gives us a range both high and low of cells to check for the requested radius
    const Cell low = cellFor(x - radius, y - radius);
    const Cell high = cellFor(x + radius, y + radius);

    //Create an empty vector to store the points that fall within our requested radius.
    std::vector<SpatialPoint> matches;

    //next we loop through all grid cells from the low coordinates to the high coordinates.
    //the outer loop handles X coordinates and the inner loop handles Y coordinates.
    for (long long cx = low.first; cx <= high.first; ++cx) {
        for (long long cy = low.second; cy <= high.second; ++cy) {

            //we find the bucket in out unordered map for our current grid cell coordinates cx and cy
            const auto it = grid_.find({static_cast<int>(cx), static_cast<int>(cy)});

            //if we don't find it, it will be equal to the grid_.end() call which means no matching entry was found
            //so we move to the next cell
            if (it == grid_.end()) continue;

            //it->second refers to the vector containing all points stored in that cell
            //so we are looking at each point in that cell one at a time
            for (const SpatialPoint& point : it->second) {

                //now we calculate the distance between the stored point and our query center
                // hypot avoids squaring large values and checks exact distance.
                //if the point distance is within the requested radius we mark it as qualified and add a copy of the point to our matches vector
                if (std::hypot(point.x - x, point.y - y) <= radius) {
                    matches.push_back(point);
                }
            }
        }
    }

    //Finally, return the matches vector containing all points within the requested radius.
    return matches;
}
