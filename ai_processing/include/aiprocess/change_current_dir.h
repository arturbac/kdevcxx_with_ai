#pragma once

#ifdef Q_MOC_RUN
#error "Dont process this with moc, use #ifndef Q_MOC_RUN"
#endif

#include "expectd.h"

namespace aiprocess
  {
enum struct change_directory_error
  {
  unhandled_exception,
  filesystem_error
  };
[[nodiscard]]
auto change_current_directory(std::string_view new_directory) noexcept -> expected<void, change_directory_error>;
  }
