#include "TriangleManipulator/TriangleManipulator.hpp"
#include "TriangleManipulator/PointLocation.hpp"
#include <iostream>
#include <math.h>
#include <vector>
#include "fmt/os.h"
typedef std::tuple<unsigned int, int, short, short> TYPES;

template<size_t TYPE = 0>
struct Message {
    std::tuple_element<TYPE, TYPES>::type* argument;
    const size_t type = TYPE;
};

template<size_t TYPE>
void print_message(Message<TYPE>* arg);

template<>
void print_message<0>(Message<0>* arg) {
    fmt::print("{} Huh?\n", *arg->argument);
}

template<>
void print_message<1>(Message<1>* arg) {
    fmt::print("HAH {}\n", *arg->argument);
}

template<>
void print_message<2>(Message<2>* arg) {
    fmt::print("{} HAH\n", *arg->argument);
}

decltype(print_message<0>)* FUNCTIONS[3] = { &print_message<0>, (decltype(print_message<0>)*) &print_message<1>, (decltype(print_message<0>)*) &print_message<2> };

template<size_t TYPE = 0>
struct MaybeMaybeMaybe {
    constexpr static decltype(print_message<TYPE>)* func = &print_message<TYPE>;
};

typedef float float32x4_t __attribute__((__vector_size__(16)));
void test() {
    int c = 5;
    // decltype(print_message<1>) a;
    Message<1> a = Message<1>(&c);
    Message<>* D = (Message<>*) & a;
    FUNCTIONS[0](D);
}
int main() {
    test();
}