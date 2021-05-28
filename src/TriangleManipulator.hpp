#pragma once

#ifndef TRIANGLEMANIPULATOR_HPP_
#define TRIANGLEMANIPULATOR_HPP_

#ifdef SINGLE
#define REAL float
#else /* not SINGLE */
#define REAL double
#endif /* not SINGLE */
#define VOID int

#include <functional>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <iterator>
#include <iostream>
#include "../triangle/triangle.h"

namespace TriangleManipulator {
    triangulateio* create_instance();
    void report(triangulateio* io, int markers, int reporttriangles, int reportneighbors, int reportsegments, int reportedges, int reportnorms);
    void filter_edges(triangulateio* input, triangulateio* output, std::function<bool(int,int,REAL,REAL)> predicate);
    void filter_points(triangulateio* input, triangulateio* output, std::function<bool(int, REAL, REAL, REAL)> predicate);
    void inject_holes(triangulateio* input, triangulateio* output);
    std::vector<float> read_line(std::istream& stream);
    void print_vector(std::vector<float>& vec);
    void read_node_section(std::istream& file, triangulateio* in);
    void write_node_section(std::ostream& file, triangulateio* out);
    void read_node_file(std::string filename, triangulateio* in);
    void write_node_file(std::string filename, triangulateio* out);
    void read_poly_file(std::string filename, triangulateio* in);
    void write_poly_file(std::string filename, triangulateio* out);
    void write_edge_file(std::string filename, triangulateio* out);
    void cleanup(triangulateio* instance);
}

#endif /* TRIANGLEMANIPULATOR_HPP_ */