#include "TriangleManipulator.hpp"
#include "PointLocation.hpp"
#include <iostream>
#include <math.h>
#include "fmt/os.h"

typedef float float32x4_t __attribute__ ((__vector_size__ (16)));
void test() {
    std::shared_ptr<triangulateio> test = TriangleManipulator::create_instance();
    TriangleManipulator::read_poly_file("test.poly", test);
    TriangleManipulator::write_poly_file("testa.poly", test);
    PointLocation::GraphInfo info = PointLocation::create_graph(test);
    info.process();
    // info.planar_graph->write_to_file("Planar");
    // PointLocation::PlanarGraph* graphh = PointLocation::PlanarGraph::from_file("Planar");
    // std::cout << graphh->all_triangles[2].vertex_three << std::endl;
    // std::cout << sizeof(int) << "," << sizeof(int[graphh->all_triangles[2].vertex_three]) << std::endl;
}
int main() {
    test();
}