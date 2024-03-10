#include <aiprocess/app_settings.h>
#include <aiprocess/universal_config.hpp>
#include <aiprocess/log.h>
#include <filesystem>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <stralgo/stralgo.h>

namespace fs = std::filesystem;

namespace aiprocess
  {
auto load_app_settings() noexcept -> app_settings_t
  {
  auto full_path{stralgo::stl::merge(get_config_path(), '/', kdevcxx_with_ai_app_settings_file_name)};
  auto cfg{unversal_config_io_t<app_settings_t>::operator()<log_general_log_e::yes>(full_path)};
  if(cfg.has_value()) [[likely]]
    return cfg.value();
  return {};
  }

auto get_config_path() noexcept -> std::string
  {
  char const * home_dir;
  if(home_dir = getenv("HOME"); home_dir == nullptr)
    home_dir = getpwuid(getuid())->pw_dir;
  if(home_dir == nullptr)
    aiprocess::log(level::err, "Cannot get HOME directory");

  if(!fs::exists(home_dir))
    fs::create_directory(home_dir);
  return (fs::path(home_dir) / ".config" / "kdevcxx_with_ai").string();
  }

auto load_ai_settings() noexcept -> ai_settings_t
  {
  auto full_path{stralgo::stl::merge(get_config_path(), '/', kdevcxx_with_ai_ai_settings_file_name)};
  auto cfg{unversal_config_io_t<ai_settings_t>::operator()(full_path)};
  if(cfg.has_value()) [[likely]]
    return cfg.value();

  return {};
  }

auto get_home_directory() noexcept -> std::string
  {
  // Attempt to retrieve the home directory from environment variables
  char const * homeDir = std::getenv("HOME");

#ifdef _WIN32
  if(!homeDir)
    {
    // On Windows, if HOME is not set, try USERPROFILE
    homeDir = std::getenv("USERPROFILE");
    if(!homeDir)
      {
      // If USERPROFILE is not set, construct home path from HOMEDRIVE and HOMEPATH
      char const * drive = std::getenv("HOMEDRIVE");
      char const * path = std::getenv("HOMEPATH");
      if(drive && path)
        {
        std::string home = std::string(drive) + std::string(path);
        return home;  // Already a std::string
        }
      }
    }
#endif

  if(homeDir)
    return std::filesystem::path(homeDir).string();

  // If unable to determine home directory, return an empty string
  return {};
  }
  }  // namespace aiprocess
