// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <string_view>

namespace aiprocess
  {
constexpr auto remove_pattern = [](std::string str, std::string_view pattern) -> std::string
{
  std::string::size_type pos = 0;
  while((pos = str.find(pattern, pos)) != std::string::npos)
    str.erase(pos, pattern.length());
  return str;
};
  }  // namespace aiprocess

