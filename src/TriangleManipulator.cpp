#include <stdio.h>

#include "TriangleManipulator.hpp"

namespace TriangleManipulator {
    triangulateio* create_instance() {
        triangulateio* res = new triangulateio;
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

    void filter_edges(triangulateio* input, triangulateio* output, std::function<bool(int,int,REAL,REAL)> predicate) {
        int edges_found = 0;
        std::vector<int> found_edges = std::vector<int>();
        int edges = input->numberofedges;
        bool markers = input->edgemarkerlist != nullptr;
        int remaining_edges = edges;
        while(remaining_edges --> 0) {
            int p1 = input->edgelist[2 * (edges - (remaining_edges + 1))];
            int p2 = input->edgelist[2 * (edges - (remaining_edges + 1)) + 1];
            REAL norm1 = input->normlist[2 * (edges - (remaining_edges + 1))];
            REAL norm2 = input->normlist[2 * (edges - (remaining_edges + 1)) + 1];
            if(predicate(p1, p2, norm1, norm2)) {
                found_edges.push_back(edges - (remaining_edges + 1));
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
        while(remaining_edges --> 0) {
            int edge = found_edges[edges_found - (remaining_edges + 1)];
            int p1 = input->edgelist[2 * edge];
            int p2 = input->edgelist[2 * edge + 1];
            REAL norm1 = input->normlist[2 * edge];
            REAL norm2 = input->normlist[2 * edge + 1];
            output->edgelist[2 * (edges_found - (remaining_edges + 1))] = p1;
            output->edgelist[2 * (edges_found - (remaining_edges + 1)) + 1] = p2;
            output->normlist[2 * (edges_found - (remaining_edges + 1))] = norm1;
            output->normlist[2 * (edges_found - (remaining_edges + 1)) + 1] = norm2;
            if(markers) {
                output->edgemarkerlist[edges_found - (remaining_edges + 1)] = input->edgemarkerlist[edge];
            }
        }
    }

    /**
     * @brief Injects points from one triangulate into the hole slots of another.
     * 
     * @param input 
     * @param output 
     */
    void inject_holes(triangulateio* input, triangulateio* output) {
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
    void filter_points(triangulateio* input, triangulateio* output, std::function<bool(int, REAL, REAL, REAL)> predicate) {
        int points_found = 0;
        std::vector<int> found_points = std::vector<int>();
        int points = input->numberofpoints;
        int attributes = input->numberofpointattributes;
        int remaining_points = points;
        bool markers = input->pointmarkerlist != nullptr;
        while(remaining_points --> 0) {
            REAL x = input->pointlist[2 * (points - (remaining_points + 1))];
            REAL y = input->pointlist[2 * (points - (remaining_points + 1)) + 1];
            int remaining_attributes = attributes;
            while(remaining_attributes --> 0) {
                REAL attr = input->pointattributelist[attributes * (points - (remaining_points + 1)) + (attributes - (remaining_attributes + 1))];
                if(predicate(points - (remaining_points), x, y, attr)) {
                    found_points.push_back((points - (remaining_points + 1)));
                    points_found++;
                    break;
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
        while(remaining_points --> 0) {
            int point = found_points[points_found - (remaining_points + 1)];
            REAL x = input->pointlist[2 * point];
            REAL y = input->pointlist[2 * point + 1];
            output->pointlist[2 * (points_found - (remaining_points + 1))] = x;
            output->pointlist[2 * (points_found - (remaining_points + 1)) + 1] = y;
            if(markers) {
                output->pointmarkerlist[points_found - (remaining_points + 1)] = input->pointmarkerlist[point];
            }
            if(attributes) {
                int remaining_attributes = attributes;
                while(remaining_attributes --> 0) {
                    REAL attr = input->pointattributelist[attributes * point + (attributes - (remaining_attributes + 1))];
                    output->pointattributelist[attributes * (points_found - (remaining_points + 1)) + (attributes - (remaining_attributes + 1))] = attr;
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
    void read_node_section(std::istream& file, triangulateio* in) {
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
            in->pointmarkerlist = (int *) malloc(points * sizeof(int));
        }
        int remaining_points = points;
        while(remaining_points --> 0) {
            std::vector<float> point = read_line(file);
            if(points > 0 && in->numberofpoints != 0) {
                int attributes = in->numberofpointattributes;
                in->pointlist[2 * (points - (remaining_points + 1))] = point[1]; // X
                in->pointlist[2 * (points - (remaining_points + 1)) + 1] = point[2]; // Y
                int remaining_attributes = attributes;
                while(remaining_attributes --> 0) {
                    in->pointattributelist[attributes * (points - remaining_points + 1) + (attributes - (remaining_attributes + 1))] = point[3 + attributes - (remaining_attributes + 1)];
                }
                in->pointmarkerlist[points - (remaining_points + 1)] = point[3 + attributes]; // Marker
            }
        }
    }

    /**
     * @brief Method to write a node section to an output stream.
     * 
     * @param file 
     * @param out 
     */
    void write_node_section(std::ostream& file, triangulateio* out) {
        int points = out->numberofpoints;
        int points_attributes = out->numberofpointattributes;
        int remaining_points = points;
        int markers = out->pointmarkerlist == nullptr ? 0 : 1;
        file << out->numberofpoints << " 2 " << out->numberofpointattributes << " " << (out->pointmarkerlist == nullptr ? 0 : 1) << std::endl;
        while(remaining_points --> 0) {
            file << (points - remaining_points) << " " << out->pointlist[2 * (points - (remaining_points + 1))] << " " << out->pointlist[2 * (points - (remaining_points + 1)) + 1];
            int attr = 0;
            if(points_attributes > 0) {
                do {
                    file << " " << out->pointattributelist[points_attributes * (points - (remaining_points + 1)) + attr++];
                } while (attr < points_attributes);
            }
            if(markers == 1) {
                file << " " << out->pointmarkerlist[points - (remaining_points + 1)];
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
    void read_node_file(std::string filename, triangulateio* in) {
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
    void write_node_file(std::string filename, triangulateio* out) {
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
    void read_poly_file(std::string filename, triangulateio* in) {
        std::ifstream file(filename);
        read_node_section(file, in);
        std::vector<float> segments_header = read_line(file);
        int segments = segments_header[0];
        int markers = segments_header[1];
        in->numberofsegments = segments;
        in->segmentlist = (int *) malloc(segments * 2 * sizeof(int));
        in->segmentmarkerlist = (int *) malloc(segments * sizeof(int));
        int remaining_segments = segments;
        while(remaining_segments --> 0) {
            std::vector<float> segment = read_line(file);
            in->segmentlist[2 * (segments - (remaining_segments + 1))] = segment[1]; // First point ID
            in->segmentlist[2 * (segments - (remaining_segments + 1)) + 1] = segment[2]; // Second point ID
            in->segmentmarkerlist[segments - (remaining_segments + 1)] = 2; // segment[3]; // Marker
        }
        std::vector<float> holes_header = read_line(file);
        int holes = holes_header[0];
        in->numberofholes = holes;
        in->holelist = (REAL *) malloc(holes * 2 * sizeof(REAL));
        int remaining_holes = holes;
        while(remaining_holes --> 0) {
            std::vector<float> hole = read_line(file);
            in->holelist[2 * (holes - (remaining_holes + 1))] = hole[0];
            in->holelist[2 * (holes - (remaining_holes + 1)) + 1] = hole[1];
        }
        file.close();
    }

    /**
     * @brief Method to write a .poly file. First writes the node section, then the segment section, then the hole section.
     * 
     * @param filename 
     * @param out 
     */
    void write_poly_file(std::string filename, triangulateio* out) {
        std::ofstream file(filename);
        write_node_section(file, out);
        int segments = out->numberofsegments;
        int markers  = out->segmentmarkerlist == nullptr ? 0 : 1;
        int remaining_segments = segments;
        file << segments << " " << markers << std::endl;
        while(remaining_segments --> 0) {
            file << segments - remaining_segments << " " << out->segmentlist[2 * (segments - (remaining_segments + 1))] << " " << out->segmentlist[2 * (segments - (remaining_segments + 1)) + 1];
            if(markers == 1) {
                file << " " << out->segmentmarkerlist[segments - (remaining_segments + 1)];
            }
            file << std::endl;
        }
        int holes = out->numberofholes;
        file << holes << std::endl;
        int remaining_holes = holes;
        while(remaining_holes --> 0) {
            file << holes - remaining_holes << " " << out->holelist[2 * (holes - (remaining_holes + 1))] << " " << out->holelist[2 * (holes - (remaining_holes + 1)) + 1] << std::endl;
        }
        file.close();
    }

    /**
     * @brief Method to write a .edge file. There is no read for this, because triangulate doesn't read them.
     * 
     * @param filename 
     * @param out 
     */
    void write_edge_file(std::string filename, triangulateio* out) {
        std::ofstream file(filename);
        int edges = out->numberofedges;
        file << edges << " " << (out->edgemarkerlist == nullptr ? 0 : 1) << std::endl;
        bool markers = (out->edgemarkerlist == (int *) NULL) ? false : true;
        int remaining_edges = edges;
        while(remaining_edges --> 0) {
            int p1 = out->edgelist[2 * (edges - (remaining_edges + 1))];
            int p2 = out->edgelist[2 * (edges - (remaining_edges + 1)) + 1];
            file << (edges - remaining_edges) << " " << p1 << " " << p2;
            if(p2 == -1) {
                REAL norm1 = out->normlist[2 * (edges - (remaining_edges + 1))];
                REAL norm2 = out->normlist[2 * (edges - (remaining_edges + 1)) + 1];
                file << " " << norm1 << " " << norm2;
            } else if(markers) {
                int a = (out->edgemarkerlist[0]);
                std::cout << markers << " " << " " << a << std::endl;
                file << " " << out->edgemarkerlist[edges - (remaining_edges + 1)];
            }
            file << std::endl;
        }
        file.close();
    }
    
    void cleanup(triangulateio* instance) {
        if(instance->pointlist != (REAL *) NULL) {
            free(instance->pointlist);
            instance->pointlist = (REAL *) NULL;
        }
        if(instance->pointattributelist != (REAL *) NULL) {
            free(instance->pointattributelist);
            instance->pointattributelist = (REAL *) NULL;
        }
        if(instance->pointmarkerlist != (int *) NULL) {
            free(instance->pointmarkerlist);
            instance->pointmarkerlist = (int *) NULL;
        }
        instance->numberofpoints = 0;
        instance->numberofpointattributes = 0;
        
        if(instance->trianglelist != (int *) NULL) {
            free(instance->trianglelist);
            instance->trianglelist = (int *) NULL;
        }
        if(instance->triangleattributelist != (REAL *) NULL) {
            free(instance->triangleattributelist);
            instance->triangleattributelist = (REAL *) NULL;
        }
        if(instance->neighborlist != (int *) NULL) {
            free(instance->neighborlist);
            instance->neighborlist = (int *) NULL;
        }
        instance->numberoftriangles = 0;
        instance->numberofcorners = 0;
        instance->numberoftriangleattributes = 0;
        
        if(instance->segmentlist != (int *) NULL) {
            free(instance->segmentlist);
            instance->segmentlist = (int *) NULL;
        }
        if(instance->segmentmarkerlist != (int *) NULL) {
            free(instance->segmentmarkerlist);
            instance->segmentmarkerlist = (int *) NULL;
        }
        instance->numberofsegments = 0;
        
        if(instance->holelist != (REAL *) NULL) {
            free(instance->holelist);
            instance->holelist = (REAL *) NULL;
        }
        instance->numberofholes = 0;

        if(instance->regionlist != (REAL *) NULL) {
            free(instance->regionlist);
            instance->regionlist = (REAL *) NULL;
        }
        instance->numberofregions = 0;
        
        if(instance->edgelist != (int *) NULL) {
            free(instance->edgelist);
            instance->edgelist = (int *) NULL;
        }
        if(instance->edgemarkerlist != (int *) NULL) {
            free(instance->edgemarkerlist);
            instance->edgemarkerlist = (int *) NULL;
        }
        if(instance->normlist != (REAL *) NULL) {
            free(instance->normlist);
            instance->normlist = (REAL *) NULL;
        }
        instance->numberofedges = 0;
    }

}
int main()
{
    triangulateio* stuff = TriangleManipulator::create_instance();
    TriangleManipulator::read_poly_file("hut.poly", stuff);
    triangulateio* outstuff = TriangleManipulator::create_instance();
    triangulateio* voutstuff = TriangleManipulator::create_instance();
    triangulateio* holeoutstuff = TriangleManipulator::create_instance();
    triangulate("pPvD", stuff, outstuff, voutstuff);
    TriangleManipulator::filter_points(voutstuff, holeoutstuff, [](int id, REAL x, REAL y, REAL attr) {
        return attr == 2;
    });
    TriangleManipulator::inject_holes(holeoutstuff, stuff);
    TriangleManipulator::filter_edges(voutstuff, outstuff, [](int p1, int p2, REAL norm1, REAL norm2) {
        return p2 != -1;
    });
    voutstuff->numberofedges = outstuff->numberofedges;
    voutstuff->edgelist = outstuff->edgelist;
    voutstuff->edgemarkerlist = outstuff->edgemarkerlist;
    voutstuff->normlist = outstuff->normlist;
    TriangleManipulator::write_node_file("hut.1.v.node", voutstuff);
    TriangleManipulator::write_edge_file("hut.1.v.edge", voutstuff);
    TriangleManipulator::write_poly_file("hut.holes.poly", stuff);
    TriangleManipulator::report(stuff, 1, 0, 0, 1, 0, 0);
  return 0;
}
