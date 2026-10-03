// SPDX-FileCopyrightText: 2026 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#include <ide_protocol.h>

#include <interfaces/iplugin.h>

#include <QHash>
#include <QPointer>
#include <QTimer>
#include <memory>

namespace ide_protocol
  {
class ide_server_t;
  }

namespace KDevelop
  {
class IDocument;
  }

class diff_dialog_t;
class claude_view_factory_t;

/// KDevelop client for Claude Code: tool view with claude in Konsole plus the IDE integration server
class kdevcxx_with_ai : public KDevelop::IPlugin
  {
  Q_OBJECT

public:
  kdevcxx_with_ai(QObject * parent, KPluginMetaData const & meta_data, QVariantList const &);
  ~kdevcxx_with_ai() override;

  auto unload() -> void override;

  auto contextMenuExtension(KDevelop::Context * context, QWidget * parent) -> KDevelop::ContextMenuExtension override;
  auto createActionsForMainWindow(Sublime::MainWindow * window, QString & xml_file, KActionCollection & actions)
    -> void override;

  [[nodiscard]]
  auto configPages() const -> int override;
  [[nodiscard]]
  auto configPage(int number, QWidget * parent) -> KDevelop::ConfigPage * override;

  [[nodiscard]]
  auto launch_command() const -> QString;
  [[nodiscard]]
  auto working_dir() const -> QString;

private:
  [[nodiscard]]
  auto tools() -> std::vector<ide_protocol::tool_t>;

  auto open_diff(QJsonObject const & arguments, ide_protocol::reply_t reply) -> void;
  auto close_tab(QJsonObject const & arguments, ide_protocol::reply_t reply) -> void;
  auto close_all_diff_tabs(QJsonObject const & arguments, ide_protocol::reply_t reply) -> void;
  auto get_diagnostics(QJsonObject const & arguments, ide_protocol::reply_t reply) -> void;

  auto write_lock_file() -> void;
  auto remove_lock_file() -> void;
  auto track_view(KDevelop::IDocument * document) -> void;
  auto schedule_selection() -> void;
  auto send_selection() -> void;
  auto send_at_mention() -> void;

  QString auth_token_;
  ide_protocol::ide_server_t * server_{};
  quint16 port_{};
  QString lock_path_;
  QHash<QString, QPointer<diff_dialog_t>> diffs_;
  QTimer selection_timer_;
  QByteArray last_selection_;
  std::unique_ptr<claude_view_factory_t> view_factory_;
  };
