// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#include <aiprocess/universal_config.h>
#include <aiprocess/log.h>

#include <glaze/glaze.hpp>
#include <expectd.h>

namespace aiprocess
  {
enum struct log_general_log_e : bool
  {
  no,
  yes
  };

inline constexpr glz::opts pretty{.prettify = true, .layout = glz::colwise};

template<concepts::unversal_config_compatibile config_type>
struct unversal_config_io_t
  {
  using config_version_enum = std::remove_cvref_t<decltype(std::declval<config_type>().version)>;
  static constexpr config_version_enum expected_version{config_type::expected_version};

  /// loads and updates file if required and stores when not existed or format was deprecated
  template<log_general_log_e ltg = log_general_log_e::no>
  [[nodiscard]]
  static auto operator()(string_view config_path) -> expected<config_type, unversal_config_io_error_e>
    {
    using enum unversal_config_io_error_e;

    if constexpr(expected_version < config_version_enum::latest)
      {
      auto constexpr msg{"reading confgiuration version {} but newer is already supported {} for file {} using "
                         "different config struct type"};
      if constexpr(ltg == log_general_log_e::yes)
        warn(msg, enum_name(expected_version), enum_name(config_version_enum::latest), config_path);
      else
        li::warn(msg, enum_name(expected_version), enum_name(config_version_enum::latest), config_path);
      }

    config_type cfg{};
    bool need_save_cfg{};

    auto previous_version{expected_version};
    bool const existed_before{std::filesystem::exists(config_path)};
    if(existed_before) [[likely]]
      {
      glz::parse_error res{
        glz::read_file_json<glz::opts{.error_on_unknown_keys = false}>(cfg, std::string{config_path}, std::string{})
      };
      if(res) [[unlikely]]
        {
        auto constexpr msg{"error during parsing configuration file {}: {}"};
        if constexpr(ltg == log_general_log_e::yes)
          error(msg, config_path, enum_name(res.ec));
        else
          li::error(msg, config_path, enum_name(res.ec));
        return unexpected{error_parsing_file};
        }

      if(expected_version < cfg.version) [[unlikely]]
        {
        auto constexpr msg{"read version {} of {} is newer than expected {}"};
        if constexpr(ltg == log_general_log_e::yes)
          error(msg, enum_name(cfg.version), config_path, enum_name(expected_version));
        else
          li::error(msg, enum_name(cfg.version), config_path, enum_name(expected_version));
        return unexpected{error_config_from_newer_version_of_program};
        }

      previous_version = cfg.version;
      if(cfg.version < expected_version)
        {
        need_save_cfg = true;
        cfg.version = expected_version;
        }
      }
    else
      need_save_cfg = true;

    if(need_save_cfg) [[unlikely]]
      {
      bool error_saving{};
      if(existed_before)
        {
        auto save_res{operator()<ltg, level::warn>(cfg, config_path)};
        error_saving = !save_res.has_value();
        }
      else
        {
        auto save_res{operator()<ltg, level::err>(cfg, config_path)};
        error_saving = !save_res.has_value();
        // if file dint exist and we can not save just fail
        if(!save_res.has_value())
          return unexpected{std::move(save_res.error())};
        }
      if(!error_saving && previous_version != expected_version)  // file existed and changed
        {
        auto constexpr msg{
          "configuration file {} has been updated from version {} to {} check and correct changed elements"
        };
        if constexpr(ltg == log_general_log_e::yes)
          warn(msg, config_path, enum_name(previous_version), enum_name(expected_version));
        else
          li::warn(msg, config_path, enum_name(previous_version), enum_name(expected_version));
        }
      }

    return cfg;
    }

  // stores given configuration file
  template<log_general_log_e ltg = log_general_log_e::no, level::level_enum lvl = level::err>
  [[nodiscard]]
  static auto operator()(config_type const & cfg, string_view config_path) -> expected<void, unversal_config_io_error_e>
    {
    using enum unversal_config_io_error_e;
    glz::write_error err{glz::write_file_json<pretty>(cfg, std::string{config_path}, std::string{})};
    if(err) [[unlikely]]
      {
      auto constexpr msg{"error during storing updated configuration file {}: {}"};
      if constexpr(ltg == log_general_log_e::yes)
        log(lvl, msg, config_path, enum_name(err.ec));
      else
        li::log(lvl, msg, config_path, enum_name(err.ec));
      if(lvl == level::err)
        return unexpected{error_creating_file};
      }
    return {};
    }
  };
  }  // namespace aiprocess
