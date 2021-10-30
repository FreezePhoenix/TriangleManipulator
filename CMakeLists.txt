# CMakeList.txt : CMake project for Assembly, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.5)
add_definitions(-std=c++20 -O3 -Wall)
project("TriangleManipulator")

add_library(TriangleManipulator_HEADERS INTERFACE)

target_include_directories(TriangleManipulator_HEADERS INTERFACE include lib/fmt/include)
target_link_libraries(TriangleManipulator_HEADERS INTERFACE Triangle_HEADERS)

add_library(TriangleManipulator OBJECT
   "src/TriangleManipulator.cpp"
   "src/ShapeManipulator.cpp"
   "src/PointLocation.cpp"
)

add_executable(TriangleManipulatorExecutable
   "src/test.cpp"
)

set_property(TARGET TriangleManipulator PROPERTY COMPILE_OPTIONS -DTRIANGLE_MANIPULATOR_LIBRARY)

add_subdirectory(lib/triangle lib/triangle EXCLUDE_FROM_ALL)
add_subdirectory(lib/fmt lib/fmt EXCLUDE_FROM_ALL)

target_link_libraries(TriangleManipulator TriangleManipulator_HEADERS Triangle fmt)
target_link_libraries(TriangleManipulatorExecutable TriangleManipulator)
