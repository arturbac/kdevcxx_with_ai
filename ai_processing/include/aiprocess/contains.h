#pragma once

#include <string_view>
#include <ranges>

namespace aiprocess
  {

constexpr auto contains = [](std::string_view haystack, std::string_view needle) noexcept -> bool
{
  auto result = std::ranges::search(haystack, needle);
  return result.begin() != haystack.end();
};
  }  // namespace aiprocess
