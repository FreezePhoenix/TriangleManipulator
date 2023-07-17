#pragma once

#ifndef TRIANGLEMANIPULATOR_HPP_
#define TRIANGLEMANIPULATOR_HPP_

#include <numbers>
#include <functional>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <memory>
#include <fmt/os.h>
#include <triangle.h>

namespace TriangleManipulator {
    template<size_t S, bool = std::has_single_bit(S)>
    struct is_power_two : public std::false_type {};
    template<size_t S>
    struct is_power_two<S, true> : public std::true_type {};
    template<size_t S>
    inline constexpr bool is_power_two_v = is_power_two<S>::value;
    /**
     * @brief A class to write binary data to files.
     */
    class binary_writer {
        private:
            FILE* file;
        public:
            /**
             * @brief Construct a new binary writer object
             * 
             * @param filename 
             */
            binary_writer(const char* filename) {
                file = std::fopen(filename, "wb");
            };
            /**
             * @brief Write a value to file. Note: Uses copy constructor. Should only be used with Primitives and POD structures.
             */
            template<typename T>
            inline void write(const T& arg) {
                std::fwrite(&arg, sizeof(T), 1, file);
            }
            /**
             * @brief Write what's at a pointer to file. Note: Does not use copy constructor. Should only be used with Primitives and POD structures.
             */
            template<typename T>
            inline void write(const T* arg) {
                std::fwrite(arg, sizeof(T), 1, file);
            }
            template<typename T>
            inline void write_array(const T* arg, size_t length) {
                std::fwrite(arg, sizeof(T), length, file);
            }
            /**
             * @brief Close the file. Flushes it, and invalidates the writer. Does not automatically cleanup the buffer.
             * 
             */
            inline void close() {
                std::fclose(file);
            }
    };
    /**
     * @brief A class to read files written using binary_writer.
     */
    class binary_reader {
        private:
            FILE* file;
        public:
            /**
             * @brief Construct a new binary reader object
             * 
             * @param filename 
             */
            binary_reader(const char* filename) {
                file = std::fopen(filename, "rb");
            };
            /**
             * @brief Read a value from file. Returns the value. Should only be used with Primitives and POD structures.
             */
            template<typename T>
            inline T read() {
                T value;
                std::fread(&value, sizeof(T), 1, file);
                return value;
            }

            /**
             * @brief Read a value from file. Assigns the value to the reference argument. Should only be used with Primitives and POD structures.
             */
            template<typename T>
            inline void read(T& arg) {
                std::fread(&arg, sizeof(T), 1, file);
            };
            
            template<typename T>
            inline void read(T* arg) {
                std::fread(arg, sizeof(T), 1, file);
            };
            
            template<typename T>
            inline void read_array(T* arg, size_t length) {
                std::fread(arg, sizeof(T), length, file);
            }

            template<typename T>
            inline std::shared_ptr<T[]> read_array(size_t length) {
                std::shared_ptr<T[]> pointer = std::make_shared_for_overwrite<T[]>(length);
                std::fread(pointer.get(), sizeof(T), length, file);
                return pointer;
            }
            /**
             * @brief Close the file. Invalidates the reader. Does not automatically cleanup the buffer.
             * 
             */
            inline void close() {
                std::fclose(file);
            }
    };
    inline std::shared_ptr<triangulateio> create_instance() {
        std::shared_ptr<triangulateio> res = std::make_shared<triangulateio>();//std::shared_ptr<triangulateio>(new triangulateio());
        res->pointlist = nullptr;
        res->pointattributelist = nullptr;
        res->pointmarkerlist = nullptr;
        res->numberofpoints = 0;
        res->numberofpointattributes = 0;
        
        res->trianglelist = nullptr;
        res->triangleattributelist = nullptr;
        res->neighborlist = nullptr;
        res->subdomainlist = nullptr;
        res->numberoftriangles = 0;
        res->numberofcorners = 0;
        res->numberofsubdomains = 0;
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
        const size_t edges = input->numberofedges;
        const bool markers = input->edgemarkerlist != nullptr;
        const int* edges_ptr = input->edgelist.get();
        const REAL* norms_ptr = input->normlist.get();
        const int* markers_ptr = input->edgemarkerlist.get();
        
        output->edgelist = trimalloc<int>(edges * 2);
        output->normlist = trimalloc<REAL>(edges * 2);
        if (markers) {
            output->edgemarkerlist = trimalloc<int>(edges);
        }
        int* out_edges_ptr = output->edgelist.get();
        REAL* out_norms_ptr = output->normlist.get();
        int* out_markers_ptr = output->edgemarkerlist.get();
        size_t num_found_edges = input->numberofedges;
        for (size_t i = 0; i < edges; i++) {
            if (predicate(edges_ptr[i * 2], edges_ptr[i * 2 + 1], norms_ptr[i * 2], norms_ptr[i * 2 + 1])) {
                num_found_edges++;
                memcpy(out_edges_ptr, edges_ptr + i * 2, sizeof(int) * 2);
                out_edges_ptr += 2;
                memcpy(out_norms_ptr, norms_ptr + i * 2, sizeof(REAL) * 2);
                out_norms_ptr += 2;
                if (markers) {
                    memcpy(out_markers_ptr, markers_ptr + i * 2, sizeof(int) * 2);
                    out_markers_ptr++;
                }
            }
        }
        output->numberofedges = num_found_edges;
    }
    void filter_points(std::shared_ptr<triangulateio> input, std::shared_ptr<triangulateio> output, std::function<bool(int, REAL, REAL, REAL)> predicate);
    void inject_holes(std::shared_ptr<triangulateio> input, std::shared_ptr<triangulateio> output);
    template <typename T>
    T parse_str(const std::string& str);
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
    
    // Standard text output, compatible with Showme
    void read_node_file(std::string filename, std::shared_ptr<triangulateio> in);
    void write_node_file(std::string filename, std::shared_ptr<const triangulateio> out);

    void read_poly_file(std::string filename, std::shared_ptr<triangulateio> in);
    void write_poly_file(std::string filename, std::shared_ptr<const triangulateio> out);
    
    void read_ele_file(std::string filename, std::shared_ptr<triangulateio> in);
    void write_ele_file(std::string filename, std::shared_ptr<const triangulateio> out);

    void read_edge_file(std::string filename, std::shared_ptr<triangulateio> in);
    void write_edge_file(std::string filename, std::shared_ptr<const triangulateio> out);
    
    void write_neigh_file(std::string filename, std::shared_ptr<triangulateio> out);
    void write_part_file(std::string filename, std::shared_ptr<const triangulateio> out);

    // Binary output. Designed to be robust, and compact.
    void read_node_file_binary(std::string filename, std::shared_ptr<triangulateio> in);
    void write_node_file_binary(std::string filename, std::shared_ptr<const triangulateio> out);

    void read_poly_file_binary(std::string filename, std::shared_ptr<const triangulateio> in);
    void write_poly_file_binary(std::string filename, std::shared_ptr<const triangulateio> out);
    
    void read_ele_file_binary(std::string filename, std::shared_ptr<triangulateio> in);
    void write_ele_file_binary(std::string filename, std::shared_ptr<const triangulateio> out);

    void read_edge_file_binary(std::string filename, std::shared_ptr<triangulateio> in);
    void write_edge_file_binary(std::string filename, std::shared_ptr<const triangulateio> out);

    void read_neigh_file_binary(std::string filename, std::shared_ptr<triangulateio> in);
    void write_neigh_file_binary(std::string filename, std::shared_ptr<const triangulateio> out);
}

#endif /* TRIANGLEMANIPULATOR_HPP_ */