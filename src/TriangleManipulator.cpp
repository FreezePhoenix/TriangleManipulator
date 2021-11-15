#include <stdio.h>

#include "TriangleManipulator/TriangleManipulatorTemplates.hpp"
#include <fmt/compile.h>
#include <experimental/array>
#include <new>

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

template<bool compact = false, size_t buffer_size = 2048, std::enable_if_t<0 == (buffer_size & (buffer_size - 1)), bool> = true>
class binary_writer {
    private:
        char buffer[buffer_size] __attribute__ ((aligned));
        FILE* file;
        void* head;
        std::size_t remaining;
    public:
        /**
         * @brief Construct a new binary writer object
         * 
         * @param filename 
         */
        binary_writer(const char* filename): buffer() {
            head = buffer;
            remaining = buffer_size;
            file = fopen(filename, "w");
            std::memset(buffer, 0, buffer_size);
        };
        /**
         * @brief Write the buffer to file, and clear it.
         */
        inline void flush() {
            head = buffer;
            fwrite(buffer, sizeof(char), buffer_size - remaining, file);
            std::memset(buffer, 0, buffer_size - remaining);
            remaining = buffer_size;
        }
        /**
         * @brief Write a value to file. Note: Uses copy constructor. Should only be used with Primitives and POD structures.
         * 
         * @tparam T The type of the value to write. You probably don't need to set this.
         * @tparam alignment The alignment to write with. You almost always won't need to set this.
         */
        template<typename T, size_t alignment = compact ? 1 : alignof(T), std::enable_if_t<!std::is_pointer_v<T> && buffer_size >= sizeof(T) && 0 == (alignment & (alignment - 1)), bool> = true>
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
        template<typename T, size_t alignment = compact ? 1 : alignof(T), std::enable_if_t<buffer_size >= sizeof(T) && 0 == (alignment & (alignment - 1)), bool> = true>
        inline void write(T* arg) {
            std::align(alignment, 0, head, remaining);
            if (remaining < sizeof(T)) {
                flush();
            }
            memcpy(head, (void*) arg, sizeof(T));
            head = (char*) head + sizeof(T);
            remaining -= sizeof(T);
        }
        template<typename T, size_t alignment = compact ? 1 : alignof(T), size_t block_size = buffer_size / sizeof(T), std::enable_if_t<buffer_size >= sizeof(T) && 0 == (alignment & (alignment - 1)), bool> = true>
        inline void write_block(T* arg) {
            memcpy(head, (void*) arg, buffer_size);
            head = (char*) head + buffer_size;
            remaining -= buffer_size;
        }
        template<typename T, size_t alignment = compact ? 1 : alignof(T), size_t block_size = buffer_size / sizeof(T), std::enable_if_t<buffer_size >= sizeof(T) && 0 == (alignment & (alignment - 1)), bool> = true>
        inline void write_array(T* arg, size_t size) {
            std::align(alignment, 0, head, remaining);
            size_t first = std::min(remaining / sizeof(T), size);
            memcpy(head, (void*)arg, sizeof(T) * first);
            head = (char*) head + sizeof(T) * first;
            remaining -= sizeof(T) * first;
            arg += first;
            size -= first;
            if (size > 0) {
                flush();
                while (size >= block_size) {
                    write_block(arg);
                    flush();
                    arg += block_size;
                    size -= block_size;
                }
                if (size > 0) {
                    std::align(alignment, 0, head, remaining);
                    memcpy(head, (void*)arg, sizeof(T) * size);
                    head = (char*)head + sizeof(T) * size;
                    remaining -= sizeof(T) * size;
                }
            }
        }
        /**
         * @brief Close the file. Flushes it, and invalidates the writer. Does not automatically cleanup the buffer.
         * 
         */
        void close() {
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
        std::size_t remaining;
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
            std::memcpy(arg, head, sizeof(T));
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
        inline void read_block(T* arg) {
            memcpy(arg, head, buffer_size);
            head = (char*) head + buffer_size;
            remaining = 0;
        }
        template<typename T, size_t alignment = compact ? 1 : alignof(T), size_t block_size = buffer_size / sizeof(T), std::enable_if_t<buffer_size >= sizeof(T) && 0 == (alignment & (alignment - 1)), bool> = true>
        inline void read_array(T* arg, size_t size) {
            std::align(alignment, 0, head, remaining);
            size_t first = std::min(remaining / sizeof(T), size);
            memcpy(arg, head, sizeof(T) * first);
            head = (char*) head + sizeof(T) * first;
            remaining -= sizeof(T) * first;
            arg += first;
            size -= first;
            if (size > 0) {
                flush();
                while (size >= block_size) {
                    read_block(arg);
                    flush();
                    arg += block_size;
                    size -= block_size;
                }
                if (size > 0) {
                    std::align(alignment, 0, head, remaining);
                    memcpy(arg, head, sizeof(T) * size);
                    head = (char*)head + sizeof(T) * size;
                    remaining -= sizeof(T) * size;
                }
            }
        }
        /**
         * @brief Close the file. Invalidates the reader. Does not automatically cleanup the buffer.
         * 
         */
        void close() {
            fclose(file);
            head = nullptr;
            remaining = 0;
        }
};
    /**
     * @brief Injects points from one triangulate into the hole slots of another.
     * 
     * @param input 
     * @param output 
     */
    void inject_holes(std::shared_ptr<triangulateio> input, std::shared_ptr<triangulateio> output) {
        output->numberofholes = input->numberofpoints;
        output->holelist = input->pointlist;
    }

