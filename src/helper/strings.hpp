/*
 * File: strings.hpp
 * Project: helper
 * File Created: 2022-09-10 22:46:32
 * Author: yangyangyang (yangyangyang0110@outlook.com)
 * -----
 * Last Modified: 2022-09-10 22:46:37
 * Modified By: yangyangyang (yangyangyang0110@outlook.com>)
 * -----
 * Copyright 2022 - 2022  ,
 */
#pragma once

#include <optional>
#include <string_view>
#include <tuple>

namespace helper::strings {

using namespace std::string_view_literals;

static std::optional<std::tuple<std::string_view, std::string_view>>
splitAddress(std::string_view const sv) {
  const size_t pos = sv.find_last_of(":"sv);
  if (pos == std::string_view::npos)
    return std::nullopt;
  return std::make_tuple(sv.substr(0, pos), sv.substr(pos + 1, sv.length()));
}

} // namespace helper::strings
