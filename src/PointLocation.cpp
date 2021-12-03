#include "TriangleManipulator/PointLocation.hpp"
#include "TriangleManipulator/ShapeManipulator.hpp"
#include "TriangleManipulator/TriangleManipulator.hpp"
#include "earcut.hpp"
#include "fmt/os.h"

// template class std::vector<PointLocation::Point>;
// template class std::vector<PointLocation::Line>;

namespace PointLocation {
    bool overlaps(short start1, short end1, short start2, short end2) {
        start1 = std::min(start1, end1);
        end1 = std::max(start1, end1);
        start2 = std::min(start2, end2);
        end2 = std::max(start2, end2);
        if ((start1 - end2) * (end1 - start2) <= 0) {
            return true;
        }
        return false;
    };
    DirectedAcyclicGraph::DirectedAcyclicGraph() : root(0), graph() {
    };

    inline void DirectedAcyclicGraph::add_directed_edge(unsigned int first, unsigned int second) {
        this->graph[first].insert(second);
    }
    Vertex::Vertex() : triangles(), point{ 0, 0 }, id(-1), removed(false) {}

    Vertex::Vertex(double x, double y) : triangles(), point{ x, y }, id(-1), removed(false) {}

    Vertex::Vertex(double x, double y, unsigned int id) : triangles(), point{ x, y }, id(id), removed(false) {}
    
    Vertex::Vertex(double x, double y, unsigned int id, bool removed) : triangles(), point{ x, y }, id(id), removed(removed) {}

    inline void Vertex::add_triangle(unsigned int triangle_id) {
        this->triangles.insert(triangle_id);
    }

    inline void Vertex::remove_triangle(unsigned int triangle_id) {
        this->triangles.erase(triangle_id);
    }

    PlanarGraph::PlanarGraph() : vertices(), adjacency_list(), all_triangles(), triangulations(), num_vertices(0) {}
    

