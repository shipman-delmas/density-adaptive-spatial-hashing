cmake_minimum_required(VERSION 3.16)
project(SpatialHashingProject LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_library(spatial_core
    src/SpatialHash.cpp src/AdaptiveSpatialHash.cpp
    src/DatasetGenerator.cpp src/Benchmark.cpp)
target_include_directories(spatial_core PUBLIC include)
add_executable(spatial_demo src/main.cpp)
target_link_libraries(spatial_demo PRIVATE spatial_core)
add_executable(spatial_tests tests/test_main.cpp)
target_link_libraries(spatial_tests PRIVATE spatial_core)
enable_testing()
add_test(NAME scaffold_tests COMMAND spatial_tests)
