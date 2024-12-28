// SPDX-FileCopyrightText: 2024 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once


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

  ~info_dialog() override;
  };

