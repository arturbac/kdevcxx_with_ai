#include <network.h>

#include <boost/ut.hpp>
#include <string_view>

int main()
  {
  using namespace boost::ut;

  "send_text_to_gpt_echo_test"_test = []
  {
    std::string_view host = "postman-echo.com";
    std::string_view port = "443";
    std::string_view target = "/post";
    std::string_view api_key = "INVALID KEY";
    std::string_view text = R"({"test": "data"})";
    int version = 11;  // HTTP/1.1

    auto result = send_text_to_gpt(host, port, target, api_key, text, version);

    // Since `expected` is hypothetical here, let's assume `result` is valid if no error is present.
    expect(result.has_value()) << "Expected a valid result";

    // Assuming the echoed back text contains the original text
    expect(result.value().find("\"test\": \"data\"") != std::string::npos) << result.value() << "\nThe response should contain the sent text\n";
  };

  "send_text_to_gpt_with_glaze_json"_test = []
  {
    // Define the JSON data using Glaze (pseudo-code, adjust according to Glaze's actual API)
    static constexpr std::string_view json_text = R"({
      {"model", "gpt-3.5-turbo-instruct"},
      {"prompt",
       "[Use C++20, prefer std::ranges and std::views over for loops, use nodiscard, use trailing return, use "
       "lower_case convention always,simplify code for c++20 use trailing return, apply lower_case naming convention] "
       "\n  ASSERT(value.isMember(szName));\n  if (!value.isMember(szName))\n    return {};\n\n  Json::Value "
       "objectTypeValue = value.get(szName, Json::Value::null);\n  if (objectTypeValue == Json::Value::null)\n    "
       "objectTypeValue = \"\";\n  if (!objectTypeValue.isString())\n    return {};\n\n  return "
       "objectTypeValue.asString();"},
      {"temperature", 0.5},
      {"max_tokens", 500},
      {"top_p", 1},
      {"frequency_penalty", 0},
      {"presence_penalty", 0}
  })";

    std::string_view host = "postman-echo.com";
    std::string_view port = "443";
    std::string_view target = "/post";
    std::string_view api_key = "INVALID KEY";
    int version = 11;               // HTTP/1.1

    auto result = send_text_to_gpt(host, port, target, api_key, json_text, version);

    // Verify the response contains the sent JSON data
    expect(result.has_value()) << "Expected a valid result";
    expect(result->find("ASSERT(value.isMember(szName)") != std::string::npos) << result.value() << "\nThe response should contain the sent JSON data\n";
  };
  }
