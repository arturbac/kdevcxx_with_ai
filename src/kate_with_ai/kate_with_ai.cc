// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#include "kate_with_ai.h"
#include <info_dialog.h>
#include <kate_with_ai_config_page.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/application.h>
#include <qaction.h>
#include <qmenu.h>

#ifndef Q_MOC_RUN
#include <aiprocess/app_settings.h>
#include <aiprocess/log.h>
#include <aiprocess/obfuscate_text.h>
#endif

using aiprocess::debug;
using aiprocess::info;
using aiprocess::log;
using aiprocess::warn;

K_PLUGIN_FACTORY_WITH_JSON(kate_with_aiFactory, "kate_with_ai.json", registerPlugin<kate_with_ai>();)

kate_with_ai::kate_with_ai(QObject * parent, QList<QVariant> const &) : KTextEditor::Plugin{parent}
  {
  log(aiprocess::level::info, "Starting plugin Kate_With_Ai");
  settings = aiprocess::load_app_settings<aiprocess::backend_type_e::kate>();
  aiprocess::setup_loggers(settings);
  info("Settings loaded");
  }

kate_with_ai::~kate_with_ai() = default;

int kate_with_ai::configPages() const { return 1; }

KTextEditor::ConfigPage * kate_with_ai::configPage(int number, QWidget * parent)
  {
  if(number == 0)
    return new kate_with_ai_config_page{parent};
  return nullptr;  // No other pages to return
  }

QObject * kate_with_ai::createView(KTextEditor::MainWindow * main_window)
  {
  auto * view = new kate_with_ai_view{this, main_window};
  return view;
  }

kate_with_ai_view::kate_with_ai_view(kate_with_ai * plugin, KTextEditor::MainWindow * main_window) :
    plugin_{plugin},
    main_window_{main_window}
  {
  openai_process_action = new QAction{tr("Process with OpenAI"), this};
  connect(openai_process_action, &QAction::triggered, this, &kate_with_ai_view::on_process_with_ai);

  // Connect to the signal that notifies about the current view change
  connect(main_window_, &KTextEditor::MainWindow::viewChanged, this, &kate_with_ai_view::on_view_changed);

  // Initialize the context menu for the current active view immediately
  init_view(main_window_->activeView());
  }

void kate_with_ai_view::on_view_changed(KTextEditor::View * view) { init_view(view); }

void kate_with_ai_view::init_view(KTextEditor::View * view)
  {
  if(!view) [[unlikely]]
    return;

  // Connect to the view's context menu signal
  connect(view, &KTextEditor::View::contextMenuAboutToShow, this, &kate_with_ai_view::on_context_menu_about_to_show);
  }

void kate_with_ai_view::on_context_menu_about_to_show(KTextEditor::View *, QMenu * menu)
  {
  // Ensure the menu is valid
  if(!menu) [[unlikely]]
    return;

  if(!menu->actions().contains(openai_process_action))
    {
    info("Context menu registered for AI");
    menu->addAction(openai_process_action);
    }
  }

void kate_with_ai_view::on_process_with_ai()
  {
  auto * view = KTextEditor::Editor::instance()->application()->activeMainWindow()->activeView();
  if(!view || !view->selection()) [[unlikely]]
    return;

  kdevcxxai::process_with_ai(*view, plugin_->settings);
  }

#include "kate_with_ai.moc"
