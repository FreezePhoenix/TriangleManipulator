# CMakeList.txt : CMake project for Assembly, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.5)
add_definitions(-std=c++20 -O3 -Wall -g)
# add_definitions(-DTRILIBRARY)

project("TriangleManipulator")

include_directories(
   "./include"
   "./lib/triangle"
)

add_library(TriangleManipulator STATIC
   "src/TriangleManipulator.cpp"
   "src/ShapeManipulator.cpp"
   "src/PointLocation.cpp"
)

add_executable(TriangleManipulatorExecutable
   "src/test.cpp"
)

set_property(TARGET TriangleManipulator PROPERTY COMPILE_OPTIONS -DTRIANGLE_MANIPULATOR_LIBRARY)

add_subdirectory(lib/triangle)
add_subdirectory(lib/fmt)

target_link_libraries(TriangleManipulator triangle fmt)
target_link_libraries(TriangleManipulatorExecutable TriangleManipulator triangle fmt)
