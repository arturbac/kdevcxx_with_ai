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
enum class get_view_file_path_error
  {
  no_document,
  unhandled_exception
  };

[[nodiscard]]
auto get_view_file_path(KTextEditor::View const & view) noexcept -> expected<std::string, get_view_file_path_error>;

auto process_with_ai(KTextEditor::View & view, aiprocess::app_settings_t const & settings) noexcept -> void;
  }  // namespace kdevcxxai
