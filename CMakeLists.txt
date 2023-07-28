cmake_minimum_required (VERSION 3.5)
project ("TriangleManipulator")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

add_subdirectory(lib/triangle lib/triangle EXCLUDE_FROM_ALL)
add_subdirectory(lib/fmt lib/fmt EXCLUDE_FROM_ALL)

add_library(TriangleManipulator_HEADERS INTERFACE)

add_library(TriangleManipulator OBJECT
   "src/TriangleManipulator.cpp"
   "src/ShapeManipulator.cpp"
   "src/PointLocation.cpp"
)

target_include_directories(TriangleManipulator_HEADERS INTERFACE include lib/fmt/include)
target_link_libraries(TriangleManipulator_HEADERS INTERFACE Triangle_HEADERS)

target_link_libraries(TriangleManipulator TriangleManipulator_HEADERS Triangle fmt)
