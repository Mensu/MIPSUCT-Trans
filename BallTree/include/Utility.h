#ifndef __UTILITY_H
#define __UTILITY_H

#define L 256

#include <cassert>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <string>
#include <type_traits>

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

template <typename Container, typename F>
void ApplyElementwise(Container& cont, F f) {
    for (auto& elem : cont) {
        elem = f(elem);
    }
}

template <typename Container, typename F>
void Combine(Container& lhs, const Container& rhs, F f) {
    assert(lhs.size() == rhs.size());
    auto iter1 = begin(lhs);
    auto iter2 = begin(rhs);
    for (; iter1 != end(lhs); ++iter1, ++iter2) {
        *iter1 = f(*iter1, *iter2);
    }
}

template <typename Ret = double, typename Container>
Ret Distance(const Container& v1, const Container& v2) {
    static_assert(std::is_arithmetic<Ret>::value, "");
    static_assert(
        std::is_arithmetic<typename Container::value_type>::value, "");
    assert(v1.size() == v2.size());
    Ret result(0);
    for (auto iter1 = begin(v1), iter2 = begin(v2); iter1 != end(v1);
         ++iter1, ++iter2) {
        result += std::pow(*iter1 - *iter2, 2);
    }
    return std::sqrt(result);
}

inline Path DirName(const Path& p) {
    auto index = p.find_last_of('/');
    return index == std::string::npos ? "" : p.substr(0, index + 1);
}

template<typename Iter>
inline Byte bitsToByte(Iter start, Iter end) {
    int i = 0;
    Byte ret = 0;
    for (Iter it = start; it != end; ++it) {
        if (i >= 8) return ret;
        if (*it == true) {
            ret |= (1 << i);
        }
        i++;
    }
    return ret;
}

#endif  //__UTILITY_H