    PlanarGraph::PlanarGraph(std::shared_ptr<triangulateio> graph) : PlanarGraph() {
        // TODO: Figure out how to make triangles be in the same order in the graph as the original.
        std::shared_ptr<triangulateio> output = TriangleManipulator::create_instance();
        if (graph->numberofholes != 0) {
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
        this->vertices.reserve(output->numberofpoints);
        const REAL* output_point_ptr = output->pointlist.get();
        for (unsigned int i = 0; i < output->numberofpoints; i++) {
            this->add_vertex(output_point_ptr[i * 2], output_point_ptr[i * 2 + 1]);
            // Allow one unit of space on edges
            
        }
        const int* output_segment_ptr = output->segmentlist.get();
        for (unsigned int i = 0; i < output->numberofsegments; i++) {
            this->connect_vertices(output_segment_ptr[i * 2], output_segment_ptr[i * 2 + 1]);
        }
        const int* output_edge_ptr = output->edgelist.get();
        for (unsigned int i = 0; i < output->numberofedges; i++) {
            this->connect_vertices(output_edge_ptr[i * 2], output_edge_ptr[i * 2 + 1]);
        }
        this->triangulations.push_back(std::set<unsigned int>());
        this->all_triangles.reserve(output->numberoftriangles);
        const unsigned int* output_triangle_ptr = output->trianglelist.get();
        for (std::size_t i = 0; i < output->numberoftriangles; i++) {
            unsigned int a = *output_triangle_ptr++;
            unsigned int b = *output_triangle_ptr++;
            unsigned int c = *output_triangle_ptr++;
            this->vertices[a].add_triangle(i);
            this->vertices[b].add_triangle(i);
            this->vertices[c].add_triangle(i);
            this->triangulations[0].insert(i);
            this->all_triangles.emplace_back(a, b, c);
        }
    }

    unsigned int PlanarGraph::add_vertex(double x, double y) {
        Vertex vert(x, y, this->vertices.size());
        this->vertices.push_back(vert);
        this->adjacency_list.push_back(std::set<unsigned int>());
        this->num_vertices++;
        
        return vert.id;
    }
    inline unsigned int PlanarGraph::degree(unsigned int vertex_id) {
        return this->adjacency_list[vertex_id].size();
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
        std::set<unsigned int>& neigh = neighbhors(vertex_id);
        const unsigned int DEGREE = neigh.size();
        for (const unsigned int other : neigh) {
            this->remove_directed_edge(other, vertex_id);
        }
        neigh.clear();

        RemovedVertexInfo result = RemovedVertexInfo();
        
        std::vector<unsigned int>& old_triangle_ids = result.old_triangle_ids;
        old_triangle_ids.reserve(DEGREE);

        std::vector<unsigned int>& polygon = result.polygon;
        polygon.reserve(DEGREE);

        std::unordered_map<unsigned int, unsigned int> triangles = std::unordered_map<unsigned int, unsigned int>();
        triangles.reserve(DEGREE);

        for (const unsigned int triangle_id : this->vertices[vertex_id].triangles) {
            const Triangle& triangle = this->all_triangles[triangle_id];
            old_triangle_ids.emplace_back(triangle_id);
            // Going clockwise the entire cycle allows us to make some optimizations.
            if (triangle.vertex_one == vertex_id) {
                triangles.emplace(triangle.vertex_two, triangle.vertex_three);
            } else if (triangle.vertex_two == vertex_id) {
                triangles.emplace(triangle.vertex_three, triangle.vertex_one);
            } else {
                triangles.emplace(triangle.vertex_one, triangle.vertex_two);
            }
        }
        unsigned int query = 0;
        {
            Triangle triangle = this->all_triangles[*old_triangle_ids.begin()];
            if (triangle.vertex_one == vertex_id) {
                query = triangle.vertex_two;
            } else if (triangle.vertex_two == vertex_id) {
                query = triangle.vertex_three;
            } else {
                query = triangle.vertex_one;
            }
        }
        polygon.emplace_back(query);
        const unsigned int MAX = DEGREE - 1;
        for (unsigned int i = 0; i < MAX; i++) {
            query = triangles.at(query);
            polygon.emplace_back(query);
        }
        for(unsigned int triangle_id : old_triangle_ids) {
            Triangle& triangle = this->all_triangles[triangle_id];
            this->vertices[triangle.vertex_one].remove_triangle(triangle_id);
            this->vertices[triangle.vertex_two].remove_triangle(triangle_id);
            this->vertices[triangle.vertex_three].remove_triangle(triangle_id);
        }

        return result;
    }

    inline std::vector<unsigned int> PlanarGraph::find_independant_set() {
        std::vector<unsigned int> res = std::vector<unsigned int>();
        std::unordered_set<unsigned int> forbidden = std::unordered_set<unsigned int>();
        for (size_t i = 0, size = this->vertices.size() - 3; i < size; i++) {
            if (!this->vertices[i].removed) {
                if (!forbidden.contains(i)) {
                    const unsigned int degree = this->degree(i);
                    if (degree < 8) {
                        res.push_back(i);
                        forbidden.insert(this->adjacency_list[i].cbegin(), this->adjacency_list[i].cend());
                    }
                }
            }
        }
        return res;
    }
    
    inline std::vector<Triangle> PlanarGraph::get_triangulation(const std::vector<unsigned int>& polygon) {
        // Using Triangle
        const size_t size = polygon.size();
        std::shared_ptr<triangulateio> poly = TriangleManipulator::create_instance();
        poly->numberofpoints = size;
        poly->pointlist = trimalloc<REAL>(size * 2);
        poly->numberofsegments = size;
        poly->segmentlist = trimalloc<int>(size * 2);
        poly->segmentmarkerlist = trimalloc<int>(size);
        const unsigned int* ptr = polygon.data();
        REAL* point_ptr = poly->pointlist.get();
        int* segment_ptr = poly->segmentlist.get();
        int* segment_marker_ptr = poly->segmentmarkerlist.get();
        for (size_t i = 0; i < size; i++) {
            const Vertex::Point& point = this->vertices[ptr[i]].point;
            point_ptr[i * 2] = point.x;
            point_ptr[i * 2 + 1] = point.y;
            segment_ptr[i * 2] = i;
            segment_ptr[i * 2 + 1] = (i + 1) % size;
            *segment_marker_ptr++ = 1;
        }
        std::shared_ptr<triangulateio> result = TriangleManipulator::create_instance();
        triangulate("pzQPN", poly, result, nullptr);
        const unsigned int* triangle_ptr = result->trianglelist.get();
        
        std::vector<Triangle> results = std::vector<Triangle>();
        results.reserve(result->numberoftriangles);
        for (size_t i = 0; i < result->numberoftriangles; i++) {
            unsigned int point_one = ptr[triangle_ptr[i * 3]];
            unsigned int point_two = ptr[triangle_ptr[i * 3 + 1]];
            unsigned int point_three = ptr[triangle_ptr[i * 3 + 2]];
            results.emplace_back(point_one, point_two, point_three);
        }
        return results;
    }

    inline std::vector<unsigned int> PlanarGraph::triangulate_polygon(const std::vector<unsigned int>& polygon) {
        const std::vector<Triangle>& triangles = get_triangulation(polygon);
        std::vector<unsigned int> new_triangle_ids = std::vector<unsigned int>();
        const std::size_t size = triangles.size();
        new_triangle_ids.reserve(size);
        const Triangle* ptr = triangles.data();
        for(std::size_t i = 0; i < size; i++) {
            const Triangle& triangle = ptr[i];
            this->connect_vertices(triangle.vertex_one, triangle.vertex_two);
            this->connect_vertices(triangle.vertex_two, triangle.vertex_three);
            this->connect_vertices(triangle.vertex_three, triangle.vertex_one);

            unsigned int new_id = this->all_triangles.size();
            new_triangle_ids.emplace_back(new_id);
            this->all_triangles.push_back(triangle);

            this->vertices[triangle.vertex_one].add_triangle(new_id);
            this->vertices[triangle.vertex_two].add_triangle(new_id); 
            this->vertices[triangle.vertex_three].add_triangle(new_id); 
        }
        return new_triangle_ids;
    }

    inline std::set<unsigned int>& DirectedAcyclicGraph::neighbhors(unsigned int n) {
        return this->graph.at(n);
    }
    
    inline bool PlanarGraph::triangles_intersect(unsigned int first, unsigned int second) {
        Triangle tri1 = this->all_triangles[first];
        Triangle tri2 = this->all_triangles[second];

        const double64x2_t t1p1 = this->vertices[tri1.vertex_one].matrix;
        const double64x2_t t1p2 = this->vertices[tri1.vertex_two].matrix;
        const double64x2_t t1p3 = this->vertices[tri1.vertex_three].matrix;
        const double64x2_t t2p1 = this->vertices[tri2.vertex_one].matrix;
        const double64x2_t t2p2 = this->vertices[tri2.vertex_two].matrix;
        const double64x2_t t2p3 = this->vertices[tri2.vertex_three].matrix;
        for (unsigned int i = 0; i < 3; i++) {
            if(point_inside_triangle(this->vertices[tri2.vertices[i]].matrix, t1p1, t1p2, t1p3)) {
                return true;
            }
        }
        for(int i = 0; i < 3; i++) {
            const double64x2_t a = this->vertices[tri1.vertices[i]].matrix; 
            const double64x2_t b = this->vertices[tri1.vertices[(i == 2) ? 0 : i+1]].matrix; 
            if (point_inside_triangle(a, t2p1, t2p2, t2p3)) {
                return true;
            }
            for (int j = 0; j < 3; j++) {
                const double64x2_t c = this->vertices[tri2.vertices[j]].matrix;
                const double64x2_t d = this->vertices[tri2.vertices[(j == 2) ? 0 : j+1]].matrix; 
                if(sides_intersect(a, b, c, d)) {
                    return true;
                }
            }
        }
        return false;
    }

    void PlanarGraph::remove_vertices(std::vector<unsigned int> vertices, std::shared_ptr<DirectedAcyclicGraph> dag) {
        std::set<unsigned int>& triangles = this->triangulations.emplace_back(this->triangulations.back());
        for (unsigned int vertex : vertices) {
            RemovedVertexInfo dat = this->remove_vertex(vertex);
            // remove old triangles from previous triangulation
            for (unsigned int tri : dat.old_triangle_ids) {
                triangles.erase(tri);
            }
            const std::vector<unsigned int>& new_triangles = this->triangulate_polygon(dat.polygon);

            for(unsigned int tri: new_triangles) {
                triangles.emplace(tri);
            }

            for(unsigned int old_tri : dat.old_triangle_ids) {
                for(unsigned int new_tri : new_triangles) {
                    if(triangles_intersect(old_tri, new_tri)) {
                        dag->add_directed_edge(new_tri, old_tri);
                    }
                }
            }
        }
    }
    void GraphInfo::process() {
        std::size_t last_run = 0;
        while (this->planar_graph->triangulations.back().size() > 1) {
            this->planar_graph->remove_vertices(this->planar_graph->find_independant_set(), this->directed_graph);
            if (last_run == this->planar_graph->triangulations.back().size()) {
                break;
            }
            last_run = this->planar_graph->triangulations.back().size();
        }
        this->directed_graph->root = this->planar_graph->all_triangles.size() - 1;
    }
    
    inline void unwrap_write(const std::set<unsigned int>& container, fmt::v8::ostream& file) {
        size_t num_iterations = container.size() / 4;
        auto cur = container.begin();
        for (size_t i = 0; i < num_iterations; i++) {
            unsigned int first = *cur++;
            unsigned int second = *cur++;
            unsigned int third = *cur++;
            unsigned int fourth = *cur++;
            file.print(" {} {} {} {}", first, second, third, fourth);
        }
        while (cur != container.end()) {
            file.print(" {}", *cur++);
        }
    }
    inline void unwrap_write(const std::set<unsigned int>& container, TriangleManipulator::binary_writer<>& file) {
        size_t num_iterations = container.size() / 4;
        auto cur = container.begin();
        for (size_t i = 0; i < num_iterations; i++) {
            file.write(*cur++);
            file.write(*cur++);
            file.write(*cur++);
            file.write(*cur++);
        }
        while (cur != container.end()) {
            file.write(*cur++);
        }
    }
    void GraphInfo::read_from_binary_file(std::string filename) {
        TriangleManipulator::binary_reader<> reader = TriangleManipulator::binary_reader<>(filename.c_str());
        const unsigned int TRIANGLES =      reader.read<unsigned int>();
        const unsigned int VERTICES =       reader.read<unsigned int>();
        const unsigned int TRIANGULATIONS = reader.read<unsigned int>();
        const unsigned int NOT_REMOVED = reader.read<unsigned int>();
        this->planar_graph->vertices.reserve(VERTICES);
        this->planar_graph->adjacency_list.reserve(VERTICES);
        this->planar_graph->triangulations.reserve(TRIANGULATIONS);
        this->planar_graph->num_vertices = NOT_REMOVED;
        for (unsigned int i = 0; i < VERTICES; i++) {
            double x = reader.read<double>();
            double y = reader.read<double>();
            unsigned char num_triangles = reader.read<unsigned char>();
            unsigned char num_neighbors = reader.read<unsigned char>();
            bool removed = reader.read<bool>();
            Vertex& vertex = this->planar_graph->vertices.emplace_back(x, y, i, removed);
            std::set<unsigned int>& neighbors = this->planar_graph->adjacency_list.emplace_back();
            if (!vertex.removed) {
                for (unsigned char i = 0; i < num_triangles; i++) {
                    vertex.triangles.emplace(reader.read<unsigned int>());
                }
                for (unsigned char i = 0; i < num_neighbors; i++) {
                    neighbors.emplace(reader.read<unsigned int>());
                }
            }
        }
        planar_graph->all_triangles.resize(TRIANGLES);
        reader.read_array((unsigned int*) planar_graph->all_triangles.data(), TRIANGLES * 3);
        for (unsigned int i = 0; i < TRIANGULATIONS; i++) {
            const unsigned int triangulation_size = reader.read<unsigned int>();
            std::set<unsigned int>& triangulation = planar_graph->triangulations.emplace_back();
            for (unsigned int i = 0; i < triangulation_size; i++) {
                triangulation.emplace(reader.read<unsigned int>());
            }
        }
        directed_graph->root = reader.read<unsigned int>();
        const unsigned int GRAPH_SIZE = reader.read<unsigned int>();
        // directed_graph->graph.reserve(GRAPH_SIZE);
        const unsigned int TRI_MAP_SIZE = reader.read<unsigned int>();
        for (unsigned int i = 0; i < GRAPH_SIZE; i++) {
            unsigned int index = reader.read<unsigned int>();
            size_t size = reader.read<size_t>();
            std::set<unsigned int>& node = directed_graph->graph[index];
            // node.reserve(size);
            for (size_t i = 0; i < size; i++) {
                node.emplace(reader.read<unsigned int>());
            }
        }
        for (unsigned int i = 0; i < TRI_MAP_SIZE; i++) {
            // const auto first = reader.read<std::tuple<unsigned int, unsigned int, unsigned int>>();
            triangle_map.insert(reader.read<std::pair<std::tuple<unsigned int, unsigned int, unsigned int>, unsigned int>>());
        }
        reader.close();
    }
    void GraphInfo::write_to_binary_file(std::string filename) {
        TriangleManipulator::binary_writer<> writer = TriangleManipulator::binary_writer<>(filename.c_str());
        const unsigned int TRIANGLES = planar_graph->all_triangles.size();
        const unsigned int VERTICES = planar_graph->vertices.size();
        const unsigned int TRIANGULATIONS = planar_graph->triangulations.size();
        const unsigned int NOT_REMOVED = planar_graph->num_vertices;
        writer.write(TRIANGLES);
        writer.write(VERTICES);
        writer.write(TRIANGULATIONS);
        writer.write(NOT_REMOVED);
        const Vertex* VERTICES_PTR = this->planar_graph->vertices.data();
        for (unsigned int i = 0; i < VERTICES; i++) {
            const Vertex& vertex = *VERTICES_PTR++;
            writer.write(vertex.point.x);
            writer.write(vertex.point.y);
            writer.write<unsigned char>(vertex.triangles.size());
            writer.write<unsigned char>(planar_graph->adjacency_list[i].size());
            writer.write(vertex.removed);
            if (!vertex.removed) {
                unwrap_write(vertex.triangles, writer);
                unwrap_write(planar_graph->adjacency_list[i], writer);
            }
        }
        writer.write_array((unsigned int*) planar_graph->all_triangles.data(), TRIANGLES * 3);
        for (unsigned int i = 0; i < planar_graph->triangulations.size(); i++) {
            writer.write<unsigned int>(planar_graph->triangulations[i].size());
            unwrap_write(planar_graph->triangulations[i], writer);
        }
        
        // Serializing the DirectedAcyclicGraph
        writer.write<unsigned int>(directed_graph->root);
        writer.write<unsigned int>(directed_graph->graph.size());
        writer.write<unsigned int>(triangle_map.size());
        for (const std::pair<unsigned int, std::set<unsigned int>>& pair : directed_graph->graph) {
            writer.write(pair.first);
            writer.write(pair.second.size());
            unwrap_write(pair.second, writer);
        }
        for (const std::pair<std::tuple<unsigned int, unsigned int, unsigned int>, unsigned int>& pair : triangle_map) {
            writer.write(pair);
            // writer.write(pair);
        }
        writer.close();
    }
    void GraphInfo::write_to_file(std::string base_filename) {
        fmt::v8::ostream file = fmt::output_file(base_filename + ".gi");
        const unsigned int VERTICES = planar_graph->vertices.size();
        file.print("{}", VERTICES);
        unsigned int NOT_REMOVED = 0;
        const Vertex* VERTICES_PTR = this->planar_graph->vertices.data();
        for (unsigned int i = 0; i < VERTICES; i++) {
            const Vertex& vertex = *VERTICES_PTR++;
            file.print("\n{} {} {}", vertex.removed, vertex.point.x, vertex.point.y);
            if (!vertex.removed) {
                NOT_REMOVED++;
                file.print(" {} {}", vertex.triangles.size(), planar_graph->adjacency_list[i].size());
            }
        }
        file.print("\n{}", NOT_REMOVED);
        VERTICES_PTR = this->planar_graph->vertices.data();
        for (unsigned int i = 0; i < VERTICES; i++) {
            const Vertex& vertex = *VERTICES_PTR++;
            if (!vertex.removed) {
                file.print("\n{}", i);
                unwrap_write(vertex.triangles, file);
                unwrap_write(planar_graph->adjacency_list[i], file);
            }
        }
        const unsigned int TRIANGLES = planar_graph->all_triangles.size();
        file.print("\n{}", TRIANGLES);
        for (unsigned int i = 0; i < TRIANGLES; i++) {
            const Triangle& tri = planar_graph->all_triangles[i];
            file.print("\n{} {} {}", tri.vertex_one, tri.vertex_two, tri.vertex_three);
        }
        file.print("\n{}", planar_graph->triangulations.size());
        for (std::size_t i = 0; i < planar_graph->triangulations.size(); i++) {
            file.print("\n{}", i);
            unwrap_write(planar_graph->triangulations[i], file);
        }
        // Serializing the DirectedAcyclicGraph
        file.print("\n{}", directed_graph->root);
        file.print("\n{}", directed_graph->graph.size());
        for (const std::pair<unsigned int, std::set<unsigned int>>& pair : directed_graph->graph) {
            file.print("\n{}", pair.first);
            unwrap_write(pair.second, file);
        }
        file.print("\n{}", triangle_map.size());
        for (const std::pair<std::tuple<unsigned int, unsigned int, unsigned int>, unsigned int>& pair : triangle_map) {
            file.print("\n{} {} {} {}", std::get<0>(pair.first), std::get<1>(pair.first), std::get<2>(pair.first), pair.second);
        }
        file.close();
    }
    int GraphInfo::locate_point(Vertex::Point point) {
        if (!triangle_contains_point(point, planar_graph->all_triangles[directed_graph->root])) {
            return -1;
        }
        int last_checked = directed_graph->root; // root
        while (directed_graph->graph.contains(last_checked)) {
            int prev = last_checked;
            for (unsigned int child : directed_graph->neighbhors(last_checked)) {
                if (triangle_contains_point(point, planar_graph->all_triangles[child])) {
                    last_checked = child;
                    break;
                }
            }
            if (prev == last_checked) {
                return -1;
            }
        }
        Triangle& tri = planar_graph->all_triangles[last_checked];
        unsigned int a = tri.vertex_one;
        unsigned int b = tri.vertex_two;
        unsigned int c = tri.vertex_three;
        sort(a, b, c);
        std::map<std::tuple<unsigned int, unsigned int, unsigned int>, unsigned int>::iterator find = triangle_map.find({a, b, c});
        if (find != triangle_map.end()) {
            return find->second;
        }
        return -1;
    }
    void GraphInfo::map_triangles(std::shared_ptr<triangulateio> others) {
        unsigned int num_triangles = others->numberoftriangles;
        unsigned int* triangle_ptr = others->trianglelist.get();
        for (unsigned int i = 0; i < num_triangles; i++) {
            unsigned int a = *triangle_ptr++;
            unsigned int b = *triangle_ptr++;
            unsigned int c = *triangle_ptr++;
            sort(a, b, c);
            triangle_map.emplace(std::tuple<unsigned int, unsigned int, unsigned int> { a, b, c }, i);
        }
    }
}