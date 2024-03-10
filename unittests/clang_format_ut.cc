#include <boost/ut.hpp>
#include <string_view>
#include <aiprocess/clang_format.h>
#include <aiprocess/contains.h>
#include <glaze/glaze.hpp>
#include <aiprocess/log.h>
#include <aiprocess/app_settings.h>
// #include <fmt/format.h>

namespace ut = boost::ut;
using namespace boost::ut;
using boost::ut::operator""_test;
using namespace ut::operators::terse;

using aiprocess::clang_format;
using aiprocess::clang_format_error;

// Include or declare clang_format and related dependencies here
static constexpr auto example_code{R"(\n
[[nodiscard]] auto get_command_text(const std::string& user_data, const std::string& command_start_delim, const int start_pos, const int end_pos)\n
{\n
return std::ranges::views::all(user_data | std::views::drop(start_pos + command_start_delim.length()) | std::views::take(end_pos - (start_pos + command_start_delim.length())) | std::views::drop(1)) | std::ranges::to<std::string>();
\n}
\n
\n
// Usage:\nstd::string command_text = get_command_text(user_data, command_start_delim, start_pos, end_pos);
\n)"};

int main()
  {
  using namespace boost::ut;
  aiprocess::setup_loggers(aiprocess::app_settings_t{.log_path = "clang_format_ut"});

  "clang_format_success"_test = []
  {
    auto result = clang_format(example_code, std::filesystem::current_path());
    expect(fatal(result.has_value())) << "Expected successful formatting";
  };

  "input_file_creation_failed"_test = []
  {
    auto result
      = clang_format("auto get_command_text(const std::string& user_data);", "/tmp/nonexistant/invalid/directory");
    expect(fatal(!result.has_value())) << "Expected input file creation failure";
    expect(result.error() == clang_format_error::cannot_change_directory)
      << "Expected specific error enum for input file creation failure";
  };
#if 0
  // TODO enable once clang-format will be a configuration parameter
  "command_execution_failed"_test = []
  {
    auto result = clang_format("valid code causing command failure", "/tmp");
    expect(fatal(!result.has_value())) << "Expected command execution failure";
    expect(result.error() == clang_format_error::cannot_change_directory)
      << "Expected specific error enum for command execution failure";
  };
#endif
  }

