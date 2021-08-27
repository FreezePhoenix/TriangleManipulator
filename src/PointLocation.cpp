#include "PointLocation.hpp"
#include "ShapeManipulator.hpp"
#include "TriangleManipulator.hpp"
#include "earcut.hpp"
#include "fmt/os.h"

// template class std::vector<PointLocation::Point>;
// template class std::vector<PointLocation::Line>;

namespace PointLocation {
    DirectedAcyclicGraph::DirectedAcyclicGraph() {
        this->_root = 0;
        this->graph = std::unordered_map<unsigned int, std::unordered_set<unsigned int>>();
    };

    void DirectedAcyclicGraph::add_directed_edge(unsigned int first, unsigned int second) {
        if(this->graph.find(first) == this->graph.end()) {
            this->graph[first] = std::unordered_set<unsigned int>();
        }
        this->graph[first].insert(second);
    }

    Vertex::Vertex(double x, double y) {
        this->id = static_cast<unsigned int>(-1); // Caution advised, 
        this->removed = false;
        this->point = Vertex::Point{ x, y };
        this->triangles = std::set<unsigned int>();
    }

    void Vertex::add_triangle(unsigned int triangle_id) {
        this->triangles.insert(triangle_id);
    }

    inline void Vertex::remove_triangle(unsigned int triangle_id) {
        this->triangles.erase(triangle_id);
    }

    PlanarGraph::PlanarGraph() {
        this->vertices = std::vector<Vertex>();
        this->all_triangles = std::vector<Triangle>();
        this->adjacency_list = std::vector<std::unordered_set<unsigned int>>();
        this->triangulations = std::vector<std::unordered_set<unsigned int>>();
        this->num_vertices = 0;
    }

