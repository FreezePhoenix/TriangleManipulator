# CMakeList.txt : CMake project for Assembly, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.5)
add_definitions(-std=c++20 -ggdb -O3 -Wall -static)
# add_definitions(-DTRILIBRARY)

project("TriangleManipulator")

include_directories(
   "./include"
)

add_library(TriangleManipulator STATIC
   "src/TriangleManipulator.cpp"
   "src/ShapeManipulator.cpp"
   "src/PointLocation.cpp"
)

add_executable(TriangleManipulatorExecutable
   "src/test.cpp"
)

<<<<<<< HEAD
set_property(TARGET TriangleManipulator PROPERTY COMPILE_OPTIONS -DTRIANGLE_MANIPULATOR_LIBRARY)

add_subdirectory(./triangle ./triangle)

target_link_libraries(TriangleManipulator triangle fmt)
target_link_libraries(TriangleManipulatorExecutable TriangleManipulator triangle fmt)
=======
set_property(TARGET TriangleManipulator PROPERTY COMPILE_OPTIONS "-DTRIANGLE_MANIPULATOR_LIBRARY -S")
add_subdirectory(./triangle ./triangle)

target_include_directories(TriangleManipulator PUBLIC "./include")
target_link_libraries(TriangleManipulator triangle)
target_link_libraries(TriangleManipulatorExecutable TriangleManipulator triangle)
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
