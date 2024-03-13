#pragma once

#include <kdevplatform/interfaces/configpage.h>
#include <qwidget.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qplaintextedit.h>

class kdevcxx_with_ai_config_page : public KDevelop::ConfigPage
  {
  Q_OBJECT

public:
  explicit kdevcxx_with_ai_config_page(KDevelop::IPlugin * plugin, QWidget * parent = nullptr);

  QString name() const override;
  QString fullName() const override;
  QIcon icon() const override;

  void apply() override;
  void reset() override;
  void defaults() override;

private:
  QPlainTextEdit * language_rules;
  QLineEdit * openai_key;
  QLineEdit * open_ai_model;

  QLineEdit * log_path_edit;
  QLineEdit * console_log_pattern_edit;
  QLineEdit * general_log_pattern_edit;
  QLineEdit * snippet_log_pattern_edit;
  QLineEdit * important_log_pattern_edit;

  QComboBox * console_log_level_combo;
  QComboBox * general_log_level_combo;
  QComboBox * snippet_log_level_combo;
  QComboBox * important_log_level_combo;

  QSpinBox * flush_every_spin;
  QSpinBox * activation_keys_spin;
  };

