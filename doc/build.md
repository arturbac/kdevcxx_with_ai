# Build

## Requirements

- CMake 3.31 or newer (required by glaze; workflow presets need 3.25), Ninja
- A C++23 compiler: clang 19 or newer (CI builds with clang 19 and clang 23)
- KDevelop 6 development files (KDevPlatform), KDE Frameworks 6 (Config, CoreAddons, I18n, Parts,
  TextEditor, XmlGui), Extra CMake Modules
- Qt 6 with Qt WebSockets
- Boost headers (KDevPlatform needs them)

Fetched with CPM during configure:

| Library | Version | Used in |
|---|---|---|
| [glaze](https://github.com/stephenberry/glaze) | v9.0.0 | core: JSON |
| [arturbac/simple_enum](https://github.com/arturbac/simple_enum) | `master` | core: enum bounds, `expected_ec` |
| [arturbac/stralgo](https://github.com/arturbac/stralgo) | `master` | Qt adapter: UTF-8 ↔ UTF-16 |
| [arturbac/small_vectors](https://github.com/arturbac/small_vectors) | `master` | dependency of stralgo, built static |
| [arturbac/ut-ext](https://github.com/arturbac/ut-ext) | v2.0.1_9 | unit tests |

During development the author's libraries track `master`. small_vectors throws exceptions, so the Qt adapter and
the plugin are built with exceptions enabled (KDE settings disable them by default).

The oldest distribution with all of these is Debian 13 (KDevelop 24.12, KF6 6.13, Qt 6.8), which CI uses.

## Presets

Configure, build and test go through CMake presets. `CMakePresets.json` includes the files in `cmake/`:

| File | Content |
|---|---|
| `cmake/preset-options.json` | hidden building blocks: `cfg-clang`, `cfg-gcc`, `cfg-debug`, `cfg-release`, `cfg-asan`, `cfg-time-trace`, `cfg-build-dir` |
| `cmake/preset-configure.json` | configure presets |
| `cmake/preset-build.json` | build presets |
| `cmake/preset-test.json` | test presets |
| `cmake/preset-workflow.json` | workflow presets (configure, build, test) |

| Preset | Build type | Notes | Workflow |
|---|---|---|---|
| `clang-debug` | Debug | | yes |
| `clang-release` | Release | | yes |
| `clang-release-test-asan` | Release | AddressSanitizer and UndefinedBehaviorSanitizer | yes |
| `clang-release-time-trace` | Release | `-ftime-trace` | no |
| `ci-clang` | Release | used by CI | yes |

Each preset builds in `build/<preset name>`. The compiler is `clang++` from `PATH`.

```bash
cmake --workflow --preset clang-release
```

The plugin is `build/<preset name>/bin/kdevcxx_with_ai.so`.

## Machine-specific presets

Settings of one machine (compiler path, parallel jobs, install prefix) belong in `CMakeUserPresets.json`, which
is not tracked by git. It can inherit the presets above, for example:

```json
{
  "version": 6,
  "configurePresets": [
    {
      "name": "local",
      "inherits": [ "clang-debug" ],
      "installDir": "$env{HOME}/.local",
      "cacheVariables": { "CMAKE_CXX_COMPILER": "/usr/lib/llvm/23/bin/clang++" }
    }
  ],
  "buildPresets": [
    { "name": "local", "configurePreset": "local", "jobs": 24 },
    { "name": "local-install", "configurePreset": "local", "jobs": 24, "targets": [ "install" ] }
  ],
  "testPresets": [
    { "name": "local", "configurePreset": "local", "output": { "outputOnFailure": true } }
  ],
  "workflowPresets": [
    {
      "name": "local",
      "steps": [
        { "type": "configure", "name": "local" },
        { "type": "build", "name": "local" },
        { "type": "test", "name": "local" },
        { "type": "build", "name": "local-install" }
      ]
    }
  ]
}
```

`cmake --workflow --preset local` then configures, builds, tests and installs into `~/.local` (see
[install.md](install.md)).

## CI

`.github/workflows/ci.yml` runs on pull requests and on pushes to `beta-1`. It builds in a `debian:trixie`
container with clang 19 (the Debian compiler, the oldest supported) and clang 23 (from apt.llvm.org), using
`cmake --workflow --preset ci-clang`. CPM sources and ccache are cached between runs.
