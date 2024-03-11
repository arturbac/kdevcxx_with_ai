// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#ifdef Q_MOC_RUN
#error "Dont process this with moc, use #ifndef Q_MOC_RUN"
#endif

#include "expectd.h"
#include <simple_enum/simple_enum.hpp>

namespace aiprocess
  {
enum struct change_directory_error
  {
  unhandled_exception,
  filesystem_error
  };

consteval auto adl_enum_bounds(change_directory_error)
  {
  using enum change_directory_error;
  return simple_enum::adl_info{unhandled_exception, filesystem_error};
  }

[[nodiscard]]
auto change_current_directory(std::string_view new_directory) noexcept -> expected<void, change_directory_error>;
  }  // namespace aiprocess
