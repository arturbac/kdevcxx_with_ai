#include <boost/ut.hpp>
#include <string_view>
#include <ai_processing.h>
#include <aiprocess/clean_string.h>
#include <fmt/format.h>

namespace ut = boost::ut;
using namespace boost::ut;
using boost::ut::operator""_test;
using namespace ut::operators::terse;
using namespace std::string_view_literals;

int main()
  {
  using namespace boost::ut;

  "empty_string"_test = []
  {
    std::string test_str = "";
    test_str = aiprocess::clean_string(std::move(test_str));
    expect(test_str.empty()) << "Expected empty string to remain unchanged";
  };

  "string_with_only_backticks"_test = []
  {
    std::string test_str = "````";
    test_str = aiprocess::clean_string(std::move(test_str));
    expect(test_str.empty()) << "Expected string with only backticks to become empty";
  };

  "string_with_backticks_at_front"_test = []
  {
    std::string test_str = "`hello";
    test_str = aiprocess::clean_string(std::move(test_str));
    expect(eq(test_str, "hello"sv)) << "Expected string with backticks at front to be cleaned";
  };

  "string_with_backticks_at_back"_test = []
  {
    std::string test_str = "world`";
    test_str = aiprocess::clean_string(std::move(test_str));
    expect(eq(test_str, "world"sv)) << "Expected string with backticks at back to be cleaned";
  };

  "string_with_backticks_at_both_ends"_test = []
  {
    std::string test_str = "`hello world`";
    test_str = aiprocess::clean_string(std::move(test_str));
    expect(eq(test_str, "hello world"sv)) << "Expected string with backticks at both ends to be cleaned";
  };

  "string_with_no_backticks"_test = []
  {
    std::string test_str = "no backticks";
    test_str = aiprocess::clean_string(std::move(test_str));
    expect(eq(test_str, "no backticks"sv)) << "Expected string with no backticks to remain unchanged";
  };
  }
