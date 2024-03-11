// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#include <concepts>
#include <aiprocess/string_view.h>

namespace aiprocess::concepts
  {
template<typename type>
concept enum_concept = std::is_enum_v<type>;

template<typename type>
concept enum_with_meta_name = enum_concept<type> && requires(type e) {
  { enum_name(e) } -> std::convertible_to<std::string_view>;
};

template<typename version_enum_type>
concept version_enum = requires {
  requires enum_concept<version_enum_type>;
  version_enum_type::latest;  // enum latest is valid enumeration for given enum
};

/// Concept to check if a type is a reference to a version enum
template<typename version_enum_type>
concept const_refrence_to_version_enum = requires { requires version_enum<std::remove_cvref_t<version_enum_type>>; };

/// Concept to check if a configuration type is universally compatible
template<typename config_type>
concept unversal_config_compatibile = requires(config_type const & cfg) {
  requires(not std::is_polymorphic_v<config_type>);
    { cfg.version } -> const_refrence_to_version_enum;
      { config_type::expected_version } -> const_refrence_to_version_enum;
      requires std::same_as<
        std::remove_cvref_t<decltype(cfg.version)>,
        std::remove_cvref_t<decltype(config_type::expected_version)>>;
    // { simple_enum::enum_name(cfg.version) } -> std::same_as<string_view>;
};
  }  // namespace aiprocess::concepts
