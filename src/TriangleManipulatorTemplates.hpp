#pragma once

#ifndef TRIANGLEMANIPULATORTEMPLATES_HPP_
#define TRIANGLEMANIPULATORTEMPLATES_HPP_

#include "TriangleManipulator.hpp"

namespace TriangleManipulator {
    template <>
    double parse_str<double>(const std::string& str) {
        return std::stod(str);
    }
    template <>
    unsigned long parse_str<unsigned long>(const std::string& str) {
        return std::stoul(str);
    }
    template <>
    unsigned int parse_str<unsigned int>(const std::string& str) {
        return std::stoul(str);
    }
    template <>
    inline int parse_str<int>(const std::string& str) {
        return std::stoi(str);
    }
}

#endif /* TRIANGLEMANIPULATORTEMPLATES_HPP_ */