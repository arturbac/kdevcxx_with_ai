// SPDX-FileCopyrightText: 2026 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

#include <ide_protocol.h>

#include <interfaces/iplugin.h>

#include <QHash>
#include <QPointer>
#include <QTimer>
#include <memory>

namespace ide_qt
  {
class ide_server_t;
  }

namespace KDevelop
  {
class IDocument;
  }

class diff_dialog_t;
class claude_view_factory_t;

/// KDevelop client for Claude Code: tool view with claude in Konsole plus the IDE integration server.
/// Implements the IDE side of the protocol tools; Qt <-> std conversion happens here.
class kdevcxx_with_ai : public KDevelop::IPlugin, public ide_protocol::ide_tools_t
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

  auto open_diff(ide_protocol::open_diff_args_t const & args, ide_protocol::diff_reply_t reply) -> void override;
  auto close_tab(std::string_view tab_name) -> void override;
  auto close_all_diff_tabs() -> std::size_t override;
  [[nodiscard]]
  auto diagnostics(std::string_view file) -> ide_protocol::diagnostics_t override;

private:
  /// slot: writes the lock file, warns the user once when that fails
  auto write_lock_file() -> void;
  [[nodiscard]]
  auto try_write_lock_file() -> ide_protocol::expected_ec<void>;
  /// visible warning in KDevelop plus a log line
  auto post_warning(QString const & text) -> void;
  auto remove_lock_file() -> void;
  auto track_view(KDevelop::IDocument * document) -> void;
  auto schedule_selection() -> void;
  auto send_selection() -> void;
  auto send_at_mention() -> void;

  std::string auth_token_;
  ide_qt::ide_server_t * server_{};
  quint16 port_{};
  QString lock_path_;
  QHash<QString, QPointer<diff_dialog_t>> diffs_;
  QTimer selection_timer_;
  std::string last_selection_;
  bool lock_file_warned_{};
  std::unique_ptr<claude_view_factory_t> view_factory_;
  };
