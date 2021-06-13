#include "ShapeManipulator.hpp"

template class std::vector<std::vector<PointLocation::Line>*>;

namespace ShapeManipulator {
    unsigned int hash(std::pair<short, short> pair) {
        return static_cast<unsigned int>(pair.first) << 16 | static_cast<unsigned short int>(pair.second);;
    }
    std::vector<std::pair<double, double>> find_points_inside(const std::vector<PointLocation::Line>& object, std::shared_ptr<triangulateio> triangle_object) {
        std::vector<std::pair<double, double>> holes = std::vector<std::pair<double, double>>();
        from_list(object, triangle_object);
        std::shared_ptr<triangulateio> triangle_vobject = TriangleManipulator::create_instance();
        std::shared_ptr<triangulateio> _output = TriangleManipulator::create_instance();
        triangulate("pzvPQD", triangle_object.get(), _output.get(), triangle_vobject.get());
        int interation = 0;
        do {
            double x = triangle_vobject->pointlist[0];
            double y = triangle_vobject->pointlist[1];
            holes.push_back(std::pair<double, double>(x, y));
            triangle_object->numberofholes = holes.size();
            free(triangle_object->holelist);
            triangle_object->holelist = (double *) malloc(holes.size() * 2 * sizeof(double));
            int hole_index = 0;
            for(const std::pair<double, double>& hole : holes) {
                triangle_object->holelist[hole_index * 2] = hole.first;
                triangle_object->holelist[hole_index * 2 + 1] = hole.second;
                hole_index++;
            }
            triangulate("pzvPQD", triangle_object.get(), _output.get(), triangle_vobject.get());
        } while(triangle_vobject->numberofpoints != 0);
        return holes;
    }
    std::vector<std::pair<double, double>> find_points_inside(std::shared_ptr<triangulateio> input) {
        std::vector<std::pair<double, double>> holes = std::vector<std::pair<double, double>>();
        std::shared_ptr<triangulateio> triangle_vobject = TriangleManipulator::create_instance();
        std::shared_ptr<triangulateio> _output = TriangleManipulator::create_instance();
        triangulate("pzvPQD", input.get(), _output.get(), triangle_vobject.get());
        do {
            if(triangle_vobject->numberofpoints < 1) {
                continue;
            }
            double x = triangle_vobject->pointlist[0];
            double y = triangle_vobject->pointlist[1];
            holes.push_back(std::pair<double, double>(x, y));
            input->numberofholes = holes.size();
            free(input->holelist);
            input->holelist = (double *) malloc(holes.size() * 2 * sizeof(double));
            int hole_index = 0;
            for(const std::pair<double, double>& hole : holes) {
                input->holelist[hole_index * 2] = hole.first;
                input->holelist[hole_index * 2 + 1] = hole.second;
                hole_index++;
            }
            triangulate("pvzPQD", input.get(), _output.get(), triangle_vobject.get());
        } while(triangle_vobject->numberofpoints != 0);
        return holes;
    }
    void from_list(const std::vector<PointLocation::Line>& list, std::shared_ptr<triangulateio> output) {
        std::unordered_map<unsigned int, unsigned short> points = std::unordered_map<unsigned int, unsigned short>();
        output->numberofsegments = list.size();
        output->segmentlist = (int *) malloc(output->numberofsegments * 2 * sizeof(int));
        output->segmentmarkerlist = (int *) malloc(output->numberofsegments * sizeof(int));
        int point_count = 0;
        std::unordered_set<unsigned int> points_set = std::unordered_set<unsigned int>();
        for(const PointLocation::Line& line : list) {
            if(!points_set.contains(line.first.hash)) {
                point_count++;
                points_set.insert(line.first.hash);
            }
            if(!points_set.contains(line.second.hash)) {
                point_count++;
                points_set.insert(line.second.hash);
            }
        }
        points_set.clear();
        output->numberofpoints = point_count;
        output->pointlist = (double *) malloc(point_count * 2 * sizeof(double));
        output->pointmarkerlist = (int *) malloc(point_count * sizeof(int));
        unsigned short point_index = 0;
        int segment_index = 0;
        for(const PointLocation::Line& line : list) {
            const PointLocation::Point& first = line.first;
            const PointLocation::Point& second = line.second;
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
    void from_list(std::vector<PointLocation::Line>* list, std::shared_ptr<triangulateio> output) {
        std::unordered_map<unsigned int, unsigned short> points = std::unordered_map<unsigned int, unsigned short>();
        output->numberofsegments = list->size();
        output->segmentlist = (int *) malloc(output->numberofsegments * 2 * sizeof(int));
        output->segmentmarkerlist = (int *) malloc(output->numberofsegments * sizeof(int));

        int point_count = 0;
        std::unordered_set<unsigned int> points_set = std::unordered_set<unsigned int>();
        
        for(std::vector<PointLocation::Line>::iterator iter = list->begin(); iter != list->end(); iter++) {
            if(!points_set.contains(iter->first.hash)) {
                point_count++;
                points_set.emplace(iter->first.hash);
            }
            if(!points_set.contains(iter->second.hash)) {
                point_count++;
                points_set.emplace(iter->second.hash);
            }
        }
        output->numberofpoints = point_count;
        output->pointlist = (double *) malloc(point_count * 2 * sizeof(double));
        output->pointmarkerlist = (int *) malloc(point_count * sizeof(int));
        unsigned point_index = 0;
        int segment_index = 0;
        for(std::vector<PointLocation::Line>::iterator iter = list->begin(); iter != list->end(); iter++) {
            const PointLocation::Point& first = iter->first;
            const PointLocation::Point& second = iter->second;
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
    void from_list(const std::vector<std::shared_ptr<std::vector<PointLocation::Line>>>& list_of_lists, std::shared_ptr<triangulateio> output) {
        std::unordered_map<unsigned int, unsigned short> points = std::unordered_map<unsigned int, unsigned short>();
        int num_segments = 0;
        for(const std::shared_ptr<std::vector<PointLocation::Line>>& list : list_of_lists) {
            num_segments += list->size();
        }
        output->numberofsegments = num_segments;
        output->segmentlist = (int *) malloc(num_segments * 2 * sizeof(int));
        output->segmentmarkerlist = (int *) malloc(num_segments * sizeof(int));

        int point_count = 0;
        std::unordered_set<unsigned int> points_set = std::unordered_set<unsigned int>();
        
        for(const std::shared_ptr<std::vector<PointLocation::Line>>& list : list_of_lists) {
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
        for(const std::shared_ptr<std::vector<PointLocation::Line>>& list : list_of_lists) {
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
}