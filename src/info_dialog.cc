#include "info_dialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

info_dialog::info_dialog(QString const & title, QString const & text, QWidget * parent) : QDialog(parent)
  {
  setWindowTitle(title);

  QVBoxLayout * layout = new QVBoxLayout(this);
  QLabel * label = new QLabel(text, this);
  layout->addWidget(label);

  // Optional: Add a button to close the dialog
  QPushButton * close_button = new QPushButton("Close", this);
  connect(close_button, &QPushButton::clicked, this, &info_dialog::accept);
  layout->addWidget(close_button);

  setLayout(layout);
  }
