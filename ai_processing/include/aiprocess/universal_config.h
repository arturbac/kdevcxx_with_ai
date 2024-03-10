#pragma once
#include <aiprocess/string_view.h>
#include <aiprocess/concepts.h>
#include <simple_enum/simple_enum.hpp>

namespace aiprocess
  {
using std::string_view;

// ---------------------------------------------------------------------------------------------------------------------
///\ brief universal json io for configuration files with versioning
template<concepts::unversal_config_compatibile config_type>
struct unversal_config_io_t;

enum struct unversal_config_io_error_e : uint8_t
  {
  error_parsing_file,
  error_creating_file,
  error_config_from_newer_version_of_program
  };

consteval auto adl_enum_bounds(unversal_config_io_error_e)
  {
  using enum unversal_config_io_error_e;
  return simple_enum::adl_info{error_parsing_file, error_config_from_newer_version_of_program};
  }
  }  // namespace aiprocess

