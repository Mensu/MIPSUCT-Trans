#ifndef __UTILITY_H
#define __UTILITY_H

#define L 256

#include <cassert>
#include <cmath>
#include <numeric>
#include <string>
#include <type_traits>
#include <cstdint>

using Path = std::string;
using Byte = std::uint8_t;

bool read_data(int n, int d, float**& data, const char* file_name);

template <typename Ret = double, typename Container>
Ret InnerProduct(const Container& v1, const Container& v2) {
    static_assert(std::is_arithmetic<Ret>::value, "");
    static_assert(
        std::is_arithmetic<typename Container::value_type>::value, "");

    assert(v1.size() == v2.size());

    return std::inner_product(
        begin(v1), end(v1), begin(v2), static_cast<Ret>(0));
}

template <typename Ret = double, typename Container>
Ret Norm(const Container& v) {
    static_assert(std::is_arithmetic<Ret>::value, "");
    static_assert(
        std::is_arithmetic<typename Container::value_type>::value, "");

    return std::sqrt(std::accumulate(
        begin(v), end(v), static_cast<Ret>(0),
        [](auto a, auto b) { return a + b * b; }));
}

inline Path DirName(const Path& p) {
    auto index = p.find_last_of('/');
    return index == std::string::npos ? "" : p.substr(0, index + 1);
}

#endif
