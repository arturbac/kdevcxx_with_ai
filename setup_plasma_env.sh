#!/bin/bash
# Makes a plugin installed into a user prefix (e.g. ~/.local) visible to every KDevelop in the Plasma session.
# Writes ~/.config/plasma-workspace/env/kdevcxx_with_ai.sh, which Plasma sources at login and which prepends the
# installed Qt plugin dir to QT_PLUGIN_PATH. Run once after the first install.
# Usage: ./setup_plasma_env.sh <build dir of an installed build, e.g. build/local>
set -euo pipefail

build_dir="${1:?usage: $0 <build dir>}"
env_dir="${XDG_CONFIG_HOME:-${HOME}/.config}/plasma-workspace/env"
env_script="${env_dir}/kdevcxx_with_ai.sh"
manifest="${build_dir}/install_manifest.txt"

plugin_so="$(grep '/kdevcxx_with_ai\.so$' "${manifest}" | head -n 1 || true)"
if [[ -z "${plugin_so}" ]]; then
  echo "kdevcxx_with_ai.so not found in ${manifest}; install the build first" >&2
  exit 1
fi
# <plugin root>/kdevplatform/<version>/kdevcxx_with_ai.so -> <plugin root>
plugin_root="$(dirname "$(dirname "$(dirname "${plugin_so}")")")"

mkdir -p "${env_dir}"
cat > "${env_script}" <<EOS
# Written by kdevcxx_with_ai setup_plasma_env.sh: Qt plugins installed in the user's prefix.
case ":\${QT_PLUGIN_PATH:-}:" in
  *":${plugin_root}:"*) ;;
  *) export QT_PLUGIN_PATH="${plugin_root}\${QT_PLUGIN_PATH:+:\${QT_PLUGIN_PATH}}" ;;
esac
EOS

echo "Wrote ${env_script}"
case ":${QT_PLUGIN_PATH:-}:" in
  *":${plugin_root}:"*) echo "QT_PLUGIN_PATH already set in this session; restart KDevelop." ;;
  *) echo "Log out and back in to Plasma once, then restart KDevelop." ;;
esac
