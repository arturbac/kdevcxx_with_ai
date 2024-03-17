// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#ifdef Q_MOC_RUN
#error "Dont process this with moc, use #ifndef Q_MOC_RUN"
#endif

#include <string_view>
#include <ranges>

namespace aiprocess
  {

// AI CODE BLOCK BEGIN
/**
 * @brief Checks if `needle` is contained within `haystack`.
 */
constexpr auto contains = [](std::string_view haystack, std::string_view needle) noexcept -> bool
{
  auto result = std::ranges::search(haystack, needle);
  return result.begin() != haystack.end();
};

// AI CODE BLOCK END

  }  // namespace aiprocess
