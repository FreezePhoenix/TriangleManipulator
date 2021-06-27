#include <stdio.h>

#include "TriangleManipulator.hpp"
#include "PointLocation.hpp"


namespace TriangleManipulator {
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
        int remaining_points = points;
        bool markers = input->pointmarkerlist != nullptr;
        for(int i = 0; i < points; i++) {
            double x = input->pointlist[2 * i];
            double y = input->pointlist[2 * i + 1];
            if(attributes) {
                for(int j = 0; j < attributes; j++) {
                    double attr = input->pointattributelist[attributes * i + j];
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
        remaining_points = points_found;
        output->numberofpoints = points_found;
        output->pointlist = (REAL *) malloc(2 * points_found * sizeof(REAL));
        if(markers) {
            output->pointmarkerlist = (int *) malloc(points_found * sizeof(int));
        }
        if(attributes > 0) {
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
            }
        }
    }

    /**
     * @brief Internal method to read a line of a stream and return it as a vector of floats. Skips lines with comments.
     * 
     * @param stream 
     * @return std::vector<float> 
     */
    std::vector<float> read_line(std::istream& stream) {
        std::string str;
        do {
            // Ignore comment lines.
            std::getline(stream, str);
        } while(str[0] == '#');
        // Turn the string into a stream.
        std::istringstream new_stream = std::istringstream(str);
        // Make an empty vector.
        std::vector<float> vec = std::vector<float>();
        // Read the line.
        while(new_stream.peek() != EOF) {
            std::string num;
            new_stream >> num;
            vec.push_back(std::stof(num));
        };
        return vec;
    }

    /**
     * @brief Debug method to print a vector.
     * 
     * @param vec 
     */
    void print_vector(std::vector<float>& vec) {
        for ( float x : vec) std::cout << x << ' ';
        std::cout << std::endl;
    }

    /**
     * @brief Method to read a node section from a stream.
     * 
     * @param file 
     * @param in 
     */
    void read_node_section(std::istream& file, std::shared_ptr<triangulateio> in) {
        std::vector<float> points_header = read_line(file);
        int points = points_header[0]; // Indicates number of points.
        int dimensions = points_header[1]; // Always 2
        int point_attributes = points_header[2]; // Usually 1
        int point_markers = points_header[3]; // Always 1 or 0
        if(points > 0 && in->numberofpoints == 0) {
            in->numberofpoints = points;
            in->numberofpointattributes = point_attributes;
            in->pointlist = (REAL *) malloc(points * 2 * sizeof(REAL));
            in->pointattributelist = (REAL *) malloc(points * point_attributes * sizeof(REAL));
            if(point_markers) {
                in->pointmarkerlist = (int *) malloc(points * sizeof(int));
            }
        }
        for(int i = 0; i < points; i++) {
            std::vector<float> point = read_line(file);
            if(points > 0 && in->numberofpoints != 0) {
                int attributes = in->numberofpointattributes;
                in->pointlist[2 * i] = point[1]; // X
                in->pointlist[2 * i + 1] = point[2]; // Y
                int remaining_attributes = attributes;
                for(int j = 0; j < remaining_attributes; j++) {
                    in->pointattributelist[attributes * i + j] = point[3 + j];
                }
                if(point_markers) {
                    in->pointmarkerlist[i] = point[3 + attributes]; // Marker
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
            }
            if(markers == 1) {
                file << " " << out->pointmarkerlist[i];
            }
            file << std::endl;
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
        std::ofstream file(filename);
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
        std::vector<float> segments_header = read_line(file);
        int segments = segments_header[0];
        int markers = segments_header[1];
        in->numberofsegments = segments;
        in->segmentlist = (int *) malloc(segments * 2 * sizeof(int));
        in->segmentmarkerlist = (int *) malloc(segments * sizeof(int));
        for(int i = 0; i < segments; i++) {
            std::vector<float> segment = read_line(file);
            in->segmentlist[2 * i] = segment[1]; // First point ID
            in->segmentlist[2 * i + 1] = segment[2]; // Second point ID
            in->segmentmarkerlist[i] = 2; // segment[3]; // Marker
        }
        std::vector<float> holes_header = read_line(file);
        int holes = holes_header[0];
        in->numberofholes = holes;
        in->holelist = (REAL *) malloc(holes * 2 * sizeof(REAL));
        for(int i = 0; i < holes; i++) {
            std::vector<float> hole = read_line(file);
            in->holelist[2 * i] = hole[0];
            in->holelist[2 * i + 1] = hole[1];
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
        std::ofstream file(filename);
        write_node_section(file, out);
        int segments = out->numberofsegments;
        int markers  = out->segmentmarkerlist == nullptr ? 0 : 1;
        file << segments << " " << markers << std::endl;
        for(int i = 0; i < segments; i++) {
            file << i << " " << out->segmentlist[2 * i] << " " << out->segmentlist[2 * i + 1];
            if(markers == 1) {
                file << " " << out->segmentmarkerlist[i];
            }
            file << std::endl;
        }
        int holes = out->numberofholes;
        file << holes << std::endl;
        for(int i = 0; i < holes; i++) {
            file << i << " " << out->holelist[2 * i] << " " << out->holelist[2 * i + 1] << std::endl;
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
            }
            file << std::endl;
        }
        file.close();
    }

    void write_ele_file(std::string filename, std::shared_ptr<triangulateio> out) {
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
            }
            file << std::endl;
        }
        file.close();
    }

    void read_ele_file(std::string filename, std::shared_ptr<triangulateio> output) {

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

}