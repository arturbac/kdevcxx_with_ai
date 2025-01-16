// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#include <plugin_common.h>

#include <ktexteditor/plugin.h>
#include <ktexteditor/mainwindow.h>
#include <ktexteditor/view.h>
#include <kpluginfactory.h>
#include <qpluginloader.h>

#ifndef Q_MOC_RUN
#include <aiprocess/app_settings.h>
#endif
class kate_with_ai_view;

class kate_with_ai : public KTextEditor::Plugin
  {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(kate_with_ai)
  friend class kate_with_ai_view;

  aiprocess::app_settings_t settings;

public:
  explicit kate_with_ai(QObject * parent, QList<QVariant> const & = QList<QVariant>{});
  ~kate_with_ai() override;

  [[nodiscard]] auto createView(KTextEditor::MainWindow * main_window) -> QObject * override;

  [[nodiscard]] auto configPages() const -> int override;
  [[nodiscard]] auto configPage(int number, QWidget * parent) -> KTextEditor::ConfigPage* override;
  };

class kate_with_ai_view : public QWidget
  {
  Q_OBJECT

public:
  explicit kate_with_ai_view(kate_with_ai * plugin, KTextEditor::MainWindow * main_window);

private Q_SLOTS:
  void on_view_changed(KTextEditor::View * view);
  void on_context_menu_about_to_show(KTextEditor::View * view, QMenu * menu);

private:
  void init_view(KTextEditor::View * view) const;
  void on_process_with_ai();

  kate_with_ai * plugin_;
  KTextEditor::MainWindow * main_window_;
  QAction * openai_process_action{};
  };
