#include <boost/ut.hpp>
#include <string_view>
#include <ai_processing.h>
#include <aiprocess/contains.h>
#include <glaze/glaze.hpp>
#include <fmt/format.h>

namespace ut = boost::ut;
using namespace boost::ut;
using boost::ut::operator""_test;
using namespace ut::operators::terse;

// Include or declare clang_format and related dependencies here

int main()
  {
  using namespace boost::ut;

  "clang_format_success"_test = []
  {
    auto result = clang_format("valid code", std::filesystem::current_path() );
    expect(result.has_value()) << "Expected successful formatting";
  };

  "input_file_creation_failed"_test = []
  {
    auto result = clang_format("valid code", "directory causing input file failure");
    expect(!result.has_value()) << "Expected input file creation failure";
    expect(result.error() == clang_format_error::input_file_creation_failed)
      << "Expected specific error enum for input file creation failure";
  };

  "command_execution_failed"_test = []
  {
    auto result = clang_format("valid code causing command failure", "valid directory");
    expect(!result.has_value()) << "Expected command execution failure";
    expect(result.error() == clang_format_error::command_execution_failed)
      << "Expected specific error enum for command execution failure";
  };

  "output_file_read_failed"_test = []
  {
    auto result = clang_format("valid code", "directory causing output file read failure");
    expect(!result.has_value()) << "Expected output file read failure";
    expect(result.error() == clang_format_error::output_file_read_failed)
      << "Expected specific error enum for output file read failure";
  };

  "unhandled_exception"_test = []
  {
    auto result = clang_format("code causing unhandled exception", "valid directory");
    expect(!result.has_value()) << "Expected unhandled exception";
    expect(result.error() == clang_format_error::unhandled_exception)
      << "Expected specific error enum for unhandled exception";
  };
  }

