// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#include <ktexteditor/configpage.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qplaintextedit.h>
#include <functional>

#ifndef Q_MOC_RUN
#include <expectd.h>
#include <aiprocess/app_settings.h>
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

namespace config_page
  {
  struct ui_t
    {
    QPlainTextEdit * language_rules;
    QLineEdit * openai_key;
    QLineEdit * open_ai_model;

    QLineEdit * log_path_edit;
    QLineEdit * console_log_pattern_edit;
    QLineEdit * general_log_pattern_edit;
    QLineEdit * snippet_log_pattern_edit;
    QLineEdit * important_log_pattern_edit;

    QComboBox * console_log_level_combo;
    QComboBox * general_log_level_combo;
    QComboBox * snippet_log_level_combo;
    QComboBox * important_log_level_combo;

    QSpinBox * flush_every_spin;
    QSpinBox * activation_keys_spin;
    };

  template<aiprocess::backend_type_e>
  auto construct(KTextEditor::ConfigPage &, ui_t &, std::function<void()>) -> void;

  extern template auto
    construct<aiprocess::backend_type_e::kdevelop>(KTextEditor::ConfigPage &, ui_t &, std::function<void()>) -> void;
  extern template auto
    construct<aiprocess::backend_type_e::kate>(KTextEditor::ConfigPage &, ui_t &, std::function<void()>) -> void;

  template<aiprocess::backend_type_e>
  auto apply(ui_t &) -> void;

  extern template auto apply<aiprocess::backend_type_e::kdevelop>(ui_t &) -> void;
  extern template auto apply<aiprocess::backend_type_e::kate>(ui_t &) -> void;

  template<aiprocess::backend_type_e>
  auto reset(ui_t &) -> void;

  extern template auto reset<aiprocess::backend_type_e::kdevelop>(ui_t &) -> void;
  extern template auto reset<aiprocess::backend_type_e::kate>(ui_t &) -> void;

  auto defaults(ui_t &) -> void;
  }  // namespace config_page
  }  // namespace kdevcxxai
