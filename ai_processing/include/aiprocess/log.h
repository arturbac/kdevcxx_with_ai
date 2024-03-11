// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#include <aiprocess/source_location.h>
#include <aiprocess/concepts.h>
#include <expectd.h>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <spdlog/spdlog.h>
#pragma clang diagnostic pop
#include <simple_enum/simple_enum.hpp>
#include <fmt/core.h>
#include <atomic>
#include <concepts>
#include <chrono>

template<simple_enum::bounded_enum T>
struct fmt::formatter<T> : fmt::formatter<std::string_view>
  {
  template<typename FormatContext>
  auto format(T c, FormatContext & ctx) noexcept
    {
    return fmt::formatter<std::string_view>::format(simple_enum::enum_name(c), ctx);
    }
  };

namespace aiprocess
  {
using simple_enum::enum_name;
struct app_settings_t;

void setup_loggers(app_settings_t const & cfg);
extern std::shared_ptr<spdlog::logger> general_logger, snippet_logger, important_logger;

namespace level = spdlog::level;

template<typename... Args>
using format_string_t = spdlog::format_string_t<Args...>;

template<typename... Args>
inline void log(level::level_enum lvl, format_string_t<Args...> fmt, Args &&... args)
  {
  spdlog::default_logger_raw()->log(lvl, fmt, std::forward<Args>(args)...);
  }

template<typename... Args>
inline void debug(format_string_t<Args...> fmt, Args &&... args)
  {
  aiprocess::log(level::level_enum::debug, fmt, std::forward<Args>(args)...);
  }

template<typename... Args>
inline void info(format_string_t<Args...> fmt, Args &&... args)
  {
  aiprocess::log(level::level_enum::info, fmt, std::forward<Args>(args)...);
  }

template<typename... Args>
inline void warn(format_string_t<Args...> fmt, Args &&... args)
  {
  aiprocess::log(level::level_enum::warn, fmt, std::forward<Args>(args)...);
  }

template<typename... Args>
inline void error(format_string_t<Args...> fmt, Args &&... args)
  {
  aiprocess::log(level::level_enum::err, fmt, std::forward<Args>(args)...);
  }

/// important logging
namespace li
  {
  template<typename... Args>
  inline void log(level::level_enum lvl, format_string_t<Args...> fmt, Args &&... args)
    {
    important_logger->log(lvl, fmt, std::forward<Args>(args)...);
    }

  template<typename... Args>
  inline void info(format_string_t<Args...> fmt, Args &&... args)
    {
    aiprocess::li::log(level::level_enum::info, fmt, std::forward<Args>(args)...);
    }

  template<typename... Args>
  inline void warn(format_string_t<Args...> fmt, Args &&... args)
    {
    aiprocess::li::log(level::level_enum::warn, fmt, std::forward<Args>(args)...);
    }

  template<typename... Args>
  inline void error(format_string_t<Args...> fmt, Args &&... args)
    {
    aiprocess::li::log(level::level_enum::err, fmt, std::forward<Args>(args)...);
    }

  template<typename... Args>
  inline void critical(format_string_t<Args...> fmt, Args &&... args)
    {
    aiprocess::li::log(level::level_enum::err, fmt, std::forward<Args>(args)...);
    }
  }  // namespace li

namespace snpt
  {
  template<typename... Args>
  inline void log(level::level_enum lvl, format_string_t<Args...> fmt, Args &&... args)
    {
    snippet_logger->log(lvl, fmt, std::forward<Args>(args)...);
    }

  template<typename... Args>
  inline void info(format_string_t<Args...> fmt, Args &&... args)
    {
    aiprocess::snpt::log(level::level_enum::info, fmt, std::forward<Args>(args)...);
    }

  template<typename... Args>
  inline void warn(format_string_t<Args...> fmt, Args &&... args)
    {
    aiprocess::snpt::log(level::level_enum::warn, fmt, std::forward<Args>(args)...);
    }

  template<typename... Args>
  inline void error(format_string_t<Args...> fmt, Args &&... args)
    {
    aiprocess::snpt::log(level::level_enum::err, fmt, std::forward<Args>(args)...);
    }

  template<typename... Args>
  inline void critical(format_string_t<Args...> fmt, Args &&... args)
    {
    aiprocess::snpt::log(level::level_enum::err, fmt, std::forward<Args>(args)...);
    }
  }  // namespace snpt

template<typename error_type, typename... Args>
[[nodiscard]]
inline auto
  unexpected_error(error_type err, format_string_t<Args...> fmt, Args &&... args) -> cxx23::unexpected<error_type>
  {
  li::error(fmt, std::forward<Args>(args)...);
  return cxx23::unexpected{err};
  }

template<typename error_type, typename... Args>
[[nodiscard]]
inline auto
  unexpected_critical(error_type err, format_string_t<Args...> fmt, Args &&... args) -> cxx23::unexpected<error_type>
  {
  li::critical(fmt, std::forward<Args>(args)...);
  return cxx23::unexpected{err};
  }
  }  // namespace aiprocess
