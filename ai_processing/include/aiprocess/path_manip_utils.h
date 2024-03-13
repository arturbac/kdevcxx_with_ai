#pragma once

#include <stralgo/stralgo.h>
#include <small_vectors/basic_fixed_string.h>

namespace aiprocess::stringutils
  {
using small_vectors::cast_fixed_string;

template<typename decl_chr_type>
struct cast_string_t
  {
  template<typename char_type, std::size_t N>
  [[nodiscard]]
  static constexpr auto operator()(char_type const (&str)[N]) noexcept
    {
    return cast_fixed_string<char_type>(str);
    }
  };

namespace concepts
  {
  using ::stralgo::concepts::char_range;
  using ::stralgo::concepts::same_range_type;
  }  // namespace concepts

struct path_with_file_name_t
  {
  template<concepts::char_range string_view_type, concepts::char_range string_view_type2>
    requires concepts::same_range_type<string_view_type, string_view_type2>
  [[nodiscard]]
  static constexpr auto operator()(string_view_type const & path, string_view_type2 const & filename)
    {
    using char_type = std::ranges::range_value_t<string_view_type>;
    using string_type = std::basic_string<char_type>;

    if(!path.empty()) [[likely]]
      {
      if(::stralgo::ends_with(path, cast_string_t<char_type>{}("/")))
        return ::stralgo::stl::merge(path, filename);
      else
        return ::stralgo::stl::merge(path, '/', filename);
      }
    return string_type{filename};
    }
  };

inline constexpr path_with_file_name_t path_with_file_name;

struct path_with_slash_t
  {
  template<concepts::char_range string_view_type>
  [[nodiscard]]
  static constexpr auto operator()(string_view_type const & path)
    {
    using char_type = std::ranges::range_value_t<string_view_type>;
    using string_type = std::basic_string<char_type>;

    if(!path.empty()) [[likely]]
      {
      if(::stralgo::ends_with(path, cast_string_t<char_type>{}("/")))
        return string_type{path};
      else
        return ::stralgo::stl::merge(path, char_type('/'));
      }
    return string_type{};
    }
  };

inline constexpr path_with_slash_t path_with_slash;
  }  // namespace aiprocess::stringutils
