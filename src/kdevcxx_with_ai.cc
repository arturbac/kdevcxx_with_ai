#include "kdevcxx_with_ai.h"

// #include <debug.h>

#include <kpluginfactory.h>
#include <info_dialog.h>
#include <kpluginloader.h>
#include <kactioncollection.h>
#include <QAction>
#include <KLocalizedString>
#include <ktexteditor/application.h>
#include <ktexteditor/mainwindow.h>
#include <KConfigCore/KConfigGroup>
#include <KSharedConfig>
#include <kcoreconfigskeleton.h>
#include <QProgressDialog>
#include <QTimer>

#ifndef Q_MOC_RUN
#include <aiprocess/app_settings.h>
#include <aiprocess/log.h>
#include <ai_processing.h>
#include <fmt/core.h>
#include <simple_enum/simple_enum.hpp>
#include <string>
#include <future>
#include <thread>
#include <chrono>
#include "document_read_only.h"
#endif

enum class get_view_file_path_error
  {
  no_document,
  unhandled_exception
  };
using aiprocess::debug;
using aiprocess::info;
using aiprocess::log;
using aiprocess::warn;

[[nodiscard]]
static auto get_view_file_path(KTextEditor::View const & view
) noexcept -> expected<std::string, get_view_file_path_error>
try
  {
  if(!view.document()) [[unlikely]]
    return aiprocess::unexpected_error(get_view_file_path_error::no_document, "Document was not saved yet");

  auto document = view.document();
  return document->url().toLocalFile().toStdString();
  }
catch(...)
  {
  return aiprocess::unexpected_error(
    get_view_file_path_error::unhandled_exception, "error getting path for current view document"
  );
  }
K_PLUGIN_FACTORY_WITH_JSON(cxx_with_gptFactory, "kdevcxx_with_ai.json", registerPlugin<kdevcxx_with_ai>();)

kdevcxx_with_ai::kdevcxx_with_ai(QObject * parent, QVariantList const &) : KDevelop::IPlugin("kdevcxx_with_ai", parent)
  {
  log(aiprocess::level::info, "Starting plugin KDevCxx_With_Ai");
  settings = aiprocess::load_app_settings();
  aiprocess::setup_loggers(settings);
  info("Settings loaded");
  QTimer::singleShot(200, this, &kdevcxx_with_ai::on_first_time);
  }
  
void kdevcxx_with_ai::on_first_time()
  {
  auto aisettings{aiprocess::load_ai_settings()};
  if(!aisettings.api_key.empty())
    {
    info_dialog dialog(
      "KDevCxx_With_Ai key setup",
      "Please edit file ~/.config/kdevcxx_with_ai/kdevcxx_with_ai_ai_settings.json\n and enter Your API key before "
      "calling any functions and adjust Your rules for AI.\n You can change them at any time without restarting KDevelop\n" "Changes to kdevcxx_with_ai_ai_settings.json will take effect on every execution"
    );
    dialog.exec();
    }
  }
void kdevcxx_with_ai::createActionsForMainWindow(Sublime::MainWindow *, QString & xmlFile, KActionCollection & actions)
  {
  QAction * myAction = new QAction(QIcon(":/icons/my_icon.png"), tr("&Process with AI"), this);
  myAction->setToolTip(tr("Do something interesting with AI"));

  myAction->setShortcut(settings.activation_keys);
  info("Key for AI binded to {}", settings.activation_keys);

  actions.addAction("process_with_ai", myAction);
  connect(myAction, &QAction::triggered, this, &kdevcxx_with_ai::on_process_with_ai);

  xmlFile = ":/ui/rcfile.ui.rc";
  }

void kdevcxx_with_ai::on_process_with_ai()
  {
    {
    auto aisettings{aiprocess::load_ai_settings()};
    if(aisettings.api_key.empty())
      {
      info_dialog dialog(
        "KDevCxx_With_Ai key setup still not done ..",
        "Please edit file ~/.config/kdevcxx_with_ai/kdevcxx_with_ai_ai_settings.json and enter Your API key before "
        "calling any functions and adjust Your rules for AI. You can change them at any time without restarting "
        "KDevelop"
      );
      dialog.exec();
      }
    }
  using namespace std::chrono_literals;
  // qDebug() << "\nMy action was triggered!\n";
  KTextEditor::View * view = KTextEditor::Editor::instance()->application()->activeMainWindow()->activeView();

  if(nullptr != view && view->selection())
    {
    KTextEditor::Document * document = view->document();
    if(nullptr == document)
      {
      qDebug() << "Invalid view->document";
      return;
      }
    // document_read_only_t read_only_guard{*document};
    info("Processing OpenAI request ...");
    QProgressDialog progressDialog("Processing OpenAI Request...", "Cancel", 0, 100);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.show();

    QString selected_text = view->selectionText();

    auto fn_call_openai = [](std::string text) -> expected<model_response_text_t, process_with_ai_error>
    { return process_with_ai(std::move(text)); };

    auto async_result = std::async(std::launch::async, fn_call_openai, selected_text.toStdString());
    debug("async to OpenAI executed ...");

    std::this_thread::yield();

    while(async_result.wait_for(50ms) != std::future_status::ready)
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    auto result = async_result.get();
    debug("async to OpenAI done ...");
    // auto result = process_with_ai(selected_text.toStdString());

    if(result)
      {
      std::string cur_work_dir{};
      if(auto path{get_view_file_path(*view)}; path.has_value())
        cur_work_dir = std::move(path.value());
      auto const & response{*result};
      auto new_text = process_ai_response(response, std::move(cur_work_dir));
      debug("Proocessed response Command Text: {}\nCode Text: {}\n", response.command, response.recived_text);

      // make it read write to apply chnages
      // read_only_guard.clear_state();
      if(!new_text.empty())
        {
        document->replaceText(view->selectionRange(), QString::fromStdString(new_text));
        debug("document->replaceText called ...");
        }
      }
    else
      {
      aiprocess::li::error("Got error from async {}\n", result.error());
      }
    }
  }

kdevcxx_with_ai::~kdevcxx_with_ai() {}

void kdevcxx_with_ai::setupConfigurationInterface()
  {
  // qDebug() << "\ncxx_with_gpt::setupConfigurationInterface\n";
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

void kdevcxx_with_ai::loadSettings()
  {
  KConfigGroup config(KSharedConfig::openConfig(), "cxx_with_gpt");
  ui.lineEditAiKey->setText(config.readEntry("ai_key", ""));
  ui.lineEditAiEndpoint->setText(config.readEntry("ai_endpoint", ""));
  }

void kdevcxx_with_ai::saveSettings()
  {
  qDebug() << "cxx_with_gpt::saveSettings\n";

  KConfigGroup config(KSharedConfig::openConfig(), "cxx_with_gpt");
  config.writeEntry("ai_key", ui.lineEditAiKey->text());
  config.writeEntry("ai_endpoint", ui.lineEditAiEndpoint->text());
  config.sync();  // Ensure changes are written to disk
  }

void kdevcxx_with_ai::applySettings()
  {
  saveSettings();
  // Here you might also want to re-initialize anything that uses these settings
  }

#include "kdevcxx_with_ai.moc"
#include "moc_kdevcxx_with_ai.cpp"
