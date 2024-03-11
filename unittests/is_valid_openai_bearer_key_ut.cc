#include <ai_processing.h>
#include <boost/ut.hpp>
#include <string>

using namespace aiprocess;

// Unit tests
int main()
  {
  using namespace boost::ut;

  "is_valid_openai_bearer_key"_test = []
  {
    expect(is_valid_openai_bearer_key("0123456789ABCDEF0123456789ABCDEF01234567") == true);
    expect(is_valid_openai_bearer_key("0123456789ABCDEF0123456789ABCDEF0123456") == false);             // Too short
    expect(is_valid_openai_bearer_key("") == false);                                                    // Empty string
    expect(is_valid_openai_bearer_key("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") == true);  // Mixed case
  };
  }
