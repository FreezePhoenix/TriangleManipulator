#include <stdio.h>

#include "TriangleManipulator.hpp"


namespace TriangleManipulator {
<<<<<<< HEAD
=======
    void report(triangulateio* io, int markers, int reporttriangles, int reportneighbors, int reportsegments,
                int reportedges, int reportnorms) {
        int i, j;

        for (i = 0; i < io->numberofpoints; i++) {
            printf("Point %4d:", i);
            for (j = 0; j < 2; j++) {
                printf("  %.6g", io->pointlist[i * 2 + j]);
            }
            if (io->numberofpointattributes > 0) {
                printf("   attributes");
            }
            for (j = 0; j < io->numberofpointattributes; j++) {
                printf("  %.6g", io->pointattributelist[i * io->numberofpointattributes + j]);
            }
            if (markers) {
                printf("   marker %d\n", io->pointmarkerlist[i]);
            } else {
                printf("\n");
            }
        }
        printf("\n"); 
        if (reporttriangles || reportneighbors) {
            for (i = 0; i < io->numberoftriangles; i++) {
                if (reporttriangles) {
                    printf("Triangle %4d points:", i);
                    for (j = 0; j < io->numberofcorners; j++) {
                        printf("  %4d", io->trianglelist[i * io->numberofcorners + j]);
                    }
                    if (io->numberoftriangleattributes > 0) {
                        printf("   attributes");
                    }
                    for (j = 0; j < io->numberoftriangleattributes; j++) {
                        printf("  %.6g", io->triangleattributelist[i * io->numberoftriangleattributes + j]);
                    }
                    printf("\n");
                }
                if (reportneighbors) {
                    printf("Triangle %4d neighbors:", i);
                    for (j = 0; j < 3; j++) {
                    printf("  %4d", io->neighborlist[i * 3 + j]);
                    }
                    printf("\n");
                }
            }
            printf("\n");
        }

        if (reportsegments) {
            for (i = 0; i < io->numberofsegments; i++) {
                printf("Segment %4d points:", i);
                for (j = 0; j < 2; j++) {
                    printf("  %4d", io->segmentlist[i * 2 + j]);
                }
                if (markers) {
                    printf("   marker %d\n", io->segmentmarkerlist[i]);
                } else {
                    printf("\n");
                }
            }
            printf("\n");
        }

        if (reportedges) {
            for (i = 0; i < io->numberofedges; i++) {
                printf("Edge %4d points:", i);
                for (j = 0; j < 2; j++) {
                    printf("  %4d", io->edgelist[i * 2 + j]);
                }
                if (reportnorms && (io->edgelist[i * 2 + 1] == -1)) {
                    for (j = 0; j < 2; j++) {
                        printf("  %.6g", io->normlist[i * 2 + j]);
                    }
                }
                if (markers) {
                    printf("   marker %d\n", io->edgemarkerlist[i]);
                } else {
                    printf("\n");
                }
            }
            printf("\n");
        }
    }

>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
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
<<<<<<< HEAD
        REAL* input_point_ptr = input->pointlist.get();
        REAL* input_point_attribute_ptr = input->pointattributelist.get();
        int* input_point_marker_ptr = input->pointmarkerlist.get();
        for (int i = 0; i < points; i++) {
            double x = input_point_ptr[2 * i];
            double y = input_point_ptr[2 * i + 1];
            if(attributes) {
                for(int j = 0; j < attributes; j++) {
                    double attr = input_point_attribute_ptr[attributes * i + j];
=======
        for(int i = 0; i < points; i++) {
            double x = input->pointlist[2 * i];
            double y = input->pointlist[2 * i + 1];
            if(attributes) {
                for(int j = 0; j < attributes; j++) {
                    double attr = input->pointattributelist[attributes * i + j];
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
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
<<<<<<< HEAD
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
=======
            output->pointattributelist = (REAL *) malloc(attributes * points_found * sizeof(REAL));
        }
        for(int i = 0; i < points_found; i++) {
            int point = found_points[i];
            double x = input->pointlist[2 * point];
            double y = input->pointlist[2 * point + 1];
            output->pointlist[2 * i] = x;
            output->pointlist[2 * i + 1] = y;
            if(markers) {
                output->pointmarkerlist[i] = input->pointmarkerlist[point];
            }
            if(attributes) {
                for(int j = 0; j < attributes; j++) {
                    double attr = input->pointattributelist[attributes * point + j];
                    output->pointattributelist[attributes * i + j] = attr;
                }
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
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
<<<<<<< HEAD
        std::vector<int> points_header = read_line<int>(file);
=======
        std::vector<float> points_header = read_line(file);
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
        int points = points_header[0]; // Indicates number of points.
        // int dimensions = points_header[1]; // Always 2
        int point_attributes = points_header[2]; // Usually 1
        int point_markers = points_header[3]; // Always 1 or 0
        if(points > 0 && in->numberofpoints == 0) {
            in->numberofpoints = points;
            in->numberofpointattributes = point_attributes;
<<<<<<< HEAD
            in->pointlist = trimalloc<REAL>(points * 2);
            in->pointattributelist = trimalloc<REAL>(points * point_attributes);
            if(point_markers) {
                in->pointmarkerlist = trimalloc<int>(points);
=======
            in->pointlist = (REAL *) malloc(points * 2 * sizeof(REAL));
            in->pointattributelist = (REAL *) malloc(points * point_attributes * sizeof(REAL));
            if(point_markers) {
                in->pointmarkerlist = (int *) malloc(points * sizeof(int));
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
            }
        }
        REAL* point_ptr = in->pointlist.get();
        REAL* point_attribute_ptr = in->pointattributelist.get();
        int* point_marker_ptr = in->pointmarkerlist.get();
        for (int i = 0; i < points; i++) {
            std::vector<REAL> point = read_line(file);
            if(points > 0 && in->numberofpoints != 0) {
                int attributes = in->numberofpointattributes;
                // point_ptr[2 * i] = point[1]; // X
                // point_ptr[2 * i + 1] = point[2]; // Y
                memcpy(point_ptr + i * 2, point.data() + 1, sizeof(REAL) * 2);
                memcpy(point_attribute_ptr + attributes * i, point.data() + 3, attributes * sizeof(REAL));
                if(point_markers) {
                    *point_marker_ptr++ = point[3 + attributes]; // Marker
                }
<<<<<<< HEAD
=======
                if(point_markers) {
                    in->pointmarkerlist[i] = point[3 + attributes]; // Marker
                }
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
            }
        }
    }

    /**
     * @brief Method to write a node section to an output stream.
     * 
     * @param file 
     * @param out 
     */
<<<<<<< HEAD
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
=======
    void write_node_section(std::ostream& file, std::shared_ptr<triangulateio> out) {
        int points = out->numberofpoints;
        int points_attributes = out->numberofpointattributes;
        int markers = out->pointmarkerlist == nullptr ? 0 : 1;
        file << out->numberofpoints << " 2 " << out->numberofpointattributes << " " << (out->pointmarkerlist == nullptr ? 0 : 1) << std::endl;
        for(int i = 0; i < points; i++) {
            file << i << " "  << out->pointlist[2 * i] << " " << out->pointlist[2 * i + 1];
            int attr = 0;
            for(int j = 0; j < points_attributes; j++) {
                file << " " << out->pointattributelist[points_attributes * i + j];
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
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
<<<<<<< HEAD
        fmt::v8::ostream file = fmt::output_file(filename.c_str());
=======
        std::ofstream file(filename);
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
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
        std::vector<int> segments_header = read_line<int>(file);
        int segments = segments_header[0];
        int markers = segments_header[1];
        in->numberofsegments = segments;
<<<<<<< HEAD
        in->segmentlist = trimalloc<int>(segments * 2);
        if (markers) {
            in->segmentmarkerlist = trimalloc<int>(segments);
        }
        int* segment_ptr = in->segmentlist.get();
        int* segment_marker_ptr = in->segmentmarkerlist.get();
        for (int i = 0; i < segments; i++) {
            std::vector<int> segment = read_line<int>(file);
            // *segment_ptr++ = segment[1]; // First point ID
            // *segment_ptr++ = segment[2]; // Second point ID
            memcpy(segment_ptr, segment.data() + 1, sizeof(int) * 2);
            if (markers) {
                *segment_marker_ptr++ = 2; // segment[3]; // Marker
            }
=======
        in->segmentlist = (int *) malloc(segments * 2 * sizeof(int));
        in->segmentmarkerlist = (int *) malloc(segments * sizeof(int));
        for(int i = 0; i < segments; i++) {
            std::vector<float> segment = read_line(file);
            in->segmentlist[2 * i] = segment[1]; // First point ID
            in->segmentlist[2 * i + 1] = segment[2]; // Second point ID
            in->segmentmarkerlist[i] = 2; // segment[3]; // Marker
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
        }
        int holes = read_line<int>(file)[0];
        in->numberofholes = holes;
<<<<<<< HEAD
        in->holelist = trimalloc<REAL>(holes * 2);
        REAL* hole_ptr = in->holelist.get();
        for (int i = 0; i < holes; i++) {
            std::vector<REAL> hole = read_line(file);
            memcpy(hole_ptr + i * 2, hole.data(), sizeof(REAL) * 2);
=======
        in->holelist = (REAL *) malloc(holes * 2 * sizeof(REAL));
        for(int i = 0; i < holes; i++) {
            std::vector<float> hole = read_line(file);
            in->holelist[2 * i] = hole[0];
            in->holelist[2 * i + 1] = hole[1];
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
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
<<<<<<< HEAD
        fmt::v8::ostream file = fmt::output_file(filename.c_str());
=======
        std::ofstream file(filename);
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
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
<<<<<<< HEAD
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
                REAL first = *norms_ptr++;
                file.print(" {} {}", first, *norms_ptr++);
            } else if (markers) {
                file.print(" {}", *markers_ptr++);
=======
        std::ofstream file(filename);
        int edges = out->numberofedges;
        file << edges << " " << (out->edgemarkerlist == nullptr ? 0 : 1) << std::endl;
        bool markers = (out->edgemarkerlist == (int *) NULL) ? false : true;
        int remaining_edges = edges;
        for(int i = 0; i < edges; i++) {
            int p1 = out->edgelist[2 * i];
            int p2 = out->edgelist[2 * i + 1];
            file << i << " " << p1 << " " << p2;
            if(p2 == -1) {
                REAL norm1 = out->normlist[2 * i];
                REAL norm2 = out->normlist[2 * i + 1];
                file << " " << norm1 << " " << norm2;
            } else if(markers) {
                int a = (out->edgemarkerlist[0]);
                file << " " << out->edgemarkerlist[i];
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
            }
        }
        file.close();
    }

    void write_ele_file(std::string filename, std::shared_ptr<triangulateio> out) {
<<<<<<< HEAD
        fmt::v8::ostream file = fmt::output_file(filename.c_str());
        const std::size_t triangles = out->numberoftriangles;
        const unsigned int num_attributes = out->numberoftriangleattributes;
        const int* triangles_ptr = out->trianglelist.get();
        const REAL* attributes_ptr = out->triangleattributelist.get();
        file.print("{} 3 {}", triangles, num_attributes);
        for (std::size_t i = 0; i < triangles; i++) {
            file.print("\n{} {} {} {}", i, triangles_ptr[3 * i], triangles_ptr[3 * i + 1], triangles_ptr[3 * i + 2]);
            for (unsigned int j = 0; j < num_attributes; j++) {
                file.print(" {}", attributes_ptr[num_attributes * i + j]);
=======
        std::ofstream file(filename);
        int triangles = out->numberoftriangles;
        // TODO: See if there's a way to detect the number of elements per triangle?
        int num_attributes = out->numberoftriangleattributes;
        file << triangles << " 3 " << num_attributes << std::endl;
        for(int i = 0; i < triangles; i++) {
            int p1 = out->trianglelist[3 * i];
            int p2 = out->trianglelist[3 * i + 1];
            int p3 = out->trianglelist[3 * i + 2];
            file << i << " " << p1 << " " << p2 << " " << p3;
            int attr = 0;
            if(num_attributes > 0) {
                do {
                    file << " " << out->triangleattributelist[num_attributes * i + attr++];
                } while (attr < num_attributes);
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
            }
        }
        file.close();
    }

    void read_ele_file(std::string filename, std::shared_ptr<triangulateio> output) {

<<<<<<< HEAD
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
=======
    }

    void write_neigh_file(std::string filename, std::shared_ptr<triangulateio> out) {
        std::ofstream file(filename);
        int triangles = out->numberoftriangles;
        file << triangles << " 3" << std::endl;
        for(int i = 0; i < triangles; i++) {
            int n1 = out->neighborlist[3 * i];
            int n2 = out->neighborlist[3 * i + 1];
            int n3 = out->neighborlist[3 * i + 2];
            file << i << " " << n1 << " " << n2 << " " << n3 << std::endl;
        }
        file.close();
    }
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8

}