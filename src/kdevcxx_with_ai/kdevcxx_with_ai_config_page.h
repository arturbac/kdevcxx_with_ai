// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#include <kdevplatform/interfaces/configpage.h>
#include <plugin_common.h>

class kdevcxx_with_ai_config_page : public KDevelop::ConfigPage
  {
  Q_OBJECT

public:
  explicit kdevcxx_with_ai_config_page(KDevelop::IPlugin * plugin, QWidget * parent = nullptr);
  ~kdevcxx_with_ai_config_page() override;

  auto name() const -> QString override;
  auto fullName() const -> QString override;
  auto icon() const -> QIcon override;

  void apply() override;
  void reset() override;
  void defaults() override;

  void emit_changed();

private:
  kdevcxxai::config_page::ui_t ui;
  };

