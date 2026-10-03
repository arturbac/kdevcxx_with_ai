# KDevCXX with AI

A KDevelop plugin that turns KDevelop into a client for [Claude Code](https://code.claude.com/docs/en/overview).

The plugin does not talk to any AI service itself. It runs the `claude` CLI in a KDevelop tool view and
implements the Claude Code IDE integration protocol, so Claude Code sees KDevelop the same way it sees the
VS Code or JetBrains extensions.

## Features

- **Claude Code tool view**: an embedded Konsole (bottom dock) that starts `claude` in the first open project
  directory, already connected to KDevelop. When the Konsole part closes (the shell exited), a new one is
  loaded.
- **Diff review**: file changes proposed by Claude open in a dialog with a unified diff. **Accept** applies the
  change, **Reject** rejects it. The terminal prompt in the tool view stays usable as well.
- **Diagnostics**: Claude reads KDevelop's language diagnostics (DUChain problems, e.g. from clang) through the
  `getDiagnostics` tool, for one file or for all open files.
- **Selection context**: the current editor selection (or cursor position) is sent to Claude whenever it
  changes.
- **Send to Claude Code**: editor context menu action (also *Send Selection to Claude Code* in the shortcut
  settings) that adds the current file and selected lines to the Claude prompt.

Protocol details: [doc/ide_integration.md](doc/ide_integration.md).

## Requirements

KDevelop 6 with KDE Frameworks 6 and Qt 6 (Qt WebSockets), CMake 3.31+, clang 19+. At runtime: Konsole
(`konsolepart`), `diff` from diffutils and the [Claude Code CLI](https://code.claude.com/docs/en/overview)
(`claude`). Details in [doc/build.md](doc/build.md).

## Build and install

```bash
git clone https://github.com/arturbac/kdevcxx_with_ai.git
cd kdevcxx_with_ai
cmake --workflow --preset clang-release
```

The plugin installs per user, without elevated privileges: [doc/install.md](doc/install.md). Presets,
machine-specific presets and CI: [doc/build.md](doc/build.md).

## Configuration

*Settings → Configure KDevelop → Claude Code*:

| Setting | Config key (`kdeveloprc`, group `[kdevcxx_with_ai]`) | Default |
|---|---|---|
| Claude command | `claude_command` | `claude` |

The command is used as typed when the tool view starts, so arguments are allowed, e.g. `claude --model opus`.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