    /**
     * @brief Find particular points that match what you want.
     * 
     * @param input 
     * @param output 
     * @param predicate 
     */
    void filter_points(std::shared_ptr<triangulateio> input, std::shared_ptr<triangulateio> output, std::function<bool(int, REAL, REAL, REAL)> predicate) {
        int points_found = 0;
        std::vector<int> found_points = std::vector<int>();
        unsigned int points = input->numberofpoints;
        unsigned int attributes = input->numberofpointattributes;
        bool markers = input->pointmarkerlist != nullptr;
        REAL* input_point_ptr = input->pointlist.get();
        REAL* input_point_attribute_ptr = input->pointattributelist.get();
        int* input_point_marker_ptr = input->pointmarkerlist.get();
        for (unsigned int i = 0; i < points; i++) {
            double x = input_point_ptr[2 * i];
            double y = input_point_ptr[2 * i + 1];
            if(attributes) {
                for(unsigned int j = 0; j < attributes; j++) {
                    double attr = input_point_attribute_ptr[attributes * i + j];
                    if(predicate(i, x, y, attr)) {
                        found_points.push_back(i);
                        points_found++;
                        break;
                    }
                }
            } else {
                if(predicate(i, x, y, 0)) {
                    found_points.push_back(i);
                    points_found++;
                }
            }
        }
        output->numberofpoints = points_found;
        output->pointlist = trimalloc<REAL>(2 * points_found);
        if(markers) {
            output->pointmarkerlist = trimalloc<int>(points_found);
        }
        if(attributes > 0) {
            output->pointattributelist = trimalloc<REAL>(attributes * points_found);
        }
        REAL* output_point_ptr = output->pointlist.get();
        REAL* output_point_attribute_ptr = output->pointattributelist.get();
        int* output_point_marker_ptr = output->pointmarkerlist.get();
        for (int i = 0; i < points_found; i++) {
            int point = found_points[i];
            memcpy(output_point_ptr + i * 2, input_point_ptr + point * 2, 2 * sizeof(REAL));
            if(markers) {
                output_point_marker_ptr[i] = input_point_marker_ptr[point];
            }
            if (attributes) {
                memcpy(output_point_attribute_ptr + attributes * i, input_point_attribute_ptr + attributes * point, attributes * sizeof(REAL));
            }
        }
    }

