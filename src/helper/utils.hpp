/*
 * File: utils.hpp
 * Project: helper
 * File Created: 2022-09-12 21:12:36
 * Author: yangyangyang (yangyangyang0110@outlook.com)
 * -----
 * Last Modified: 2022-09-12 21:12:42
 * Modified By: yangyangyang (yangyangyang0110@outlook.com>)
 * -----
 * Copyright 2022 - 2022  ,
 */

#pragma once
#include <cstddef>

namespace helper::utils {
template <typename T, size_t N> size_t getArraySize(const T (&)[N]) { return N; }
} // namespace helper::utils
