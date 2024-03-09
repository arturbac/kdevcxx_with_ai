#pragma once

#ifdef Q_MOC_RUN
#error "Dont process this with moc, use #ifndef Q_MOC_RUN"
#endif

#include <string>
#include "expectd.h"

namespace aiprocess
  {
enum struct clang_format_error
  {
  success,
  input_file_creation_failed,
  command_execution_failed,
  output_file_read_failed,
  unhandled_exception
  };

using clang_format_result = expected<std::string, clang_format_error>;

auto clang_format(std::string const & code, std::string const & working_directory) noexcept -> clang_format_result;

  }  // namespace aiprocess
