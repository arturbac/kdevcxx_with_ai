// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#include <aiprocess/clang_format.h>
#include <aiprocess/change_current_dir.h>
#include <aiprocess/log.h>
#include <filesystem>
#include <fstream>
#include <stralgo/stralgo.h>

using namespace std::string_view_literals;

namespace aiprocess
  {
auto clang_format(std::string const & code, std::string const & working_directory) noexcept -> clang_format_result
try
  {
    {
    using enum clang_format_error;
    // Temporary file names
    auto const input_filename = std::filesystem::temp_directory_path() / "temp_code_input.cpp";
    auto const output_filename = std::filesystem::temp_directory_path() / "temp_code_output.cpp";

      // Write code to a temporary file
      {
      std::ofstream input_file(input_filename);
      if(!input_file)
        return unexpected_error(
          input_file_creation_failed, "Failed to open file for write temporary file {}", input_filename.string()
        );
      input_file << code;
      }
    if(!working_directory.empty())
      if(auto res{
           change_current_directory(working_directory).transform_error([](auto) { return cannot_change_directory; })
         };
         !res.has_value())
        return unexpected_error(res.error(), "Failed");

    // Construct the clang-format command
    auto input_filename_string{input_filename.string()};
    auto command{stralgo::stl::merge(
      "clang-format"sv,
      " --assume-filename="sv,
      input_filename_string,
      " < "sv,
      input_filename_string,
      " > "sv,
      output_filename.string()
    )};
    // Execute clang-format
    if(std::system(command.c_str()) != 0)
      return unexpected_error(command_execution_failed, "Could not read back formatted file {}", command);

    std::ifstream output_file(output_filename);
    if(!output_file)
      return unexpected_error(output_file_read_failed, "Reading back file failed {}", command);
    std::string formatted_code((std::istreambuf_iterator<char>(output_file)), std::istreambuf_iterator<char>());

    std::filesystem::remove(input_filename);
    std::filesystem::remove(output_filename);

    return formatted_code;
    }
  }
catch(std::exception const & ec)
  {
  return unexpected_error(clang_format_error::unhandled_exception, "Unhandled exception {}", ec.what());
  }
catch(...)
  {
  return unexpected_error(clang_format_error::unhandled_exception, "Unhandled exception");
  }
  }  // namespace aiprocess
