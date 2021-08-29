#include <stdio.h>

#include "TriangleManipulatorTemplates.hpp"
#include <fmt/os.h>

namespace TriangleManipulator {
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
        int points = input->numberofpoints;
        int attributes = input->numberofpointattributes;
        bool markers = input->pointmarkerlist != nullptr;
        REAL* input_point_ptr = input->pointlist.get();
        REAL* input_point_attribute_ptr = input->pointattributelist.get();
        int* input_point_marker_ptr = input->pointmarkerlist.get();
        for (int i = 0; i < points; i++) {
            double x = input_point_ptr[2 * i];
            double y = input_point_ptr[2 * i + 1];
            if(attributes) {
                for(int j = 0; j < attributes; j++) {
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
        for (int i = 0; i < points; i++) {
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
        const bool markers = out->pointmarkerlist != nullptr;
        const double* points_ptr = out->pointlist.get();
        const double* attributes_ptr = out->pointattributelist.get();
        const int* marker_ptr = out->pointmarkerlist.get();
        file.print("{} 2 {} {}", points, points_attributes, markers);
        for (unsigned int i = 0; i < points; i++) {
            const double p1 = *points_ptr++;
            const double p2 = *points_ptr++;
            file.print("\n{} {} {}", i, p1, p2);
            for (unsigned int j = 0; j < points_attributes; j++) {
                file.print(" {}", *attributes_ptr++);
            }
            if (markers) {
                file.print(" {}", *marker_ptr++);
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
            memcpy(hole_ptr + i * 2, hole.data(), sizeof(REAL) * 2);
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
        const bool markers  = out->segmentmarkerlist != nullptr;
        const int* segments_ptr = out->segmentlist.get();
        const int* markers_ptr = out->segmentmarkerlist.get();
        file.print("\n{} {}", segments, markers);
        for (unsigned int i = 0; i < segments; i++) {
            const int s1 = *segments_ptr++;
            const int s2 = *segments_ptr++;
            file.print("\n{} {} {}", i, s1, s2);
            if (markers) {
                file.print(" {}", markers_ptr[i]);
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

    /**
     * @brief Method to write a .edge file. There is no read for this, because triangulate doesn't read them.
     * 
     * @param filename 
     * @param out 
     */
    void write_edge_file(std::string filename, std::shared_ptr<triangulateio> out) {
        fmt::v8::ostream file = fmt::output_file(filename.c_str());
        const unsigned int edges = out->numberofedges;
        const bool markers = out->edgemarkerlist != nullptr;
        const int* edges_ptr = out->edgelist.get();
        const REAL* norms_ptr = out->normlist.get();
        const int* markers_ptr = out->edgemarkerlist.get();
        file.print("{} {}", edges, markers);
        for (unsigned int i = 0; i < edges; i++) {
            int p1 = *edges_ptr++;
            int p2 = *edges_ptr++;
            file.print("\n{} {} {}", i, p1, p2);
            if (p2 == -1) {
                file.print(" {} {}", norms_ptr[i * 2], norms_ptr[i * 2 + 1]);
            } else if (markers) {
                file.print(" {}", markers_ptr[i]);
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

}