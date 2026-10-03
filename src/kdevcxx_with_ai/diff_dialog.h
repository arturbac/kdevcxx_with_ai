// SPDX-FileCopyrightText: 2026 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#include <ide_protocol.h>

#include <QDialog>

/// openDiff: shows the proposed change as a unified diff, replies once with FILE_SAVED or DIFF_REJECTED
class diff_dialog_t : public QDialog
  {
  Q_OBJECT

public:
  diff_dialog_t(
    QString const & tab_name,
    QString const & file_path,
    QString new_contents,
    ide_protocol::reply_t reply,
    QWidget * parent
  );
  ~diff_dialog_t() override;

  /// closed from claude (close_tab, closeAllDiffTabs), the decision was made elsewhere
  auto close_tab() -> void;

private:
  auto send(QJsonObject const & result) -> void;

  QString new_contents_;
  ide_protocol::reply_t reply_;
  };
