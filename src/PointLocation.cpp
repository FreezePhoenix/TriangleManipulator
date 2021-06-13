#include "PointLocation.hpp"
#include "ShapeManipulator.hpp"
#include <map>
#include <math.h>

// template class std::vector<PointLocation::Point>;
// template class std::vector<PointLocation::Line>;

namespace PointLocation {
    DirectedAcyclicGraph::DirectedAcyclicGraph() {
        this->_root = 0;
        this->graph = std::unordered_map<unsigned int, std::set<unsigned int>*>();
    };

    void DirectedAcyclicGraph::add_directed_edge(unsigned int first, unsigned int second) {
        if(this->graph.find(first) == this->graph.end()) {
            this->graph[first] = new std::set<unsigned int>();
        }
        this->graph[first]->insert(second);
    }

    Vertex::Vertex(double x, double y) {
        this->id = static_cast<unsigned int>(-1); // Caution advised, 
        this->removed = false;
        this->point = Vertex::Point(x, y);
        this->triangles = std::unordered_set<unsigned int>();
    }

    void Vertex::add_triangle(unsigned int triangle_id) {
        this->triangles.insert(triangle_id);
    }

    void Vertex::remove_triangle(unsigned int triangle_id) {
        this->triangles.erase(triangle_id);
    }

    PlanarGraph::PlanarGraph() {
        this->vertices = std::vector<Vertex>();
        this->all_triangles = std::vector<Triangle>();
        this->adjacency_list = std::vector<std::unordered_set<unsigned int>*>();
        this->triangulations = std::vector<std::vector<unsigned int>*>();
        this->num_vertices = 0;
    }

    PlanarGraph::PlanarGraph(triangulateio* graph): PlanarGraph() {
        for(int i = 0; i < graph->numberofpoints; i++) {
            this->add_vertex(graph->pointlist[i * 2], graph->pointlist[i * 2 + 1]);
        }
        for(int i = 0; i < graph->numberofsegments; i++) {
            this->connect_vertices(graph->segmentlist[i * 2], graph->segmentlist[i * 2 + 1]);
        }
    }

    unsigned int PlanarGraph::add_vertex(double x, double y) {
        Vertex vert(x, y);
        vert.id = this->vertices.size();
        
        this->vertices.push_back(vert);
        this->adjacency_list.push_back(new std::unordered_set<unsigned int>());
        this->num_vertices++;
        
        return vert.id;
    }

    std::unordered_set<unsigned int>* PlanarGraph::neighbhors(unsigned int vertex_id) {
        // We know that if the vertex id is a valid vertex, then it must be less than the
        // number of vertices we have 
        if(this->num_vertices > vertex_id) {
            return this->adjacency_list[vertex_id];
        }
        return nullptr;
    }

    unsigned int PlanarGraph::degree(unsigned int vertex_id) {
        if(this->num_vertices > vertex_id) {
            return this->adjacency_list[vertex_id]->size();
        }
        return 0;
    }

    void PlanarGraph::add_directed_edge(unsigned int first_vertex, unsigned int second_vertex) {
        this->adjacency_list[first_vertex]->insert(second_vertex);
    }

    void PlanarGraph::remove_directed_edge(unsigned int first_vertex, unsigned int second_vertex) {
        this->adjacency_list[first_vertex]->erase(second_vertex);
    }

