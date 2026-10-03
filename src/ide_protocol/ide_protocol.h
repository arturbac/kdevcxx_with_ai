// SPDX-FileCopyrightText: 2026 Artur Bać
// SPDX-License-Identifier: MIT

#pragma once

// Claude Code IDE integration protocol: MCP (JSON-RPC 2.0) over a local WebSocket.
// Only the parts the claude CLI actually uses are implemented: tools openDiff, close_tab, closeAllDiffTabs,
// getDiagnostics and the notifications selection_changed, at_mentioned.
// No Qt here: all strings are UTF-8, the IDE side implements ide_tools_t.

#include <simple_enum/generic_error_category.hpp>
#include <simple_enum/simple_enum.hpp>

#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace ide_protocol
  {
using simple_enum::expected_ec;

inline constexpr std::string_view auth_header{"x-claude-code-ide-authorization"};
inline constexpr std::string_view ide_name{"KDevelop"};

// openDiff results understood by the claude CLI
inline constexpr std::string_view file_saved{"FILE_SAVED"};
inline constexpr std::string_view diff_rejected{"DIFF_REJECTED"};
inline constexpr std::string_view tab_closed{"TAB_CLOSED"};

/// 32 lowercase hex chars (128 bits) from the kernel CSPRNG
[[nodiscard]]
auto make_auth_token() -> expected_ec<std::string>;

/// Directory the claude CLI scans for lock files: $CLAUDE_CONFIG_DIR/ide, else ~/.claude/ide.
/// The CLI always scans ~/.claude/ide as well, so the fallback is visible to every profile.
[[nodiscard]]
auto lock_dir(std::string_view claude_config_dir, std::filesystem::path const & home) -> std::filesystem::path;

/// Content of <lock_dir>/<port>.lock
[[nodiscard]]
auto lock_file_json(std::int64_t pid, std::span<std::string const> workspace_folders, std::string_view auth_token)
  -> std::string;

/// Shell command line that starts claude connected to the IDE server on port
[[nodiscard]]
auto claude_launch_command(std::uint16_t port, std::string_view claude_command) -> std::string;

/// file:// URI of an absolute path, bytes outside RFC 3986 pchar are percent-encoded
[[nodiscard]]
auto file_uri(std::string_view path) -> std::string;

/// Accepts "file://" URIs (percent-decoded) and plain paths
[[nodiscard]]
auto uri_to_path(std::string_view uri) -> std::string;

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

consteval auto adl_enum_bounds(severity_e) -> simple_enum::adl_info<severity_e>
  { return {severity_e::error, severity_e::hint}; }

/// name used in getDiagnostics results
[[nodiscard]]
auto severity_name(severity_e severity) -> std::string_view;

struct diagnostic_t
  {
  std::string file_path;
  std::string message;
  severity_e severity;
  position_t start;
  position_t end;
  std::string source;
  };

struct diagnostics_t
  {
  /// files that must be present in the result even without diagnostics
  std::vector<std::string> files;
  std::vector<diagnostic_t> diagnostics;
  };

struct open_diff_args_t
  {
  std::string old_file_path;
  std::string new_file_path;
  std::string new_file_contents;
  std::string tab_name;
  };

enum struct diff_outcome_e : std::uint8_t
  {
  accepted,
  rejected,
  tab_closed
  };

consteval auto adl_enum_bounds(diff_outcome_e) -> simple_enum::adl_info<diff_outcome_e>
  { return {diff_outcome_e::accepted, diff_outcome_e::tab_closed}; }

/// must be called exactly once, may be called later (openDiff waits for the user)
using diff_reply_t = std::function<void(diff_outcome_e outcome)>;

/// IDE side of the tools, implemented by the plugin
class ide_tools_t
  {
public:
  ide_tools_t() = default;
  ide_tools_t(ide_tools_t const &) = delete;
  ide_tools_t(ide_tools_t &&) = delete;
  auto operator=(ide_tools_t const &) -> ide_tools_t & = delete;
  auto operator=(ide_tools_t &&) -> ide_tools_t & = delete;
  virtual ~ide_tools_t() = default;

  virtual auto open_diff(open_diff_args_t const & args, diff_reply_t reply) -> void = 0;
  /// a pending openDiff for tab_name, if any, replies tab_closed
  virtual auto close_tab(std::string_view tab_name) -> void = 0;
  /// returns the number of closed tabs
  virtual auto close_all_diff_tabs() -> std::size_t = 0;
  /// file empty: all open files
  [[nodiscard]]
  virtual auto diagnostics(std::string_view file) -> diagnostics_t = 0;
  };

using send_t = std::function<void(std::string message)>;

/// Handles one incoming JSON-RPC message, responses go through send (possibly later, for openDiff).
/// Returns the method of a received notification (e.g. ide_connected), otherwise empty.
auto handle_message(std::string_view message, ide_tools_t & tools, send_t const & send) -> std::string;

[[nodiscard]]
auto selection_changed(std::string_view path, std::string_view text, position_t start, position_t end) -> std::string;

[[nodiscard]]
auto at_mentioned(std::string_view path, std::optional<int> line_start, std::optional<int> line_end) -> std::string;
  }  // namespace ide_protocol