    PlanarGraph::PlanarGraph(std::shared_ptr<triangulateio> graph) : PlanarGraph() {
        // TODO: Figure out how to make triangles be in the same order in the graph as the original.
        std::shared_ptr<triangulateio> output = TriangleManipulator::create_instance();
        if (graph->numberofholes != 0) {
            fmt::print("Planar graph encountered holes in constructor. Unintended behavior may occur.\n");
            graph->numberofholes = 0;
        }
        std::shared_ptr<triangulateio> real_graph = TriangleManipulator::create_instance();
        real_graph->numberofpoints = graph->numberofpoints + 3;
        real_graph->numberofsegments = graph->numberofsegments + 3;
        real_graph->pointlist = trimalloc<REAL>(real_graph->numberofpoints * 2 * sizeof(REAL));
        real_graph->segmentlist = trimalloc<int>(real_graph->numberofsegments * 2 * sizeof(int));
        real_graph->segmentmarkerlist = trimalloc<int>(real_graph->numberofsegments * sizeof(int));
        double min_y = INFINITY;
        double left_border = -INFINITY;
        double right_border = -INFINITY;
        const REAL* graph_point_ptr = graph->pointlist.get();
        for (std::size_t i = 0; i < graph->numberofpoints; i++) {
            double x = graph_point_ptr[i * 2];
            double y = graph_point_ptr[i * 2 + 1];
            if(y - 1 < min_y) {
                min_y = y - 1;
            }
            double dif = x * sqrt(3);
            if(y - dif + 2 > left_border) {
                left_border = y - dif + 2;
            } 
            if(y + dif + 2 > right_border) {
                right_border = y + dif + 2;
            }
        }
        double top_x = (right_border - left_border) / (2 * sqrt(3));
        double top_y = (right_border + left_border) / 2;
        double deviation = (top_y - min_y) / sqrt(3);
        double left_x = top_x - deviation;
        double left_y = min_y; 
        double right_x = top_x + deviation;
        double right_y = min_y;
        memcpy(real_graph->pointlist.get(), graph->pointlist.get(), graph->numberofpoints * 2 * sizeof(double));
        memcpy(real_graph->segmentlist.get(), graph->segmentlist.get(), graph->numberofsegments * 2 * sizeof(int));
        memcpy(real_graph->segmentmarkerlist.get(), graph->segmentmarkerlist.get(), graph->numberofsegments * sizeof(int));
        real_graph->pointlist.get()[graph->numberofpoints * 2] = top_x;
        real_graph->pointlist.get()[graph->numberofpoints * 2 + 1] = top_y;
        real_graph->pointlist.get()[graph->numberofpoints * 2 + 2] = left_x;
        real_graph->pointlist.get()[graph->numberofpoints * 2 + 3] = left_y;
        real_graph->pointlist.get()[graph->numberofpoints * 2 + 4] = right_x;
        real_graph->pointlist.get()[graph->numberofpoints * 2 + 5] = right_y;
        real_graph->segmentlist.get()[graph->numberofsegments * 2] = graph->numberofpoints;
        real_graph->segmentlist.get()[graph->numberofsegments * 2 + 1] = graph->numberofpoints + 1;
        real_graph->segmentlist.get()[graph->numberofsegments * 2 + 2] = graph->numberofpoints + 1;
        real_graph->segmentlist.get()[graph->numberofsegments * 2 + 3] = graph->numberofpoints + 2;
        real_graph->segmentlist.get()[graph->numberofsegments * 2 + 4] = graph->numberofpoints + 2;
        real_graph->segmentlist.get()[graph->numberofsegments * 2 + 5] = graph->numberofpoints;
        real_graph->segmentmarkerlist.get()[graph->numberofsegments] = 1;
        real_graph->segmentmarkerlist.get()[graph->numberofsegments + 1] = 1;
        real_graph->segmentmarkerlist.get()[graph->numberofsegments + 2] = 1;
        
        triangulate("pzeQ", real_graph, output, nullptr);
        std::shared_ptr<triangulateio> empty = TriangleManipulator::create_instance();
        TriangleManipulator::write_poly_file("test.1.poly", output);
        TriangleManipulator::write_ele_file("test.1.ele", output);
        TriangleManipulator::write_node_file("test.1.node", output);
        this->vertices.reserve(output->numberofpoints);
        const REAL* output_point_ptr = output->pointlist.get();
        for (std::size_t i = 0; i < output->numberofpoints; i++) {
            this->add_vertex(output_point_ptr[i * 2], output_point_ptr[i * 2 + 1]);
            // Allow one unit of space on edges
            
        }
        const int* output_segment_ptr = output->segmentlist.get();
        for (std::size_t i = 0; i < output->numberofsegments; i++) {
            this->connect_vertices(output_segment_ptr[i * 2], output_segment_ptr[i * 2 + 1]);
        }
        const int* output_edge_ptr = output->edgelist.get();
        for (std::size_t i = 0; i < output->numberofedges; i++) {
            this->connect_vertices(output_edge_ptr[i * 2], output_edge_ptr[i * 2 + 1]);
        }
        this->triangulations.push_back(std::unordered_set<unsigned int>());
        this->all_triangles.reserve(output->numberoftriangles);
        const int* output_triangle_ptr = output->trianglelist.get();
        for (std::size_t i = 0; i < output->numberoftriangles; i++) {
            this->vertices[output_triangle_ptr[i * 3]].add_triangle(i);
            this->vertices[output_triangle_ptr[i * 3 + 1]].add_triangle(i);
            this->vertices[output_triangle_ptr[i * 3 + 2]].add_triangle(i);
            this->triangulations[0].insert(i);
            this->all_triangles.push_back(Triangle {(unsigned int) output_triangle_ptr[i * 3], (unsigned int) output_triangle_ptr[i * 3 + 1], (unsigned int) output_triangle_ptr[i * 3 + 2] });
        }
    }

    unsigned int PlanarGraph::add_vertex(double x, double y) {
        Vertex vert(x, y);
        vert.id = this->vertices.size();
        
        this->vertices.push_back(vert);
        this->adjacency_list.push_back(std::unordered_set<unsigned int>());
        this->num_vertices++;
        
        return vert.id;
    }

    inline std::unordered_set<unsigned int>* PlanarGraph::neighbhors(unsigned int vertex_id) {
        // We know that if the vertex id is a valid vertex, then it must be less than the
        // number of vertices we have 
        if(this->num_vertices > vertex_id) {
            return &this->adjacency_list[vertex_id];
        }
        return nullptr;
    }

    inline unsigned int PlanarGraph::degree(unsigned int vertex_id) {
        if(this->num_vertices > vertex_id) {
            return this->adjacency_list[vertex_id].size();
        }
        return 0;
    }

