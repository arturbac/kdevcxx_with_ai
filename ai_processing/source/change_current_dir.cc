#include <aiprocess/change_current_dir.h>
#include <aiprocess/log.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace aiprocess
  {
auto change_current_directory(std::string_view new_directory) noexcept -> expected<void, change_directory_error>
  {
  try
    {
    fs::current_path(fs::path{new_directory});
    return {};
    }
  catch(std::filesystem::filesystem_error const & ec)
    {
    return unexpected_error(
      change_directory_error::filesystem_error,
      "filesystem exception during creation of directory {} {}",
      new_directory,
      ec.what()
    );
    }
  catch(...)
    {
    return unexpected_error(
      change_directory_error::unhandled_exception, "Unhandled exception during creation of directory {}", new_directory
    );
    }
  }
  }  // namespace aiprocess