    void PlanarGraph::connect_vertices(unsigned int first_vertex, unsigned int second_vertex) {
        if(!this->adjacency_list[first_vertex]->contains(second_vertex) && !this->adjacency_list[second_vertex]->contains(first_vertex)) {
            this->add_directed_edge(first_vertex, second_vertex);
            this->add_directed_edge(second_vertex, first_vertex);
        }
    }
    RemovedVertexInfo PlanarGraph::remove_vertex(unsigned int vertex_id) {
        this->vertices[vertex_id].removed = true;
        this->num_vertices--;
        
        std::unordered_set<unsigned int> neighbhors = *(this->adjacency_list[vertex_id]);
        for(unsigned int neighbhor : neighbhors) {
            this->remove_directed_edge(vertex_id, neighbhor);
            this->remove_directed_edge(neighbhor, vertex_id);
        }
        std::unordered_set<unsigned int> old_triangle_ids = std::unordered_set(this->vertices[vertex_id].triangles);

        std::vector<unsigned int> polygon = std::vector<unsigned int>();

        std::set<std::pair<unsigned int, unsigned int>> triangles = std::set<std::pair<unsigned int, unsigned int>>();
        std::pair<unsigned int, unsigned int> first_triangle;
        bool first = true;
        for(unsigned int triangle_id : old_triangle_ids) {
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
            for(std::pair<unsigned int, unsigned int> tri : triangles) {
                if(tri.first == query || tri.second == query) {
                    // std::cout << " Triangle found! Query: " << query << " triangle: " << tri.first << ", " << tri.second << std::endl;
                    next = tri;
                    break;
                }
            }
            unsigned int old_query = query;
            if(next.first == query) {
                query = next.second;
            } else {
                query = next.first;
            }
            // std::cout << "Polygon: " << query << std::endl;
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

    std::vector<Triangle> get_triangulation(PlanarGraph& graph, std::vector<unsigned int>* polygon, std::vector<unsigned int>* hole) {
        // Using Triangle
        std::shared_ptr<triangulateio> instance = TriangleManipulator::create_instance();
        instance->numberofpoints = polygon->size();
        instance->pointlist = (double *) malloc(instance->numberofpoints * 2 * sizeof(double));
        std::map<unsigned int, unsigned int> point_to_mapping = std::map<unsigned int, unsigned int>();
        std::map<unsigned int, unsigned int> mapping_to_point = std::map<unsigned int, unsigned int>();
        instance->numberofsegments = polygon->size();
        instance->segmentlist = (int *) malloc(instance->numberofsegments * 2 * sizeof(int));
        instance->segmentmarkerlist = (int *) malloc(instance->numberofsegments * sizeof(int));
        int index = 0;
        for(unsigned int point : *polygon) {
            point_to_mapping.emplace(std::pair<unsigned int, unsigned int>(point, index));
            mapping_to_point.emplace(std::pair<unsigned int, unsigned int>(index, point));
            instance->pointlist[index * 2] = graph.vertices[point].point.x;
            instance->pointlist[index * 2 + 1] = graph.vertices[point].point.y;
            index++;
        }
        for(int i = 0; i < polygon->size(); i++) {
            unsigned int mapping1 = point_to_mapping[polygon->at(i)];
            unsigned int mapping2;
            if(i < polygon->size() - 1) {
                mapping2 = point_to_mapping[polygon->at(i + 1)];
            } else {
                mapping2 = point_to_mapping[polygon->at(0)];
            }
            instance->segmentlist[i * 2] = mapping1;
            instance->segmentlist[i * 2 + 1] = mapping2;
            instance->segmentmarkerlist[i] = 1;
        }
        if(hole != nullptr) {
            std::map<unsigned int, unsigned int> point_to_mapping2 = std::map<unsigned int, unsigned int>();
            std::map<unsigned int, unsigned int> mapping_to_point2 = std::map<unsigned int, unsigned int>();
            int index = 0;
            std::vector<Line> hole_boundary = std::vector<Line>();
            for(unsigned int point: *hole) {
                point_to_mapping2.emplace(std::pair<unsigned int, unsigned int>(point, index));
                mapping_to_point2.emplace(std::pair<unsigned int, unsigned int>(index, point));
                index++;
            }
            for(int i = 0; i < hole->size() - 1; i++) {
                Vertex first_point_double = graph.vertices[hole->at(i)];
                Vertex second_point_double = graph.vertices[hole->at(i + 1)];
                Point first_point = Point((unsigned int) std::round(first_point_double.point.x), (unsigned int) std::round(first_point_double.point.y));
                Point second_point = Point((unsigned int) std::round(second_point_double.point.x), (unsigned int) std::round(second_point_double.point.y));
                hole_boundary.push_back(Line(first_point, second_point));
            }
            std::shared_ptr<triangulateio> temporary = TriangleManipulator::create_instance();
            std::vector<std::pair<double, double>> holes = ShapeManipulator::find_points_inside(hole_boundary, temporary);
            instance->numberofholes = holes.size();
            instance->holelist = (double *) malloc(instance->numberofholes * 2 * sizeof(double));
            for(int i = 0; i < holes.size(); i++) {
                instance->holelist[i * 2] = holes[i].first;
                instance->holelist[i * 2 + 1] = holes[i].second;
            }
        }
        std::shared_ptr<triangulateio> output = TriangleManipulator::create_instance();
        std::vector<Triangle> result = std::vector<Triangle>();
        TriangleManipulator::write_poly_file("test2.poly", instance.get());
        triangulate("pzQ", instance.get(), output.get(), nullptr);
        for(int i = 0; i < output->numberoftriangles; i++) {
            unsigned int point_one = mapping_to_point[output->trianglelist[i * 3]];
            unsigned int point_two = mapping_to_point[output->trianglelist[i * 3 + 1]];
            unsigned int point_three = mapping_to_point[output->trianglelist[i * 3 + 2]];
            result.push_back(Triangle { point_one, point_two, point_three });
        }

        return result;
    }

    std::vector<unsigned int> triangulate(PlanarGraph& graph, std::vector<unsigned int>* polygon, std::vector<unsigned int>* hole) {
        std::vector<Triangle> triangles = get_triangulation(graph, polygon, hole);
        std::vector<unsigned int> new_triangle_ids = std::vector<unsigned int>();

        for(const Triangle& triangle : triangles) {
            graph.connect_vertices(triangle.vertex_one, triangle.vertex_two);
            graph.connect_vertices(triangle.vertex_two, triangle.vertex_three);
            graph.connect_vertices(triangle.vertex_three, triangle.vertex_one);

            unsigned int new_id = graph.all_triangles.size();
            new_triangle_ids.push_back(new_id);
            graph.all_triangles.push_back(triangle);

            graph.vertices[triangle.vertex_one].add_triangle(new_id); 
            graph.vertices[triangle.vertex_two].add_triangle(new_id); 
            graph.vertices[triangle.vertex_three].add_triangle(new_id); 
        };

        return new_triangle_ids;
    }

    std::vector<unsigned int> triangulate(PlanarGraph& graph, std::vector<unsigned int>* polygon) {
        return triangulate(graph, polygon, nullptr);
    }

    const std::set<unsigned int> DirectedAcyclicGraph::neighbhors(unsigned int n) {
        return *(this->graph.at(n));
    }

    unsigned int DirectedAcyclicGraph::root() {
        return this->_root;
    }
}