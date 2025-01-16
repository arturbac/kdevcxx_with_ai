// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#include <plugin_common.h>

#ifndef Q_MOC_RUN
#include <aiprocess/app_settings.h>
#endif
#include <kdevplatform/interfaces/iplugin.h>

#include <qobject.h>

class kdevcxx_with_ai : public KDevelop::IPlugin
  {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(kdevcxx_with_ai)
  aiprocess::app_settings_t settings;

public:
  kdevcxx_with_ai(QObject * parent, QVariantList const & args);
  ~kdevcxx_with_ai() override;

  [[nodiscard]] auto contextMenuExtension(KDevelop::Context * context, QWidget * parent) -> KDevelop::ContextMenuExtension override;

  auto createActionsForMainWindow(Sublime::MainWindow * window, QString & xmlFile, KActionCollection & actions)
    -> void override;

  auto unload() -> void override;

  [[nodiscard]] auto configPage(int number, QWidget * parent) ->KDevelop::ConfigPage* override;
  [[nodiscard]] auto configPages() const -> int override;

private Q_SLOTS:

  void on_process_with_ai();
  void on_first_time();

  };
