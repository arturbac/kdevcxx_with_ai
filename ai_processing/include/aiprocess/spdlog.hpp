#pragma once

#include <simple_enum/simple_enum.hpp>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <spdlog/spdlog.h>
#pragma clang diagnostic pop

namespace spdlog::level
  {
consteval auto adl_enum_bounds(level_enum)
  {
  using enum level_enum;
  return simple_enum::adl_info{trace, critical};
  }
  }  // namespace spdlog::level

