#pragma once

#ifndef POINT_LOCATION_HPP_
#define POINT_LOCATION_HPP_

#include <triangle.h>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include <memory>
#include <iostream>
#include <cmath>

namespace PointLocation {
    typedef double double64x2_t __attribute__((__vector_size__(16)));
    typedef long int64x2_t __attribute__((__vector_size__(16)));
    typedef short int16x2_t __attribute__((vector_size(4)));
    bool overlaps(short start1, short end1, short start2, short end2);
    bool overlaps(int16x2_t first, int16x2_t second);
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
        const bool operator==(const Point& other) {
            return other.hash == this->hash;
        }
        inline double angle(const Point& p2) {
            double xDiff = p2.x - x;
            double yDiff = p2.y - y;
            return std::atan2(yDiff, xDiff);
        }
        inline double distance(const Point& p2) {
            return std::sqrt(std::pow(p2.x - x, 2) + std::pow(p2.y - y, 2));
        }
    };
    
    union Line {
        typedef unsigned long int hash_t;
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
        hash_t hash;
        constexpr Line() : hash{0} {
            // this->hash = 0;
        }
        constexpr Line(Point& first, Point& second) {
            if (first.x < second.x || first.y < second.y) {
                this->first = first;
                this->second = second;
            } else {
                this->first = second;
                this->second = first;
            }
        }
        constexpr Line(const Point& first, const Point& second) {
            if (first.x < second.x || first.y < second.y) {
                this->first = first;
                this->second = second;
            } else {
                this->first = second;
                this->second = first;
            }
        }
        constexpr Line(short x1, short y1, short x2, short y2) : Line{Point {x1, y1}, Point{x2, y2}} {
        }
        constexpr bool operator==(const Line& other) {
            return other.hash == this->hash;
        }
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
            unsigned int vertex_two;
            unsigned int vertex_three;
            unsigned int id;
        };
        unsigned int vertices[4];
        constexpr Triangle(unsigned int a, unsigned int b, unsigned int c, unsigned int id) : vertices{ a, b, c, id} {}
        constexpr Triangle() : vertices{ 0, 0, 0, 0 } {}
        inline bool operator==(const Triangle& rhs) {
            return this->id == rhs.id;
        }
    };
    struct Vertex {
        std::vector<unsigned int> triangles;
        std::vector<unsigned int> neighs;
        struct Point {
            double x;
            double y;
        };
        union {
            Point point;
            double64x2_t matrix;
        };
        size_t degree() const {
            return neighs.size();
        }
        unsigned int id;
        bool removed;
        bool forbidden;
        inline Vertex() : triangles(), neighs(), point{ 0, 0 }, id(-1), removed(false), forbidden(false) {
        }
        inline Vertex(double x, double y) : triangles(), neighs(), point{ x, y }, id(-1), removed(false), forbidden(false) {}
        inline Vertex(double x, double y, unsigned int id) : triangles(), neighs(), point{ x, y }, id(id), removed(false), forbidden(false) {}
        inline Vertex(double x, double y, unsigned int id, bool removed, bool forbidden) : triangles(), neighs(), point{ x, y }, id(id), removed(removed), forbidden(forbidden) {
        }
        
        inline void add_triangle(unsigned int triangle_id) {
            this->triangles.emplace_back(triangle_id);
        }

        inline void remove_triangle(unsigned int triangle_id) {
            std::erase(this->triangles, triangle_id);
        }
    };
    struct RemovedVertexInfo {
        std::vector<unsigned int> old_triangle_ids;
        // Array of point ID's
        std::vector<unsigned int> polygon;
        const Vertex& removed;
        RemovedVertexInfo(const Vertex& removed) : old_triangle_ids(), polygon(), removed(removed) {
            
        }
    };

    class DirectedAcyclicGraph {
        public:
            unsigned int root;
            std::map<unsigned int, std::vector<unsigned int>> graph;
            DirectedAcyclicGraph();
            void add_directed_edge(unsigned int first, unsigned int second);
            const std::vector<unsigned int>& neighbhors(unsigned int n);
            void write_to_file(std::string base_name);
    };

    class PlanarGraph {
        public:
            PlanarGraph();
            PlanarGraph(std::shared_ptr<triangulateio> input);
            std::vector<Vertex> vertices;
            // std::vector<std::set<unsigned int>> adjacency_list;
            std::vector<Triangle> all_triangles;
            std::vector<std::unordered_set<unsigned int>> triangulations;
            unsigned int num_vertices;
            unsigned int add_vertex(double x, double y);
            void add_directed_edge(unsigned int first_vertex, unsigned int second_vertex);
            void remove_directed_edge(unsigned int first_vertex, unsigned int second_vertex);
            void connect_vertices(unsigned int first_vertex, unsigned int second_vertex);
            RemovedVertexInfo remove_vertex(unsigned int vertex_id);
            std::vector<unsigned int> find_independant_set(); 
            std::vector<Triangle> get_triangulation(const std::vector<unsigned int>& polygon);
            std::vector<unsigned int> triangulate_polygon(const std::vector<unsigned int>& polygon);
            void remove_vertices(const std::vector<unsigned int>& vertices, std::shared_ptr<DirectedAcyclicGraph> dag);
            bool triangles_intersect(unsigned int first, unsigned int second);
    };
    
    inline constexpr double ccw(const Vertex::Point& a, const Vertex::Point& b, const Vertex::Point& c) { 
        return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x); 
    };
    inline constexpr bool point_inside_triangle(const Vertex::Point& p, const Vertex::Point& p1, const Vertex::Point& p2, const Vertex::Point& p3) {
        return (((ccw(p1, p2, p) > 0) && (ccw(p2, p3, p) > 0) && (ccw(p3, p1, p) > 0))) || (((ccw(p1, p2, p) < 0) && (ccw(p2, p3, p) < 0) && (ccw(p3, p1, p) < 0))); 
    };
    inline constexpr bool sides_intersect(const Vertex::Point& a, const Vertex::Point& b, const Vertex::Point& c, const Vertex::Point& d) {
        return ((ccw(a, b, c) > 0) ? (ccw(a, b, d) < 0) : (ccw(a, b, d) > 0)) && ((ccw(c, d, a) > 0) ? (ccw(c, d, b) < 0) : (ccw(c, d, b) > 0));
    };
    inline constexpr double ccw(const double64x2_t a, const double64x2_t b, const double64x2_t c) {
        double64x2_t a2 = __builtin_shuffle(a, int64x2_t{ 1, 0 });
        double64x2_t c2 = __builtin_shuffle(c, int64x2_t{ 1, 0 });
        double64x2_t d = (b - a) * (c2 - a2);
        return d[0] - d[1];
    };
    inline constexpr bool point_inside_triangle(const double64x2_t& p, const double64x2_t& p1, const double64x2_t& p2, const double64x2_t& p3) {
        return ((ccw(p1, p2, p) > 0) && (ccw(p2, p3, p) > 0) && (ccw(p3, p1, p) > 0));
    };
    inline constexpr bool sides_intersect(const double64x2_t a, const double64x2_t b, const double64x2_t c, const double64x2_t d) {
        
        return ((ccw(a, b, c) == 0) ? true : ((ccw(a, b, c) > 0) ? (ccw(a, b, d) < 0) : (ccw(a, b, d) > 0))) && ((ccw(c, d, a) == 0) ? true : ((ccw(c, d, a) > 0) ? (ccw(c, d, b) < 0) : (ccw(c, d, b) > 0)));
    };
    class GraphInfo {
        public:
            std::shared_ptr<PlanarGraph> planar_graph;
            std::shared_ptr<DirectedAcyclicGraph> directed_graph;
            std::map<std::tuple<unsigned int, unsigned int, unsigned int>, unsigned int> triangle_map;
            GraphInfo() : planar_graph(new PlanarGraph()), directed_graph(new DirectedAcyclicGraph()), triangle_map() {};
            GraphInfo(std::shared_ptr<triangulateio> input) : planar_graph(new PlanarGraph(input)), directed_graph(new DirectedAcyclicGraph()), triangle_map() {};
            void process();
            int locate_point(Vertex::Point point);
            inline bool triangle_contains_point(const Vertex::Point& p, const Triangle& tri) {
                return point_inside_triangle(p, this->planar_graph->vertices[tri.vertex_one].point, this->planar_graph->vertices[tri.vertex_two].point, this->planar_graph->vertices[tri.vertex_three].point);
            };
            void map_triangles(std::shared_ptr<triangulateio> others);
            void write_to_file(std::string base_filename);
            void write_to_binary_file(std::string filename);
            void read_from_binary_file(std::string filename);
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
    template<>
    struct hash<PointLocation::Point> {
        inline unsigned int operator () (const PointLocation::Point &p) const {
            return p.hash;
        }
        inline std::size_t operator () (const PointLocation::Point &p, const PointLocation::Point &p2) const {
            return p.hash - p2.hash;
        }
    };
    inline bool operator==(const PointLocation::Line& first, const PointLocation::Line& second) {
        return first.hash == second.hash;
    }
}

#endif /* POINT_LOCATION_HPP_ */