    /**
     * @brief Method to read a node section from a stream.
     * 
     * @param file 
     * @param in 
     */
    void read_node_section(std::istream& file, std::shared_ptr<triangulateio> in) {
        std::vector<unsigned int> points_header = read_line<unsigned int>(file);
        unsigned int points = points_header[0]; // Indicates number of points.
        unsigned int point_attributes  = points_header[2]; // Usually 1
        bool point_markers = points_header[3]; // Always 1 or 0
        // std::tie(points, std::ignore, point_attributes, point_markers) = read_many<unsigned int, unsigned int, unsigned int, bool>(file);
        // fmt::print("{} {} {}\n", points, point_attributes, point_markers);
        if (points > 0 && in->numberofpoints == 0) {
            in->numberofpoints = points;
            in->numberofpointattributes = point_attributes;
            in->pointlist = trimalloc<REAL>(points * 2);
            in->pointattributelist = trimalloc<REAL>(points * point_attributes);
            if(point_markers) {
                in->pointmarkerlist = trimalloc<int>(points);
            }
        }
        REAL* point_ptr = in->pointlist.get();
        REAL* point_attribute_ptr = in->pointattributelist.get();
        int* point_marker_ptr = in->pointmarkerlist.get();
        for (unsigned int i = 0; i < points; i++) {
            std::vector<REAL> point = read_line<REAL>(file);
            if(points > 0 && in->numberofpoints != 0) {
                int attributes = in->numberofpointattributes;
                // point_ptr[2 * i] = point[1]; // X
                // point_ptr[2 * i + 1] = point[2]; // Y
                memcpy(point_ptr + i * 2, point.data() + 1, sizeof(REAL) * 2);
                memcpy(point_attribute_ptr + attributes * i, point.data() + 3, attributes * sizeof(REAL));
                if(point_markers) {
                    *point_marker_ptr++ = point[3 + attributes]; // Marker
                }
            }
        }
    }

    /**
     * @brief Method to write a node section to an output stream.
     * 
     * @param file 
     * @param out 
     */
    inline void write_node_section(fmt::v8::ostream& file, std::shared_ptr<triangulateio> out) {
        const std::size_t points = out->numberofpoints;
        const unsigned int points_attributes = out->numberofpointattributes;
        const int markers = out->pointmarkerlist != nullptr;
        const double* points_ptr = out->pointlist.get();
        const double* attributes_ptr = out->pointattributelist.get();
        const int* marker_ptr = out->pointmarkerlist.get();
        file.print("{} 2 {} {}", points, points_attributes, markers);
        if (markers) {
            for (unsigned int i = 0; i < points; i++) {
                const double p1 = *points_ptr++;
                const double p2 = *points_ptr++;
                file.print("\n{} {} {}", i, p1, p2);
                for (unsigned int j = 0; j < points_attributes; j++) {
                    file.print(" {}", *attributes_ptr++);
                }
                file.print(" {}", *marker_ptr++);
            }
        } else {
            for (unsigned int i = 0; i < points; i++) {
                const double p1 = *points_ptr++;
                const double p2 = *points_ptr++;
                file.print("\n{} {} {}", i, p1, p2);
                for (unsigned int j = 0; j < points_attributes; j++) {
                    file.print(" {}", *attributes_ptr++);
                }
            }
        }
    }

    /**
     * @brief Method to read a .node file. Simply reads it as a node section.
     * 
     * @param filename 
     * @param in 
     */
    void read_node_file(std::string filename, std::shared_ptr<triangulateio> in) {
        std::ifstream file(filename);
        read_node_section(file, in);
        file.close();
    }

    /**
     * @brief Method to write a .node file. Simply writes it as a node section.
     * 
     * @param filename 
     * @param out 
     */
    void write_node_file(std::string filename, std::shared_ptr<triangulateio> out) {
        fmt::v8::ostream file = fmt::output_file(filename.c_str());
        write_node_section(file, out);
        file.close();
    }

    void read_node_section_binary(binary_reader<>& reader, std::shared_ptr<triangulateio> in) {
        const unsigned int points = reader.read<unsigned int>();
        const unsigned int points_attributes = reader.read<unsigned int>();
        const char markers = reader.read<char>();
        if (points > 0 && in->numberofpoints == 0) {
            in->numberofpoints = points;
            in->numberofpointattributes = points_attributes;
            in->pointlist = trimalloc<REAL>(points * 2);
            in->pointattributelist = trimalloc<REAL>(points * points_attributes);
            if(markers) {
                in->pointmarkerlist = trimalloc<int>(points);
            }
        }
        double* points_ptr = in->pointlist.get();
        double* attributes_ptr = in->pointattributelist.get();
        int* marker_ptr = in->pointmarkerlist.get();
        if (points > 0) {
            reader.read_array(points_ptr, points * 2);
            
            if (points_attributes > 0) {
                reader.read_array(attributes_ptr, points * points_attributes);
            }
            if (markers) {
                reader.read_array(marker_ptr, points);
            }
        }
    }
    
