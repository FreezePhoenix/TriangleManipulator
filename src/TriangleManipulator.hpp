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
    inline std::shared_ptr<triangulateio> create_instance() {
        std::shared_ptr<triangulateio> res = std::shared_ptr<triangulateio>(new triangulateio(), [](triangulateio* instance) { 
            if(instance->pointlist != nullptr) {
                free(instance->pointlist);
                instance->pointlist = nullptr;
            }
            if(instance->pointattributelist != nullptr) {
                free(instance->pointattributelist);
                instance->pointattributelist = nullptr;
            }
            if(instance->pointmarkerlist != nullptr) {
                free(instance->pointmarkerlist);
                instance->pointmarkerlist = nullptr;
            }
            instance->numberofpoints = 0;
            instance->numberofpointattributes = 0;
            
            if(instance->trianglelist != nullptr) {
                free(instance->trianglelist);
                instance->trianglelist = nullptr;
            }
            if(instance->triangleattributelist != nullptr) {
                free(instance->triangleattributelist);
                instance->triangleattributelist = nullptr;
            }
            if(instance->neighborlist != nullptr) {
                free(instance->neighborlist);
                instance->neighborlist = nullptr;
            }
            instance->numberoftriangles = 0;
            instance->numberofcorners = 0;
            instance->numberoftriangleattributes = 0;
            
            if(instance->segmentlist != nullptr) {
                free(instance->segmentlist);
                instance->segmentlist = nullptr;
            }
            if(instance->segmentmarkerlist != nullptr) {
                free(instance->segmentmarkerlist);
                instance->segmentmarkerlist = nullptr;
            }
            instance->numberofsegments = 0;
            
            if(instance->holelist != nullptr) {
                free(instance->holelist);
                instance->holelist = nullptr;
            }
            instance->numberofholes = 0;

            if(instance->regionlist != nullptr) {
                free(instance->regionlist);
                instance->regionlist = nullptr;
            }
            instance->numberofregions = 0;
            
            if(instance->edgelist != nullptr) {
                free(instance->edgelist);
                instance->edgelist = nullptr;
            }
            if(instance->edgemarkerlist != nullptr) {
                free(instance->edgemarkerlist);
                instance->edgemarkerlist = nullptr;
            }
            if(instance->normlist != nullptr) {
                free(instance->normlist);
                instance->normlist = nullptr;
            }
            instance->numberofedges = 0;
        });
        res->pointlist = nullptr;
        res->pointattributelist = nullptr;
        res->pointmarkerlist = nullptr;
        res->numberofpoints = 0;
        res->numberofpointattributes = 0;
        
        res->trianglelist = nullptr;
        res->triangleattributelist = nullptr;
        res->neighborlist = nullptr;
        res->numberoftriangles = 0;
        res->numberofcorners = 0;
        res->numberoftriangleattributes = 0;
        
        res->segmentlist = nullptr;
        res->segmentmarkerlist = nullptr;
        res->numberofsegments = 0;
        
        res->holelist = nullptr;
        res->numberofholes = 0;

        res->regionlist = nullptr;
        res->numberofregions = 0;
        
        res->edgelist = nullptr;
        res->edgemarkerlist = nullptr;
        res->normlist = nullptr;
        res->numberofedges = 0;
        return res;
    }
    void report(std::shared_ptr<triangulateio> io, int markers, int reporttriangles, int reportneighbors, int reportsegments, int reportedges, int reportnorms);
    inline void filter_edges(std::shared_ptr<triangulateio> input, std::shared_ptr<triangulateio> output, std::function<bool(const int&, const int& , const double&, const double&)> predicate) {
        int edges_found = 0;
        std::vector<int> found_edges = std::vector<int>();
        int edges = input->numberofedges;
        bool markers = input->edgemarkerlist != nullptr;
        int remaining_edges = edges;
        while(remaining_edges --> 0) {
            const int offset = edges - remaining_edges - 1;
            const int double_offset = offset * 2;
            if(predicate(input->edgelist[double_offset], input->edgelist[double_offset + 1], input->normlist[double_offset], input->normlist[double_offset + 1])) {
                found_edges.push_back(offset);
                edges_found++;
            }
        }
        remaining_edges = edges_found;
        output->numberofedges = edges_found;
        output->edgelist = (int *) malloc(edges_found * 2 * sizeof(int));
        output->normlist = (REAL *) malloc(edges_found * 2 * sizeof(REAL));
        if(markers) {
            output->edgemarkerlist = (int *) malloc(edges_found * sizeof(int));
        }
        if(markers) {
            while(remaining_edges --> 0) {
                const int edge = found_edges[edges_found - (remaining_edges + 1)];
                int p1 = input->edgelist[2 * edge];
                int p2 = input->edgelist[2 * edge + 1];
                REAL norm1 = input->normlist[2 * edge];
                REAL norm2 = input->normlist[2 * edge + 1];
                output->edgelist[2 * (edges_found - (remaining_edges + 1))] = p1;
                output->edgelist[2 * (edges_found - (remaining_edges + 1)) + 1] = p2;
                output->normlist[2 * (edges_found - (remaining_edges + 1))] = norm1;
                output->normlist[2 * (edges_found - (remaining_edges + 1)) + 1] = norm2;
                output->edgemarkerlist[edges_found - (remaining_edges + 1)] = input->edgemarkerlist[edge];
            }
        } else {
            while(remaining_edges --> 0) {
                const int edge = found_edges[edges_found - (remaining_edges + 1)];
                int p1 = input->edgelist[2 * edge];
                int p2 = input->edgelist[2 * edge + 1];
                REAL norm1 = input->normlist[2 * edge];
                REAL norm2 = input->normlist[2 * edge + 1];
                output->edgelist[2 * (edges_found - (remaining_edges + 1))] = p1;
                output->edgelist[2 * (edges_found - (remaining_edges + 1)) + 1] = p2;
                output->normlist[2 * (edges_found - (remaining_edges + 1))] = norm1;
                output->normlist[2 * (edges_found - (remaining_edges + 1)) + 1] = norm2;
            }
        }
    }
    void filter_points(std::shared_ptr<triangulateio> input, std::shared_ptr<triangulateio> output, std::function<bool(int, REAL, REAL, REAL)> predicate);
    void inject_holes(std::shared_ptr<triangulateio> input, std::shared_ptr<triangulateio> output);
    std::vector<float> read_line(std::istream& stream);
    void print_vector(std::vector<float>& vec);
    void read_node_section(std::istream& file, std::shared_ptr<triangulateio> in);
    void write_node_section(std::ostream& file, std::shared_ptr<triangulateio> out);
    void read_node_file(std::string filename, std::shared_ptr<triangulateio> in);
    void read_poly_file(std::string filename, std::shared_ptr<triangulateio> in);
    void write_node_file(std::string filename, std::shared_ptr<triangulateio> out);
    void write_poly_file(std::string filename, std::shared_ptr<triangulateio> out);
    void write_edge_file(std::string filename, std::shared_ptr<triangulateio> out);
    void write_ele_file(std::string filename, std::shared_ptr<triangulateio> out);
    void write_neigh_file(std::string filename, std::shared_ptr<triangulateio> out);
}

#endif /* TRIANGLEMANIPULATOR_HPP_ */