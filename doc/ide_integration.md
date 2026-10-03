# Claude Code IDE integration

The plugin implements the IDE side of the Claude Code IDE integration protocol: MCP (JSON-RPC 2.0) over a
WebSocket.

## Code layout

| Directory | Layer | Depends on |
|---|---|---|
| `src/ide_protocol` | core: JSON-RPC/MCP handling, message shapes, lock file, URIs; all strings UTF-8 | C++23, glaze, simple_enum (no Qt) |
| `src/ide_qt` | Qt adapter: WebSocket transport (`ide_server_t`) and the UTF-8 ↔ UTF-16 conversion (`qt_bridge.h`, stralgo) | core, Qt Core, Qt WebSockets |
| `src/kdevcxx_with_ai` | KDevelop plugin: tool view, diff dialog, settings, DUChain diagnostics | Qt adapter, KF6, KDevPlatform |

The core declares `ide_protocol::ide_tools_t` (open a diff, close diff tabs, collect diagnostics) in standard
types; the plugin implements it. Strings are converted between Qt and the core only at that boundary, once per
call. The core target does not link Qt, so a Qt include in it fails to compile. Unit tests cover the core
(`ide_protocol_ut`), the string conversion (`qt_bridge_ut`) and the transport with a real `QWebSocket` client
(`ide_server_ut`).

Only the parts the `claude` CLI uses are implemented (checked against Claude Code 2.1.283).

## Discovery

On load the plugin:

1. starts a WebSocket server on `127.0.0.1` with a free port (`ide_server_t`), and
2. writes a lock file `<dir>/<port>.lock`, readable only by the owner, where `<dir>` is
   `$CLAUDE_CONFIG_DIR/ide` when that variable is set, otherwise `~/.claude/ide`.
   The CLI always scans `~/.claude/ide` as well, so either location works for any CLI profile.

```json
{"pid": 12345, "workspaceFolders": ["/path/to/project"], "ideName": "KDevelop",
 "transport": "ws", "authToken": "<32 lowercase hex chars>"}
```

`workspaceFolders` holds the open KDevelop projects. The file is rewritten when a project opens or closes,
and removed when the plugin unloads.

The tool view starts the CLI as

```
env CLAUDE_CODE_SSE_PORT=<port> ENABLE_IDE_INTEGRATION=true <claude command>
```

With `CLAUDE_CODE_SSE_PORT` set, the CLI connects to that port directly, whatever its working directory is.

A client must send the header `x-claude-code-ide-authorization: <authToken>`, otherwise the server closes the
connection.

## Requests handled

| Method | Result |
|---|---|
| `initialize` | echoes the client's `protocolVersion`, capability `tools` |
| `tools/list` | the four tools below |
| `tools/call` | runs the tool, unknown tool → error `-32602` |
| `ping` | `{}` |
| anything else with an `id` | error `-32601` |

Notifications from the CLI get no answer. `ide_connected` makes the plugin resend the last selection.

## Tools

| Tool | Arguments | Result text |
|---|---|---|
| `openDiff` | `old_file_path`, `new_file_path`, `new_file_contents`, `tab_name` | `FILE_SAVED` + final contents, or `DIFF_REJECTED` |
| `close_tab` | `tab_name` | `TAB_CLOSED` |
| `closeAllDiffTabs` | none | `CLOSED_<n>_DIFF_TABS` |
| `getDiagnostics` | `uri` (optional, `file://` URI or path) | JSON array of `{uri, diagnostics[]}` |

`openDiff` answers only after the user decides: **Accept** → `FILE_SAVED`, **Reject** or closing the dialog
→ `DIFF_REJECTED`. The CLI writes the file itself. When the CLI closes the dialog with `close_tab` or
`closeAllDiffTabs` (e.g. after an answer in the terminal), the pending `openDiff` gets `TAB_CLOSED`.
The diff is `diff -u` of the file on disk (or `/dev/null` for a new file) against the proposed contents.

`getDiagnostics` reads the DUChain problems of each file's top context (500 ms lock timeout, so a busy
DUChain returns no problems instead of blocking). Each diagnostic has `message`, `severity`, `range`
(0-based `line`/`character`) and `source`. Severity mapping: Error → `Error`, Warning → `Warning`,
Hint → `Info`, no severity → `Hint`. Without `uri`, all open local documents are reported.

## Notifications sent

- `selection_changed` — `text`, `filePath`, `fileUrl`, `selection{start, end, isEmpty}` for the active editor,
  sent 100 ms after the selection or cursor stops changing, only when it differs from the last one.
- `at_mentioned` — `filePath`, plus 0-based `lineStart`/`lineEnd` when there is a selection, sent by the
  *Send to Claude Code* action. A selection ending at column 0 does not include its last line.

## Errors

Exceptions are enabled for the whole project. The project's own code does not throw. Errors travel as
`expected_ec` (`std::error_code`, project codes in `ide_protocol::ide_error_e`):

- Core functions are `noexcept`. An exception from code outside the project (standard library, glaze) is
  caught in the function that called it (`catch_to_expected`) and returned as an error code. `std::bad_alloc` maps
  to `out_of_memory`, `std::system_error` keeps its code, and anything else maps to `internal_error`.
- `handle_message` answers a failure while handling a request, also one thrown by an `ide_tools_t`
  implementation, with JSON-RPC error `-32603` and the exception text. The claude CLI gets an answer either way.
- Qt and KDE report errors through return values, `error()`/`errorString()`, out parameters and error signals.
  The plugin checks them right after each call, converts them to error codes and logs the Qt text there.
- Code called from the Qt event loop (slots, timers, socket callbacks) runs inside `ide_qt::event_guard`, which
  logs and stops anything that escaped, so no exception passes through Qt. The guard sits at the event loop
  entry, not in every function called from there.
- A repeated failure is logged once per streak (`ide_protocol::failure_streak_t`): the first failure is reported,
  the next ones are not until a success ends the streak.

Where the error path ends:

| Error | Result |
|---|---|
| a selection update cannot be built | logged once per streak, skipped; the next update replaces it |
| a message from the claude CLI cannot be handled | logged once per streak |
| no auth token, or no free port on 127.0.0.1 | IDE integration disabled with a warning in KDevelop; the Claude Code tool view still starts `claude` |
| the lock file cannot be written | warning in KDevelop (once until a write succeeds again); retried when a project opens or closes |
| a failure in a tool call | JSON-RPC error to the claude CLI |
| the lock file cannot be removed when the plugin unloads | logged |
| settings cannot be saved | logged; the new value applies until KDevelop exits |

The plugin never ends the KDevelop process.