    inline void PlanarGraph::add_directed_edge(unsigned int first_vertex, unsigned int second_vertex) {
        this->adjacency_list[first_vertex].insert(second_vertex);
    }

    inline void PlanarGraph::remove_directed_edge(unsigned int first_vertex, unsigned int second_vertex) {
        this->adjacency_list[first_vertex].erase(second_vertex);
    }

    inline void PlanarGraph::connect_vertices(unsigned int first_vertex, unsigned int second_vertex) {
        if(!this->adjacency_list[first_vertex].contains(second_vertex) && !this->adjacency_list[second_vertex].contains(first_vertex)) {
            this->add_directed_edge(first_vertex, second_vertex);
            this->add_directed_edge(second_vertex, first_vertex);
        }
    }
    RemovedVertexInfo PlanarGraph::remove_vertex(unsigned int vertex_id) {
        this->vertices[vertex_id].removed = true;
        this->num_vertices--;
        
        std::unordered_set<unsigned int> neighbhors = this->adjacency_list[vertex_id];
        for(unsigned int neighbhor : neighbhors) {
            this->remove_directed_edge(vertex_id, neighbhor);
            this->remove_directed_edge(neighbhor, vertex_id);
        }
        std::set<unsigned int> old_triangle_ids = std::set<unsigned int>(this->vertices[vertex_id].triangles);

        std::vector<unsigned int> polygon = std::vector<unsigned int>();

        std::unordered_set<std::pair<unsigned int, unsigned int>, pair_hash> triangles = std::unordered_set<std::pair<unsigned int, unsigned int>, pair_hash>();
        std::pair<unsigned int, unsigned int> first_triangle;
        bool first = true;
        for(const unsigned int triangle_id : old_triangle_ids) {
            Triangle triangle = this->all_triangles[triangle_id];
            std::pair<unsigned int, unsigned int> vertices_of_triangle;
            if(triangle.vertex_one == vertex_id) {
                vertices_of_triangle = std::pair<unsigned int, unsigned int>(triangle.vertex_two, triangle.vertex_three);
            } else if(triangle.vertex_two == vertex_id) {
                vertices_of_triangle = std::pair<unsigned int, unsigned int>(triangle.vertex_one, triangle.vertex_three);
            } else {
                // We know that the vertex has to be one of the vertices of the triangle, and since it wasn't 1 or 2, it must be 3
                vertices_of_triangle = std::pair<unsigned int, unsigned int>(triangle.vertex_one, triangle.vertex_two);
            }
            if(first) {
                // std::cout << "Found first triangle: " << vertices_of_triangle.first << ", " << vertices_of_triangle.second << std::endl;
                first_triangle = vertices_of_triangle;
                first = false;
            } else {
                triangles.insert(vertices_of_triangle);
            }
        }
        std::pair<unsigned int, unsigned int> next = first_triangle;
        polygon.push_back(next.first);
        unsigned int query = next.second;
        polygon.push_back(query);
        // std::cout << "Huh " << next.first << " " << next.second << std::endl;
        for(int i = 0, max = neighbhors.size() - 2; i < max; i++) {
            for(const std::pair<unsigned int, unsigned int> tri : triangles) {
                if(tri.first == query) {
                    next = tri;
                    query = next.second;
                    break;
                }
                if(tri.second == query) {
                    next = tri;
                    query = next.first;
                    break;
                }
            }
            polygon.push_back(query);
            triangles.erase(next);
        }

        for(unsigned int triangle_id : old_triangle_ids) {
            Triangle& triangle = this->all_triangles[triangle_id];
            this->vertices[triangle.vertex_one].remove_triangle(triangle_id);
            this->vertices[triangle.vertex_two].remove_triangle(triangle_id);
            this->vertices[triangle.vertex_three].remove_triangle(triangle_id);
        }

        return RemovedVertexInfo { old_triangle_ids, polygon };
    }

    std::vector<unsigned int> PlanarGraph::find_independant_set() {
        std::vector<unsigned int> res = std::vector<unsigned int>();
        std::unordered_set<unsigned int> forbidden = std::unordered_set<unsigned int>();
        
        for (int i = 0, size = this->vertices.size() - 3; i < size; i++) {
            if (!this->vertices[i].removed) {
                if(!forbidden.contains(i)) {
                    if(this->degree(i) < 8) {
                        res.push_back(i);
                        forbidden.insert(this->adjacency_list[i].begin(), this->adjacency_list[i].end());
                    }
                }
            }
        }
        return res;
    }
    
