// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#include <KCModule>
#include "ui_plugin_settings.h"

class plugin_settings : public KCModule
  {
  Q_OBJECT
public:
  plugin_settings(QWidget * parent, QVariantList const & args);

  ~plugin_settings() override { delete ui; }

private:
  Ui::plugin_settings * ui;
  };
