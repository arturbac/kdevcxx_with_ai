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

struct text_test
  {
  std::string text;
  };

struct choices
  {
  std::vector<model_choice_data_t> choices;
  };

static suite<"glaze_tests"> glaze_tests = []
{
  "test text json"_test = []
  {
    static constexpr std::string_view value_expected{
      "```\n[[nodiscard]] auto get_command_text(const std::string& user_data, const std::string& command_start_delim, "
      "const int start_pos, const int end_pos)\n{\n    return std::ranges::views::all(user_data | "
      "std::views::drop(start_pos + command_start_delim.length()) | std::views::take(end_pos - (start_pos + "
      "command_start_delim.length())) | std::views::drop(1)) | std::ranges::to<std::string>();\n}\n\n// "
      "Usage:\nstd::string command_text = get_command_text(user_data, command_start_delim, start_pos, end_pos);\n```"
    };
    std::string text{fmt::format("{{\"text\" : \"{}\" }}", value_expected)};

    auto s = glz::read_json<text_test>(text);
    expect(fatal(s.has_value()));
    auto value{s.value()};
    expect(eq(value.text, value_expected));
  };
  //
  "test model_choice_data json"_test = []
  {
    static constexpr std::string_view source_text = R"({
      "message" :{
        "content": "```\n[[nodiscard]] auto get_command_text(const std::string& user_data, const std::string& command_start_delim, const int start_pos, const int end_pos)\n{\n    return std::ranges::views::all(user_data | std::views::drop(start_pos + command_start_delim.length()) | std::views::take(end_pos - (start_pos + command_start_delim.length())) | std::views::drop(1)) | std::ranges::to<std::string>();\n}\n\n// Usage:\nstd::string command_text = get_command_text(user_data, command_start_delim, start_pos, end_pos);\n```"
        },
      "index": 0,
      "logprobs": null,
      "finish_reason": "stop"
    }
)";
    auto s = glz::read_json<model_choice_data_t>(source_text);
    expect(fatal(s.has_value())) << [&s]() { return s.error().includer_error; };
  };
  "test model_choice_data array json"_test = []
  {
    static constexpr std::string_view source_text = R"({"choices": [
    {
      "message" :{
        "content": "```\n[[nodiscard]] auto get_command_text(const std::string& user_data, const std::string& command_start_delim, const int start_pos, const int end_pos)\n{\n    return std::ranges::views::all(user_data | std::views::drop(start_pos + command_start_delim.length()) | std::views::take(end_pos - (start_pos + command_start_delim.length())) | std::views::drop(1)) | std::ranges::to<std::string>();\n}\n\n// Usage:\nstd::string command_text = get_command_text(user_data, command_start_delim, start_pos, end_pos);\n```"
      },
      "index": 0,
      "logprobs": null,
      "finish_reason": "stop"
    }
  ]})";
    auto s = glz::read_json<choices>(source_text);
    expect(fatal(s.has_value())) << [&s]() { return s.error().includer_error; };
  };
  "test model_response_t json"_test = []
  {
    static constexpr std::string_view source_text = R"({
  "id": "cmpl-90uK2FFgygd7tOjaUY7pGa2ddYVIf",
  "object": "text_completion",
  "created": 1710003530,
  "model": "gpt-4-1106-preview",
  "choices": [
    {
      "index": 0,
      "message": 
        {
        "role" : "assistant",
        "content" : "```\n[[nodiscard]] auto get_command_text(const std::string& user_data, const std::string& command_start_delim, const int start_pos, const int end_pos)\n{\n    return std::ranges::views::all(user_data | std::views::drop(start_pos + command_start_delim.length()) | std::views::take(end_pos - (start_pos + command_start_delim.length())) | std::views::drop(1)) | std::ranges::to<std::string>();\n}\n\n// Usage:\nstd::string command_text = get_command_text(user_data, command_start_delim, start_pos, end_pos);\n```"
        },
      "logprobs": null,
      "finish_reason": "stop"
    }
  ],
  "usage": {
    "prompt_tokens": 79,
    "completion_tokens": 130,
    "total_tokens": 209
  }
})";
    auto s = glz::read_json<model_response_t>(source_text);
    expect(fatal(s.has_value())) << [&s]() { return s.error().includer_error; };
  };
};
static suite<"model_response_tests"> model_response_tests = []
{
  "success_case"_test = []
  {
    constexpr std::string_view json_data = R"({
            "id": "cmpl-90eNRqqPnxj7Yh9hWftWCufTH9wMP",
            "object": "text_completion",
            "created": 1709942237,
            "model": "gpt-3.5-turbo-instruct",
            "choices": [
                {
                "message": {
                  "content": "{ void fn(\"Test response text\"); }", "finish_reason": "length", "index": 0, "logprobs": 0.99}
                }
            ]
        })";

    auto pr{parse_json_response(json_data)};

    expect(aiprocess::contains(pr, "id : cmpl-90eNRqqPnxj7Yh9hWftWCufTH9wMP"));
    expect(aiprocess::contains(pr, "object : text_completion"));
    expect(aiprocess::contains(pr, "created : 1709942237"));
    expect(aiprocess::contains(pr, "model : gpt-3.5-turbo-instruct"));
    expect(aiprocess::contains(pr, "{ void fn(\"Test response text\"); }"));
    expect(aiprocess::contains(pr, "choices :"));
  };
