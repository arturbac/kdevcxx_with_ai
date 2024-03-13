// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#include "kdevcxx_with_ai.h"
#include "kdevcxx_with_ai_config_page.h"

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <info_dialog.h>
#include <ktexteditor/application.h>
#include <ktexteditor/mainwindow.h>
#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/view.h>
#include <kactioncollection.h>

#include <kdevplatform/interfaces/contextmenuextension.h>
#include <kdevplatform/interfaces/context.h>
#include <kdevplatform/interfaces/configpage.h>

#include <qaction.h>
#include <qtimer.h>

#ifndef Q_MOC_RUN
#include <aiprocess/app_settings.h>
#include <aiprocess/log.h>
#include <aiprocess/obfuscate_text.h>
#endif

using aiprocess::debug;
using aiprocess::info;
using aiprocess::log;
using aiprocess::warn;

K_PLUGIN_FACTORY_WITH_JSON(cxx_with_gptFactory, "kdevcxx_with_ai.json", registerPlugin<kdevcxx_with_ai>();)

kdevcxx_with_ai::kdevcxx_with_ai(QObject * parent, QVariantList const &) : KDevelop::IPlugin("kdevcxx_with_ai", parent)
  {
  log(aiprocess::level::info, "Starting plugin KDevCxx_With_Ai");
  settings = aiprocess::load_app_settings();
  aiprocess::setup_loggers(settings);
  info("Settings loaded");
  QTimer::singleShot(200, this, &kdevcxx_with_ai::on_first_time);
  }

/**
 * @brief Handles first-time initialization for the AI settings.
 *
 * This function is responsible for checking whether the AI settings have
 * been properly set up with an API key. If the API key is missing, it prompts
 * the user to enter the API key by displaying an information dialog.
 */

void kdevcxx_with_ai::on_first_time()
  {
  auto aisettings = aiprocess::load_ai_settings();
  aiprocess::warn("api_key {}", aiprocess::obfuscate_text(aisettings.api_key));
  aiprocess::warn("cxx_rules {}", aisettings.cxx_rules);
  if(aisettings.api_key.empty())
    {
    info_dialog dialog( "KDevCxx_With_Ai Initialization", kdevcxxai::fist_time_message );
    dialog.exec();
    }
  }

using namespace std::string_view_literals;

void kdevcxx_with_ai::on_process_with_ai()
  {

  auto * view = KTextEditor::Editor::instance()->application()->activeMainWindow()->activeView();
  if(!view || !view->selection()) [[unlikely]]
    return;

  kdevcxxai::process_with_ai(*view, settings);

  }

void kdevcxx_with_ai::createActionsForMainWindow(Sublime::MainWindow *, QString &, KActionCollection & actions)
  {
  QAction * process_with_ai_action = new QAction(QIcon(":/icons/my_icon.png"), tr("Process with OpenAI"), this);
  process_with_ai_action->setToolTip(tr("Do something interesting with AI"));

  process_with_ai_action->setShortcut(settings.activation_keys);
  info("Key for AI binded to {}", settings.activation_keys);

  actions.addAction("process_with_ai", process_with_ai_action);
  connect(process_with_ai_action, &QAction::triggered, this, &kdevcxx_with_ai::on_process_with_ai);
  }

auto kdevcxx_with_ai::contextMenuExtension(KDevelop::Context * context, QWidget * parent)
  -> KDevelop::ContextMenuExtension
  {
  KDevelop::ContextMenuExtension extension;
  // Example context check (you'll need to replace this with actual logic based on your context)
  if(context->type() == KDevelop::Context::EditorContext)
    {
    info("Context menu registered for AI");
    // This is just an example; customize it based on your plugin's functionality
    QAction * process_with_ai_action = new QAction(QIcon::fromTheme("system-run"), tr("Process with OpenAI"), parent);
    connect(process_with_ai_action, &QAction::triggered, this, &kdevcxx_with_ai::on_process_with_ai);

    // Add your action to the extension
    extension.addAction(KDevelop::ContextMenuExtension::EditGroup, process_with_ai_action);
    }

  return extension;
  }

int kdevcxx_with_ai::configPages() const { return 1; }

auto kdevcxx_with_ai::configPage(int number, QWidget * parent) -> KDevelop::ConfigPage *
  {
  if(number == 0)
    return new kdevcxx_with_ai_config_page(this, parent);
  return nullptr;  // No other pages to return
  }

void kdevcxx_with_ai::unload() {}

kdevcxx_with_ai::~kdevcxx_with_ai() {}

#include "kdevcxx_with_ai.moc"
// #include "moc_kdevcxx_with_ai.cpp"
