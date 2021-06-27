#include "PointLocation.hpp"
#include <iostream>
#include <math.h>

/******************************************************************************

                              Online C++ Compiler.
               Code, Compile, Run and Debug C++ program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/


void test() {
    std::shared_ptr<triangulateio> test = TriangleManipulator::create_instance();
    TriangleManipulator::read_poly_file("test.poly", test);
    TriangleManipulator::write_poly_file("testa.poly", test);
    PointLocation::GraphInfo info = PointLocation::create_graph(test);
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
}
int main() {
    test();
}