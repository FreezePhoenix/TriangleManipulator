# CMakeList.txt : CMake project for Assembly, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.5)
add_definitions(-std=c++17 -w)
add_definitions(-DTRILIBRARY)

add_definitions(-g)
project("TriangleManipulator")

add_library(TriangleManipulator OBJECT
   "src/TriangleManipulator.cpp"
)

add_subdirectory(./triangle ./triangle)

target_link_libraries(TriangleManipulator triangle)