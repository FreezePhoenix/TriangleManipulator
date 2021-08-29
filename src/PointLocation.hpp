#pragma once

#ifndef POINT_LOCATION_HPP_
#define POINT_LOCATION_HPP_

#include "../triangle/triangle.h"
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include <memory>

namespace PointLocation {
    struct pair_hash {
        inline std::size_t operator()(const std::pair<unsigned int, unsigned int> & v) const {
            return ((unsigned long) v.first << 32) + v.second;
        }
        inline std::size_t operator () (const std::pair<unsigned int, unsigned int> & v1, const std::pair<unsigned int, unsigned int> & v2) const {
            return this->operator()(v1) - this->operator()(v2);
        }
    };
    union Point {
        struct {
            short x;
            short y;
        };
        unsigned int hash;
        bool operator==(const Point& other) {
            return other.hash == this->hash;
        }
    };
    struct PointHash {
        inline unsigned int operator () (const Point &p) const {
            return p.hash;
        }
        inline std::size_t operator () (const Point &p, const Point &p2) const {
            return p.hash - p2.hash;
        }
    };
    union Line {
        struct {
            Point first;
            Point second;
        };
        struct {
            short x1;
            short y1;
            short x2;
            short y2;
        };
        unsigned long int hash;
        constexpr Line() {
            this->first = Point{0, 0};
            this->second = Point{0, 0};
            // this->hash = 0;
        }
        constexpr Line(Point& first, Point& second) {
            if(first.x < second.x || first.y < second.y) {
                this->first = first;
                this->second = second;
            } else {
                this->first = second;
                this->second = first;
            }
            // this->hash = ((unsigned long int) this->first.hash << 32) + (unsigned long int) this->second.hash;
        }
        constexpr Line(const Point& first, const Point& second) {
            if(first.x < second.x || first.y < second.y) {
                this->first = first;
                this->second = second;
            } else {
                this->first = second;
                this->second = first;
            }
            // this->hash = ((unsigned long int) this->first.hash << 32) + (unsigned long int) this->second.hash;
        }
        bool operator==(const Line& other) {
            return other.hash == this->hash;
        }
        Line(short x1, short y1, short x2, short y2): Line(Point{x1, y1}, Point{x2, y2}) {}
    };
    struct LineHash {
        inline unsigned long int operator () (const Line &l) const {
            return l.hash;
        }
        inline std::size_t operator () (const Line &p, const Line &p2) const {
            return p.hash - p2.hash;
        }
    };

    union Triangle {
        struct {
            unsigned int vertex_one;
            unsigned int vertex_two;;
            unsigned int vertex_three;
        };
        unsigned int vertices[3];
        Triangle(unsigned int a, unsigned int b, unsigned int c) : vertex_one(a), vertex_two(b), vertex_three(c) {
        }
        inline bool operator==(const Triangle& rhs) {
            std::set<unsigned int> first { vertex_one, vertex_two, vertex_three };
            return (!first.emplace(rhs.vertex_one).second) && (!first.emplace(rhs.vertex_two).second) && (!first.emplace(rhs.vertex_three).second);
        }
    };
    typedef double double64x2_t __attribute__ ((__vector_size__ (16)));
    struct Vertex {
        unsigned int id;
        struct Point {
            double x;
            double y;
        };
        union {
            Point point;
            double64x2_t matrix;
        };
        Vertex(double x, double y);
        std::unordered_set<unsigned int> triangles;
        bool removed = false;
        void add_triangle(unsigned int triangle_id);
        void remove_triangle(unsigned int triangle_id);
    };

    struct RemovedVertexInfo {
        std::unordered_set<unsigned int> old_triangle_ids;
        // Array of point ID's
        std::vector<unsigned int> polygon;
    };

    class DirectedAcyclicGraph {
        public:
            unsigned int _root;
            std::unordered_map<unsigned int, std::unordered_set<unsigned int>> graph;
            DirectedAcyclicGraph();
            void add_directed_edge(unsigned int first, unsigned int second);
            std::unordered_set<unsigned int>& neighbhors(unsigned int n);
            unsigned int root();
            void write_to_file(std::string base_name);
    };

