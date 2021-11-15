#include <iostream>
#include <cstring>
#include <iomanip>
#include <memory>
#include "TriangleManipulator/TriangleManipulator.hpp"

using namespace std;

int main() {
  std::shared_ptr<triangulateio> in = TriangleManipulator::create_instance();
  // TriangleManipulator::read_ele_file("hut.ele", in);
  TriangleManipulator::read_ele_file_binary("hut.ele.bin", in);
  TriangleManipulator::write_ele_file_binary("hut.ele.bin", in);
  // TriangleManipulator::write_ele_file("ahut.ele", in);
  return 0;
}