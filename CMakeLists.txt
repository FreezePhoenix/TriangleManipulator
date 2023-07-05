cmake_minimum_required (VERSION 3.5)
project ("TriangleManipulator")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall")


# set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
# set(BUILD_SHARED_LIBS OFF)
# set(CMAKE_EXE_LINKER_FLAGS "-static")

add_library(TriangleManipulator_HEADERS INTERFACE)

target_include_directories(TriangleManipulator_HEADERS INTERFACE include lib/fmt/include)
target_link_libraries(TriangleManipulator_HEADERS INTERFACE Triangle_HEADERS)

add_library(TriangleManipulator OBJECT
   "src/TriangleManipulator.cpp"
   "src/ShapeManipulator.cpp"
   "src/PointLocation.cpp"
)

set_property(TARGET TriangleManipulator PROPERTY COMPILE_OPTIONS -DTRIANGLE_MANIPULATOR_LIBRARY)

add_subdirectory(lib/triangle lib/triangle EXCLUDE_FROM_ALL)
add_subdirectory(lib/fmt lib/fmt EXCLUDE_FROM_ALL)

target_link_libraries(TriangleManipulator TriangleManipulator_HEADERS Triangle fmt)
