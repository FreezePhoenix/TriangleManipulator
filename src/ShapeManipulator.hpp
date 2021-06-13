#pragma once

#ifndef ALBOT_SHAPEMANIPULATOR_HPP_
#define ALBOT_SHAPEMANIPULATOR_HPP_

#include "TriangleManipulator.hpp"
#include "PointLocation.hpp"
#include <memory>

namespace ShapeManipulator {
    void from_list(const std::vector<PointLocation::Line>& list, std::shared_ptr<triangulateio> output);
    void from_list(std::vector<PointLocation::Line>* list, std::shared_ptr<triangulateio> output);
    void from_list(const std::vector<std::vector<PointLocation::Line>*>& list_of_lists, std::shared_ptr<triangulateio> output);
    void from_list(const std::vector<std::shared_ptr<std::vector<PointLocation::Line>>>& list_of_lists, std::shared_ptr<triangulateio> output);
    std::vector<std::pair<double, double>> find_points_inside(const std::vector<PointLocation::Line>& object, std::shared_ptr<triangulateio> output);
    std::vector<std::pair<double, double>> find_points_inside(std::shared_ptr<triangulateio> input);
}

#endif /* ALBOT_SHAPEMANIPULATOR_HPP_ */