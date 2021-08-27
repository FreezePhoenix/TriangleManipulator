#pragma once

#ifndef SHAPEMANIPULATOR_HPP_
#define SHAPEMANIPULATOR_HPP_

#include "PointLocation.hpp"
#include <functional>
#include <memory>
#include <map>

inline bool intersects(const PointLocation::Line& line_one, const PointLocation::Line& line_two) {
    if(line_one.first.x == line_one.second.x) {
        // line_one is vertical.
        if(line_two.first.x == line_two.second.x) {
            // line_two is also vertical? This isn't looking that good...
            return line_two.first.x == line_one.first.x && line_one.first.y <= line_two.second.y && line_one.second.y >= line_two.first.y;
        } 
        // line_two is horizontal.
        return line_two.first.x <= line_one.first.x && line_two.second.x >= line_one.first.x && line_one.first.y <= line_two.first.y && line_one.second.y >= line_two.first.y;
    } else {
        // line_one is horizontal.
        if(line_two.first.x == line_two.second.x) {
            // line_two is vertical.
            return line_two.first.y <= line_one.first.y && line_two.second.y >= line_one.first.y && line_one.first.x <= line_two.first.x && line_one.second.x >= line_two.first.x;
        } else if(line_one.first.y == line_two.first.y) {
            // line_two is horizontal as well... treat as normal range overlap
            return line_one.first.x <= line_two.second.x && line_one.second.x >= line_two.first.x;
        }
    }
    return false;
}

namespace ShapeManipulator {
    enum MARKER {
        BEGIN = 0,
        END = 1
    };
    struct Point_Marker {
        PointLocation::Point point;
        short line;
        MARKER marker;
        Point_Marker(const PointLocation::Point& point, short line, MARKER marker) {
            this->point = point;
            this->line = line;
            this->marker = marker;
        }
        constexpr bool operator<(const Point_Marker& rhs) const {
            // if (this->point.x < rhs.point.x) {
            //     return true;
            // }
            // if (this->point.x == rhs.point.x) {
            //     if (this->point.y < rhs.point.y) {
            //         return true;
            //     }
            //     return this->point.y == rhs.point.y && this->marker < rhs.marker;
            // }
            // return false;
            return (this->point.x < rhs.point.x) || (this->point.x == rhs.point.x && (this->point.y < rhs.point.y || (this->point.y == rhs.point.y && this->marker < rhs.marker)));
        }
    };
    
    void from_list(const std::vector<PointLocation::Line>& list, std::shared_ptr<triangulateio> output);
    inline void from_list(std::vector<PointLocation::Line>* list, std::shared_ptr<triangulateio> output) {
        return from_list(*list, output);
    }
    void from_list(const std::vector<std::shared_ptr<std::vector<PointLocation::Line>>>& list_of_lists, std::shared_ptr<triangulateio> output);
    // Length of the vector is twice the number of holes.
<<<<<<< HEAD
    std::shared_ptr<std::vector<double>> find_points_inside(const std::vector<PointLocation::Line>& object);
    inline std::shared_ptr<std::vector<double>> find_points_inside(std::shared_ptr<std::vector<PointLocation::Line>> object) {
        return find_points_inside(*object);
    };
    std::shared_ptr<std::vector<double>> find_points_inside(std::shared_ptr<triangulateio> input);
    inline void handle_intersections(std::vector<PointLocation::Line>& lines, std::function<void(const PointLocation::Line&, const PointLocation::Line&)> callback) {
        std::multiset<Point_Marker> points = std::multiset<Point_Marker>();
        const PointLocation::Line* lines_ptr = lines.data();
        for (int i = 0, size = lines.size(); i < size; i++) {
            const PointLocation::Line& line = *lines_ptr++;
            points.emplace(line.first, i, BEGIN);
            points.emplace(line.second, i, END);
        }
        // std::set<std::pair<short, short>> beginnings = std::set<std::pair<short, short>>();
        // for (const Point_Marker& first : points) {
        //     short line = first.line;
        //     const std::set<std::pair<short, short>>::iterator& index = beginnings.find({ lines[line].y1, line });
        //     std::set<std::pair<short, short>>::iterator mutable_index = index;
        //     if (index != beginnings.end()) {
        //         if (index != beginnings.begin()) {
        //             short first = (*(--mutable_index)).second;
        //             short second = (*(++++mutable_index)).second;
        //             if (mutable_index != beginnings.end()) {
        //                 if (intersects(lines[first], lines[second])) {
        //                     callback(lines[first], lines[second]);
        //                     // std::cout << "INTERSECTION CAUGHT A: " << first.second << "," << second.second << std::endl;
        //                 }
        //             }
        //         }
        //         beginnings.erase(index);
        //     } else {
        //         std::set<std::pair<short, short>>::iterator index = beginnings.insert({ first.point.y, line }).first;
        //         // std::cout << "INSERTING: " << line << std::endl;
        //         // std::cout << "STACK: " << std::endl;
        //         // for (const std::pair<short, short>& other_line : beginnings) {
        //         //     std::cout << other_line.second << "," << other_line.first << std::endl;
        //         // }
        //         if (index != beginnings.begin()) {
        //             const std::pair<short, short>& checking = *(--index);
        //             if (intersects(lines[line], lines[checking.second])) {
        //                 callback(lines[line], lines[checking.second]);
        //                 // std::cout << "INTERSECTION CAUGHT B: " << line << "," << checking.second << std::endl;
        //             }
        //             index++;
        //         }
        //         if (++index != beginnings.end()) {
        //             std::pair<short, short> checking = *(index);
        //             if (intersects(lines[line], lines[checking.second])) {
        //                 callback(lines[line], lines[checking.second]);
        //                 // std::cout << "INTERSECTION CAUGHT C: " << line << "," << checking.second << std::endl;
        //             }
        //         }
        //     }
        // }

        // Ironically, this short little piece of code is somehow faster than the commented stuff above...
        std::set<short> beginnings = std::set<short>();
        for (const Point_Marker& first : points) {
            if (first.marker == BEGIN) {
                beginnings.insert(first.line);
                continue;
            }
            beginnings.erase(first.line);
            for (const short other_line : beginnings) {
                if (intersects(lines[first.line], lines[other_line])) {
                    callback(lines[first.line], lines[other_line]);
                }
            }
        }
    }
=======
    std::vector<double>* find_points_inside(const std::vector<PointLocation::Line>& object);
    inline std::vector<double>* find_points_inside(std::shared_ptr<std::vector<PointLocation::Line>> object) {
        return find_points_inside(*object);
    };
    std::vector<double>* find_points_inside(std::shared_ptr<triangulateio> input);
>>>>>>> 113f69d646b8a9a4c002af5486ab261cdea94bb8
}

#endif /* ALBOT_SHAPEMANIPULATOR_HPP_ */