    void write_node_section_binary(binary_writer<>& writer, std::shared_ptr<triangulateio> out) {
        const unsigned int points = out->numberofpoints;
        const unsigned int points_attributes = out->numberofpointattributes;
        const char node_markers = out->pointmarkerlist != nullptr;
        const double* points_ptr = out->pointlist.get();
        const double* attributes_ptr = out->pointattributelist.get();
        const int* marker_ptr = out->pointmarkerlist.get();
        writer.write(points);
        writer.write(points_attributes);
        writer.write(node_markers);
        if (points > 0) {
            writer.write_array(points_ptr, points * 2);
            
            if (points_attributes > 0) {
                writer.write_array(attributes_ptr, points * points_attributes);
            }
            if (node_markers) {
                writer.write_array(marker_ptr, points);
            }
        }
    }

    void read_node_file_binary(std::string filename, std::shared_ptr<triangulateio> in) {
        binary_reader<> reader = binary_reader<>(filename.c_str());
        read_node_section_binary(reader, in);
        reader.close();
    }
    
    void write_node_file_binary(std::string filename, std::shared_ptr<triangulateio> out) {
        binary_writer<> writer = binary_writer<>(filename.c_str());
        write_node_section_binary(writer, out);
        writer.close();
    }

    /**
     * @brief Method to read a .poly file. First, reads the node section, then the segment section, and then the hole section.
     * 
     * @param filename 
     * @param in 
     */
    void read_poly_file(std::string filename, std::shared_ptr<triangulateio> in) {
        std::ifstream file(filename);
        read_node_section(file, in);
        std::vector<unsigned int> segments_header = read_line<unsigned int>(file);
        unsigned int segments = segments_header[0];
        unsigned int markers = segments_header[1];
        // std::tie(segments, markers) = read_many<int, int>(file);
        in->numberofsegments = segments;
        in->segmentlist = trimalloc<int>(segments * 2);
        if (markers) {
            in->segmentmarkerlist = trimalloc<int>(segments);
        }
        int* segment_ptr = in->segmentlist.get();
        int* segment_marker_ptr = in->segmentmarkerlist.get();
        for (unsigned int i = 0; i < segments; i++) {
            std::vector<unsigned int> segment = read_line<unsigned int>(file);
            // *segment_ptr++ = segment[1]; // First point ID
            // *segment_ptr++ = segment[2]; // Second point ID
            memcpy(segment_ptr + i * 2, segment.data() + 1, sizeof(unsigned int) * 2);
            if (markers) {
                *segment_marker_ptr++ = 2; // segment[3]; // Marker
            }
        }
        unsigned int holes = read_single<unsigned int>(file);
        in->numberofholes = holes;
        in->holelist = trimalloc<REAL>(holes * 2);
        REAL* hole_ptr = in->holelist.get();
        for (unsigned int i = 0; i < holes; i++) {
            std::vector<REAL> hole = read_line<REAL>(file);
            memcpy(hole_ptr + i * 2, hole.data() + 1, sizeof(REAL) * 2);
        }
        file.close();
    }

    /**
     * @brief Method to write a .poly file. First writes the node section, then the segment section, then the hole section.
     * 
     * @param filename 
     * @param out 
     */
    void write_poly_file(std::string filename, std::shared_ptr<triangulateio> out) {
        fmt::v8::ostream file = fmt::output_file(filename.c_str());
        write_node_section(file, out);
        const unsigned int segments = out->numberofsegments;
        const int markers  = out->segmentmarkerlist != nullptr;
        const int* segments_ptr = out->segmentlist.get();
        const int* markers_ptr = out->segmentmarkerlist.get();
        file.print("\n{} {}", segments, markers);
        for (unsigned int i = 0; i < segments; i++) {
            const int s1 = *segments_ptr++;
            const int s2 = *segments_ptr++;
            if (markers) {
                file.print("\n{} {} {} {}",  i, s1, s2, markers_ptr[i]);
            } else {
                file.print("\n{} {} {}", i, s1, s2);
            }
        }
        const unsigned int holes = out->numberofholes;
        const REAL* holes_ptr = out->holelist.get();
        file.print("\n{}", holes);
        for (unsigned int i = 0; i < holes; i++) {
            const double h1 = *holes_ptr++;
            const double h2 = *holes_ptr++;
            file.print("\n{} {} {}", i, h1, h2);
        }
        file.close();
    }

