#include <aiprocess/log.h>
#include <aiprocess/app_settings.h>

#include <stralgo/stralgo.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <boost/stacktrace.hpp>
#pragma clang diagnostic pop

namespace aiprocess
  {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wglobal-constructors"

std::shared_ptr<spdlog::logger> general_logger;    // NOLINT ...
std::shared_ptr<spdlog::logger> snippet_logger;    // NOLINT ...
std::shared_ptr<spdlog::logger> important_logger;  // NOLINT ...

static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console_sink;                          // NOLINT ...
static std::shared_ptr<spdlog::sinks::basic_file_sink_mt> file_general_sink, file_important_sink,  // NOLINT ...
  file_snippet_sink;                                                                               // NOLINT ...
#pragma clang diagnostic pop

static auto create_log_sink(std::string const & path, std::string const & filename)
  -> std::shared_ptr<spdlog::sinks::basic_file_sink_mt>
  {
  return std::make_shared<spdlog::sinks::basic_file_sink_mt>(stralgo::stl::merge(path, '/', filename), false);
  }

static std::shared_ptr<spdlog::logger> create_logger(std::string name, spdlog::sinks_init_list sinks)
  {
  auto logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
  logger->flush_on(spdlog::level::err);
  return logger;
  }

void setup_loggers(app_settings_t const & cfg)
  {
  std::string app_path{get_config_path()};
  std::string log_path{stralgo::stl::merge(app_path, '/', cfg.log_path)};

  console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_level(cfg.console_log_level);
  console_sink->set_pattern(cfg.console_log_pattern);

  file_general_sink = create_log_sink(log_path, "general.log");
  file_important_sink = create_log_sink(log_path, "important.log");
  file_snippet_sink = create_log_sink(log_path, "snippet.log");

  console_sink->set_level(cfg.console_log_level);
  file_general_sink->set_level(cfg.general_log_level);
  file_important_sink->set_level(cfg.important_log_level);
  file_snippet_sink->set_level(cfg.snippet_log_level);

  spdlog::set_pattern(cfg.general_log_pattern);

  general_logger = create_logger("", {console_sink, file_general_sink});
  snippet_logger = create_logger("snippet", {console_sink, file_snippet_sink, file_general_sink});
  important_logger = create_logger("important", {console_sink, file_important_sink, file_general_sink});

  spdlog::set_default_logger(general_logger);
  spdlog::flush_every(std::chrono::seconds(cfg.flush_every));

  general_logger->set_level(cfg.general_log_level);
  important_logger->set_level(cfg.important_log_level);
  snippet_logger->set_level(cfg.snippet_log_level);
  }
  }  // namespace aiprocess
