#ifndef TYPES_H
#define TYPES_H

#include <array>
#include <cstdint>
#include <cstdio>

using u8 = int8_t;
using u16 = int16_t;
using u32 = int32_t;
using u64 = int64_t;
using usize = size_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using isize = ssize_t;

using f32 = float;
using f64 = double;

template<typename T>
using ptr = T const*;

template<typename T>
using ptr_mut = T*;

template<typename T>
using ref = T const&;

template<typename T>
using ref_mut = T&;

template<typename T, size_t N>
using array = std::array<T, N>;


#endif //TYPES_H
