# Install

The plugin installs per user, without elevated privileges.

1. Install into a user prefix with a configure preset that sets `installDir` (see the `local` example in
   [build.md](build.md)):

   ```bash
   cmake --workflow --preset local          # configure, build, test, install
   # or, after a configure:
   cmake --build --preset local-install
   ```

   With prefix `~/.local` the plugin lands in `~/.local/<lib dir>/plugins/kdevplatform/<KDevelop plugin version>/`,
   e.g. `~/.local/lib64/plugins/kdevplatform/66/kdevcxx_with_ai.so`. CPM dependencies are not installed.

2. Once, make the plugin visible to KDevelop:

   ```bash
   ./setup_plasma_env.sh build/local
   ```

   KDevelop finds plugins only in the Qt plugin paths, so the prefix must be in `QT_PLUGIN_PATH`. The script reads
   the installed plugin path from `build/local/install_manifest.txt` and writes
   `~/.config/plasma-workspace/env/kdevcxx_with_ai.sh`. Plasma sources that file at login, and the file prepends
   the plugin root (e.g. `~/.local/lib64/plugins`) to `QT_PLUGIN_PATH`. Every KDevelop started in the Plasma
   session then finds the plugin.

After `setup_plasma_env.sh`, log out of Plasma and back in once. After later installs, only restart KDevelop.

- A KDevelop started outside the Plasma session needs `QT_PLUGIN_PATH` set by hand.
- Rebuild and reinstall after every KDevelop upgrade, because the plugin is built against the installed
  KDevPlatform.
- To uninstall, delete the installed `kdevcxx_with_ai.so` and `~/.config/plasma-workspace/env/kdevcxx_with_ai.sh`.
