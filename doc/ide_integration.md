# Claude Code IDE integration

The plugin implements the IDE side of the Claude Code IDE integration protocol: MCP (JSON-RPC 2.0) over a
WebSocket. The core is in `src/ide_protocol` (Qt Core and Qt WebSockets only, covered by unit tests). The
KDevelop glue is in `src/kdevcxx_with_ai`.

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
