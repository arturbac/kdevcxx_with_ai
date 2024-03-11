// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#include <aiprocess/string_view.h>
#include <stralgo/stralgo.h>

namespace aiprocess
  {
constexpr auto obfuscate_text(std::string_view input) -> std::string
  {
  if(input.length() <= 6) [[unlikely]]
    return std::string(input);

  return stralgo::stl::merge(
    stralgo::left(input, 3),
    std::views::iota(0u, input.size() - 6) | std::views::transform([](auto) { return '*'; }),
    stralgo::right(input, 3)
  );
  }

  }  // namespace aiprocess
