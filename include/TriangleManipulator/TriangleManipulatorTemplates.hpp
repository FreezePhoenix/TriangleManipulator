#pragma once

#ifndef TRIANGLEMANIPULATORTEMPLATES_HPP_
#define TRIANGLEMANIPULATORTEMPLATES_HPP_

#include "TriangleManipulator/TriangleManipulator.hpp"

namespace TriangleManipulator {
    template<>
    inline float parse_str<float>(const std::string& str) {
        return std::stof(str);
    }
    template<>
    inline long parse_str<long>(const std::string& str) {
        return std::stol(str);
    }
    template<>
    inline long long parse_str<long long>(const std::string& str) {
        return std::stoll(str);
    }
    template <>
    inline double parse_str<double>(const std::string& str) {
        return std::stod(str);
    }
    template <>
    inline unsigned long parse_str<unsigned long>(const std::string& str) {
        return std::stoul(str);
    }
    template <>
    inline unsigned int parse_str<unsigned int>(const std::string& str) {
        return std::stoul(str);
    }
    template <>
    inline int parse_str<int>(const std::string& str) {
        return std::stoi(str);
    }
}

#endif /* TRIANGLEMANIPULATORTEMPLATES_HPP_ */