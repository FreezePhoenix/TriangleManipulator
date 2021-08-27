#include "TriangleManipulator.hpp"
#include "PointLocation.hpp"
#include <iostream>
#include <math.h>
<<<<<<< HEAD
#include "fmt/os.h"

typedef float float32x4_t __attribute__ ((__vector_size__ (16)));
=======

/******************************************************************************

                              Online C++ Compiler.
               Code, Compile, Run and Debug C++ program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/


>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
void test() {
    std::shared_ptr<triangulateio> test = TriangleManipulator::create_instance();
    TriangleManipulator::read_poly_file("test.poly", test);
    TriangleManipulator::write_poly_file("testa.poly", test);
    PointLocation::GraphInfo info = PointLocation::create_graph(test);
<<<<<<< HEAD
    info.process();
    // info.planar_graph->write_to_file("Planar");
    // PointLocation::PlanarGraph* graphh = PointLocation::PlanarGraph::from_file("Planar");
    // std::cout << graphh->all_triangles[2].vertex_three << std::endl;
    // std::cout << sizeof(int) << "," << sizeof(int[graphh->all_triangles[2].vertex_three]) << std::endl;
=======
    int last = 0;
    while(info.planar_graph->triangulations[info.planar_graph->triangulations.size() - 1].size() > 1) {
        info.planar_graph->remove_vertices(info.planar_graph->find_independant_set(), info.directed_graph);
        if(last == info.planar_graph->triangulations.back().size()) {
            std::cout << "Error: Could not simplify more" << std::endl;
            std::cout << last << std::endl;
            break;
        }
        last = info.planar_graph->triangulations.back().size();
    }
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
}
int main() {
    test();
}