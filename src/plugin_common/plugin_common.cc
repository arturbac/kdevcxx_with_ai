// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#include <plugin_common.h>
#include <info_dialog.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/application.h>
#include <ktexteditor/mainwindow.h>

#include <kmessagebox.h>
#include <klocalizedstring.h>

#include <qprogressdialog.h>
#include <qaction.h>
#include <qapplication.h>
#include <qboxlayout.h>
#include <qformlayout.h>
#include <qicon.h>
#include <qtabwidget.h>

#ifndef Q_MOC_RUN
#include <aiprocess/log.h>
#include <aiprocess/app_settings.h>
#include <ai_processing.h>
#include <simple_enum/enum_index.hpp>
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

  namespace config_page
  {
    template<typename T>
    concept callable_with_no_args_returning_void = requires(T t) {
      { t() } -> std::same_as<void>;
    };

    template<typename WidgetType>
    struct config_widget_creator_t
    {
      QWidget * parent;
      QFormLayout * form_layout;

      template<callable_with_no_args_returning_void Callable>
      auto operator()(QString const & label, Callable emit_changed) const -> WidgetType *
      {
        auto * widget = new WidgetType(parent);
        QObject::connect(widget, &WidgetType::textChanged, parent, emit_changed);
        form_layout->addRow(label, widget);
        return widget;
      }

      template<callable_with_no_args_returning_void Callable>
      auto operator()(QString const & label, QStringList const & logLevels, Callable emit_changed) const -> WidgetType *
      {
        auto * widget = new WidgetType(parent);
        QObject::connect(
          widget, static_cast<void (WidgetType::*)(int)>(&QComboBox::currentIndexChanged), parent, emit_changed
        );
        form_layout->addRow(label, widget);
        widget->addItems(logLevels);
        return widget;
      }
    };

    template<aiprocess::backend_type_e backend>
    auto construct(KTextEditor::ConfigPage & w, ui_t & ui, std::function<void()> emit_changed) -> void
    {
      aiprocess::debug("construct() called");
      QVBoxLayout * main_layout = new QVBoxLayout(&w);
      QTabWidget * ai_tab_widget = new QTabWidget(&w);
      main_layout->addWidget(ai_tab_widget);

      // auto emit_changed = [&]() { pemit_chnaged(); };
      // auto emit_changed = [&]<typename... Args>(Args &&...) { pemit_chnaged(); };

      // Create a widget to hold the form layout
      QWidget * ai_form_widget = new QWidget();
      QFormLayout * ai_form_layout = new QFormLayout(ai_form_widget);

      ui.openai_key = config_widget_creator_t<QLineEdit>{ai_form_widget, ai_form_layout}("OpenAI Key:", emit_changed);
      ui.open_ai_model
      = config_widget_creator_t<QLineEdit>{ai_form_widget, ai_form_layout}("OpenAI Model:", emit_changed);
      ui.language_rules
      = config_widget_creator_t<QPlainTextEdit>{ai_form_widget, ai_form_layout}("Language Rules:", emit_changed);

      ai_tab_widget->addTab(ai_form_widget, i18n("AI Configuration"));

      // second page

      QWidget * log_form_widget = new QWidget();
      QFormLayout * log_form_layout = new QFormLayout(log_form_widget);

      {
        config_widget_creator_t<QLineEdit> creator{log_form_widget, log_form_layout};
        ui.log_path_edit = creator("Log Path:", emit_changed);
        ui.console_log_pattern_edit = creator("Console Log Pattern:", emit_changed);
        ui.general_log_pattern_edit = creator("General Log Pattern:", emit_changed);
        ui.snippet_log_pattern_edit = creator("Snippet Log Pattern:", emit_changed);
        ui.important_log_pattern_edit = creator("Important Log Pattern:", emit_changed);
      }

      {
        QStringList logLevels = {"trace", "debug", "info", "warn", "error", "critical"};
        config_widget_creator_t<QComboBox> creator{log_form_widget, log_form_layout};
        ui.console_log_level_combo = creator("Console Log Pattern:", logLevels, emit_changed);
        ui.general_log_level_combo = creator("General Log Level:", logLevels, emit_changed);
        ui.snippet_log_level_combo = creator("Snippet Log Pattern:", logLevels, emit_changed);
        ui.important_log_level_combo = creator("Important Log Pattern:", logLevels, emit_changed);
      }

      ui.flush_every_spin = new QSpinBox(log_form_widget);
      ui.flush_every_spin->setMinimum(1);
      ui.flush_every_spin->setMaximum(60);
      ui.flush_every_spin->setValue(3);
      log_form_layout->addRow(i18n("Flush Every (seconds):"), ui.flush_every_spin);

      ui.activation_keys_spin = new QSpinBox(log_form_widget);
      ui.activation_keys_spin->setMinimum(0);
      ui.activation_keys_spin->setMaximum(std::numeric_limits<int>::max());
      log_form_layout->addRow(i18n("Activation Keys:"), ui.activation_keys_spin);

      ai_tab_widget->addTab(log_form_widget, i18n("Logging"));
      reset<backend>(ui);
      aiprocess::debug("construct() end");
    }

    template auto
    construct<aiprocess::backend_type_e::kdevelop>(KTextEditor::ConfigPage &, ui_t &, std::function<void()>) -> void;
    template auto
    construct<aiprocess::backend_type_e::kate>(KTextEditor::ConfigPage &, ui_t &, std::function<void()>) -> void;

    template<aiprocess::backend_type_e backend>
    auto apply(ui_t & ui) -> void
    {
      aiprocess::debug("apply() called");
      aiprocess::store_ai_settings(aiprocess::ai_settings_t{
        .api_key = ui.openai_key->text().toStdString(),
                                   .cxx_rules = ui.language_rules->toPlainText().toStdString(),
                                   .gpt_model = ui.open_ai_model->text().toStdString()
      });
      using level_enum = aiprocess::app_settings_t::level_enum;
      aiprocess::store_app_settings<backend>(aiprocess::app_settings_t{
        .log_path = ui.log_path_edit->text().toStdString(),
                                             .console_log_pattern = ui.console_log_pattern_edit->text().toStdString(),
                                             .general_log_pattern = ui.general_log_pattern_edit->text().toStdString(),
                                             .snippet_log_pattern = ui.snippet_log_pattern_edit->text().toStdString(),
                                             .important_log_pattern = ui.important_log_pattern_edit->text().toStdString(),
                                             .console_log_level = level_enum(ui.console_log_level_combo->currentIndex()),
                                             .general_log_level = level_enum(ui.general_log_level_combo->currentIndex()),
                                             .snippet_log_level = level_enum(ui.snippet_log_level_combo->currentIndex()),
                                             .important_log_level = level_enum(ui.important_log_level_combo->currentIndex()),
                                             .activation_keys = ui.activation_keys_spin->value()
      });
      aiprocess::debug("apply() end");
    }

    template auto apply<aiprocess::backend_type_e::kdevelop>(ui_t &) -> void;
    template auto apply<aiprocess::backend_type_e::kate>(ui_t &) -> void;

    template<aiprocess::backend_type_e backend>
    auto reset(ui_t & ui) -> void
    {
      aiprocess::debug("reset() called");
      {
        aiprocess::ai_settings_t settings = aiprocess::load_ai_settings();
        ui.language_rules->setPlainText(QString::fromStdString(settings.cxx_rules));
        ui.openai_key->setText(QString::fromStdString(settings.api_key));
        ui.open_ai_model->setText(QString::fromStdString(settings.gpt_model));
      }

      // Second page settings

      {
        aiprocess::app_settings_t settings{aiprocess::load_app_settings<backend>()};
        ui.log_path_edit->setText(QString::fromStdString(settings.log_path));
        ui.console_log_pattern_edit->setText(QString::fromStdString(settings.console_log_pattern));
        ui.general_log_pattern_edit->setText(QString::fromStdString(settings.general_log_pattern));
        ui.snippet_log_pattern_edit->setText(QString::fromStdString(settings.snippet_log_pattern));
        ui.important_log_pattern_edit->setText(QString::fromStdString(settings.important_log_pattern));

        auto set_combo_index = [](QComboBox * comboBox, auto enumValue)
        {
          auto ix{simple_enum::enum_index(enumValue)};
          if(ix.has_value())
            comboBox->setCurrentIndex(*ix);
        };
        set_combo_index(ui.console_log_level_combo, settings.console_log_level);
        set_combo_index(ui.general_log_level_combo, settings.general_log_level);
        set_combo_index(ui.snippet_log_level_combo, settings.snippet_log_level);
        set_combo_index(ui.important_log_level_combo, settings.important_log_level);

        ui.activation_keys_spin->setValue(settings.activation_keys);
      }
    }

    template auto reset<aiprocess::backend_type_e::kdevelop>(ui_t &) -> void;
    template auto reset<aiprocess::backend_type_e::kate>(ui_t &) -> void;

    auto defaults(ui_t & ui) -> void
    {
      aiprocess::debug("defaults() called");
      ui.language_rules->setPlainText(
        QString::fromUtf8(aiprocess::default_ai_rules.data(), aiprocess::default_ai_rules.size())
      );
      ui.open_ai_model->setText(
        QString::fromUtf8(aiprocess::default_gpt_model.data(), aiprocess::default_gpt_model.size())
      );
    }
  }  // namespace config_page
}  // namespace kdevcxxai
