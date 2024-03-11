// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#ifndef CXX_WITH_GPT_H
#define CXX_WITH_GPT_H

#ifndef Q_MOC_RUN
#include <aiprocess/app_settings.h>
#endif
#include <kdevplatform/interfaces/iplugin.h>
#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/view.h>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KActionCollection>
#include <QAction>
#include <KLocalizedString>
#include <KConfigDialog>
#include <QObject>
#include <memory>

class kdevcxx_with_ai : public KDevelop::IPlugin
  {
  Q_OBJECT
  aiprocess::app_settings_t settings;

public:
  kdevcxx_with_ai(QObject * parent, QVariantList const & args);
  ~kdevcxx_with_ai() override;

  void
    createActionsForMainWindow(Sublime::MainWindow * window, QString & xmlFile, KActionCollection & actions) override;

private slots:

  void on_process_with_ai();
  void on_first_time();

private:
  };

#endif  // CXX_WITH_GPT_H
