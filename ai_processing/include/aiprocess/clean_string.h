#pragma once

#include <string_view>
#include <ranges>

namespace aiprocess
  {

constexpr auto clean_string(std::string && str) -> std::string
  {
  auto not_backtick = [](char c) noexcept -> bool { return c != '`'; };

  auto start = std::ranges::find_if(str, not_backtick);
  auto end = std::ranges::find_if(str | std::views::reverse, not_backtick);

  if(start == str.end()) [[unlikely]]
    {
    str.clear();
    return str;
    }

  auto new_end = end.base();
  str.erase(new_end, str.end());
  str.erase(str.begin(), start);
  return str;
  }
  }  // namespace aiprocess