    void read_poly_file_binary(std::string filename, std::shared_ptr<triangulateio> in) {
        binary_reader<> reader = binary_reader<>(filename.c_str());
        read_node_section_binary(reader, in);
        const unsigned int segments = reader.read<unsigned int>();
        const unsigned int holes = reader.read<unsigned int>();
        const unsigned int segment_markers = reader.read<unsigned int>();
        
        in->numberofsegments = segments;
        in->segmentlist = trimalloc<int>(segments * 2);
        if (segment_markers) {
            in->segmentmarkerlist = trimalloc<int>(segments);
        }
        
        in->numberofholes = holes;
        in->holelist = trimalloc<REAL>(holes * 2);
        
        int* segment_ptr = in->segmentlist.get();
        int* markers_ptr = in->segmentmarkerlist.get();
        REAL* holes_ptr = in->holelist.get();
        if (segments > 0) {
            reader.read_array(segment_ptr, segments * 2);
            
            if (segment_markers) {
                reader.read_array(markers_ptr, segments);
            }
        }
        if (holes > 0) {
            reader.read_array(holes_ptr, holes * 2);
        }
        reader.close();
    }
    
    void write_poly_file_binary(std::string filename, std::shared_ptr<triangulateio> out) {
        binary_writer<> writer = binary_writer<>(filename.c_str());
        write_node_section_binary(writer, out);
        const unsigned int segments = out->numberofsegments;
        const unsigned int markers  = out->segmentmarkerlist != nullptr;
        const int* segments_ptr = out->segmentlist.get();
        const int* markers_ptr = out->segmentmarkerlist.get();
        const unsigned int holes = out->numberofholes;
        const REAL* holes_ptr = out->holelist.get();
        writer.write(segments);
        writer.write(holes);
        writer.write(markers);
        if (segments > 0) {
            writer.write_array(segments_ptr, segments * 2);
            
            if (markers) {
                writer.write_array(markers_ptr, segments);
            }
        }
        if (holes > 0) {
            writer.write_array(holes_ptr, holes * 2);
        }
        
        writer.close();
    }

    /**
     * @brief Method to write a .edge file. There is no read for this, because triangulate doesn't read them.
     * 
     * @param filename 
     * @param out 
     */
    void write_edge_file(std::string filename, std::shared_ptr<triangulateio> out) {
        fmt::v8::ostream file = fmt::output_file(filename.c_str());
        const unsigned int edges = out->numberofedges;
        const int markers = out->edgemarkerlist != nullptr;
        const int* edges_ptr = out->edgelist.get();
        const REAL* norms_ptr = out->normlist.get();
        const int* markers_ptr = out->edgemarkerlist.get();
        file.print("{} {}", edges, markers);
        for (unsigned int i = 0; i < edges; i++) {
            int p1 = *edges_ptr++;
            int p2 = *edges_ptr++;
            if (p2 == -1) {
                file.print("\n{} {} {} {} {}", i, p1, p2, norms_ptr[i * 2], norms_ptr[i * 2 + 1]);
            } else {
                if (markers) {
                    file.print("\n {} {} {} {}", i, p1, p2, markers_ptr[i]);
                } else {
                    
                    file.print("\n {} {} {}", i, p1, p2);
                }
            }
        }
        file.close();
    }

    void read_ele_file(std::string filename, std::shared_ptr<triangulateio> in) {
        std::ifstream file(filename);
        std::vector<unsigned int> header = read_line<unsigned int>(file);
        in->numberoftriangles = header[0];
        in->numberoftriangleattributes = header[2];
        
        in->trianglelist = trimalloc<unsigned int>(in->numberoftriangles * 3);
        if (in->numberoftriangleattributes > 0) {
            in->triangleattributelist = trimalloc<REAL>(in->numberoftriangles * 3);
        }
        unsigned int* triangles_ptr = in->trianglelist.get();
        REAL* attributes_ptr = in->triangleattributelist.get();
        for (std::size_t i = 0; i < in->numberoftriangles; i++) {
            std::vector<REAL> triangle = read_line<REAL>(file);
            triangles_ptr[i*3]      = (unsigned int) triangle[1];
            triangles_ptr[i*3 + 1]  = (unsigned int) triangle[2];
            triangles_ptr[i*3 + 2]  = (unsigned int) triangle[3];
            for (std::size_t j = 0; j < in->numberoftriangleattributes; j++) {
                attributes_ptr[i * in->numberoftriangleattributes + j] = triangle[4 + j];
            }
        }
        file.close();
    }
    
