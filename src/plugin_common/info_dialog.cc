// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#include "info_dialog.h"
#include <qboxlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>

info_dialog::info_dialog(QString const & title, QString const & text, QWidget * parent) : QDialog{parent}
  {
  setWindowTitle(title);

  auto * layout = new QVBoxLayout{this};
  auto * label = new QLabel{text, this};
  layout->addWidget(label);

  // Optional: Add a button to close the dialog
  auto * close_button = new QPushButton{"Close", this};
  connect(close_button, &QPushButton::clicked, this, &info_dialog::accept);
  layout->addWidget(close_button);

  setLayout(layout);
  }

<<<<<<< HEAD
info_dialog::~info_dialog() = default;
=======
info_dialog::~info_dialog()
{}
>>>>>>> parent of 11cd9f3 (Edit cmake to search locally at first for 3rd party libraries)

