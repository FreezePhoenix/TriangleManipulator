# CMakeList.txt : CMake project for Assembly, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.5)
add_definitions(-std=c++20 -w -g)
add_definitions(-DTRILIBRARY)

project("TriangleManipulator")

add_library(TriangleManipulator STATIC
   "src/TriangleManipulator.cpp"
   "src/ShapeManipulator.cpp"
   "src/PointLocation.cpp"
)

add_executable(TriangleManipulatorExecutable
   "src/test.cpp"
)

set_property(TARGET TriangleManipulator PROPERTY COMPILE_OPTIONS "-DTRIANGLE_MANIPULATOR_LIBRARY -S")
add_subdirectory(./triangle ./triangle)

target_include_directories(TriangleManipulator PUBLIC "./include")
target_link_libraries(TriangleManipulator triangle)
target_link_libraries(TriangleManipulatorExecutable TriangleManipulator triangle)
