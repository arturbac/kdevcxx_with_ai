#include <aiprocess/change_current_dir.h>
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
  catch(std::filesystem::filesystem_error const &)
    {
    return unexpected{change_directory_error::filesystem_error};
    }
  catch(...)
    {
    return unexpected{change_directory_error::unhandled_exception};
    }
  }
  }
