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

enum struct ai_settings_version_e
  {
  v1 = 1,

  latest = v1
  };

consteval auto adl_enum_bounds(ai_settings_version_e)
  {
  using enum ai_settings_version_e;
  return simple_enum::adl_info{v1, v1};
  }

static constexpr std::string_view default_ai_rules{
  "You are great c++23 coder, prefering std::ranges and std::views over while and for loops,"
  " using nodiscard if needed, you prefer short and fast code, when writing function you are using trailing return,"
  " using lower_case convention always, if implementing  unit tests You use boost-ext/ut,"
  " you dont produce explanations unless asked for, you always return ONLY CODE unless asked for something else"
};

struct ai_settings_t
  {
  std::string api_key{};
  std::string cxx_rules{default_ai_rules};

  ai_settings_version_e version{simple_enum::limits::max<ai_settings_version_e>()};
  static constexpr ai_settings_version_e expected_version{simple_enum::limits::max<ai_settings_version_e>()};
  };

auto load_ai_settings() noexcept -> ai_settings_t;

auto get_home_directory() noexcept -> std::string;
auto get_config_path() noexcept -> std::string;

  }  // namespace aiprocess
