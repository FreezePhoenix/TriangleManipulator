#pragma once

#ifndef TRIANGLEMANIPULATORTEMPLATES_HPP_
#define TRIANGLEMANIPULATORTEMPLATES_HPP_

#include "TriangleManipulator/TriangleManipulator.hpp"

namespace TriangleManipulator {
    template<>
    float parse_str<float>(const std::string& str) {
        return std::stof(str);
    }
    template<>
    long parse_str<long>(const std::string& str) {
        return std::stol(str);
    }
    template<>
    long long parse_str<long long>(const std::string& str) {
        return std::stoll(str);
    }
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