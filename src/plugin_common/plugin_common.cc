// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#include <plugin_common.h>
#include <info_dialog.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/application.h>
#include <ktexteditor/mainwindow.h>
#include <kmessagebox.h>

#include <qprogressdialog.h>
#include <qaction.h>
#include <qapplication.h>

#ifndef Q_MOC_RUN
#include <aiprocess/log.h>
#include <aiprocess/app_settings.h>
#include <ai_processing.h>

#include <stralgo/stralgo.h>
#include <future>
#endif

namespace kdevcxxai
  {
using namespace std::string_view_literals;
using aiprocess::debug;
using aiprocess::info;
using aiprocess::log;
using aiprocess::warn;

auto get_view_file_path(KTextEditor::View const & view) noexcept -> expected<std::string, get_view_file_path_error>
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

auto process_with_ai(KTextEditor::View & view, aiprocess::app_settings_t const & settings) noexcept -> void
  {
    {
    auto aisettings{aiprocess::load_ai_settings()};
    if(aisettings.api_key.empty())
      {
      info_dialog dialog{"KDevCxx_With_Ai key setup", fist_time_message};
      dialog.exec();
      return;
      }
    }

  using namespace std::chrono_literals;

  auto * document = view.document();
  if(!document) [[unlikely]]
    {
    warn("Invalid view->document");
    return;
    }

  info("Processing OpenAI request ...");
  QProgressDialog progressDialog{"Processing OpenAI Request...", "Cancel", 0, 100};
  progressDialog.setWindowModality(Qt::WindowModal);
  progressDialog.show();

  QString selected_text = view.selectionText();

  auto async_result = std::async(
    std::launch::async,
    [](QString text) -> expected<aiprocess::model_response_text_t, aiprocess::process_with_ai_error>
    { return aiprocess::process_with_ai(text.toStdString()); },
    selected_text
  );

  debug("async to OpenAI executed ...");
  std::this_thread::yield();

  while(async_result.wait_for(50ms) != std::future_status::ready)
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

  auto result = async_result.get();
  debug("async to OpenAI done ...");

  if(!result)
    {
    aiprocess::li::error("Got error from async {}\n", result.error());
    KMessageBox::error(
      &view,
      QString::fromStdString(stralgo::stl::merge(
        "Error processing AI request "sv,
        simple_enum::enum_name(result.error()),
        "\ncheck detailed log at ~/.config/kdevcxx_with_ai/"sv,
        settings.log_path
      )),
      "Error during processing AI request"
    );
    return;
    }

  std::string cur_work_dir{};
  if(auto path = kdevcxxai::get_view_file_path(view); path.has_value())
    cur_work_dir = std::move(path.value());

  auto const & response = *result;
  auto new_text = aiprocess::process_openai_json_response(response, std::move(cur_work_dir));
  debug("Proocessed response Command Text: {}\nCode Text: {}\n", response.command, response.recived_text);

  if(new_text.empty()) [[unlikely]]
    return;

  document->replaceText(view.selectionRange(), QString::fromStdString(new_text));
  debug("document->replaceText called ...");
  }
  }  // namespace kdevcxxai
