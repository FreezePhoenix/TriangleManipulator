#pragma once

#ifndef TRIANGLEMANIPULATOR_HPP_
#define TRIANGLEMANIPULATOR_HPP_


#define REAL double
#define VOID int

#include <functional>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <iterator>
#include <memory>
#include <iostream>
#include "../triangle/triangle.h"

namespace TriangleManipulator {
    void cleanup(triangulateio* instance);
    struct free_delete {
        void operator()(void * x) { 
            triangulateio* inp = (triangulateio *) x;
            std::cout << "Cleaning up triangulateio instance" << std::endl;
            free(inp->pointlist);
            free(inp->pointattributelist);
            free(inp->pointmarkerlist);
            free(inp->trianglelist);
            free(inp->triangleattributelist);
            free(inp->neighborlist);
            free(inp->segmentlist);
            free(inp->segmentmarkerlist);
            std::cout << inp << std::endl;
            std::cout << (double *) NULL << std::endl;
            std::cout << inp->holelist << std::endl;
            if(inp->holelist != (double *) NULL) {
                // free(inp->holelist);
                inp->holelist = (double *) NULL;
            }
            free(inp->regionlist);
            free(inp->edgelist);
            free(inp->edgemarkerlist);
            free(inp->normlist);
        }
    };
    inline std::shared_ptr<triangulateio> create_instance() {
        std::shared_ptr<triangulateio> res = std::shared_ptr<triangulateio>(new triangulateio(), cleanup);
        res->pointlist = (REAL *) NULL;
        res->pointattributelist = (REAL *) NULL;
        res->pointmarkerlist = (int *) NULL;
        res->numberofpoints = 0;
        res->numberofpointattributes = 0;
        
        res->trianglelist = (int *) NULL;
        res->triangleattributelist = (REAL *) NULL;
        res->neighborlist = (int *) NULL;
        res->numberoftriangles = 0;
        res->numberofcorners = 0;
        res->numberoftriangleattributes = 0;
        
        res->segmentlist = (int *) NULL;
        res->segmentmarkerlist = (int *) NULL;
        res->numberofsegments = 0;
        
        res->holelist = (REAL *) NULL;
        res->numberofholes = 0;

        res->regionlist = (REAL *) NULL;
        res->numberofregions = 0;
        
        res->edgelist = (int *) NULL;
        res->edgemarkerlist = (int *) NULL;
        res->normlist = (REAL *) NULL;
        res->numberofedges = 0;
        return res;
    }
    void report(triangulateio* io, int markers, int reporttriangles, int reportneighbors, int reportsegments, int reportedges, int reportnorms);
    void filter_edges(triangulateio* input, triangulateio* output, std::function<bool(int,int,REAL,REAL)> predicate);
    void filter_points(triangulateio* input, triangulateio* output, std::function<bool(int, REAL, REAL, REAL)> predicate);
    void inject_holes(triangulateio* input, triangulateio* output);
    std::vector<float> read_line(std::istream& stream);
    void print_vector(std::vector<float>& vec);
    void read_node_section(std::istream& file, triangulateio* in);
    void write_node_section(std::ostream& file, triangulateio* out);
    void read_node_file(std::string filename, triangulateio* in);
    void read_poly_file(std::string filename, triangulateio* in);
    void write_node_file(std::string filename, triangulateio* out);
    void write_poly_file(std::string filename, triangulateio* out);
    void write_edge_file(std::string filename, triangulateio* out);
    void write_ele_file(std::string filename, triangulateio* out);
    void write_neigh_file(std::string filename, triangulateio* out);
}

#endif /* TRIANGLEMANIPULATOR_HPP_ */