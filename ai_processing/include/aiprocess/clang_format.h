#pragma once

#ifdef Q_MOC_RUN
#error "Dont process this with moc, use #ifndef Q_MOC_RUN"
#endif

#include <simple_enum/simple_enum.hpp>
#include <string>
#include "expectd.h"

namespace aiprocess
  {
enum struct clang_format_error
  {
  input_file_creation_failed,
  command_execution_failed,
  output_file_read_failed,
  cannot_change_directory,
  unhandled_exception
  };

consteval auto adl_enum_bounds(clang_format_error)
  {
  using enum clang_format_error;
  return simple_enum::adl_info{input_file_creation_failed, unhandled_exception};
  }

using clang_format_result = expected<std::string, clang_format_error>;

/**
 * @brief Applies clang formatting to the provided code using settings from the specified working directory.
 *
 * @param code The code to format as a std::string.
 * @param working_directory The directory where the clang-format settings file is located.
 * @return clang_format_result The result of the formatting operation.
 */
auto clang_format(std::string const & code, std::string const & working_directory) noexcept -> clang_format_result;

  }  // namespace aiprocess
