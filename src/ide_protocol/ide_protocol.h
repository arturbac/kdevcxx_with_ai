// SPDX-FileCopyrightText: 2026 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

// Claude Code IDE integration protocol: MCP (JSON-RPC 2.0) over a local WebSocket.
// Only the parts the claude CLI actually uses are implemented: tools openDiff, close_tab, closeAllDiffTabs,
// getDiagnostics and the notifications selection_changed, at_mentioned.

#include <QByteArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <cstdint>
#include <functional>
#include <optional>
#include <span>

namespace ide_protocol
  {
inline constexpr auto auth_header{"x-claude-code-ide-authorization"};
inline constexpr auto ide_name{"KDevelop"};

// openDiff results understood by the claude CLI
inline constexpr auto diff_rejected{"DIFF_REJECTED"};
inline constexpr auto tab_closed{"TAB_CLOSED"};

/// 32 lowercase hex chars (128 bits) from the system CSPRNG
[[nodiscard]]
auto make_auth_token() -> QString;

/// Directory the claude CLI scans for lock files: $CLAUDE_CONFIG_DIR/ide, else ~/.claude/ide.
/// The CLI always scans ~/.claude/ide as well, so the fallback is visible to every profile.
[[nodiscard]]
auto lock_dir(QString const & claude_config_dir, QString const & home) -> QString;

/// Content of <lock_dir>/<port>.lock
[[nodiscard]]
auto lock_file_json(qint64 pid, QStringList const & workspace_folders, QString const & auth_token) -> QByteArray;

/// Shell command line that starts claude connected to the IDE server on port
[[nodiscard]]
auto claude_launch_command(quint16 port, QString const & claude_command) -> QString;

[[nodiscard]]
auto file_uri(QString const & path) -> QString;

/// Accepts "file://" URIs and plain paths
[[nodiscard]]
auto uri_to_path(QString const & uri) -> QString;

/// 0-based, like LSP
struct position_t
  {
  int line;
  int character;
  };

enum struct severity_e : std::uint8_t
  {
  error,
  warning,
  info,
  hint
  };

struct diagnostic_t
  {
  QString file_path;
  QString message;
  severity_e severity;
  position_t start;
  position_t end;
  QString source;
  };

/// tools/call result {"content":[{"type":"text","text":...}]}
[[nodiscard]]
auto text_result(QString const & text) -> QJsonObject;

[[nodiscard]]
auto error_result(QString const & text) -> QJsonObject;

/// openDiff accepted: FILE_SAVED followed by the final file contents
[[nodiscard]]
auto file_saved_result(QString const & contents) -> QJsonObject;

/// getDiagnostics result: JSON array of {uri, diagnostics[]} grouped by file, in the text content.
/// files lists paths that must be present even without diagnostics.
[[nodiscard]]
auto diagnostics_result(QStringList const & files, std::span<diagnostic_t const> diagnostics) -> QJsonObject;

[[nodiscard]]
auto selection_changed(QString const & path, QString const & text, position_t start, position_t end) -> QByteArray;

[[nodiscard]]
auto at_mentioned(QString const & path, std::optional<int> line_start, std::optional<int> line_end) -> QByteArray;

using reply_t = std::function<void(QJsonObject const & result)>;
using send_t = std::function<void(QByteArray const & message)>;

struct tool_t
  {
  QString name;
  QString description;
  QJsonObject input_schema;
  /// must call reply exactly once, may do it later (openDiff waits for the user)
  std::function<void(QJsonObject const & arguments, reply_t reply)> call;
  };

/// Handles one incoming JSON-RPC message, responses go through send.
/// Returns the method of a received notification (e.g. ide_connected), otherwise empty.
auto handle_message(QByteArray const & message, std::span<tool_t const> tools, send_t const & send) -> QString;
  }  // namespace ide_protocol
