#pragma once

#ifdef Q_MOC_RUN
#error "Dont process this with moc, use #ifndef Q_MOC_RUN"
#endif
#include <string>
#include "expectd.h"
#include <optional>

#define ENABLE_CHAT_COMPLETIONS
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
enum struct process_with_ai_error
  {
  other_error,
  no_valid_command
  };

struct model_response_text_t
  {
  std::string command;
  std::string send_text;
  std::string recived_text;
  };

expected<model_response_text_t, process_with_ai_error> process_with_ai(std::string && user_data);

auto parse_json_response(
  std::string_view response_json_data, std::string && clang_format_working_directory = std::string{}
) -> std::string;
auto process_ai_response(model_response_text_t const & data, std::string && clang_format_working_directory)
  -> std::string;
struct message_t
  {
  std::string role;
  std::string content;
  };
struct model_choice_data_t
  {
#ifndef ENABLE_CHAT_COMPLETIONS
  // text: The generated text by the model.
  std::string text;
#else
  message_t message;
#endif
  // finish_reason: The reason why the model stopped generating text, such as reaching a token limit or encountering a
  // stop sequence.
  // "length" indicates a natural conclusion based on the token limit set by the user.
  // "stop" suggests the generation met a user-defined textual condition to end.
  std::string finish_reason;
  // index: The index of the choice, starting from 0.
  std::size_t index;
  // logprobs: Optionally includes the log probabilities for each token in the completion, if requested.
  std::optional<double> logprobs;
  };

struct model_usage_t
  {
  uint32_t prompt_tokens;
  uint32_t completion_tokens;
  uint32_t total_tokens;
  };

struct model_response_t
  {
  /// A unique identifier for the request. This can be used for logging, debugging, or tracking requests and their
  /// responses over time.
  std::string id;  //"cmpl-90eNRqqPnxj7Yh9hWftWCufTH9wMP",
  // The type of object returned, typically "text_completion" for responses related to generating text completions. This
  // field helps in identifying the response's purpose.
  std::string object;  // "text_completion",
  /// A Unix timestamp indicating when the response was created. This can be useful for tracking when requests were made
  /// or for logging purposes.
  uint64_t created;  // 1709942237,
  /// Specifies the model used for generating the response, e.g., "text-davinci-004" for a version of GPT-3.5 or a
  /// similar identifier for GPT-4 models
  std::string model;  // "gpt-3.5-turbo-instruct",
  /// An array containing the completion choices provided by the model. Typically, for straightforward requests, this
  /// array contains a single item, but it can include more based on the request parameters. Each object in this array
  /// can contain several fields:
  std::vector<model_choice_data_t> choices;
  //   "choices": [
  //     {
  // text: The generated text by the model.
  // index: The index of the choice, starting from 0.
  // logprobs: Optionally includes the log probabilities for each token in the completion, if requested.
  // finish_reason: The reason why the model stopped generating text, such as reaching a token limit or encountering a
  // stop sequence.
  //       "text": "...",
  //       "index": 0,
  //       "logprobs": null,
  //       "finish_reason": "stop"
  //     }
  //   ],
  model_usage_t usage;
  //   "usage": {
  //     "prompt_tokens": 155,
  //     "completion_tokens": 94,
  //     "total_tokens": 249
  //   }
  };

// object: The type of object returned, in this case, text_completion.
// created: A Unix timestamp indicating when the completion was created.
// model: Specifies the model used for the completion, here gpt-3.5-turbo-instruct.
// choices: An array of completion options returned by the model. Typically, for straightforward requests, this array

