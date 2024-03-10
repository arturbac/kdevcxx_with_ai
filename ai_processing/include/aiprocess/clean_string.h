#pragma once

#ifdef Q_MOC_RUN
#error "Dont process this with moc, use #ifndef Q_MOC_RUN"
#endif

#include <string_view>
#include <ranges>

namespace aiprocess
  {
/// \brief removes ``` from beginning and end of string
constexpr auto clean_string = [](std::string && str) -> std::string
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
  str.erase(new_end, str.end());  // Adjust for the reverse view's base iterator
  str.erase(str.begin(), start);
  return str;
};
  }  // namespace aiprocess
