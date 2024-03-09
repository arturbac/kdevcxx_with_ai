#ifndef Q_MOC_RUN
#include "ai_processing.h"
#include "network.h"
#include <aiprocess/change_current_dir.h>
#include <aiprocess/clang_format.h>

#include <fmt/core.h>
#include <simple_enum/simple_enum.hpp>
#include <glaze/json/write.hpp>
#include <glaze/json/read.hpp>
#include <stralgo/strconv_numeric.h>
#include <temp_auth_data.h>
#include <aiprocess/trim_white_space.h>

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
static constexpr std::string_view command_start_delim = "[AI";
static constexpr std::string_view command_end_delim = "]";
static constexpr std::string_view ai_rules{
  "Use C++20, prefer std::ranges and std::views over for loops, use nodiscard, use trailing return, use lower_case "
  "convention always"
};

// gpt-3.5-turbo
// gpt-3.5-turbo-instruct
// gpt-4
// gpt-4-turbo-preview
// gpt-4-1106-preview
struct ai_command_json
  {
  std::string model{"gpt-3.5-turbo-instruct"};
  std::string prompt;
  double temperature{0.5};
  uint16_t max_tokens = 500;
  double top_p{1.0};
  double frequency_penalty{0.0};
  double presence_penalty{0.0};
  };

// static std::string_view api_url{"https://api.openai.com/v1/engines/code-davinci-002/completions"};
// https://platform.openai.com/docs/deprecations
expected<model_response_text_t, process_with_ai_error> process_with_ai(std::string && user_data)
try
  {
  using enum process_with_ai_error;
  std::string result;

  fmt::print("\nAI: Selecetd [{}]\n", user_data);
  auto start_pos = user_data.find(command_start_delim);
  auto end_pos = user_data.find(command_end_delim, start_pos);

  if(start_pos == std::string::npos || end_pos == std::string::npos)
    return unexpected{no_valid_command};

  std::string command_text
    = user_data.substr(start_pos + command_start_delim.length(), end_pos - (start_pos + command_start_delim.length()))
        .erase(0, 1);

  if(command_text.empty())
    return unexpected{no_valid_command};

  std::string code_text = user_data.substr(end_pos + command_end_delim.length());

  // prepare json
  ai_command_json command{.prompt = fmt::format("[{},{}] {}", ai_rules, command_text, code_text)};
  size_t const aprox_tokens{code_text.size() / 2 + command_text.size()};
  if(command.max_tokens < aprox_tokens)
    command.max_tokens = uint16_t(aprox_tokens);
  // Serialize the object to a JSON string
  std::string serialized = glz::write_json(command);
  // https://openai.com/blog/new-models-and-developer-products-announced-at-devday

  /// v1/edits
  auto res{
    send_text_to_gpt("api.openai.com", "443", "/v1/completions", api_key, aiprocess::trim_white_space(serialized), 11)
  };

  // auto res{send_text_to_gpt("api.openai.com", "443", "/v1/engines/gpt-3.5-turbo/completions", api_key, serialized,
  // 11)};
  if(res)
    {
    fmt::print("\nAI: response [{}]\n", res.value());
    return model_response_text_t{
      .command = std::move(command_text), .send_text = code_text, .recived_text = std::move(res.value())
    };
    }
  else
    fmt::print("\nAI: response error [{}]\n", simple_enum::enum_name(res.error()));

  return unexpected{other_error};
  }
catch(...)
  {
  return unexpected{process_with_ai_error::other_error};
  }

using namespace std::string_view_literals;
static constexpr glz::opts default_json_parse_opts{.error_on_unknown_keys = false, .skip_null_members = true};

auto parse_json_response(std::string_view response_json_data, std::string && clang_format_working_directory)
  -> std::string
  {
  model_response_t mr;
  glz::context ctx{};
  auto result{glz::read<glz::set_json<default_json_parse_opts>()>(mr, std::string{response_json_data}, ctx)};
  if(result.ec == glz::error_code::none && !mr.id.empty())  // if id is empty en it is invalid json for sure
    {
    std::size_t choice_nr{};
    auto fn_format_choice = [&](std::string && str, model_choice_data_t const & mcd) -> std::string
    {
      auto unformatted{stralgo::stl::compose(
        // clang-format off
        "\t // ["sv, choice_nr, "]\n"sv,
        "\t"sv, mcd.text, '\n'
        // clang-format on
      )};
      auto formatted{aiprocess::clang_format(unformatted, clang_format_working_directory)};
      // TODO use the path of current file name
      if(formatted.has_value())
        str.append(formatted.value());
      else
        str.append(unformatted);
      return str;
    };
    return stralgo::stl::compose(
      // clang-format off
      "/*"sv,
      " id : "sv, mr.id, '\n',
      " object : "sv, mr.object, '\n',
      " created : "sv, mr.created, '\n',
      " model : "sv, mr.model, '\n',
      " choices :\n"sv,
      "*/"sv,
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

auto process_ai_response(model_response_text_t const & data, std::string && clang_format_working_directory)
  -> std::string
  {
  if(data.send_text.empty())
    return fmt::format(
      R"(
// Command send {}
#if 0
{}
#endif
)",
      data.command,
      parse_json_response(data.recived_text, std::move(clang_format_working_directory))
    );
  else
    return fmt::format(
      R"(
// Command send {}
#if 1
{}
#else
{}
#endif
)",
      data.command,
      data.send_text,
      parse_json_response(data.recived_text, std::move(clang_format_working_directory))
    );
  }

#endif

