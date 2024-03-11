#include <plugin_settings.h>

plugin_settings::plugin_settings(QWidget * parent, QVariantList const & args) :
    KCModule(parent, args),
    ui(new Ui::plugin_settings)
  {
  ui->setupUi(this);

  connect(
    ui->editApiKey,
    &QLineEdit::editingFinished,
    this,
    [&]()
    {
      emit changed(true);
      // emit apiKeyChanged(ui->editApiKey->text());
    }
  );
  }

#include "plugin_settings.moc"
#include "moc_plugin_settings.cpp"
