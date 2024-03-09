#pragma once

#include <stralgo/stralgo.h>

namespace aiprocess
  {
inline constexpr std::array white_chars{' ', '\t', '\n'};

inline constexpr auto trim_white_space(stralgo::concepts::char_range auto const & text) noexcept
  {
  return stralgo::trim_pred(text, stralgo::detail::not_is_any_of{white_chars.begin(), white_chars.end()});
  }
  }  // namespace aiprocess
