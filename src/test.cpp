#include "PointLocation.hpp"
#include <iostream>
void test() {
    std::shared_ptr<triangulateio> test = TriangleManipulator::create_instance();
    TriangleManipulator::read_poly_file("test.poly", test.get());
    PointLocation::PlanarGraph graph = PointLocation::PlanarGraph(test.get());
    PointLocation::RemovedVertexInfo info = graph.remove_vertex(4);
    std::vector<unsigned int> polygon = {0, 1, 2, 3, 4};
    std::vector<unsigned int> triangulation = PointLocation::triangulate(graph, &polygon, nullptr);
    for(unsigned int triangle : triangulation) {
        PointLocation::Triangle tri = graph.all_triangles[triangle];
        std::cout << tri.vertex_one << ", " << tri.vertex_two << ", " << tri.vertex_three << std::endl;
    }
}
int main() {
    test();
}