#if 0
  "failure_case"_test = []
  {
    constexpr std::string_view invalid_json_data = R"({"invalid": "json"})";
    auto parsed{parse_json_response(invalid_json_data)};
    expect(eq(parsed, std::string{invalid_json_data}))
      << "The parser should return the original JSON string on failure.";
  };

  "complex_json_with_code_snippet"_test = []
  {
    constexpr std::string_view json_data = R"({
            "id": "cmpl-90mvpOgGxQ5x9Q3dmYsZcTRohgtsN",
            "object": "text_completion",
            "created": 1709975121,
            "model": "gpt-3.5-turbo-instruct",
            "choices": [{
                "text": "\n\n\n// C++20\n#include <iostream>\n#include <string>\n#include <vector>\n#include <algorithm>\n#include <ranges>\n#include <boost/ut.hpp>\n\n// Use std::ranges and std::views\nusing namespace std::ranges;\nusing namespace std::views;\n\n// Use nodiscard\n[[nodiscard]] std::string camel_case(std::string text) {\n    // Split text into words\n    auto words = text | split(' ');"
                "\n\n    // Capitalize first letter of each word\n    words = words | transform([](std::string word) {\n        return word | views::take(1) | to_upper | join | views::drop(1) | to_lower;\n    });\n\n    // Join words together\n   ",
                "index": 0,
                "logprobs": null,
                "finish_reason": "length"
            }],
            "usage": {
                "prompt_tokens": 44,
                "completion_tokens": 150,
                "total_tokens": 194
            }
        })";
    auto const expected_start = R"(id : cmpl-90mvpOgGxQ5x9Q3dmYsZcTRohgtsN)";
    auto const expected_choice_contains = R"(// C++20\n#include <iostream>)";

    auto const result = parse_json_response(json_data);
    expect(aiprocess::contains(result, expected_start))
      << "The output should start with the formatted header comment including the 'id'.";
    expect(aiprocess::contains(result, expected_choice_contains))
      << "The formatted string should contain the C++ code snippet from the 'choices' array.";
  };
#endif
};

static suite<"model_response_tests_basic"> model_response_tests_basic = [] {
#if 0
  "empty_json"_test = []
  {
    constexpr std::string_view json_data = R"({})";
    std::string const expected = "{}";
    auto parsed{parse_json_response(json_data)};
    expect(eq(parsed, expected))
      << "The parser should return the original JSON string for an empty JSON.";
  };

  "missing_id"_test = []
  {
    constexpr std::string_view json_data = R"({
            "object": "text_completion",
            "created": 1709942237,
            "model": "gpt-3.5-turbo-instruct",
        })";
    // Expecting it to process without 'id' field, since logic of formatting doesn't depend on 'id' being present
    std::string const expected_start = "object : text_completion\n";
    auto parsed{parse_json_response(json_data)};
    expect(parsed == json_data) << "The parser should handle missing 'id' field gracefully.";
  };

  "empty_choices_array"_test = []
  {
    constexpr std::string_view json_data = R"({
            "id": "cmpl-example",
            "object": "text_completion",
            "created": 1709942237,
            "model": "gpt-3.5-turbo-instruct",
        })";
    std::string const expected_contains = " choices :\n*/\n#if 1\n#endif\n";
    auto parsed{parse_json_response(json_data)};
    expect(aiprocess::contains(parsed, expected_contains)) << parsed;
  };

  "invalid_choice_structure"_test = []
  {
    constexpr std::string_view json_data = R"({
            "id": "cmpl-example",
            "object": "text_completion",
            "created": 1709942237,
            "model": "gpt-3.5-turbo-instruct",
        })";
    // Expecting the function to process even if 'choices' items are missing expected fields
    expect(aiprocess::contains(parse_json_response(json_data), "#elif 0 \n"))
      << "The parser should gracefully handle invalid structure within 'choices'.";
  };

  "missing_created_field"_test = []
  {
    constexpr std::string_view json_data = R"({
            "id": "cmpl-example",
            "object": "text_completion",
            "model": "gpt-3.5-turbo-instruct",
        })";
    // Expecting it to process without 'created' field
    std::string const expected_start = "/*\n id : cmpl-example\n";
    expect(aiprocess::contains(parse_json_response(json_data), expected_start))
      << "The parser should handle missing 'created' field gracefully.";
  };

  "missing_model_field"_test = []
  {
    constexpr std::string_view json_data = R"({
            "id": "cmpl-example",
            "object": "text_completion",
            "created": 1709942237,
            "choices": []
        })";
    // Expecting it to process without 'model' field
    std::string const expected_contains = " created : 1709942237\n";
    expect(aiprocess::contains(parse_json_response(json_data), expected_contains))
      << "The parser should handle missing 'model' field gracefully.";
  };



  "null_fields"_test = []
  {
    constexpr std::string_view json_data = R"({
            "id": null,
            "object": null,
            "created": null,
            "model": null,
            "choices": null
        })";
    // Expecting it to return the original string or process it if implementation allows null handling
    expect(
      bool(parse_json_response(json_data) == json_data) || aiprocess::contains(parse_json_response(json_data), "id :")
    ) << "The parser should handle null fields according to its implementation.";
  };

  "incomplete_json"_test = []
  {
    constexpr std::string_view json_data = R"({"id": "cmpl-example",)";
    expect(eq(parse_json_response(json_data), std::string{json_data}))
      << "The parser should return the original string on incomplete JSON.";
  };

  "completely_invalid_json"_test = []
  {
    constexpr std::string_view json_data = R"({:,,})";
    expect(eq(parse_json_response(json_data), std::string{json_data}))
      << "The parser should return the original string on completely invalid JSON.";
  };
#endif
};

int main() { return 0; }

