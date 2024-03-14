// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#include <ktexteditor/configpage.h>
#include <plugin_common.h>

class kate_with_ai_config_page : public KTextEditor::ConfigPage
  {
  Q_OBJECT

public:
  explicit kate_with_ai_config_page(QWidget * parent = nullptr);

  QString name() const override;
  QString fullName() const override;
  QIcon icon() const override;

  void apply() override;
  void reset() override;
  void defaults() override;

  void emit_changed();

private:
  kdevcxxai::config_page::ui_t ui;
  };

