
#include "SpatialHash.h"
#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>

//This is our custom hashing function for integer grid-cell coordinates.
//The two coordinates are hashed separately and combined into one hash value.
std::size_t CellHash::operator()(const std::pair<int, int>& cell) const noexcept {
    const std::size_t h1 = std::hash<int>{}(cell.first);
    const std::size_t h2 = std::hash<int>{}(cell.second);
    return h1 ^ (h2 + 0x9e3779b9U + (h1 << 6) + (h1 >> 2));
}

//This constructor initializes the fixed size of our grid cells.
//The cell size must be a finite number greater than zero.
SpatialHash::SpatialHash(double cellSize) : cellSize_(cellSize) {
    if (!std::isfinite(cellSize) || cellSize <= 0.0) {
        throw std::invalid_argument("Cell size must be finite and positive");
    }
}

//Convert original X and Y point coordinates into integer grid-cell coordinates.
SpatialHash::Cell SpatialHash::cellFor(double x, double y) const {
    //Make sure both coordinates are finite numeric values.
    if (!std::isfinite(x) || !std::isfinite(y)) {
        throw std::invalid_argument("Point coordinates must be finite");
    }

    //Divide each coordinate by the cell size and round down.
    //floor() is important for correctly handling negative coordinates.
    const double cx = std::floor(x / cellSize_);
    const double cy = std::floor(y / cellSize_);

    //Retrieve the minimum and maximum values supported by an int.
    const double minCell = static_cast<double>(std::numeric_limits<int>::min());
    const double maxCell = static_cast<double>(std::numeric_limits<int>::max());

    //Check that the calculated grid coordinates fit within our allowed range.
    if (cx < minCell || cx >= maxCell || cy < minCell || cy >= maxCell) {
        throw std::out_of_range("Grid coordinates exceed supported range");
    }

    //Return the calculated grid-cell coordinates as a pair of integers.
    return {static_cast<int>(cx), static_cast<int>(cy)};
}

//Receive a spatial point by reference and store a copy in our grid.
void SpatialHash::insert(const SpatialPoint& point) {
    //Find the appropriate cell and create a bucket if one doesn't exist.
    //push_back() adds the point to the cell's vector.
    grid_[cellFor(point.x, point.y)].push_back(point);

    //Increment the total number of stored points.
    ++pointCount_;
}

//Search nearby grid cells and return points within the requested radius.
std::vector<SpatialPoint> SpatialHash::radiusQuery(
    double x, double y, double radius
) const {
    //Make sure the radius is finite and nonnegative.
    if (!std::isfinite(radius) || radius < 0.0) {
        throw std::invalid_argument("Radius must be finite and nonnegative");
    }

    //Determine the low and high grid cells of the query's bounding square.
    const Cell low = cellFor(x - radius, y - radius);
    const Cell high = cellFor(x + radius, y + radius);

    //Create an empty vector to store qualifying points.
    std::vector<SpatialPoint> matches;

    //Loop through all grid cells from low to high.
    //The outer loop handles X; the inner loop handles Y.
    for (long long cx = low.first; cx <= high.first; ++cx) {
        for (long long cy = low.second; cy <= high.second; ++cy) {

            //Find the bucket for the current grid-cell coordinates.
            const auto it = grid_.find({
                static_cast<int>(cx),
                static_cast<int>(cy)
            });

            //If no bucket exists, move to the next cell.
            if (it == grid_.end()) continue;

            //Examine each point stored in this cell.
            for (const SpatialPoint& point : it->second) {

                //Calculate the distance from the point to the query center.
                //hypot() avoids manually squaring potentially large values.
                if (std::hypot(point.x - x, point.y - y) <= radius) {

                    //Add a copy of the qualifying point to our results.
                    matches.push_back(point);
                }
            }
        }
    }

    //Return all points that fall within the requested radius.
    return matches;
}
