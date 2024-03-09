#include "cxx_with_gpt.h"

// #include <debug.h>

#include <kpluginfactory.h>

#include <kpluginloader.h>
#include <kactioncollection.h>
#include <QAction>
#include <KLocalizedString>
#include <ktexteditor/application.h>
#include <ktexteditor/mainwindow.h>
#include <KConfigCore/KConfigGroup>
#include <KSharedConfig>
#include <kcoreconfigskeleton.h>

#ifndef Q_MOC_RUN
#include <ai_processing.h>
#include <fmt/core.h>
#include <simple_enum/simple_enum.hpp>
#include <string>
#endif

K_PLUGIN_FACTORY_WITH_JSON(cxx_with_gptFactory, "cxx_with_gpt.json", registerPlugin<cxx_with_gpt>();)

cxx_with_gpt::cxx_with_gpt(QObject * parent, QVariantList const & args) : KDevelop::IPlugin("cxx_with_gpt", parent)
  {
  qDebug() << "cxx_with_gpt::cxx_with_gpt\n";
  //   setupConfigurationInterface();
  //   auto editor = KTextEditor::Editor::instance();
  //   connect(editor, &KTextEditor::Editor::documentCreated, this, &cxx_with_gpt::attachActionsToDocument);
  //
  //   QAction* myAction = new QAction(i18n("Process with AI"), this);
  //   myAction->setIcon(QIcon::fromTheme("document-new")); // Example icon
  //   myAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M)); // Example shortcut
  //   actionCollection()->addAction("process_with_ai", myAction);
  //
  //   connect(myAction, &QAction::triggered, this, &cxx_with_gpt::onMyActionTriggered);
  }

void cxx_with_gpt::createActionsForMainWindow(
  Sublime::MainWindow *, QString & xmlFile, KActionCollection & actions
)
  {
  QAction * myAction = new QAction(QIcon(":/icons/my_icon.png"), tr("&Process with AI"), this);
  myAction->setToolTip(tr("Do something interesting with AI"));
  myAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));

  actions.addAction("process_with_ai", myAction);
  connect(myAction, &QAction::triggered, this, &cxx_with_gpt::on_process_with_ai);

  xmlFile = ":/ui/rcfile.ui.rc";
  }

void cxx_with_gpt::on_process_with_ai()
  {
  // qDebug() << "\nMy action was triggered!\n";
  auto * view = KTextEditor::Editor::instance()->application()->activeMainWindow()->activeView();
  if(view && view->selection())
    {
    // Read the current selected text
    QString selected_text = view->selectionText();
    qDebug() << "\ncxx_with_gpt: selecetd [" << selected_text << "]\n";

    auto result = process_with_ai(selected_text.toStdString());

    if(result)
      {
      auto const & response{*result};
      auto new_text = navive_response_format(response);
      fmt::print("cxx_with_gpt: Command Text: {}\nCode Text: {}\n", response.command, response.recived_text);
      if(!new_text.empty())
        view->document()->replaceText(view->selectionRange(), QString::fromStdString(new_text));
      }
    else
      {
      fmt::print("Error: {}\n", simple_enum::enum_name(result.error()));
      }
    }
  }

cxx_with_gpt::~cxx_with_gpt() {}

void cxx_with_gpt::setupConfigurationInterface()
  {
  qDebug() << "\ncxx_with_gpt::setupConfigurationInterface\n";
  // Check if the dialog already exists to avoid duplicates
  if(KConfigDialog::exists("cxx_with_gpt_settings"))
    return;

  // auto settings = CxxWithGpt::Settings::self();

  // Now use 'settings' to access AiKey and AiEndpoint
  // For example, to load and save these settings in your UI

  // Create the settings widget from the UI file
  settingsWidget = std::make_unique<QWidget>();
  ui.setupUi(settingsWidget.get());

  // auto settings = Settings::self();
  // ui.lineEditAiKey->setText(settings->aiKey());
  // ui.lineEditAiEndpoint->setText(settings->aiEndpoint());

  ui.lineEditAiKey->setText("");
  ui.lineEditAiEndpoint->setText("https://api.openai.com/v1/engines/code-davinci-002/completions");
  //
  KConfigGroup config(KSharedConfig::openConfig(), "cxx_with_gpt");
  //   // Register the settings widget in KDevelop's configuration dialog
  //   auto configPage = config->addPage(settingsWidget, i18n("AI Settings"), QStringLiteral("settings-configure"));
  //   connect(configPage, &KCoreConfigSkeleton::configCommitted, this, &cxx_with_gpt::applySettings);

  loadSettings();
  }

void cxx_with_gpt::loadSettings()
  {
  KConfigGroup config(KSharedConfig::openConfig(), "cxx_with_gpt");
  ui.lineEditAiKey->setText(config.readEntry("ai_key", ""));
  ui.lineEditAiEndpoint->setText(config.readEntry("ai_endpoint", ""));
  }

void cxx_with_gpt::saveSettings()
  {
  qDebug() << "cxx_with_gpt::saveSettings\n";

  KConfigGroup config(KSharedConfig::openConfig(), "cxx_with_gpt");
  config.writeEntry("ai_key", ui.lineEditAiKey->text());
  config.writeEntry("ai_endpoint", ui.lineEditAiEndpoint->text());
  config.sync();  // Ensure changes are written to disk
  }

void cxx_with_gpt::applySettings()
  {
  saveSettings();
  // Here you might also want to re-initialize anything that uses these settings
  }

#include "cxx_with_gpt.moc"
#include "moc_cxx_with_gpt.cpp"