    void write_ele_file(std::string filename, std::shared_ptr<triangulateio> out) {
        fmt::v8::ostream file = fmt::output_file(filename.c_str());
        const unsigned int triangles = out->numberoftriangles;
        const unsigned int num_attributes = out->numberoftriangleattributes;
        const unsigned int* triangles_ptr = out->trianglelist.get();
        const REAL* attributes_ptr = out->triangleattributelist.get();
        file.print("{} 3 {}", triangles, num_attributes);
        for (unsigned int i = 0; i < triangles; i++) {
            file.print("\n{} {} {} {}", i, triangles_ptr[3 * i], triangles_ptr[3 * i + 1], triangles_ptr[3 * i + 2]);
            for (unsigned int j = 0; j < num_attributes; j++) {
                file.print(" {}", *attributes_ptr++);
            }
        }
        file.close();
    }

    void read_ele_file_binary(std::string filename, std::shared_ptr<triangulateio> in) {
        binary_reader<> reader = binary_reader<>(filename.c_str());
        const unsigned int triangles = reader.read<unsigned int>();
        const unsigned int num_attributes = reader.read<unsigned int>();
        in->numberoftriangles = triangles;
        in->numberoftriangleattributes = num_attributes;

        in->trianglelist = trimalloc<unsigned int>(in->numberoftriangles * 3);
        if (in->numberoftriangleattributes > 0) {
            in->triangleattributelist = trimalloc<REAL>(in->numberoftriangles * 3);
        }
        unsigned int* triangles_ptr = in->trianglelist.get();
        REAL* attributes_ptr = in->triangleattributelist.get();
        reader.read_array(triangles_ptr, triangles * 3);
        
        if (num_attributes > 0) {
            reader.read_array(attributes_ptr, triangles * num_attributes);
        }
        reader.close();
    }

    void write_ele_file_binary(std::string filename, std::shared_ptr<triangulateio> out) {
        binary_writer<> writer = binary_writer<>(filename.c_str());
        const unsigned int triangles = out->numberoftriangles;
        const unsigned int num_attributes = out->numberoftriangleattributes;
        const unsigned int* triangles_ptr = out->trianglelist.get();
        const REAL* attributes_ptr = out->triangleattributelist.get();
        writer.write<int>(triangles);
        writer.write<int>(num_attributes);
        if (triangles > 0) {
            writer.write_array(triangles_ptr, triangles * 3);
            
            if (num_attributes > 0) {
                writer.write_array(attributes_ptr, triangles * num_attributes);
            }
        }
        writer.close();
    }

    void write_neigh_file(std::string filename, std::shared_ptr<triangulateio> out) {
        fmt::v8::ostream file = fmt::output_file(filename.c_str());
        const unsigned int triangles = out->numberoftriangles;
        const int* neighbors_ptr = out->neighborlist.get();
        file.print("{} 3\n", triangles);
        for (unsigned int i = 0; i < triangles; i++) {
            file.print("{} {} {} {}\n", i, neighbors_ptr[3 * i], neighbors_ptr[3 * i + 1], neighbors_ptr[3 * i + 2]);
        }
        file.close();
    }

    void write_part_file(std::string filename, std::shared_ptr<triangulateio> out) {
        fmt::v8::ostream file = fmt::output_file(filename.c_str());
        const unsigned int triangles = out->numberoftriangles;
        const int* subdomain_ptr = out->subdomainlist.get();
        file.print("{} {}\n", triangles, out->numberofsubdomains);
        for (unsigned int i = 0; i < triangles; i++) {
            file.print("{} {}\n", i, *subdomain_ptr++);
        }
        file.close();
    }

}