// SPDX-FileCopyrightText: 2026 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#include <QPointer>
#include <QWidget>

namespace KParts
  {
class ReadOnlyPart;
  }

/// Tool view with an embedded Konsole running claude connected to the plugin's IDE server
class claude_view_t : public QWidget
  {
  Q_OBJECT

public:
  claude_view_t(QString launch_command, QString working_dir, QWidget * parent);

private:
  auto load_part() -> void;

  QString launch_command_;
  QString working_dir_;
  QPointer<KParts::ReadOnlyPart> part_;
  };
