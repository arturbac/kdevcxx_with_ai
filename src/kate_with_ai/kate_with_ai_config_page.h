// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#include <ktexteditor/configpage.h>
#include <plugin_common.h>

class kate_with_ai_config_page : public KTextEditor::ConfigPage
  {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(kate_with_ai_config_page)

public:
  explicit kate_with_ai_config_page(QWidget * parent = nullptr);
  ~kate_with_ai_config_page() override;

  [[nodiscard]] auto name() const -> QString override;
  [[nodiscard]] auto fullName() const -> QString override;
  [[nodiscard]] auto icon() const -> QIcon override;

  void apply() override;
  void reset() override;
  void defaults() override;

  void emit_changed();

private:
  kdevcxxai::config_page::ui_t ui;
  };

