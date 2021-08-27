#include "ShapeManipulator.hpp"
<<<<<<< HEAD
#include "TriangleManipulator.hpp"
=======
#include <cstring>
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
template class std::vector<std::vector<PointLocation::Line>*>;

namespace ShapeManipulator {
    unsigned int hash(std::pair<short, short> pair) {
        return static_cast<unsigned int>(pair.first) << 16 | static_cast<unsigned short int>(pair.second);;
    }
<<<<<<< HEAD
    std::shared_ptr<std::vector<double>> find_points_inside(const std::vector<PointLocation::Line>& object) {
=======
    std::vector<double>* find_points_inside(const std::vector<PointLocation::Line>& object) {
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
        std::shared_ptr<triangulateio> triangle_object = TriangleManipulator::create_instance();
        from_list(object, triangle_object);
        return find_points_inside(triangle_object);
    }
<<<<<<< HEAD
    std::shared_ptr<std::vector<double>> find_points_inside(std::shared_ptr<triangulateio> triangle_object) {
        std::shared_ptr<triangulateio> input = std::shared_ptr<triangulateio>(new triangulateio(*triangle_object));
        std::shared_ptr<std::vector<double>> holes = std::shared_ptr<std::vector<double>>(new std::vector<double>());
        std::shared_ptr<triangulateio> triangle_vobject = TriangleManipulator::create_instance();
        std::shared_ptr<triangulateio> _output = TriangleManipulator::create_instance();
        
        triangulate("pvzjiPEQDBNq", input, _output, triangle_vobject);
        int iteration = 0;
        do {
            if (triangle_vobject->numberofpoints < 1) {
                break;
            }
            iteration++;
            double x = triangle_vobject->pointlist.get()[0];
            double y = triangle_vobject->pointlist.get()[1];
            holes->reserve(holes->size() + 2);
            holes->push_back(x);
            holes->push_back(y);
            input->numberofholes = iteration;
            input->holelist = std::shared_ptr<double>(holes->data(), [](void*) {});
            triangulate("pvzjPEQDBNq", input, _output, triangle_vobject);
=======
    std::vector<double>* find_points_inside(std::shared_ptr<triangulateio> triangle_object) {
        std::shared_ptr<triangulateio> input = std::shared_ptr<triangulateio>(new triangulateio(*triangle_object));
        std::vector<double>* holes = new std::vector<double>();
        std::shared_ptr<triangulateio> triangle_vobject = TriangleManipulator::create_instance();
        std::shared_ptr<triangulateio> _output = TriangleManipulator::create_instance();
        triangulate("pvzjPEQD", input.get(), _output.get(), triangle_vobject.get());
        int iteration = 0;
        do {
            iteration++;
            double x = triangle_vobject->pointlist[0];
            double y = triangle_vobject->pointlist[1];
            holes->push_back(x);
            holes->push_back(y);
            input->numberofholes = iteration;
            input->holelist = holes->data();
            triangulate("pvzjPEQD", input.get(), _output.get(), triangle_vobject.get());
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
        } while(triangle_vobject->numberofpoints != 0);
        return holes;
    }
    void from_list(const std::vector<PointLocation::Line>& list, std::shared_ptr<triangulateio> output) {
        std::unordered_map<unsigned int, unsigned short> points = std::unordered_map<unsigned int, unsigned short>();
        output->numberofsegments = list.size();
        output->segmentlist = trimalloc<int>(output->numberofsegments * 2);
        output->segmentmarkerlist = trimalloc<int>(output->numberofsegments);
        int point_count = 0;
        std::unordered_set<unsigned int> points_set = std::unordered_set<unsigned int>();
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
        for (const PointLocation::Line& line : list) {
            const PointLocation::Point& first = line.first;
            const PointLocation::Point& second = line.second;
            std::unordered_map<unsigned int, unsigned short>::iterator first_find = points.find(first.hash);
            if(first_find == points.end()) {
<<<<<<< HEAD
                output_point_ptr[point_index * 2] = first.x;
                output_point_ptr[point_index * 2 + 1] = first.y;
                output_point_marker_ptr[point_index] = 1;
=======
                output->pointlist[point_index * 2] = first.x;
                output->pointlist[point_index * 2 + 1] = first.y;
                output->pointmarkerlist[point_index] = 1;
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
                first_find = points.emplace(first.hash, point_index++).first;
            }
            std::unordered_map<unsigned int, unsigned short>::iterator second_find = points.find(second.hash);
            if(second_find == points.end()) {
<<<<<<< HEAD
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
=======
                output->pointlist[point_index * 2] = second.x;
                output->pointlist[point_index * 2 + 1] = second.y;
                output->pointmarkerlist[point_index] = 1;
                second_find = points.emplace(second.hash, point_index++).first;
            }
            output->segmentlist[segment_index * 2] = first_find->second;
            output->segmentlist[segment_index * 2 + 1] = second_find->second;
            output->segmentmarkerlist[segment_index] = 1;
            segment_index++;
        }
    }
    void from_list(const std::vector<std::vector<PointLocation::Line>*>& list_of_lists, std::shared_ptr<triangulateio> output) {
        std::unordered_map<unsigned int, unsigned short> points = std::unordered_map<unsigned int, unsigned short>();
        int num_segments = 0;
        for(const std::vector<PointLocation::Line>* list : list_of_lists) {
            num_segments += list->size();
        }
        output->numberofsegments = num_segments;
        output->segmentlist = (int *) malloc(num_segments * 2 * sizeof(int));
        output->segmentmarkerlist = (int *) malloc(num_segments * sizeof(int));

        int point_count = 0;
        std::unordered_set<unsigned int> points_set = std::unordered_set<unsigned int>();
        
        for(std::vector<PointLocation::Line>* list : list_of_lists) {
            for(std::vector<PointLocation::Line>::iterator iter = list->begin(); iter != list->end(); iter++) {
                if(!points_set.contains(iter->first.hash)) {
                    point_count++;
                    points_set.emplace(iter->first.hash);
                }
                if(!points_set.contains(iter->second.hash)) {
                    point_count++;
                    points_set.emplace(iter->second.hash);
                }
            };
        }
        points_set.clear();
        output->numberofpoints = point_count;
        output->pointlist = (double *) malloc(point_count * 2 * sizeof(double));
        output->pointmarkerlist = (int *) malloc(point_count * sizeof(int));
        unsigned short point_index = 0;
        int segment_index = 0;
        for(std::vector<PointLocation::Line>* list : list_of_lists) {
            for(std::vector<PointLocation::Line>::iterator iter = list->begin(); iter != list->end(); iter++) {
                const PointLocation::Point first = iter->first;
                const PointLocation::Point second = iter->second;
                std::unordered_map<unsigned int, unsigned short>::iterator first_find = points.find(first.hash);
                if(first_find == points.end()) {
                    output->pointlist[point_index * 2] = first.x;
                    output->pointlist[point_index * 2 + 1] = first.y;
                    output->pointmarkerlist[point_index] = 1;
                    first_find = points.emplace(first.hash, point_index++).first;
                }
                std::unordered_map<unsigned int, unsigned short>::iterator second_find = points.find(second.hash);
                if(second_find == points.end()) {
                    output->pointlist[point_index * 2] = second.x;
                    output->pointlist[point_index * 2 + 1] = second.y;
                    output->pointmarkerlist[point_index] = 1;
                    second_find = points.emplace(second.hash, point_index++).first;
                }
                output->segmentlist[segment_index * 2] = first_find->second;
                output->segmentlist[segment_index * 2 + 1] = second_find->second;
                output->segmentmarkerlist[segment_index] = 1;
                segment_index++;
            }
        }
    }
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
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