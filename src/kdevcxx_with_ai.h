// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#ifndef CXX_WITH_GPT_H
#define CXX_WITH_GPT_H

#include <plugin_common.h>

#ifndef Q_MOC_RUN
#include <aiprocess/app_settings.h>
#endif
#include <kdevplatform/interfaces/iplugin.h>

#include <qobject.h>

class kdevcxx_with_ai : public KDevelop::IPlugin
  {
  Q_OBJECT
  aiprocess::app_settings_t settings;

public:
  kdevcxx_with_ai(QObject * parent, QVariantList const & args);
  ~kdevcxx_with_ai() override;

  auto contextMenuExtension(KDevelop::Context * context, QWidget * parent) -> KDevelop::ContextMenuExtension override;

  auto createActionsForMainWindow(Sublime::MainWindow * window, QString & xmlFile, KActionCollection & actions)
    -> void override;

  void unload() override;

  KDevelop::ConfigPage * configPage(int number, QWidget * parent) override;
  int configPages() const override;

private slots:

  void on_process_with_ai();
  void on_first_time();

private:
  };

#endif  // CXX_WITH_GPT_H
