#!/bin/bash
# Installs the KDevelop plugin for the current user only, without elevated privileges.
#  - copies kdevcxx_with_ai.so to ~/.local/<qt plugin dir>/kdevplatform/<version>/
#  - writes ~/.config/plasma-workspace/env/kdevcxx_with_ai.sh, which Plasma sources at login and which prepends
#    that plugin dir to QT_PLUGIN_PATH, so every KDevelop started in the Plasma session finds the plugin.
# Usage: ./install_user.sh [build dir, default build/clang-release]
set -euo pipefail

build_dir="${1:-build/clang-release}"
prefix="${HOME}/.local"
env_dir="${XDG_CONFIG_HOME:-${HOME}/.config}/plasma-workspace/env"
env_script="${env_dir}/kdevcxx_with_ai.sh"
manifest="${build_dir}/install_manifest_kdevcxx_with_ai.txt"

cmake --install "${build_dir}" --component kdevcxx_with_ai --prefix "${prefix}"

plugin_so="$(grep '/kdevcxx_with_ai\.so$' "${manifest}" | head -n 1)"
if [[ -z "${plugin_so}" ]]; then
  echo "kdevcxx_with_ai.so not found in ${manifest}" >&2
  exit 1
fi
# <plugin root>/kdevplatform/<version>/kdevcxx_with_ai.so -> <plugin root>
plugin_root="$(dirname "$(dirname "$(dirname "${plugin_so}")")")"

mkdir -p "${env_dir}"
cat > "${env_script}" <<EOF
# Written by kdevcxx_with_ai install_user.sh: Qt plugins installed in the user's prefix.
case ":\${QT_PLUGIN_PATH:-}:" in
  *":${plugin_root}:"*) ;;
  *) export QT_PLUGIN_PATH="${plugin_root}\${QT_PLUGIN_PATH:+:\${QT_PLUGIN_PATH}}" ;;
esac
EOF

echo "Installed ${plugin_so}"
echo "Wrote ${env_script}"
case ":${QT_PLUGIN_PATH:-}:" in
  *":${plugin_root}:"*) echo "QT_PLUGIN_PATH already set in this session; restart KDevelop." ;;
  *) echo "Log out and back in to Plasma once, then restart KDevelop." ;;
esac
