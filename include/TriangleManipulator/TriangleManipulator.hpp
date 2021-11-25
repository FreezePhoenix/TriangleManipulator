#pragma once

#ifndef TRIANGLEMANIPULATOR_HPP_
#define TRIANGLEMANIPULATOR_HPP_

#include <functional>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <memory>
#include <fmt/os.h>
#include <triangle.h>

namespace TriangleManipulator {
    /**
     * @brief A class to write binary data to files.
     * 
     * Setting compact mode to true will not give large benefits in file size. If you are writing types with sizes that are powers of 2, and the LCM and GCF of their alignments are the same, then enabling compact mode will net you nothing.
     * If you are writing a large object, and there isn't enough space left in the buffer, the entire buffer is flushed, including any remaining space. This encourages you to write the data in as low a level as you can, preferably integers,
     * doubles, shorts, or chars at a time. Note: Since there is no overload for writing a boolean, you are expected to write them as either entire chars or pack them into chars yourself.
     *
     * @tparam compact Whether or not the file should be written in compact mode. Expect lower performance with compact mode enabled, but the resulting file will be smaller (or the same size). Files must be read in the same mode as they were written.
     * @tparam buffer_size The size of the internal buffer. An object can only be written if it's size is smaller than the buffer.
     */
    template<bool compact = false, size_t buffer_size = 2048, std::enable_if_t<std::has_single_bit(buffer_size), bool> = true>
    class binary_writer {
        private:
            char buffer[buffer_size];
            FILE* file;
            void* head;
            size_t remaining;
            /**
             * @brief Write the buffer to file, and clear it.
             */
            inline void flush() {
                head = buffer;
                fwrite(buffer, sizeof(char), buffer_size - remaining, file);
                // std::memset(buffer, 0, buffer_size - remaining);
                remaining = buffer_size;
            }
        public:
            /**
             * @brief Construct a new binary writer object
             * 
             * @param filename 
             */
            binary_writer(const char* filename): buffer() {
                head = buffer;
                remaining = buffer_size;
                file = fopen(filename, "wb");
                memset(buffer, 0, buffer_size);
            };
            /**
             * @brief Write a value to file. Note: Uses copy constructor. Should only be used with Primitives and POD structures.
             * 
             * @tparam T The type of the value to write. You probably don't need to set this.
             * @tparam alignment The alignment to write with. You almost always won't need to set this.
             */
            template<typename T, size_t alignment = compact ? alignof(char) : alignof(T), std::enable_if_t<!std::is_pointer_v<T> && buffer_size >= sizeof(T) && 0 == (alignment & (alignment - 1)), bool> = true>
            inline void write(T arg) {
                std::align(alignment, 0, head, remaining);
                if (remaining < sizeof(T)) {
                    flush();
                }
                memcpy(head, (void*)&arg, sizeof(T));
                head = (char*) head + sizeof(T);
                remaining -= sizeof(T);
            }
            /**
             * @brief Write what's at a pointer to file. Note: Does not use copy constructor. Should only be used with Primitives and POD structures.
             * 
             * @tparam T The type of the value to write. You probably don't need to set this.
             * @tparam alignment The alignment to write with. You almost always won't need to set this.
             */
            template<typename T, size_t alignment = compact ? alignof(char) : alignof(T), std::enable_if_t<buffer_size >= sizeof(T) && 0 == (alignment & (alignment - 1)), bool> = true>
            inline void write(T* arg) {
                std::align(alignment, 0, head, remaining);
                if (remaining < sizeof(T)) {
                    flush();
                }
                remaining -= sizeof(T);
                memcpy(head, arg, sizeof(T));
                head = (char*)head + sizeof(T);
                
            }
            template<typename T, size_t alignment = compact ? alignof(char) : alignof(T), size_t block_size = buffer_size / sizeof(T), std::enable_if_t<buffer_size >= sizeof(T) && 0 == (alignment & (alignment - 1)), bool> = true>
            inline void write_array(T* arg, size_t length) {
                // Align the head pointer to the type's alignment.
                std::align(alignment, 0, head, remaining);
                
                // Flush the buffer.
                flush();

                // Write all of the array directly to the file that we can.
                size_t blocks = length / block_size;
                fwrite(arg, sizeof(char), block_size * blocks * sizeof(T), file);
                arg += block_size * blocks;
                length %= block_size;

                // Write the rest to the buffer.
                memcpy(head, (void*)arg, sizeof(T) * length);
                head = (char*)head + sizeof(T) * length;
                remaining -= sizeof(T) * length;
            }
            /**
             * @brief Close the file. Flushes it, and invalidates the writer. Does not automatically cleanup the buffer.
             * 
             */
            inline void close() {
                flush();
                fclose(file);
                head = nullptr;
                remaining = 0;
            }
    };
    /**
     * @brief A class to read files written using binary_writer.
     * 
     * @tparam compact Whether or not the file should be read in compact mode. Expect lower performance with compact mode enabled, but the resulting file will be smaller. Files must be read in the same mode as they were written.
     * @tparam buffer_size The size of the internal buffer. An object can only be written if it's size is smaller than the buffer.
     */
    template<bool compact = false, size_t buffer_size = 2048, std::enable_if_t<0 == (buffer_size & (buffer_size - 1)), bool> = true>
    class binary_reader {
        private:
            char buffer[buffer_size] __attribute__ ((aligned));;
            FILE* file;
            void* head;
            size_t remaining;
            /**
             * @brief Flush the buffer, keeping the remaining portions of the data if the need arises.
             * 
             */
            inline void flush() {
                if (remaining > 0) {
                    std::memcpy(buffer, head, remaining);
                }
                head = buffer;
                fread(buffer + remaining, sizeof(char), buffer_size - remaining, file);
                remaining = buffer_size;
            }
            /**
             * @brief Flush the buffer completely. Discards any excess data, and completely overwrites it.
             * 
             */
            inline void flush_full() {
                head = buffer;
                fread(buffer, sizeof(char), buffer_size, file);
                remaining = buffer_size;
            }
        public:
            /**
             * @brief Construct a new binary reader object
             * 
             * @param filename 
             */
            binary_reader(const char* filename) {
                head = buffer;
                remaining = 0;
                file = fopen(filename, "rb");
                flush();
            };
            /**
             * @brief Read a value from file. Assigns the value to the reference argument. Should only be used with Primitives and POD structures.
             * 
             * @tparam T The type of the value to read. You probably don't need to set this.
             * @tparam alignment The alignment to read with. You almost always won't need to set this.
             */
            template<typename T, size_t alignment = compact ? 1 : alignof(T), std::enable_if_t<!std::is_pointer_v<T> && buffer_size >= sizeof(T) && 0 == (alignment & (alignment - 1)), bool> = true>
            inline void read(T& arg) {
                std::align(alignment, 0, head, remaining);
                if (remaining < sizeof(T)) {
                    flush();
                }
                const T& res = *reinterpret_cast<T*>(head);
                head = (char*) head + sizeof(T);
                remaining -= sizeof(T);
                arg = res;
            };
            /**
             * @brief Read a value from file. Directly assigns the value to the pointer argument. Should only be used with Primitives and POD structures.
             * 
             * @tparam T The type of the value to read. You probably don't need to set this.
             * @tparam alignment The alignment to read with. You almost always won't need to set this.
             */
            template<typename T, size_t alignment = compact ? 1 : alignof(T), std::enable_if_t<buffer_size >= sizeof(T) && 0 == (alignment & (alignment - 1)), bool> = true>
            inline void read(T* arg) {
                std::align(alignment, 0, head, remaining);
                if (remaining < sizeof(T)) {
                    flush();
                }
                memcpy(arg, head, sizeof(T));
                head = (char*)head + sizeof(T);
                remaining -= sizeof(T);
            };
            /**
             * @brief Read a value from file. Returns the value. Should only be used with Primitives and POD structures.
             * 
             * @tparam T The type of the value to read. You probably don't need to set this.
             * @tparam alignment The alignment to read with. You almost always won't need to set this.
             */
            template<typename T, size_t alignment = compact ? 1 : alignof(T), std::enable_if_t<!std::is_pointer_v<T> && buffer_size >= sizeof(T) && 0 == (alignment & (alignment - 1)), bool> = true>
            inline T read() {
                std::align(alignment, 0, head, remaining);
                if (remaining < sizeof(T)) {
                    flush();
                }
                const T& res = *reinterpret_cast<T*>(head);
                head = (char*) head + sizeof(T);
                remaining -= sizeof(T);
                return res;
            }
            template<typename T, size_t alignment = compact ? 1 : alignof(T), size_t block_size = buffer_size / sizeof(T), std::enable_if_t<buffer_size >= sizeof(T) && 0 == (alignment & (alignment - 1)), bool> = true>
            inline void read_array(T* arg, size_t size) {
                std::align(alignment, 0, head, remaining);
                if (size >= block_size) {
                    fseek(file, -remaining, SEEK_CUR);
                    size_t blocks = size / block_size;
                    size_t read = fread(arg, sizeof(T), block_size * blocks, file);
                    arg += read;
                    size -= read;
                    flush_full();
                    memcpy(arg, head, sizeof(T) * size);
                    head = (char*) head + sizeof(T) * size;
                    remaining -= sizeof(T) * size;
                    return;
                }
                flush();
                memcpy(arg, head, sizeof(T) * size);
                head = (char*) head + sizeof(T) * size;
                remaining -= sizeof(T) * size;
            }
            /**
             * @brief Close the file. Invalidates the reader. Does not automatically cleanup the buffer.
             * 
             */
            inline void close() {
                fclose(file);
                head = nullptr;
                remaining = 0;
            }
    };
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
        for (int i = 0; i < edges; i++) {
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
    
    // Standard text output, compatible with Showme
    void read_node_file(std::string filename, std::shared_ptr<triangulateio> in);
    void write_node_file(std::string filename, std::shared_ptr<triangulateio> out);

    void read_poly_file(std::string filename, std::shared_ptr<triangulateio> in);
    void write_poly_file(std::string filename, std::shared_ptr<triangulateio> out);
    
    void read_ele_file(std::string filename, std::shared_ptr<triangulateio> in);
    void write_ele_file(std::string filename, std::shared_ptr<triangulateio> out);

    void read_edge_file(std::string filename, std::shared_ptr<triangulateio> in);
    void write_edge_file(std::string filename, std::shared_ptr<triangulateio> out);
    
    void write_neigh_file(std::string filename, std::shared_ptr<triangulateio> out);
    void write_part_file(std::string filename, std::shared_ptr<triangulateio> out);

    // Binary output. Designed to be robust, and compact.
    void write_node_file_binary(std::string filename, std::shared_ptr<triangulateio> out);
    void read_node_file_binary(std::string filename, std::shared_ptr<triangulateio> in);

    void write_poly_file_binary(std::string filename, std::shared_ptr<triangulateio> out);
    void read_poly_file_binary(std::string filename, std::shared_ptr<triangulateio> in);

    void read_ele_file_binary(std::string filename, std::shared_ptr<triangulateio> in);
    void write_ele_file_binary(std::string filename, std::shared_ptr<triangulateio> out);

    void read_edge_file_binary(std::string filename, std::shared_ptr<triangulateio> in);
    void write_edge_file_binary(std::string filename, std::shared_ptr<triangulateio> out);
}

#endif /* TRIANGLEMANIPULATOR_HPP_ */