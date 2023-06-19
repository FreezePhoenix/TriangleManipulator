#include "TriangleManipulator/ShapeManipulator.hpp"
#include "TriangleManipulator/TriangleManipulator.hpp"
#include <iostream>
template class std::vector<std::vector<PointLocation::Line>*>;

namespace ShapeManipulator {
    unsigned int hash(std::pair<short, short> pair) {
        return static_cast<unsigned int>(pair.first) << 16 | static_cast<unsigned short int>(pair.second);;
    }
    std::shared_ptr<std::vector<double>> find_points_inside(const std::vector<PointLocation::Line>& object) {
        std::shared_ptr<triangulateio> triangle_object = TriangleManipulator::create_instance();
        from_list(object, triangle_object);
        return find_points_inside(triangle_object);
    }
    std::shared_ptr<std::vector<double>> find_points_inside(std::shared_ptr<triangulateio> triangle_object) {
        std::shared_ptr<triangulateio> input = std::shared_ptr<triangulateio>(new triangulateio(*triangle_object));
        std::shared_ptr<std::vector<double>> holes = std::shared_ptr<std::vector<double>>(new std::vector<double>());
        std::shared_ptr<std::vector<double>> new_holes = std::shared_ptr<std::vector<double>>(new std::vector<double>());
        std::shared_ptr<triangulateio> triangle_vobject = TriangleManipulator::create_instance();
        std::shared_ptr<triangulateio> _output = TriangleManipulator::create_instance();
        double* holeptr = triangle_object->holelist.get();
        int starting_holes = triangle_object->numberofholes;
        for (size_t i = 0; i < triangle_object->numberofholes; i++) {
            holes->push_back(holeptr[i * 2]);
            holes->push_back(holeptr[i * 2 + 1]);
        }
        triangulate("pvzXEQD", input, _output, triangle_vobject);
        int iteration = 0;
        do {
            if (triangle_vobject->numberofpoints < 1) {
                break;
            }
            
            iteration++;
            double x = triangle_vobject->pointlist.get()[0];
            double y = triangle_vobject->pointlist.get()[1];
            holes->reserve(holes->size() + 2);
            new_holes->reserve(new_holes->size() + 2);
            holes->push_back(x);
            holes->push_back(y);
            new_holes->push_back(x);
            new_holes->push_back(y);
            input = _output;
            input->numberofholes = iteration + starting_holes;
            input->holelist = std::shared_ptr<double[]>(holes->data(), [](void*) {});
            triangulate("pvzXEQD", input, _output, triangle_vobject);
            TriangleManipulator::write_poly_file("Maps/abtesting.object.1." + std::to_string(iteration) + ".poly", input);
        } while(triangle_vobject->numberofpoints != 0);
        return new_holes;
    }
    void from_list(const std::vector<PointLocation::Line>& list, std::shared_ptr<triangulateio> output) {
        std::unordered_map<unsigned int, unsigned short> points = std::unordered_map<unsigned int, unsigned short>();
        output->numberofsegments = list.size();
        output->segmentlist = trimalloc<int>(output->numberofsegments * 2);
        output->segmentmarkerlist = trimalloc<int>(output->numberofsegments);
        int point_count = 0;
        std::unordered_set<int> points_set = std::unordered_set<int>();
        points_set.reserve(list.size() * 2);
        for(const PointLocation::Line& line : list) {
            points_set.insert(line.first.hash).second && point_count++;
            points_set.insert(line.second.hash).second && point_count++;
        }
        points_set.clear();
        output->numberofpoints = point_count;
        output->pointlist = trimalloc<REAL>(point_count * 2);
        output->pointmarkerlist = trimalloc<int>(point_count);
        unsigned short point_index = 0;
        int segment_index = 0;
        REAL* output_point_ptr = output->pointlist.get();
        int* output_point_marker_ptr = output->pointmarkerlist.get();
        int* output_segment_ptr = output->segmentlist.get();
        int* output_segment_marker_ptr = output->segmentmarkerlist.get();
        std::memset(output_point_marker_ptr, 1, sizeof(int) * point_count);
        std::memset(output_segment_marker_ptr, 1, sizeof(int) * point_count);
        for (const PointLocation::Line& line : list) {
            const PointLocation::Point& first = line.first;
            const PointLocation::Point& second = line.second;
            std::unordered_map<unsigned int, unsigned short>::iterator first_find = points.find(first.hash);
            if(first_find == points.end()) {
                output_point_ptr[point_index * 2] = first.x;
                output_point_ptr[point_index * 2 + 1] = first.y;
                first_find = points.emplace(first.hash, point_index++).first;
            }
            std::unordered_map<unsigned int, unsigned short>::iterator second_find = points.find(second.hash);
            if(second_find == points.end()) {
                output_point_ptr[point_index * 2] = second.x;
                output_point_ptr[point_index * 2 + 1] = second.y;
                second_find = points.emplace(second.hash, point_index++).first;
            }
            output_segment_ptr[segment_index * 2] = first_find->second;
            output_segment_ptr[segment_index * 2 + 1] = second_find->second;
            segment_index++;
        }
    }
    void from_list(const std::vector<std::shared_ptr<std::vector<PointLocation::Line>>>& list_of_lists, std::shared_ptr<triangulateio> output) {
        std::unordered_map<unsigned int, unsigned short> points = std::unordered_map<unsigned int, unsigned short>();
        int num_segments = 0;
        for(const std::shared_ptr<std::vector<PointLocation::Line>>& list : list_of_lists) {
            num_segments += list->size();
        }
        output->numberofsegments = num_segments;
        output->segmentlist = trimalloc<int>(num_segments * 2);
        output->segmentmarkerlist = trimalloc<int>(num_segments);

        int point_count = 0;
        std::unordered_set<unsigned int> points_set = std::unordered_set<unsigned int>();
        
        for(const std::shared_ptr<std::vector<PointLocation::Line>>& list : list_of_lists) {
            for(const PointLocation::Line& line : *list) {
                points_set.insert(line.first.hash).second && point_count++;
                points_set.insert(line.second.hash).second && point_count++;
            }
        }
        points_set.clear();
        output->numberofpoints = point_count;
        output->pointlist = trimalloc<REAL>(point_count * 2);
        output->pointmarkerlist = trimalloc<int>(point_count);
        unsigned short point_index = 0;
        int segment_index = 0;
        REAL* output_point_ptr = output->pointlist.get();
        int* output_point_marker_ptr = output->pointmarkerlist.get();
        int* output_segment_ptr = output->segmentlist.get();
        int* output_segment_marker_ptr = output->segmentmarkerlist.get();
        for(const std::shared_ptr<std::vector<PointLocation::Line>>& list : list_of_lists) {
            for(const PointLocation::Line& line : *list) {
                const PointLocation::Point& first = line.first;
                const PointLocation::Point& second = line.second;
                std::unordered_map<unsigned int, unsigned short>::iterator first_find = points.find(first.hash);
                if(first_find == points.end()) {
                    output_point_ptr[point_index * 2] = first.x;
                    output_point_ptr[point_index * 2 + 1] = first.y;
                    output_point_marker_ptr[point_index] = 1;
                    first_find = points.emplace(first.hash, point_index++).first;
                }
                std::unordered_map<unsigned int, unsigned short>::iterator second_find = points.find(second.hash);
                if(second_find == points.end()) {
                    output_point_ptr[point_index * 2] = second.x;
                    output_point_ptr[point_index * 2 + 1] = second.y;
                    output_point_marker_ptr[point_index] = 1;
                    second_find = points.emplace(second.hash, point_index++).first;
                }
                output_segment_ptr[segment_index * 2] = first_find->second;
                output_segment_ptr[segment_index * 2 + 1] = second_find->second;
                output_segment_marker_ptr[segment_index] = 1;
                segment_index++;
            }
        }
    }
}