#include "TriangleManipulator/PointLocation.hpp"
#include "TriangleManipulator/ShapeManipulator.hpp"
#include "TriangleManipulator/TriangleManipulator.hpp"
#include "earcut.hpp"
#include "fmt/os.h"
#include <map>
#include <unordered_map>

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

    PlanarGraph::PlanarGraph() : vertices(), all_triangles(), triangulations(), num_vertices(0) {
    }


    PlanarGraph::PlanarGraph(std::shared_ptr<triangulateio> graph) : PlanarGraph() {
        // TODO: Figure out how to make triangles be in the same order in the graph as the original.
        std::shared_ptr<triangulateio> output = TriangleManipulator::create_instance();
        if (graph->numberofholes != 0) {
            graph->numberofholes = 0;
        }
        std::shared_ptr<triangulateio> real_graph = TriangleManipulator::create_instance();
        real_graph->numberofpoints = graph->numberofpoints + 3;
        real_graph->numberofsegments = graph->numberofsegments + 3;
        // I may be allocating like 3x more memory than I need to lol
        real_graph->pointlist = trimalloc<REAL>(real_graph->numberofpoints * 2);
        real_graph->segmentlist = trimalloc<int>(real_graph->numberofsegments * 2);
        real_graph->segmentmarkerlist = trimalloc<int>(real_graph->numberofsegments);
        real_graph->numberoftriangles = graph->numberoftriangles;
        real_graph->numberofcorners = 3;
        real_graph->trianglelist = graph->trianglelist;
        double min_y = INFINITY;
        double left_border = -INFINITY;
        double right_border = -INFINITY;
        const REAL* graph_point_ptr = graph->pointlist.get();
        for (std::size_t i = 0; i < graph->numberofpoints; i++) {
            double x = graph_point_ptr[i * 2];
            double y = graph_point_ptr[i * 2 + 1];
            if (y - 1 < min_y) {
                min_y = y - 1;
            }
            double dif = x * sqrt(3);
            if (y - dif + 2 > left_border) {
                left_border = y - dif + 2;
            }
            if (y + dif + 2 > right_border) {
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

        triangulate("pzeBPQN", real_graph, output, nullptr);
        std::shared_ptr<triangulateio> empty = TriangleManipulator::create_instance();
        this->vertices.reserve(real_graph->numberofpoints);
        const REAL* output_point_ptr = real_graph->pointlist.get();
        for (size_t i = 0; i < real_graph->numberofpoints; i++) {
            this->add_vertex(output_point_ptr[i * 2], output_point_ptr[i * 2 + 1]);
        }
        const int* output_edge_ptr = output->edgelist.get();
        for (size_t i = 0; i < output->numberofedges; i++) {
            this->connect_vertices(output_edge_ptr[i * 2], output_edge_ptr[i * 2 + 1]);
        }
        for (Vertex& vertex : this->vertices) {
            vertex.triangles.reserve(vertex.neighs.size());
        }
        this->triangulations.emplace_back(output->numberoftriangles);
        // triangulation.reserve(output->numberoftriangles);
        this->all_triangles.reserve(output->numberoftriangles);
        const unsigned int* output_triangle_ptr = output->trianglelist.get();

        for (std::size_t i = 0; i < output->numberoftriangles; i++) {
            unsigned int a = *output_triangle_ptr++;
            unsigned int b = *output_triangle_ptr++;
            unsigned int c = *output_triangle_ptr++;
            this->all_triangles.emplace_back(a, b, c);
            this->vertices[a].add_triangle(i);
            this->vertices[b].add_triangle(i);
            this->vertices[c].add_triangle(i);
            // triangulation.emplace(i);
        }
    }

    void PlanarGraph::add_vertex(double x, double y) {
        this->vertices.emplace_back(x, y);
    }

    inline void PlanarGraph::add_directed_edge(unsigned int first_vertex, unsigned int second_vertex) {
        this->vertices[first_vertex].neighs.emplace_back(second_vertex);
    }

    inline void PlanarGraph::remove_directed_edge(unsigned int first_vertex, unsigned int second_vertex) {
        std::erase(this->vertices[first_vertex].neighs, second_vertex);
    }

    inline void PlanarGraph::connect_vertices(unsigned int first_vertex, unsigned int second_vertex) {
        auto& first_neighs = this->vertices[first_vertex].neighs;
        auto& second_neighs = this->vertices[second_vertex].neighs;
        if ((std::find(first_neighs.begin(), first_neighs.end(), second_vertex) == first_neighs.end()) || (std::find(second_neighs.begin(), second_neighs.end(), first_vertex) == second_neighs.end())) {
            first_neighs.emplace_back(second_vertex);
            second_neighs.emplace_back(first_vertex);
            // this->add_directed_edge(first_vertex, second_vertex);
            // this->add_directed_edge(second_vertex, first_vertex);
        }
    }
    RemovedVertexInfo PlanarGraph::remove_vertex(unsigned int vertex_id) {
        Vertex& vertex = this->vertices[vertex_id];

        vertex.removed = true;
        std::vector<unsigned int>& neigh = vertex.neighs;

        const size_t DEGREE = vertex.degree();
        for (const unsigned int other : neigh) {
            this->remove_directed_edge(other, vertex_id);
        }

        neigh.clear();

        RemovedVertexInfo result = RemovedVertexInfo(vertex);

        std::vector<unsigned int>& old_triangle_ids = result.old_triangle_ids;
        old_triangle_ids.reserve(DEGREE);

        std::vector<unsigned int>& polygon = result.polygon;
        polygon.reserve(DEGREE);

        std::unordered_map<unsigned int, unsigned int> pointmap = std::unordered_map<unsigned int, unsigned int>();
        pointmap.reserve(DEGREE);
        // So, we have triangles.
        for (const unsigned int triangle_id : vertex.triangles) {
            const Triangle& triangle = this->all_triangles[triangle_id];
            old_triangle_ids.emplace_back(triangle_id);
            // Going clockwise the entire cycle allows us to make some optimizations.
            if (triangle.vertex_one == vertex_id) {
                pointmap.emplace(triangle.vertex_two, triangle.vertex_three);
            } else if (triangle.vertex_two == vertex_id) {
                pointmap.emplace(triangle.vertex_three, triangle.vertex_one);
            } else {
                pointmap.emplace(triangle.vertex_one, triangle.vertex_two);
            }
        }

        unsigned int query = pointmap.begin()->first;
        polygon.push_back(query);
        const size_t MAX = DEGREE - 1;

        for (size_t i = 0; i < MAX; i++) {
            query = pointmap[query];
            polygon.emplace_back(query);
        }

        for (unsigned int triangle_id : old_triangle_ids) {
            Triangle& triangle = this->all_triangles[triangle_id];
            this->vertices[triangle.vertex_one].remove_triangle(triangle_id);
            this->vertices[triangle.vertex_two].remove_triangle(triangle_id);
            this->vertices[triangle.vertex_three].remove_triangle(triangle_id);
        }

        return result;
    }

    inline std::vector<unsigned int> PlanarGraph::find_independant_set() {
        std::vector<unsigned int> res = std::vector<unsigned int>();
        // TODO: More efficient way of doing this?
        for (size_t i = 0, size = this->vertices.size() - 3; i < size; i++) {
            const Vertex& vertex = this->vertices[i];
            if (!vertex.removed && !vertex.forbidden) {
                const size_t degree = vertex.degree();
                if (degree < 8) {
                    res.emplace_back(i);
                    for (unsigned int neigh : vertex.neighs) {
                        this->vertices[neigh].forbidden = true;
                    }
                }
            }
        }
        for (size_t i = 0; i < this->vertices.size() - 3; i++) {
            this->vertices[i].forbidden = false;
        }
        return res;
    }

    inline std::vector<Triangle> PlanarGraph::get_triangulation(const std::vector<unsigned int>& polygon) const {
        // Using Earcut

        std::vector<std::vector<double64x2_t>> earcut_polygon = std::vector<std::vector<double64x2_t>>();

        const size_t size = polygon.size();

        std::vector<double64x2_t>& current_polygon = earcut_polygon.emplace_back();
        current_polygon.reserve(size);
        for (const unsigned int& point_id : polygon) {
            current_polygon.emplace_back(this->vertices[point_id].matrix);
        }

        const std::vector<unsigned int>& triangles = mapbox::earcut<unsigned int>(earcut_polygon);

        // For a polygon of size N, the minimum number of triangles to represent it is N - 2. This is the same number as yielded by earcut.
        const size_t num_triangles = size - 2;

        const unsigned int* triangle_ptr = triangles.data();

        std::vector<Triangle> result = std::vector<Triangle>();
        result.reserve(num_triangles);
        for (size_t i = 0; i < num_triangles * 3; i += 3) {
            result.emplace_back(polygon[triangle_ptr[i]], polygon[triangle_ptr[i + 1]], polygon[triangle_ptr[i + 2]]);
        }

        return result;
    }

    inline std::vector<unsigned int> PlanarGraph::triangulate_polygon(const std::vector<unsigned int>& polygon) {
        const std::vector<Triangle>& triangles = get_triangulation(polygon);
        std::vector<unsigned int> new_triangle_ids(triangles.size());
        // Copy the elements over, while also doing an operation on them.

        unsigned int new_id = this->all_triangles.size();
        for (size_t i = 0; i < triangles.size(); i++) {
            const Triangle& triangle = triangles[i];
            this->connect_vertices(triangle.vertex_one, triangle.vertex_two);
            this->connect_vertices(triangle.vertex_two, triangle.vertex_three);
            this->connect_vertices(triangle.vertex_three, triangle.vertex_one);

            this->all_triangles.emplace_back(triangle);

            this->vertices[triangle.vertex_one].add_triangle(new_id);
            this->vertices[triangle.vertex_two].add_triangle(new_id);
            this->vertices[triangle.vertex_three].add_triangle(new_id);
            new_triangle_ids[i] = new_id++;
        }
        return new_triangle_ids;
    }

    inline bool PlanarGraph::triangles_intersect(unsigned int first, unsigned int second) const {
        const Triangle& tri1 = this->all_triangles[first];
        const Triangle& tri2 = this->all_triangles[second];

        const double64x2_t t1p1 = this->vertices[tri1.vertex_one].matrix;
        const double64x2_t t1p2 = this->vertices[tri1.vertex_two].matrix;
        const double64x2_t t1p3 = this->vertices[tri1.vertex_three].matrix;
        const double64x2_t t2p1 = this->vertices[tri2.vertex_one].matrix;
        const double64x2_t t2p2 = this->vertices[tri2.vertex_two].matrix;
        const double64x2_t t2p3 = this->vertices[tri2.vertex_three].matrix;
        for (size_t i = 0; i < 3; i++) {
            if (point_inside_triangle(this->vertices[tri2.vertices[i]].matrix, t1p1, t1p2, t1p3)) {
                return true;
            }
        }
        for (size_t i = 0; i < 3; i++) {
            const double64x2_t a = this->vertices[tri1.vertices[i]].matrix;
            const double64x2_t b = this->vertices[tri1.vertices[(i == 2) ? 0 : i + 1]].matrix;
            if (point_inside_triangle(a, t2p1, t2p2, t2p3)) {
                return true;
            }
            for (size_t j = 0; j < 3; j++) {
                const double64x2_t c = this->vertices[tri2.vertices[j]].matrix;
                const double64x2_t d = this->vertices[tri2.vertices[(j == 2) ? 0 : j + 1]].matrix;
                if (sides_intersect(a, b, c, d)) {
                    return true;
                }
            }
        }
        return false;
    }

    void PlanarGraph::remove_vertices(const std::vector<unsigned int>& vertices, DirectedAcyclicGraph& dag) {
        size_t new_tricount = this->triangulations.back();
        for (unsigned int vertex : vertices) {
            const RemovedVertexInfo dat = this->remove_vertex(vertex);

            const std::vector<unsigned int>& new_triangles = this->triangulate_polygon(dat.polygon);
            new_tricount -= dat.old_triangle_ids.size();
            for (unsigned int new_tri : new_triangles) {
                for (unsigned int old_tri : dat.old_triangle_ids) {
                    if (triangles_intersect(old_tri, new_tri)) {
                        dag.add_directed_edge(new_tri, old_tri);
                    }
                }
            }
            new_tricount += new_triangles.size();
            triangulations.emplace_back(new_tricount);
            // triangles.insert(new_triangles.begin(), new_triangles.end());
        }
    }

    void GraphInfo::process() {
        std::size_t last_run = 0;
        while (planar_graph.triangulations.back() > 1) {
            planar_graph.remove_vertices(planar_graph.find_independant_set(), directed_graph);
            if (last_run == planar_graph.triangulations.back()) {
                break;
            }
            last_run = planar_graph.triangulations.back();
        }

        directed_graph.root = planar_graph.all_triangles.size() - 1;
    }

    void GraphInfo::read_from_binary_file(std::string filename) {
        TriangleManipulator::binary_reader reader(filename.c_str());
        directed_graph.root = reader.read<unsigned int>();

        size_t directed_graph_size = reader.read<size_t>();
        auto& directed_graph_entries = directed_graph.graph;
        directed_graph_entries.resize(directed_graph_size);

        reader.read_array(directed_graph_entries.data(), directed_graph_size);

        auto& vertices = planar_graph.vertices;
        size_t planar_graph_vertices_size = reader.read<size_t>();
        vertices.reserve(planar_graph_vertices_size);

        for (size_t i = 0; i < planar_graph_vertices_size; i++) {
            size_t triangles_size = reader.read<size_t>();
            size_t neighs_size = reader.read<size_t>();
            double x = reader.read<double>();
            double y = reader.read<double>();
            bool removed = reader.read<bool>();
            bool forbidden = reader.read<bool>();
            Vertex& vertex = vertices.emplace_back(triangles_size, neighs_size, x, y, removed, forbidden);
            reader.read_array(vertex.triangles.data(), triangles_size);
            reader.read_array(vertex.neighs.data(), neighs_size);
        }
        size_t triangle_count = reader.read<size_t>();
        auto& triangles = planar_graph.all_triangles;
        triangles.resize(triangle_count);
        reader.read_array(triangles.data(), triangle_count);

        size_t triangulations_count = reader.read<size_t>();
        auto& triangulations = planar_graph.triangulations;
        triangulations.resize(triangulations_count);
        reader.read_array(triangulations.data(), triangulations_count);

        reader.read(planar_graph.num_vertices);
        size_t map_size = reader.read<size_t>();
        triangle_map.resize(map_size);
        reader.read_array(triangle_map.data(), map_size);
        reader.close();
    }
    void GraphInfo::write_to_binary_file(std::string filename) const {
        TriangleManipulator::binary_writer writer(filename.c_str());
        writer.write(directed_graph.root);

        writer.write(directed_graph.graph.size());
        writer.write_array(directed_graph.graph.data(), directed_graph.graph.size());

        const auto& vertices = planar_graph.vertices;
        writer.write(vertices.size());
        for (size_t i = 0; i < vertices.size(); i++) {
            const Vertex& vertex = vertices[i];
            writer.write(vertex.triangles.size());
            writer.write(vertex.neighs.size());

            writer.write(vertex.point.x);
            writer.write(vertex.point.y);

            writer.write(vertex.removed);
            writer.write(vertex.forbidden);

            writer.write_array(vertex.triangles.data(), vertex.triangles.size());
            writer.write_array(vertex.neighs.data(), vertex.neighs.size());
        }

        writer.write(planar_graph.all_triangles.size());
        writer.write_array(planar_graph.all_triangles.data(), planar_graph.all_triangles.size());

        writer.write(planar_graph.triangulations.size());
        writer.write_array(planar_graph.triangulations.data(), planar_graph.triangulations.size());

        writer.write(planar_graph.num_vertices);
        writer.write(this->triangle_map.size());
        writer.write_array(this->triangle_map.data(), this->triangle_map.size());
        writer.close();
    }
    std::optional<unsigned int> GraphInfo::locate_point(Vertex::Point point) const {
        if (!triangle_contains_point(point, planar_graph.all_triangles[directed_graph.root])) {
            return std::nullopt;
        }
        unsigned int last_checked = directed_graph.root, prev;
        do {
            prev = last_checked;
            auto [first, last] = directed_graph.neighbhors(last_checked);
            if (std::distance(first, last) == 0) {
                if (triangle_map[last_checked] == -1) {
                    return std::nullopt;
                }
                return triangle_map[last_checked];
            }
            for (auto current = first;current != last; current++) {
                auto child = current->second;
                if (triangle_contains_point(point, planar_graph.all_triangles[child])) {
                    last_checked = child;
                    break;
                }
            }
        } while (prev != last_checked);

        // This really shouldn't happen. If this happens, this means that a point was somewhere where it was inside
        // A triangle, but not inside any of its children. The children of a triangle should cover all of a triangle
        // With no gaps. Thus... it should be impossible.
        return std::nullopt;
    }
    void GraphInfo::map_triangles(std::shared_ptr<triangulateio> others) {
        std::map<std::tuple<unsigned int, unsigned int, unsigned int>, unsigned int> temp_triangle_map = std::map<std::tuple<unsigned int, unsigned int, unsigned int>, unsigned int>();

        unsigned int num_triangles = others->numberoftriangles;
        const unsigned int* triangle_ptr = others->trianglelist.get();
        for (unsigned int i = 0; i < num_triangles; i++) {
            unsigned int a = *triangle_ptr++;
            unsigned int b = *triangle_ptr++;
            unsigned int c = *triangle_ptr++;
            sort(a, b, c);
            temp_triangle_map.emplace(std::forward_as_tuple(a, b, c), i);
        }
        const auto& triangles = planar_graph.all_triangles;
        this->triangle_map.resize(planar_graph.triangulations.front(), -1);
        for (unsigned int i = 0; i < planar_graph.triangulations.front(); i++) {
            const Triangle& tri = triangles[i];
            unsigned int a = tri.vertex_one;
            unsigned int b = tri.vertex_two;
            unsigned int c = tri.vertex_three;
            sort(a, b, c);
            std::map<std::tuple<unsigned int, unsigned int, unsigned int>, unsigned int>::iterator find = temp_triangle_map.find({ a, b, c });
            if (find != temp_triangle_map.end()) {
                triangle_map[i] = find->second;
            }
        }
    }
}

namespace mapbox {
    namespace util {

        template <>
        struct nth<0, PointLocation::double64x2_t> {
            inline static auto get(const PointLocation::double64x2_t& v) {
                return v[0];
            };
        };
        template <>
        struct nth<1, PointLocation::double64x2_t> {
            inline static auto get(const PointLocation::double64x2_t& v) {
                return v[1];
            };
        };
    }
}
