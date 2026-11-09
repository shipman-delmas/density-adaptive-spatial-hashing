
/*
 * SpatialHash.h
 *
 * Defines the interface for a traditional fixed-grid spatial hashing algorithm.
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

#pragma once

#include "SpatialPoint.h"
#include <cstddef>
#include <functional>
#include <unordered_map>
#include <utility>
#include <vector>

//Hash a pair of integer grid coordinates for use as a key in our unordered_map
struct CellHash {
    std::size_t operator()(const std::pair<int, int>& cell) const noexcept;
};

//TEAM 1: Traditional fixed-resolution spatial hashing baseline
//This class stores points in fixed-size grid cells and supports radius searches
class SpatialHash {
public:
    //Constructor that initializes the size of each grid cell
    explicit SpatialHash(double cellSize);

    //Insert a spatial point into its appropriate grid cell
    void insert(const SpatialPoint& point);

    //Return all stored points within the requested radius of (x, y)
    std::vector<SpatialPoint> radiusQuery(double x, double y, double radius) const;

    //Return the total number of points stored in our grid
    std::size_t size() const noexcept { return pointCount_; }

private:
    //Represent grid cell coordinates as a pair of integers
    using Cell = std::pair<int, int>;

    //Convert original X and Y coordinates into grid cell coordinates
    Cell cellFor(double x, double y) const;

    //Store the fixed size of each grid cell
    double cellSize_;

    //Track the total number of stored points
    std::size_t pointCount_{0};

    //Store grid cells as keys and vectors of spatial points as values
    //CellHash provides the hashing function for our grid cell keys
    std::unordered_map<Cell, std::vector<SpatialPoint>, CellHash> grid_;
};
