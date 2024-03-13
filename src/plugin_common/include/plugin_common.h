// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#ifndef Q_MOC_RUN
#include <expectd.h>
#endif

namespace aiprocess
  {
struct app_settings_t;
  }

namespace kdevcxxai
  {
inline constexpr auto fist_time_message
  = "Kindly navigate to the KDevelop settings to provide your API key for activation.\n"
    " Additionally, fine-tune the AI behavior by adjusting the rule set to your preference.\n"
    "Modifications to the API key or rules can be made at any moment and will be applied immediately,\n"
    "negating the necessity for a KDevelop restart.\n"
    "Alterations to AI settings will be effective upon the next invocation.";

enum class get_view_file_path_error
  {
  no_document,
  unhandled_exception
  };

[[nodiscard]]
auto get_view_file_path(KTextEditor::View const & view) noexcept -> expected<std::string, get_view_file_path_error>;

auto process_with_ai(KTextEditor::View & view, aiprocess::app_settings_t const & settings) noexcept -> void;
  }  // namespace kdevcxxai
