#pragma once

#ifndef POINT_LOCATION_HPP_
#define POINT_LOCATION_HPP_

#include "TriangleManipulator.hpp"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>

namespace PointLocation {
    struct Point {
        short x;
        short y;
        unsigned int hash;
        Point() {
            this->x = this->y = 0;
            this->hash = 0;
        }
        Point(short x, short y) {
            this->x = x;
            this->y = y;
            this->hash = ((unsigned int) x << 16) + (unsigned int) y;
        };
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
    struct Line {
        Point first;
        Point second;
        unsigned long int hash;
        Line() {
            this->first = Point(0, 0);
            this->second = Point(0, 0);
            this->hash = 0;
        }
        Line(Point& first, Point& second) {
            if(first.x < second.x || first.y < second.y) {
                this->first = first;
                this->second = second;
            } else {
                this->first = second;
                this->second = first;
            }
            this->hash = ((unsigned long int) this->first.hash << 32) + (unsigned long int) this->second.hash;
        }
        Line(const Point& first, const Point& second) {
            if(first.x < second.x || first.y < second.y) {
                this->first = first;
                this->second = second;
            } else {
                this->first = second;
                this->second = first;
            }
            this->hash = ((unsigned long int) this->first.hash << 32) + (unsigned long int) this->second.hash;
        }
        bool operator==(const Line& other) {
            return other.hash == this->hash;
        }
        Line(short x1, short x2, short y1, short y2): Line(Point(x1, y1), Point(x2, y2)) {}
    };
    struct LineHash {
        inline unsigned long int operator () (const Line &l) const {
            return l.hash;
        }
        inline std::size_t operator () (const Line &p, const Line &p2) const {
            return p.hash - p2.hash;
        }
    };

    struct Triangle {
        unsigned int vertex_one = (unsigned int) -1;
        unsigned int vertex_two = (unsigned int) -1;
        unsigned int vertex_three = (unsigned int) -1;
    };

    struct Vertex {
        unsigned int id;
        struct Point {
            double x;
            double y;
        } point;
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

    class PlanarGraph {
        public:
            PlanarGraph();
            PlanarGraph(triangulateio* input);
            std::vector<Vertex> vertices;
            std::vector<std::unordered_set<unsigned int>*> adjacency_list;
            std::vector<Triangle> all_triangles;
            std::vector<std::vector<unsigned int>*> triangulations;
            unsigned int num_vertices;
            unsigned int add_vertex(double x, double y);
            std::unordered_set<unsigned int>* neighbhors(unsigned int vertex_id);
            unsigned int degree(unsigned int vertex_id);
            void add_directed_edge(unsigned int first_vertex, unsigned int second_vertex);
            void remove_directed_edge(unsigned int first_vertex, unsigned int second_vertex);
            void connect_vertices(unsigned int first_vertex, unsigned int second_vertex);
            RemovedVertexInfo remove_vertex(unsigned int vertex_id);
    };

    class DirectedAcyclicGraph {
        private:
            unsigned int _root;
            std::unordered_map<unsigned int, std::set<unsigned int>*> graph;
        public:
            DirectedAcyclicGraph();
            void add_directed_edge(unsigned int first, unsigned int second);
            const std::set<unsigned int> neighbhors(unsigned int n);
            unsigned int root();
    };

    std::vector<Triangle> get_triangulation(PlanarGraph& graph, std::vector<unsigned int>* polygon, std::vector<unsigned int>* hole);
    std::vector<unsigned int> triangulate(PlanarGraph& graph, std::vector<unsigned int>* polygon, std::vector<unsigned int>* hole);
    std::vector<unsigned int> triangulate(PlanarGraph& graph, std::vector<unsigned int>* polygon);
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