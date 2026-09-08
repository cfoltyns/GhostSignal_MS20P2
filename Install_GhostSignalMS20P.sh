#!/bin/bash
# ============================================================================
#  Ghost Signal MS20P - One-Click Installer (Linux)
#  Run:  ./Install_GhostSignalMS20P.sh
#  Installs the VST3 plugin and Standalone app for the current user.
# ============================================================================

set -e
APP_NAME="GhostSignalMS20P"
VST3_FOLDER="${APP_NAME}.vst3"

echo ""
echo " ============================================================"
echo "  Ghost Signal MS20P - Installer"
echo " ============================================================"
echo ""

SRC_ROOT="$(cd "$(dirname "$0")" && pwd)"

# ---------------------------------------------------------------
# 1. Locate the VST3 bundle (packaged layout or source build)
# ---------------------------------------------------------------
VST3_SRC=""
for cand in \
    "${SRC_ROOT}/VST3/${VST3_FOLDER}" \
    "${SRC_ROOT}/build/GhostSignalMS20P_artefacts/Release/VST3/${VST3_FOLDER}" \
    "${SRC_ROOT}/build/GhostSignalMS20P_artefacts/VST3/${VST3_FOLDER}" \
    "${SRC_ROOT}/build/GhostSignalMS20P_artefacts/Release/vst3/${VST3_FOLDER}"; do
    if [ -z "${VST3_SRC}" ] && [ -d "${cand}" ]; then
        VST3_SRC="${cand}"
    fi
done

if [ -z "${VST3_SRC}" ]; then
    echo " [ERROR] Could not find ${VST3_FOLDER}."
    echo "         Build the project first:"
    echo "           cmake -S . -B build"
    echo "           cmake --build build"
    echo ""
    read -r -p "Press ENTER to close..." _x
    exit 1
fi

# ---------------------------------------------------------------
# 2. Locate the Standalone binary
# ---------------------------------------------------------------
BIN_SRC=""
for cand in \
    "${SRC_ROOT}/bin/${APP_NAME}" \
    "${SRC_ROOT}/build/GhostSignalMS20P_artefacts/Release/Standalone/${APP_NAME}" \
    "${SRC_ROOT}/build/GhostSignalMS20P_artefacts/Standalone/${APP_NAME}"; do
    if [ -z "${BIN_SRC}" ] && [ -f "${cand}" ]; then BIN_SRC="${cand}"; fi
done

# ---------------------------------------------------------------
# 3. Install the VST3 plugin (user-level, no admin required)
# ---------------------------------------------------------------
VST3_DEST="${HOME}/.vst3"
mkdir -p "${VST3_DEST}"
rm -rf "${VST3_DEST}/${VST3_FOLDER}"
cp -R "${VST3_SRC}" "${VST3_DEST}/${VST3_FOLDER}"
echo " [OK] VST3 plugin installed to:"
echo "      ${VST3_DEST}/${VST3_FOLDER}"

# If running as root, also install system-wide.
if [ "$(id -u)" = "0" ]; then
    SYS_VST3="/usr/lib/vst3"
    mkdir -p "${SYS_VST3}"
    rm -rf "${SYS_VST3}/${VST3_FOLDER}"
    cp -R "${VST3_SRC}" "${SYS_VST3}/${VST3_FOLDER}"
    echo " [OK] VST3 plugin also installed to:"
    echo "      ${SYS_VST3}/${VST3_FOLDER}"
fi

# ---------------------------------------------------------------
# 4. Install the Standalone app
# ---------------------------------------------------------------
if [ -n "${BIN_SRC}" ]; then
    if [ "$(id -u)" = "0" ]; then
        BIN_DEST="/usr/local/bin"
        cp -f "${BIN_SRC}" "${BIN_DEST}/${APP_NAME}"
    else
        BIN_DEST="${XDG_BIN_HOME:-${HOME}/.local/bin}"
        mkdir -p "${BIN_DEST}"
        cp -f "${BIN_SRC}" "${BIN_DEST}/${APP_NAME}"
    fi
    chmod +x "${BIN_DEST}/${APP_NAME}"
    echo " [OK] Standalone app installed to:"
    echo "      ${BIN_DEST}/${APP_NAME}"
else
    echo " [SKIP] Standalone app not found."
fi

echo ""
echo " ============================================================"
echo "  Install complete!"
echo "  - Restart your DAW / plug-in manager and rescan VST3."
echo "  - Launch the Standalone app by running:  ${APP_NAME}"
echo " ============================================================"
echo ""
read -r -p "Press ENTER to close..." _x
exit 0