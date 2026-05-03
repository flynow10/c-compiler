//
// Created by Natalie Wagner on 5/1/26.
//

#ifndef C_COMPILER_HASH_COMBINE_HPP
#define C_COMPILER_HASH_COMBINE_HPP
#include <functional>

template<class T>
inline void hash_combine(std::size_t& seed, const T & v) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<class T, class ...Rest>
inline void hash_combine(std::size_t& seed, const T & v, const Rest & ... rest) {
    hash_combine(seed, v);
    hash_combine(seed, rest...);
}

#endif //C_COMPILER_HASH_COMBINE_HPP
