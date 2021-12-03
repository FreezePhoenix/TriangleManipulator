#include <iostream>
#include <cstring>
#include <iomanip>
#include <span>
#include <memory>
#include "TriangleManipulator/TriangleManipulator.hpp"
#include "TriangleManipulator/PointLocation.hpp"

// using namespace std;
// struct complicated_struct {
//   const int i;
//   short f;
// };
// namespace Reflection {
//   template<size_t I>
//   constexpr auto id_to_type(std::integral_constant<size_t, I>) noexcept;
// #define REGISTER_TYPE(Type, Index)                                                \
//   template<>                                                                      \
//   constexpr auto id_to_type<Index>(std::integral_constant<size_t, Index>) noexcept {     \
//     Type res{};                                                                   \
//     return res;                                                                   \
//   };                                                                              \
//   constexpr size_t type_to_id(type_identity<Type>) {                              \
//     return Index;                                                                 \
//   };
//   REGISTER_TYPE(unsigned char, 1);
//   REGISTER_TYPE(unsigned short, 2);
//   REGISTER_TYPE(unsigned int, 3);
//   REGISTER_TYPE(unsigned long, 4);
//   REGISTER_TYPE(unsigned long long, 5);
//   REGISTER_TYPE(signed char, 6);
//   REGISTER_TYPE(short, 7);
//   REGISTER_TYPE(int, 8);
//   REGISTER_TYPE(long, 9);
//   REGISTER_TYPE(long long, 10);
//   REGISTER_TYPE(double, 11);
//   struct ubiq {
//     template <class Type>
//     constexpr operator Type& () const;
//   };

//   template<size_t I>
//   struct ubiq_constructor {
//     template <class Type>
//     constexpr operator Type& () const noexcept;
//   };

//   template<size_t I>
//   struct ubiq_val {
//     size_t* ref;
//     template<class Type>
//     constexpr operator Type() const noexcept {
//       ref[I] = type_to_id(std::type_identity<Type>{});
//       return Type{};
//     }
//   };

//   template<class T, size_t I0, size_t... I>
//   constexpr auto detect_fields_count(size_t& out, std::index_sequence<I0, I...>) noexcept -> decltype(T{ ubiq_constructor<I0>{} , ubiq_constructor<I>{}... }) {
//     out = sizeof...(I) + 1;
//     return T{};
//   }

//   template<class T, size_t... I>
//   constexpr void detect_fields_count(size_t& out, std::index_sequence<I...>) noexcept {
//     detect_fields_count<T>(out, std::make_index_sequence<sizeof...(I) - 1>{});
//   }

//   template<class T>
//   constexpr size_t count_fields() noexcept {
//     size_t temp = 0;
//     detect_fields_count<T>(temp, std::make_index_sequence<sizeof(T)>{});
//     return temp;
//   }

//   template<class T, size_t N, size_t... I>
//   constexpr auto type_to_array_of_type_ids(size_t* types) noexcept
//     -> decltype(T{ ubiq_constructor<I>{} ... }) {
//     T tmp{ ubiq_val<I>{types} ... };
//     return tmp;
//   }
  
//   template<class T, size_t N, size_t... I>
//   struct arr {
//       constexpr arr() : values() {
//           T temp {ubiq_val<I>{values}...};
//       }
//       size_t values[N];
//   };

//   template<class T, size_t N, size_t... I>
//   constexpr auto as_tuple_impl(std::index_sequence<I...>) noexcept {
//     constexpr auto types = arr<T, count_fields<T>(), I...>();

//     return std::tuple<
//       decltype(id_to_type(
//         std::integral_constant<size_t, types.values[I]>{}
//       ))...
//     >{};
//   }

//   template<class T>
//   constexpr auto tuple_impl() noexcept {
//     return as_tuple_impl<T, count_fields<T>()>(std::make_index_sequence<count_fields<T>()>{});
//   }
  
//   template<class T, size_t... I, size_t size = sizeof(T)>
//   constexpr size_t get_padding(std::index_sequence<I...>) noexcept {
//     constexpr auto result = size - (sizeof(std::tuple_element_t<I, decltype(tuple_impl<T>())>) + ...);
//     return result;
//   }

//   template<class T>
//   constexpr size_t padding() noexcept {
//     return get_padding<T>(std::make_index_sequence<count_fields<T>()>{});
//   }
// }
int add(int a, int b) {
  return a + b;
}
int main() {
std::shared_ptr<triangulateio> test = TriangleManipulator::create_instance();
    TriangleManipulator::read_poly_file("test.poly", test);
    TriangleManipulator::read_ele_file("test.ele", test);
    TriangleManipulator::write_poly_file("testa.poly", test);
    PointLocation::GraphInfo info = PointLocation::GraphInfo(test);
    std::shared_ptr<triangulateio> other = TriangleManipulator::create_instance();
    info.process();
    TriangleManipulator::read_ele_file("test.ele", other);
    info.map_triangles(other);
    info.write_to_file("Planar");
    int a = info.locate_point(PointLocation::Vertex::Point{ 44, -28 });
    PointLocation::Triangle& b = info.planar_graph->all_triangles[a];
}