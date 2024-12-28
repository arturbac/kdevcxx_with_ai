// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#ifdef Q_MOC_RUN
#error "Dont process this with moc, use #ifndef Q_MOC_RUN"
#endif
#include <string>
#include "expectd.h"
#include <optional>
#include <simple_enum/simple_enum.hpp>
#include <small_vectors/small_vector.h>

#define ENABLE_CHAT_COMPLETIONS

namespace aiprocess
  {
namespace sv = small_vectors;
// https://platform.openai.com/docs/models/gpt-4-and-gpt-4-turbo
// Model endpoint compatibility
// Endpoint	Latest models

// /v1/assistants	All GPT-4 and GPT-3.5 Turbo models except gpt-3.5-turbo-0301 supported. The retrieval tool requires
// gpt-4-turbo-preview (and subsequent dated model releases) or gpt-3.5-turbo-1106 (and subsequent versions).
// /v1/audio/transcriptions	whisper-1
// /v1/audio/translations	whisper-1
// /v1/audio/speech	tts-1, tts-1-hd

// /v1/chat/completions	gpt-4 and dated model releases, gpt-4-turbo-preview and dated model releases,
// gpt-4-vision-preview, gpt-4-32k and dated model releases, gpt-3.5-turbo and dated model releases, gpt-3.5-turbo-16k
// and dated model releases, fine-tuned versions of gpt-3.5-turbo /v1/completions (Legacy)	gpt-3.5-turbo-instruct,
// babbage-002, davinci-002 /v1/embeddings	text-embedding-3-small, text-embedding-3-large, text-embedding-ada-002
// /v1/fine_tuning/jobs	gpt-3.5-turbo, babbage-002, davinci-002
// /v1/moderations	text-moderation-stable, text-moderation-latest
// /v1/images/generations	dall-e-2, dall-e-3

/// @brief Enumeration for \ref process_with_ai errors.
enum struct process_with_ai_error
  {
  other_error,               ///< An unspecified error.
  invalid_api_key,           ///< The provided API key is invalid.
  no_gpt_model_specified,    ///< Gpt model value from settings is empty.
  json_serialization_error,  ///< Error occurred during JSON serialization.
  no_valid_command           ///< No valid command was found in the request.
  };

consteval auto adl_enum_bounds(process_with_ai_error)
  {
  using enum process_with_ai_error;
  return simple_enum::adl_info{.first = other_error, .last = no_valid_command};
  }

struct model_response_text_t
  {
  std::string command;
  std::string send_text;
  std::string recived_text;
  };

/**
 * @brief Prepares and sends a request to OpenAI using user input.
 *
 * @param user_data A string representing data provided by the user.
 * @return An expected object containing either the model's response text
 *         or an error code with an associated error enumeration.
 *
 * @details This function takes the user input, ensures it contains a valid
 *          AI command, and sends a request to OpenAI's API. It then
 *          processes the API response, returning either the result or
 *          an error.
 * @ref user_data
 */
auto process_with_ai(std::string && user_data) -> expected<model_response_text_t, process_with_ai_error>;

/**
 * @brief Processes choices from OpenAI's JSON response
 *
 * @param response_json_data The JSON string choices from json containing the response from OpenAI.
 * @param clang_format_working_directory An optional working directory to run clang-format in.
 * @return std::string The processed choice text.
 */
[[nodiscard]]
auto parse_json_choices(
  std::string_view response_json_data, std::string && clang_format_working_directory = std::string{}
) -> std::string;

// AI CODE BLOCK BEGIN
/**
 * Processes the response received from OpenAI.
 *
 * @param data The JSON response text from OpenAI as a model-specific type.
 * @param clang_format_working_directory Temporary working directory for clang-format to get context of .clang-format.
 * @return A processed string based on OpenAI's response.
 */
[[nodiscard]]
auto process_openai_json_response(model_response_text_t const & data, std::string && clang_format_working_directory)
  -> std::string;

/**
 * @struct message_t
 * @brief This structure represents a message with a role and content.
 */
struct message_t
  {
  std::string role;     ///< Role of the sender of the message.
  std::string content;  ///< Actual content of the message.
  };

struct model_choice_data_t
  {
#ifndef ENABLE_CHAT_COMPLETIONS
  std::string text;  // Generated text by the model.
#else
  message_t message;  // Message object representing the generated content.
#endif
  std::string finish_reason;       // Reason for the model's stopping point ('length' or 'stop').
  std::size_t index;               // Index of the choice, starting from 0.
  std::optional<double> logprobs;  // Optional log probabilities for tokens in the completion.
  };

struct model_usage_t
  {
  uint32_t prompt_tokens;
  uint32_t completion_tokens;
  uint32_t total_tokens;
  };

/**
 * @struct model_response_t
 * @brief A structure representing the response from a model request.
 *
 * @details This structure contains data such as the unique identifier, the type of object,
 * the creation timestamp, the model used, an array of choices given by the model, and the model usage information.
 */
struct model_response_t
  {
  using choices_vector = sv::small_vector<model_choice_data_t, uint8_t, sv::at_least<model_choice_data_t, uint8_t>(1u)>;
  std::string id;          ///< Unique identifier for the request.
  std::string object;      ///< Type of object returned, usually "text_completion".
  uint64_t created;        ///< Unix timestamp of when the response was created.
  std::string model;       ///< Specifies the model used for generating the response.
  choices_vector choices;  ///< Array of completion choices provided by the model.
  model_usage_t usage;     ///< Information about the usage of the model.
  };

auto is_valid_openai_bearer_key(std::string const & key) noexcept -> bool;
  }  // namespace aiprocess
