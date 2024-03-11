// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#ifndef INFO_DIALOG_H
#define INFO_DIALOG_H

#include <QDialog>

// Forward declaration of Qt classes used in the header
class QVBoxLayout;
class QLabel;
class QPushButton;

class info_dialog : public QDialog
  {
  Q_OBJECT

public:
  explicit info_dialog(QString const & title, QString const & text, QWidget * parent = nullptr);
  };

#endif  // INFO_DIALOG_H