    std::vector<Triangle> PlanarGraph::get_triangulation(const std::vector<unsigned int>& polygon, std::vector<unsigned int>* hole) {
        // Using Triangle
        std::vector<std::vector<std::pair<double, double>>> earcut_polygon = std::vector<std::vector<std::pair<double, double>>>();
        const size_t size = polygon.size();
        std::vector<std::pair<double, double>> current_polygon = std::vector<std::pair<double, double>>();
        current_polygon.reserve(polygon.size());
        const unsigned int* ptr = polygon.data();
        for(size_t i = 0; i < size; i++) {
            const Vertex::Point point = this->vertices[ptr[i]].point;
            current_polygon.push_back(std::pair<double, double>(point.x, point.y));
        }
        earcut_polygon.push_back(current_polygon);
        unsigned int* hole_ptr;
        if(hole != nullptr) {
            const size_t size_hole = hole->size();
            std::vector<std::pair<double, double>> current_hole = std::vector<std::pair<double, double>>();
            current_hole.reserve(hole->size());
            const unsigned int* const_hole_ptr = hole_ptr = hole->data();
            for(int i = 0; i < size_hole; i++) {
                const Vertex::Point point = this->vertices[const_hole_ptr[i]].point;
                current_hole.push_back(std::pair<double, double>(point.x, point.y));
            }
            earcut_polygon.push_back(current_hole);
        }
        std::vector<unsigned int> triangles = mapbox::earcut<unsigned int>(earcut_polygon);
        const size_t num_triangles = triangles.size() / 3;
        const unsigned int* triangle_ptr = triangles.data();
        std::vector<Triangle> result = std::vector<Triangle>();
        result.reserve(num_triangles);
        for(int i = 0; i < num_triangles; i++) {
            unsigned int point_one = triangle_ptr[i * 3];
            if(point_one > size) {
                point_one = hole_ptr[point_one - size];
            } else {
                point_one = ptr[point_one];
            }
            unsigned int point_two = triangle_ptr[i * 3 + 1];
            if(point_two > size) {
                point_two = hole_ptr[point_two - size];
            } else {
                point_two = ptr[point_two];
            }
            unsigned int point_three = triangle_ptr[i * 3 + 2];
            if(point_three > size) {
                point_three = hole_ptr[point_three - size];
            } else {
                point_three = ptr[point_three];
            }
            result.push_back(Triangle { point_one, point_two, point_three });
        }
        return result;
    }

    std::unordered_set<unsigned int> PlanarGraph::triangulate_polygon(const std::vector<unsigned int>& polygon, std::vector<unsigned int>* hole) {
        std::vector<Triangle> triangles = get_triangulation(polygon, hole);
        std::unordered_set<unsigned int> new_triangle_ids = std::unordered_set<unsigned int>();
        const std::size_t size = triangles.size();
        const Triangle* ptr = triangles.data();
        for(std::size_t i = 0; i < size; i++) {
            const Triangle& triangle = ptr[i];
            this->connect_vertices(triangle.vertex_one, triangle.vertex_two);
            this->connect_vertices(triangle.vertex_two, triangle.vertex_three);
            this->connect_vertices(triangle.vertex_three, triangle.vertex_one);

            unsigned int new_id = this->all_triangles.size();
            new_triangle_ids.emplace(new_id);
            this->all_triangles.push_back(triangle);

            this->vertices[triangle.vertex_one].add_triangle(new_id); 
            this->vertices[triangle.vertex_two].add_triangle(new_id); 
            this->vertices[triangle.vertex_three].add_triangle(new_id); 
        }
        return new_triangle_ids;
    }

    std::unordered_set<unsigned int> PlanarGraph::triangulate_polygon(const std::vector<unsigned int>& polygon) {
        return triangulate_polygon(polygon, nullptr);
    }

    std::unordered_set<unsigned int>& DirectedAcyclicGraph::neighbhors(unsigned int n) {
        return this->graph.at(n);
    }

    unsigned int DirectedAcyclicGraph::root() {
        return this->_root;
    }

