#include "kdevcxx_with_ai_config_page.h"
#include <klocalizedstring.h>
#include <qboxlayout.h>
#include <qformlayout.h>
#include <qicon.h>
#include <qtabwidget.h>

#ifndef Q_MOC_RUN
#include <aiprocess/app_settings.h>
#include <aiprocess/spdlog.hpp>
#include <simple_enum/enum_cast.hpp>
#include <simple_enum/enum_index.hpp>
#include <aiprocess/log.h>
#endif

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

kdevcxx_with_ai_config_page::kdevcxx_with_ai_config_page(KDevelop::IPlugin * plugin, QWidget * parent) :
    KDevelop::ConfigPage(plugin, nullptr, parent)
  {
  QVBoxLayout * main_layout = new QVBoxLayout(this);
  QTabWidget * ai_tab_widget = new QTabWidget(this);
  main_layout->addWidget(ai_tab_widget);

  auto emit_changed = [this]<typename... Args>(Args &&...) { emit changed(); };
  // Create a widget to hold the form layout
  QWidget * ai_form_widget = new QWidget();
  QFormLayout * ai_form_layout = new QFormLayout(ai_form_widget);

  openai_key = config_widget_creator_t<QLineEdit>{ai_form_widget, ai_form_layout}("OpenAI Key:", emit_changed);
  open_ai_model = config_widget_creator_t<QLineEdit>{ai_form_widget, ai_form_layout}("OpenAI Model:", emit_changed);
  language_rules
    = config_widget_creator_t<QPlainTextEdit>{ai_form_widget, ai_form_layout}("Language Rules:", emit_changed);

  ai_tab_widget->addTab(ai_form_widget, i18n("AI Configuration"));

  // second page

  QWidget * log_form_widget = new QWidget();
  QFormLayout * log_form_layout = new QFormLayout(log_form_widget);

    {
    config_widget_creator_t<QLineEdit> creator{log_form_widget, log_form_layout};
    log_path_edit = creator("Log Path:", emit_changed);
    console_log_pattern_edit = creator("Console Log Pattern:", emit_changed);
    general_log_pattern_edit = creator("General Log Pattern:", emit_changed);
    snippet_log_pattern_edit = creator("Snippet Log Pattern:", emit_changed);
    important_log_pattern_edit = creator("Important Log Pattern:", emit_changed);
    }

    {
    QStringList logLevels = {"trace", "debug", "info", "warn", "error", "critical"};
    config_widget_creator_t<QComboBox> creator{log_form_widget, log_form_layout};
    console_log_level_combo = creator("Console Log Pattern:", logLevels, emit_changed);
    general_log_level_combo = creator("General Log Level:", logLevels, emit_changed);
    snippet_log_level_combo = creator("Snippet Log Pattern:", logLevels, emit_changed);
    important_log_level_combo = creator("Important Log Pattern:", logLevels, emit_changed);
    }

  flush_every_spin = new QSpinBox(log_form_widget);
  flush_every_spin->setMinimum(1);
  flush_every_spin->setMaximum(60);
  flush_every_spin->setValue(3);
  log_form_layout->addRow(i18n("Flush Every (seconds):"), flush_every_spin);

  activation_keys_spin = new QSpinBox(log_form_widget);
  activation_keys_spin->setMinimum(0);
  activation_keys_spin->setMaximum(std::numeric_limits<int>::max());
  log_form_layout->addRow(i18n("Activation Keys:"), activation_keys_spin);

  ai_tab_widget->addTab(log_form_widget, i18n("Logging"));
  reset();
  }

QString kdevcxx_with_ai_config_page::name() const { return i18n("OpenAI Configuration"); }

QString kdevcxx_with_ai_config_page::fullName() const { return i18n("Configure Open AI Settings"); }

QIcon kdevcxx_with_ai_config_page::icon() const { return QIcon::fromTheme("preferences-other"); }

void kdevcxx_with_ai_config_page::apply()
  {
  aiprocess::debug("apply() called");
  aiprocess::store_ai_settings(aiprocess::ai_settings_t{
    .api_key = openai_key->text().toStdString(),
    .cxx_rules = language_rules->toPlainText().toStdString(),
    .gpt_model = open_ai_model->text().toStdString()
  });
  using level_enum = aiprocess::app_settings_t::level_enum;
  aiprocess::store_app_settings(aiprocess::app_settings_t{
    .log_path = log_path_edit->text().toStdString(),
    .console_log_pattern = console_log_pattern_edit->text().toStdString(),
    .general_log_pattern = general_log_pattern_edit->text().toStdString(),
    .snippet_log_pattern = snippet_log_pattern_edit->text().toStdString(),
    .important_log_pattern = important_log_pattern_edit->text().toStdString(),
    .console_log_level = level_enum(console_log_level_combo->currentIndex()),
    .general_log_level = level_enum(general_log_level_combo->currentIndex()),
    .snippet_log_level = level_enum(snippet_log_level_combo->currentIndex()),
    .important_log_level = level_enum(important_log_level_combo->currentIndex()),
    .activation_keys = activation_keys_spin->value()
  });
  }

void kdevcxx_with_ai_config_page::reset()
  {
  aiprocess::debug("reset() called");
    {
    aiprocess::ai_settings_t settings = aiprocess::load_ai_settings();
    language_rules->setPlainText(QString::fromStdString(settings.cxx_rules));
    openai_key->setText(QString::fromStdString(settings.api_key));
    open_ai_model->setText(QString::fromStdString(settings.gpt_model));
    }

    // Second page settings

    {
    aiprocess::app_settings_t settings{aiprocess::load_app_settings()};
    log_path_edit->setText(QString::fromStdString(settings.log_path));
    console_log_pattern_edit->setText(QString::fromStdString(settings.console_log_pattern));
    general_log_pattern_edit->setText(QString::fromStdString(settings.general_log_pattern));
    snippet_log_pattern_edit->setText(QString::fromStdString(settings.snippet_log_pattern));
    important_log_pattern_edit->setText(QString::fromStdString(settings.important_log_pattern));

    auto set_combo_index = [](QComboBox * comboBox, auto enumValue)
    {
      auto ix{simple_enum::enum_index(enumValue)};
      if(ix.has_value())
        comboBox->setCurrentIndex(*ix);
    };
    set_combo_index(console_log_level_combo, settings.console_log_level);
    set_combo_index(general_log_level_combo, settings.general_log_level);
    set_combo_index(snippet_log_level_combo, settings.snippet_log_level);
    set_combo_index(important_log_level_combo, settings.important_log_level);

    activation_keys_spin->setValue(settings.activation_keys);
    }
  }

void kdevcxx_with_ai_config_page::defaults()
  {
  aiprocess::debug("defaults() called");
  language_rules->setPlainText(QString::fromUtf8(aiprocess::default_ai_rules.data(), aiprocess::default_ai_rules.size())
  );
  open_ai_model->setText(QString::fromUtf8(aiprocess::default_gpt_model.data(), aiprocess::default_gpt_model.size()));
  }