    class PlanarGraph {
        public:
            PlanarGraph();
            PlanarGraph(std::shared_ptr<triangulateio> input);
            std::vector<Vertex> vertices;
            std::vector<std::unordered_set<unsigned int>> adjacency_list;
            std::vector<Triangle> all_triangles;
            std::vector<std::set<unsigned int>> triangulations;
            unsigned int num_vertices;
            unsigned int add_vertex(double x, double y);
            inline std::unordered_set<unsigned int>& neighbhors(unsigned int vertex_id) {
                // We know that if the vertex id is a valid vertex, then it must be less than the
                // number of vertices we have 
                return this->adjacency_list[vertex_id];
            }
            unsigned int degree(unsigned int vertex_id);
            void add_directed_edge(unsigned int first_vertex, unsigned int second_vertex);
            void remove_directed_edge(unsigned int first_vertex, unsigned int second_vertex);
            void connect_vertices(unsigned int first_vertex, unsigned int second_vertex);
            RemovedVertexInfo remove_vertex(unsigned int vertex_id);
            std::vector<unsigned int> find_independant_set(); 
            std::vector<Triangle> get_triangulation(const std::vector<unsigned int>& polygon, std::vector<unsigned int>* hole);
            std::set<unsigned int> triangulate_polygon(const std::vector<unsigned int>& polygon, std::vector<unsigned int>* hole);
            std::set<unsigned int> triangulate_polygon(const std::vector<unsigned int>& polygon);
            void remove_vertices(std::vector<unsigned int> vertices, std::shared_ptr<DirectedAcyclicGraph> dag);
            bool triangles_intersect(unsigned int first, unsigned int second);
    };
    
    inline constexpr double ccw(const Vertex::Point& a, const Vertex::Point& b, const Vertex::Point& c) { 
        return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x); 
    };
    inline constexpr bool point_inside_triangle(const Vertex::Point& p, const Vertex::Point& p1, const Vertex::Point& p2, const Vertex::Point& p3) {
        return ((ccw(p1, p2, p) > 0) && (ccw(p2, p3, p) > 0) && (ccw(p3, p1, p) > 0)); 
    };
    inline constexpr bool sides_intersect(const Vertex::Point& a, const Vertex::Point& b, const Vertex::Point& c, const Vertex::Point& d) { 
        return ((ccw(a, b, c) > 0) ? (ccw(a, b, d) < 0) : (ccw(a, b, d) > 0)) && ((ccw(c, d, a) > 0) ? (ccw(c, d, b) < 0) : (ccw(c, d, b) > 0));
    };

    class GraphInfo {
        public:
            std::shared_ptr<PlanarGraph> planar_graph;
            std::shared_ptr<DirectedAcyclicGraph> directed_graph;
            std::map<std::tuple<unsigned int, unsigned int, unsigned int>, unsigned int> triangle_map;
            void process();
            void write_to_file(std::string base_filename);
            int locate_point(Vertex::Point point);
            inline bool triangle_contains_point(const Vertex::Point& p, const Triangle& tri) {
                return point_inside_triangle(p, this->planar_graph->vertices[tri.vertex_one].point, this->planar_graph->vertices[tri.vertex_two].point, this->planar_graph->vertices[tri.vertex_three].point);
            };
            void map_triangles(std::shared_ptr<triangulateio> others);
    };

    inline GraphInfo create_graph(std::shared_ptr<triangulateio> input) {
        return GraphInfo(std::shared_ptr<PlanarGraph>(new PlanarGraph(input)), std::shared_ptr<DirectedAcyclicGraph>(new DirectedAcyclicGraph()));
    };
    inline constexpr void sort(unsigned int& a, unsigned int& b, unsigned int& c) {
        if (a < b) {
            a ^= b;
            b ^= a;
            a ^= b;
        }
        if (b < c) {
            b ^= c;
            c ^= b;
            b ^= c;
        }
        if (a < b) {
            a ^= b;
            b ^= a;
            a ^= b;
        }
    }
}

namespace std {
    inline bool operator==(const PointLocation::Point& first, const PointLocation::Point& second) {
        return first.hash == second.hash;
    }
    inline bool operator==(const PointLocation::Line& first, const PointLocation::Line& second) {
        return first.hash == second.hash;
    }
}

#endif /* POINT_LOCATION_HPP_ */