    inline bool PlanarGraph::triangles_intersect(unsigned int first, unsigned int second) {
        Triangle tri1 = this->all_triangles[first];
        Triangle tri2 = this->all_triangles[second];

        unsigned int triangle_one[3] = { tri1.vertex_one, tri1.vertex_two, tri1.vertex_three };
        unsigned int triangle_two[3] = { tri2.vertex_one, tri2.vertex_two, tri2.vertex_three };

        const Vertex::Point& t1p1 = this->vertices[tri1.vertex_one].point;
        const Vertex::Point& t1p2 = this->vertices[tri1.vertex_two].point;
        const Vertex::Point& t1p3 = this->vertices[tri1.vertex_three].point;
        const Vertex::Point& t2p1 = this->vertices[tri2.vertex_one].point;
        const Vertex::Point& t2p2 = this->vertices[tri2.vertex_two].point;
        const Vertex::Point& t2p3 = this->vertices[tri2.vertex_three].point;
        for(int i = 0; i < 3; i++) {
            const PointLocation::Vertex::Point& a = this->vertices[triangle_two[i]].point;
            if(triangle_contains_point(a, t1p1, t1p2, t1p3)) {
                return true;
            }
        }
        for(int i = 0; i < 3; i++) {
            const PointLocation::Vertex::Point& a = this->vertices[triangle_one[i]].point; 
            const PointLocation::Vertex::Point& b = this->vertices[triangle_one[(i == 2) ? 0 : i+1]].point; 
            if (triangle_contains_point(a, t2p1, t2p2, t2p3)) {
                return true;
            }
            for(int j = 0; j < 3; j++) {
                const PointLocation::Vertex::Point& c = this->vertices[triangle_two[j]].point; 
                const PointLocation::Vertex::Point& d = this->vertices[triangle_two[(j == 2) ? 0 : j+1]].point; 
                if(sides_intersect(a, b, c, d)) {
                    return true;
                }
            }
        }
        return false;
    }

    void PlanarGraph::remove_vertices(std::vector<unsigned int> vertices, std::shared_ptr<DirectedAcyclicGraph> dag) {
        std::unordered_set<unsigned int> triangles = std::unordered_set<unsigned int>(this->triangulations.back());
        for(std::size_t i = 0; i < vertices.size(); i++) {
            unsigned int vertex = vertices[i];
            RemovedVertexInfo dat = this->remove_vertex(vertex);
            // remove old triangles from previous triangulation
            for(unsigned int tri : dat.old_triangle_ids) {
                triangles.erase(tri);
            }
            std::unordered_set<unsigned int> new_triangles = this->triangulate_polygon(dat.polygon);

            for(unsigned int tri: new_triangles) {
                triangles.emplace(tri);
            }

            for(unsigned int old_tri : dat.old_triangle_ids) {
                for(unsigned int new_tri : new_triangles) {
                    if(triangles_intersect(old_tri, new_tri)) {
                        dag->add_directed_edge(old_tri, new_tri);
                    }
                }
            }
        }
        this->triangulations.push_back(triangles);
        std::shared_ptr<triangulateio> output = TriangleManipulator::create_instance();
        int i = 0;
        output->trianglelist = trimalloc<int>(triangles.size() * 3 * sizeof(int));
        output->numberoftriangles = triangles.size();
        int* output_triangle_ptr = output->trianglelist.get();
        for (unsigned int tri : triangles) {
            Triangle triangle = this->all_triangles[tri];
            output_triangle_ptr[i * 3] = triangle.vertex_one;
            output_triangle_ptr[i * 3 + 1] = triangle.vertex_two;
            output_triangle_ptr[i * 3 + 2] = triangle.vertex_three;
            i++;
        }
        TriangleManipulator::write_ele_file("test." + std::to_string(this->triangulations.size()) + ".ele", output);
    }
    void GraphInfo::process() {
        std::size_t last_run = 0;
        while (this->planar_graph->triangulations.back().size() > 1) {
            this->planar_graph->remove_vertices(this->planar_graph->find_independant_set(), this->directed_graph);
            if (last_run == this->planar_graph->triangulations.back().size()) {
                fmt::print("Could not simplify further.\n");
                break;
            }
            last_run = this->planar_graph->triangulations.back().size();
        }
    }
}