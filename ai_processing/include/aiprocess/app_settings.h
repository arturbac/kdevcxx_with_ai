// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#include <aiprocess/universal_config.h>
#include <string>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <spdlog/common.h>
#pragma clang diagnostic pop

namespace aiprocess
  {
inline constexpr string_view kdevcxx_with_ai_app_settings_file_name{"kdevcxx_with_ai_app_settings.json"};
inline constexpr string_view kdevcxx_with_ai_ai_settings_file_name{"kdevcxx_with_ai_ai_settings.json"};

enum struct app_settings_version_e
  {
  v1 = 1,

  latest = v1
  };

consteval auto adl_enum_bounds(app_settings_version_e)
  {
  using enum app_settings_version_e;
  return simple_enum::adl_info{v1, v1};
  }

struct app_settings_t
  {
  std::string log_path{"logs_ai"};
  // https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
  std::string console_log_pattern = "[thread %t] [%n] %v";
  std::string general_log_pattern = "[%H:%M:%S %z] [%^%L%$] [thread %t] [%n] %v";
  std::string snippet_log_pattern = "[%H:%M:%S %z] [%^%L%$] [thread %t] [%n] %v";
  std::string important_log_pattern = "[%H:%M:%S %z] [%^%L%$] [thread %t] [%n] %v";

  using level_enum = spdlog::level::level_enum;
  level_enum console_log_level{level_enum::trace};
  level_enum general_log_level{level_enum::trace};
  level_enum snippet_log_level{level_enum::trace};
  level_enum important_log_level{level_enum::info};

  std::size_t flush_every{3};
  // /usr/include/qt5/QtCore/qnamespace.h
  // ControlModifier      = 0x04000000
  // Key_1 = 0x31,
  int activation_keys{0x04000000 | 0x31};

  app_settings_version_e version{simple_enum::limits::max<app_settings_version_e>()};
  static constexpr app_settings_version_e expected_version{simple_enum::limits::max<app_settings_version_e>()};
  };

auto load_app_settings() noexcept -> app_settings_t;
auto store_app_settings(app_settings_t const & cfg) noexcept -> bool;

enum struct ai_settings_version_e
  {
  v1 = 1,
  v2,
  latest = v2
  };

consteval auto adl_enum_bounds(ai_settings_version_e)
  {
  using enum ai_settings_version_e;
  return simple_enum::adl_info{v1, latest};
  }

//
#if 1
// executed [AI below string is declaration of rules for openai gpt compress them without changing meaning but for
// optimising network connection and improving ai use] AI CODE BLOCK BEGIN
inline constexpr std::string_view default_ai_rules{
  "Great c++23 coder, prefers std::ranges/views over loops, uses nodiscard and constexpr,"
  " favors short/fast code with trailing return, adheres to lower_case, uses boost-ext/ut for tests,"
  " code-only responses without explanations, unless requested, comments with // or /* */."
};

// AI CODE BLOCK END

#else

inline constexpr std::string_view default_ai_rules{
  "You are great c++23 coder, preferring std::ranges and std::views over while and for loops,"
  " using nodiscard attibute where appropriate, when possible You use constexpr, you prefer short and fast code,"
  " when writing function you are using trailing return type,"
  " using lower_case convention always, if implementing unit tests You use boost-ext/ut,"
  " you don't produce explanations unless asked for, you return only code unless being asked for something else"
  "when commenting code You use c++ comments like // or for comments block /* */"
};
#endif

inline constexpr std::string_view default_gpt_model{"gpt-4-1106-preview"};

/**
 * @struct ai_settings_t
 *
 * @brief Holds the configuration settings for AI interaction.
 *
 * This structure is used to store configuration parameters for AI interaction,
 * including API access and behavior rules.
 */
struct ai_settings_t
  {
  /// API key for authentication with the AI service.
  std::string api_key{};

  /// Rules that the AI should follow when processing requests.
  std::string cxx_rules{default_ai_rules};

  /// Identifier for the GPT model to be used for AI completions.
  std::string gpt_model{default_gpt_model};

  /// Version of the AI settings currently in use.
  ai_settings_version_e version{simple_enum::limits::max<ai_settings_version_e>()};

  /// Expected version of the AI settings structure.
  static constexpr ai_settings_version_e expected_version{simple_enum::limits::max<ai_settings_version_e>()};
  };

/**
 * Loads AI settings from a default or specified location.
 *
 * @return ai_settings_t containing the settings for the AI.
 */
[[nodiscard]]
auto load_ai_settings() noexcept -> ai_settings_t;

/**
 * Stores AI settings to a default or specified location.
 *
 * @param cfg The AI settings to store.
 * @return True if successful, false otherwise.
 */
auto store_ai_settings(ai_settings_t const & cfg) noexcept -> bool;

/**
 * Retrieves the home directory of the current user.
 *
 * @return A string containing the path to the home directory.
 */
[[nodiscard]]
auto get_home_directory() noexcept -> std::string;

/**
 * Constructs the full path to the configuration file based on the home directory.
 *
 * @return A string containing the full path to the configuration file.
 */
[[nodiscard]]
auto get_config_path() noexcept -> std::string;

  }  // namespace aiprocess
