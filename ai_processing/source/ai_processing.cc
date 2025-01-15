// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#ifndef Q_MOC_RUN
#include "ai_processing.h"
#include "network.h"
#include <aiprocess/change_current_dir.h>
#include <aiprocess/clang_format.h>
#include <aiprocess/trim_white_space.h>
#include <aiprocess/remove_pattern.h>
#include <aiprocess/app_settings.h>
#include <aiprocess/log.h>

#include <fmt/core.h>
#include <simple_enum/simple_enum.hpp>
#include <glaze/json/write.hpp>
#include <glaze/json/read.hpp>
#include <stralgo/strconv_numeric.h>
#include <cctype>
#include <algorithm>
#include <ranges>

// 1. Text Completion (/v1/completions)
//
//     Purpose: Generate text completions or continuations based on a prompt.
//     Engines: GPT-3, GPT-3.5, and potentially newer versions like GPT-4 if available.
//
// 2. Chat (/v1/chat/completions)
//
//     Purpose: Have a conversational interaction. This endpoint is optimized for chat-like exchanges.
//     Engines: GPT-3.5-turbo and newer chat-optimized models.
//
// 3. Edits (/v1/edits)
//
//     Purpose: Modify or improve existing text based on instructions or corrections.
//     Engines: Text-editing models like instruct-gpt.
//
// 4. Image Generation (/v1/images/generations)
//
//     Purpose: Create images from textual descriptions.
//     Engines: DALL·E, DALL·E 2, and potentially newer versions for high-quality image generation.
//
// 5. Image Editing (/v1/images/edits)
//
//     Purpose: Edit existing images based on textual descriptions.
//     Engines: DALL·E 2 for tasks like adding elements to images, changing styles, or altering content according to
//     descriptions.
//
// 6. Image-to-Image (/v1/images/variations)
//
//     Purpose: Generate variations of an input image.
//     Engines: DALL·E 2 for creating variations of provided images.
//
// 7. Embeddings (/v1/embeddings)
//
//     Purpose: Generate embeddings (vector representations) of text or images for use in machine learning applications.
//     Engines: Various models optimized for generating embeddings, supporting both text and image inputs.
//
// 8. Classifications (/v1/classifications)
//
//     Purpose: Categorize text into predefined labels.
//     Engines: Fine-tuned versions of GPT and other models optimized for classification tasks.
//
// 9. Answers (/v1/answers)
//
//     Purpose: Generate answers to questions based on provided documents or knowledge.
//     Engines: GPT-3 and other advanced models trained for Q&A tasks.
//
// 10. Moderations (/v1/moderations)
//
//     Purpose: Identify potentially unsafe or problematic content within text.
//     Engines: Specialized moderation models designed to flag content issues.
namespace aiprocess
  {

static constexpr std::string_view command_start_delim = "[AI";
static constexpr std::string_view command_end_delim = "]";

using namespace std::string_view_literals;

// gpt-3.5-turbo
// gpt-3.5-turbo-instruct
// gpt-4
// gpt-4-turbo-preview
// gpt-4-1106-preview
#ifndef ENABLE_CHAT_COMPLETIONS
struct ai_command_json
  {
  std::string model;
  std::string prompt;
  double temperature{0.5};
  uint16_t max_tokens = 4096;
  double top_p{1.0};
  double frequency_penalty{0.0};
  double presence_penalty{0.0};
  };
#else

struct ai_chat_command_json
  {
  std::string model;
  std::array<message_t, 2> messages;
  double temperature{1.0};
  uint16_t max_tokens = 4096;
  double top_p{0.95};
  double frequency_penalty{0.0};
  double presence_penalty{0.0};
  };

#endif

auto is_valid_openai_bearer_key(std::string const & key) noexcept -> bool
  {
  // Example criteria: key should be 41 characters long and only contain alphanumeric characters
  constexpr size_t expected_length = 40;
  if(key.length() < expected_length) [[unlikely]]
    return false;

  // Check if all characters in the key are alphanumeric using std::ranges::all_of
  return std::ranges::all_of(key, [](unsigned char c) { return std::isprint(c); });
  }

// static std::string_view api_url{"https://api.openai.com/v1/engines/code-davinci-002/completions"};
// https://platform.openai.com/docs/deprecations

auto process_with_ai(std::string && user_data) -> expected<model_response_text_t, process_with_ai_error>
try
  {
  using enum process_with_ai_error;

  aiprocess::ai_settings_t aisettings{aiprocess::load_ai_settings()};
  info("checking api key ..");
  if(!is_valid_openai_bearer_key(aisettings.api_key)) [[unlikely]]
    return unexpected_error(invalid_api_key, "invalid key bailing out");

  if(aisettings.gpt_model.empty()) [[unlikely]]
    return unexpected_error(no_gpt_model_specified, "No gpt model specified in settings");

  auto start_pos = user_data.find(command_start_delim);
  auto end_pos = user_data.find(command_end_delim, start_pos);

  if(start_pos == std::string::npos || end_pos == std::string::npos) [[unlikely]]
    return unexpected_error(no_valid_command, "AI command was not formatted coretly within [AI block]");

  std::string command_text
    = user_data.substr(start_pos + command_start_delim.length(), end_pos - (start_pos + command_start_delim.length()))
        .erase(0, 1);

  if(command_text.empty()) [[unlikely]]
    return unexpected_error(no_valid_command, "command_text is empty nothing to do ..");

  std::string code_text = user_data.substr(end_pos + command_end_delim.length());

#ifndef ENABLE_CHAT_COMPLETIONS
  ai_command_json command{.prompt = fmt::format("[{},{}] {}", ai_rules, command_text, code_text)};
  size_t const aprox_tokens{code_text.size() / 2 + command_text.size()};
#else
  ai_chat_command_json command{};
  command.model = aisettings.gpt_model;
  command.messages[0].role = "system";
  command.messages[0].content = aisettings.cxx_rules;
  command.messages[1].role = "user";
  command.messages[1].content = stralgo::stl::merge('[', command_text, "]\n"sv, code_text);
  size_t const aprox_tokens{code_text.size() / 2 + command_text.size()};
#endif
  if(command.max_tokens < aprox_tokens)
    command.max_tokens = uint16_t(std::min<uint32_t>(4096u, uint32_t(aprox_tokens)));
  // Serialize the object to a JSON string
  std::string serialized = glz::write_json(command);
  if(serialized.empty()) [[unlikely]]
    return unexpected_error(
      json_serialization_error,
      "json serialization failed of command.. system {} user {}",
      command.messages[0].content,
      command.messages[1].content
    );
  // https://openai.com/blog/new-models-and-developer-products-announced-at-devday

#ifndef ENABLE_CHAT_COMPLETIONS
  auto res{send_text_to_gpt(
    "api.openai.com", "443", "/v1/completions", aisettings.api_key, aiprocess::trim_white_space(serialized), 11
  )};
#else
  auto res{send_text_to_gpt(
    "api.openai.com", "443", "/v1/chat/completions", aisettings.api_key, aiprocess::trim_white_space(serialized), 11
  )};
#endif
  // auto res{send_text_to_gpt("api.openai.com", "443", "/v1/engines/gpt-3.5-turbo/completions", api_key, serialized,
  // 11)};
  if(res)
    {
    debug("AI: response recived");
    return model_response_text_t{
      .command = std::move(command_text), .send_text = code_text, .recived_text = std::move(res.value())
    };
    }
  else
    return unexpected_error(other_error, "AI: send_text_to_gpt returned error {}", res.error());
  }
catch(...)
  {
  return unexpected{process_with_ai_error::other_error};
  }

using namespace std::string_view_literals;
static constexpr glz::opts default_json_parse_opts{.error_on_unknown_keys = false, .skip_null_members = true};

auto parse_json_choices(std::string_view response_json_data, std::string && clang_format_working_directory)
  -> std::string
  {
  model_response_t mr;
  glz::context ctx{};
  auto result{glz::read<glz::set_json<default_json_parse_opts>()>(mr, std::string{response_json_data}, ctx)};
  if(result.ec == glz::error_code::none && !mr.id.empty())  // if id is empty then it is invalid json for sure
    {
    auto fn_format_choice = [&](std::string && str, model_choice_data_t const & mcd) -> std::string
    {
#ifndef ENABLE_CHAT_COMPLETIONS
      auto unformatted{stralgo::stl::compose(
        // clang-format off
        "\t // ["sv, choice_nr, "]\n"sv,
        "\t"sv, mcd.text, '\n'
        // clang-format on
      )};
#else
      auto unformatted{stralgo::stl::compose(
        // clang-format off
        "\t"sv, aiprocess::remove_pattern(aiprocess::remove_pattern(mcd.message.content, "```cpp\n"sv), "```"sv), '\n'
        // clang-format on
      )};
#endif
      auto formatted{aiprocess::clang_format(unformatted, clang_format_working_directory)};
      if(formatted.has_value())
        str.append(formatted.value());
      else
        {
        li::error("Recived error from clang format continuing with unformatted code ..", formatted.error());
        str.append(unformatted);
        }
      return str;
    };

    return stralgo::stl::compose(
      // clang-format off
      "/* id : "sv, mr.id, " object : "sv, mr.object, " created : "sv, mr.created, " model : "sv, mr.model, "\n"
      " choices :\n"
      "*/\n"
      "// AI CODE BLOCK BEGIN\n"sv,
        small_vectors::ranges::accumulate( mr.choices, std::string{}, fn_format_choice ),
      "// AI CODE BLOCK END\n"sv
      // clang-format on
    );
    }
  else
    {
    return stralgo::stl::merge(
      // clang-format off
      "parsing json error ["sv, simple_enum::enum_name(result.ec), "] "sv,
      result.includer_error,'\n',
      "response_json_data:\n"sv, response_json_data
      // clang-format on
    );
    }
  }

auto process_openai_json_response(model_response_text_t const & data, std::string && clang_format_working_directory)
  -> std::string
  {
  if(data.send_text.empty())
    return stralgo::stl::merge(
      // clang-format off
      "\n"
      "// executed [AI "sv, data.command,"]\n"sv,
      parse_json_choices(data.recived_text, std::move(clang_format_working_directory)), '\n',
      "#endif\n"sv
      // clang-format on
    );
  else
    return stralgo::stl::merge(
      // clang-format off
      "\n// executed [AI "sv, data.command," ]\n"
      "//-----------------------------------------------------------------------------\n"sv,
      parse_json_choices(data.recived_text, std::move(clang_format_working_directory)), '\n',
      "//-----------------------------------------------------------------------------\n"
      "#if 0\n"sv,
      data.send_text, '\n',
      "#endif\n"sv
      // clang-format on
    );
  }
  }  // namespace aiprocess
#endif

