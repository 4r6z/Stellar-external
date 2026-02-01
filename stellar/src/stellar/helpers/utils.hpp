#pragma once

#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>

#include "../driver/driver.hpp"

template <typename T = Instance>
bool is_valid_instance(T instance) {
    if (instance.self == 0)
        return false;

    T read_instance = read<T>(instance.self);

    return read_instance.self != 0;
}

template <typename T = Instance>
bool is_valid_instance(T instance, T expected_instance) {
    if (instance.self == 0 || expected_instance.self == 0)
        return false;

    T read_instance = T(instance);

    return read_instance.self != 0 && (read_instance.self == expected_instance.self);
}

#endif