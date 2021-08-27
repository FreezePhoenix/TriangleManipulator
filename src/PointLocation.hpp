#pragma once

#ifndef POINT_LOCATION_HPP_
#define POINT_LOCATION_HPP_

#include "../triangle/triangle.h"
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
<<<<<<< HEAD
        struct {
            short x1;
            short y1;
            short x2;
            short y2;
        };
=======
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
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
    };
    typedef double double64x2_t __attribute__ ((__vector_size__ (16)));
    struct Vertex {
        unsigned int id;
        struct Point {
            double x;
            double y;
        };
<<<<<<< HEAD
        union {
            Point point;
            double64x2_t matrix;
        };
=======
        Point point;
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
        Vertex(double x, double y);
        std::set<unsigned int> triangles;
        bool removed = false;
        void add_triangle(unsigned int triangle_id);
        void remove_triangle(unsigned int triangle_id);
    };

    struct RemovedVertexInfo {
        std::set<unsigned int> old_triangle_ids;
        // Array of point ID's
        std::vector<unsigned int> polygon;
    };

    class DirectedAcyclicGraph {
        private:
            unsigned int _root;
<<<<<<< HEAD
            std::unordered_map<unsigned int, std::unordered_set<unsigned int>> graph;
        public:
            DirectedAcyclicGraph();
            void add_directed_edge(unsigned int first, unsigned int second);
            std::unordered_set<unsigned int>& neighbhors(unsigned int n);
            unsigned int root();
            void write_to_file(std::string base_name);
=======
            std::unordered_map<unsigned int, std::unordered_set<unsigned int>*> graph;
        public:
            DirectedAcyclicGraph();
            void add_directed_edge(unsigned int first, unsigned int second);
            std::unordered_set<unsigned int>* neighbhors(unsigned int n);
            unsigned int root();
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
    };

    class PlanarGraph {
        public:
            PlanarGraph();
            PlanarGraph(std::shared_ptr<triangulateio> input);
            std::vector<Vertex> vertices;
            std::vector<std::unordered_set<unsigned int>> adjacency_list;
            std::vector<Triangle> all_triangles;
            std::vector<std::unordered_set<unsigned int>> triangulations;
            unsigned int num_vertices;
            unsigned int add_vertex(double x, double y);
            std::unordered_set<unsigned int>* neighbhors(unsigned int vertex_id);
            unsigned int degree(unsigned int vertex_id);
            void add_directed_edge(unsigned int first_vertex, unsigned int second_vertex);
            void remove_directed_edge(unsigned int first_vertex, unsigned int second_vertex);
            void connect_vertices(unsigned int first_vertex, unsigned int second_vertex);
            RemovedVertexInfo remove_vertex(unsigned int vertex_id);
            std::vector<unsigned int> find_independant_set(); 
            std::vector<Triangle> get_triangulation(const std::vector<unsigned int>& polygon, std::vector<unsigned int>* hole);
            std::unordered_set<unsigned int> triangulate_polygon(const std::vector<unsigned int>& polygon, std::vector<unsigned int>* hole);
            std::unordered_set<unsigned int> triangulate_polygon(const std::vector<unsigned int>& polygon);
<<<<<<< HEAD
            void remove_vertices(std::vector<unsigned int> vertices, std::shared_ptr<DirectedAcyclicGraph> dag);
            bool triangles_intersect(unsigned int first, unsigned int second);
            void write_to_file(std::string base_name);
    };

    class GraphInfo {
        public:
            std::shared_ptr<PlanarGraph> planar_graph;
            std::shared_ptr<DirectedAcyclicGraph> directed_graph;
            void process();
    };

    inline GraphInfo create_graph(std::shared_ptr<triangulateio> input) {
        return GraphInfo(std::shared_ptr<PlanarGraph>(new PlanarGraph(input)), std::shared_ptr<DirectedAcyclicGraph>(new DirectedAcyclicGraph()));
    };
    
    inline constexpr double ccw(const Vertex::Point& a, const Vertex::Point& b, const Vertex::Point& c) { 
        return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x); 
    };

=======
            void remove_vertices(std::vector<unsigned int> vertices, DirectedAcyclicGraph* dag);
            bool triangles_intersect(unsigned int first, unsigned int second);
    };

    struct GraphInfo {
        PlanarGraph* planar_graph;
        DirectedAcyclicGraph* directed_graph;        
    };

    inline GraphInfo create_graph(std::shared_ptr<triangulateio> input) {
        PlanarGraph* graph = new PlanarGraph(input);
        DirectedAcyclicGraph* dag = new DirectedAcyclicGraph();

        return GraphInfo(graph, dag);
    };
    
    inline constexpr double ccw(const Vertex::Point& a, const Vertex::Point& b, const Vertex::Point& c) { 
        return (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y); 
    };

>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
    inline constexpr bool triangle_contains_point(const Vertex::Point& p, const Vertex::Point& p1, const Vertex::Point& p2, const Vertex::Point& p3) {
        return ((ccw(p1, p2, p) > 0) && (ccw(p2, p3, p) > 0) && (ccw(p3, p1, p) > 0)); 
    }; 
    inline constexpr bool sides_intersect(const Vertex::Point& a, const Vertex::Point& b, const Vertex::Point& c, const Vertex::Point& d) { 
        return ((ccw(a, b, c) > 0) ? (ccw(a, b, d) < 0) : (ccw(a, b, d) > 0)) && ((ccw(c, d, a) > 0) ? (ccw(c, d, b) < 0) : (ccw(c, d, b) > 0));
<<<<<<< HEAD
    };
    void write_to_file(std::string base_name, PlanarGraph& pgraph, DirectedAcyclicGraph& dagraph);
=======
    }; 
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
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