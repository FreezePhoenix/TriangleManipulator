#pragma once

#ifndef TRIANGLEMANIPULATOR_HPP_
#define TRIANGLEMANIPULATOR_HPP_

#include <functional>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <memory>
#include "../include/fmt/os.h"
#include "../triangle/triangle.h"

namespace TriangleManipulator {
    inline std::shared_ptr<triangulateio> create_instance() {
        std::shared_ptr<triangulateio> res = std::shared_ptr<triangulateio>(new triangulateio());
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
        std::vector<std::size_t> found_edges = std::vector<std::size_t>();
        const std::size_t edges = input->numberofedges;
        const bool markers = input->edgemarkerlist != nullptr;
        const int* edges_ptr = input->edgelist.get();
        const REAL* norms_ptr = input->normlist.get();
        const int* markers_ptr = input->edgemarkerlist.get();
        // found_edges.reserve(edges);
        for (std::size_t i = 0; i < edges; i++) {
            if (predicate(edges_ptr[i * 2], edges_ptr[i * 2 + 1], norms_ptr[i * 2], norms_ptr[i * 2 + 1])) {
                found_edges.push_back(i);
            }
        }
        const std::size_t edges_found = found_edges.size();
        output->numberofedges = edges_found;
        output->edgelist = trimalloc<int>(edges_found * 2);
        output->normlist = trimalloc<REAL>(edges_found * 2);
        if (markers) {
            output->edgemarkerlist = trimalloc<int>(edges_found);
        }
        int* out_edges_ptr = output->edgelist.get();
        REAL* out_norms_ptr = output->normlist.get();
        int* out_markers_ptr = output->edgemarkerlist.get();
        const std::size_t* found_edges_ptr = found_edges.data();
        for (std::size_t i = 0; i < edges_found; i++) {
            const std::size_t edge = found_edges_ptr[i];
            memcpy(out_edges_ptr + i * 2, edges_ptr + edge * 2, 2 * sizeof(int));
            memcpy(out_norms_ptr + i * 2, norms_ptr + edge * 2, 2 * sizeof(REAL));
            if (markers) {
                out_markers_ptr[i] = markers_ptr[edge];
            }
        }
    }
    void filter_points(std::shared_ptr<triangulateio> input, std::shared_ptr<triangulateio> output, std::function<bool(int, REAL, REAL, REAL)> predicate);
    void inject_holes(std::shared_ptr<triangulateio> input, std::shared_ptr<triangulateio> output);
    template <typename T>
    T parse_str(const std::string& str) {
        return std::stof(str);
    }
    template <typename T>
    inline std::vector<T> read_line(std::istream& stream) {
        std::string str;
        do {
        // Ignore comment lines.
            std::getline(stream, str);
        } while (str.front() == '#');

        std::istringstream new_stream = std::istringstream(str);
        
        std::vector<T> vec = std::vector<T>();
        vec.reserve(std::count(str.cbegin(), str.cend(), ' ') + 1);
        
        while (new_stream >> str) {
            vec.emplace_back(parse_str<T>(str));
        };
        return vec;
    };
    template <typename T = REAL>
    inline T read_single(std::istream& stream) {
        std::string str;
        do {
            // Ignore comment lines.
            std::getline(stream, str);
        } while (str.front() == '#');
        // Turn the string into a stream.
        std::istringstream new_stream(str);
        new_stream >> str;
        return parse_str<T>(str);
    };

    template<typename T>
    inline T read_var(std::istringstream& stream) {
        std::string num;
        stream >> num;
        return parse_str<T>(num);
    }
    template <typename ...Args>
    inline std::tuple<Args...> read_many(std::istream& stream) {
        std::string str;
        do {
            // Ignore comment lines.
            std::getline(stream, str);
        } while (str.front() == '#');
        // Turn the string into a stream.
        std::istringstream new_stream = std::istringstream(str);
        return { read_var<Args>(new_stream)... };
    }
    // template std::tuple<unsigned int, unsigned int, unsigned int, bool> read_many<unsigned int, unsigned int, unsigned int, bool>(std::istream& stream);
    void print_vector(std::vector<float>& vec);
    void read_node_section(std::istream& file, std::shared_ptr<triangulateio> in);
    void write_node_section(fmt::v8::ostream& file, std::shared_ptr<triangulateio> out);
    void read_node_file(std::string filename, std::shared_ptr<triangulateio> in);
    void read_poly_file(std::string filename, std::shared_ptr<triangulateio> in);
    void write_node_file(std::string filename, std::shared_ptr<triangulateio> out);
    void write_poly_file(std::string filename, std::shared_ptr<triangulateio> out);
    void write_edge_file(std::string filename, std::shared_ptr<triangulateio> out);
    void write_ele_file(std::string filename, std::shared_ptr<triangulateio> out);
    void read_ele_file(std::string filename, std::shared_ptr<triangulateio> in);
    void write_neigh_file(std::string filename, std::shared_ptr<triangulateio> out);
}

#endif /* TRIANGLEMANIPULATOR_HPP_ */