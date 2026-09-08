#!/bin/bash
# ============================================================================
#  Ghost Signal MS20P - One-Click Installer (macOS)
#  Double-click this file (or run: bash "Install_GhostSignalMS20P.command")
#  to install the VST3 plugin, AU component and Standalone app.
# ============================================================================

set -e
APP_NAME="GhostSignalMS20P"
VST3_FOLDER="${APP_NAME}.vst3"
AU_COMPONENT="${APP_NAME}.component"

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
    "${SRC_ROOT}/build/GhostSignalMS20P_artefacts/VST3/${VST3_FOLDER}"; do
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
# 2. Locate the AU component and Standalone app
# ---------------------------------------------------------------
AU_SRC=""; APP_BUNDLE_SRC=""
for cand in \
    "${SRC_ROOT}/AU/${AU_COMPONENT}" \
    "${SRC_ROOT}/build/GhostSignalMS20P_artefacts/Release/AU/${AU_COMPONENT}" \
    "${SRC_ROOT}/build/GhostSignalMS20P_artefacts/AU/${AU_COMPONENT}"; do
    if [ -z "${AU_SRC}" ] && [ -d "${cand}" ]; then AU_SRC="${cand}"; fi
done
for cand in \
    "${SRC_ROOT}/App/${APP_NAME}.app" \
    "${SRC_ROOT}/build/GhostSignalMS20P_artefacts/Release/Standalone/${APP_NAME}.app" \
    "${SRC_ROOT}/build/GhostSignalMS20P_artefacts/Standalone/${APP_NAME}.app"; do
    if [ -z "${APP_BUNDLE_SRC}" ] && [ -d "${cand}" ]; then APP_BUNDLE_SRC="${cand}"; fi
done

# ---------------------------------------------------------------
# 3. Install the VST3 plugin (user-level, no admin required)
# ---------------------------------------------------------------
VST3_DEST="${HOME}/Library/Audio/Plug-Ins/VST3"
mkdir -p "${VST3_DEST}"
rm -rf "${VST3_DEST}/${VST3_FOLDER}"
cp -R "${VST3_SRC}" "${VST3_DEST}/${VST3_FOLDER}"
echo " [OK] VST3 plugin installed to:"
echo "      ${VST3_DEST}/${VST3_FOLDER}"

# ---------------------------------------------------------------
# 4. Install the AU component
# ---------------------------------------------------------------
if [ -n "${AU_SRC}" ]; then
    AU_DEST="${HOME}/Library/Audio/Plug-Ins/Components"
    mkdir -p "${AU_DEST}"
    rm -rf "${AU_DEST}/${AU_COMPONENT}"
    cp -R "${AU_SRC}" "${AU_DEST}/${AU_COMPONENT}"
    echo " [OK] AU component installed to:"
    echo "      ${AU_DEST}/${AU_COMPONENT}"
    echo "      (If Logic Pro / GarageBand was running, restart it.)"
else
    echo " [SKIP] AU component not found (may require a macOS build)."
fi

# ---------------------------------------------------------------
# 5. Install the Standalone app
# ---------------------------------------------------------------
if [ -n "${APP_BUNDLE_SRC}" ]; then
    if [ -w "/Applications" ]; then
        APP_DEST="/Applications"
    else
        APP_DEST="${HOME}/Applications"
        mkdir -p "${APP_DEST}"
    fi
    rm -rf "${APP_DEST}/${APP_NAME}.app"
    cp -R "${APP_BUNDLE_SRC}" "${APP_DEST}/${APP_NAME}.app"
    echo " [OK] Standalone app installed to:"
    echo "      ${APP_DEST}/${APP_NAME}.app"
    echo "      (First launch: right-click the app -> Open to bypass Gatekeeper.)"
else
    echo " [SKIP] Standalone app not found."
fi

echo ""
echo " ============================================================"
echo "  Install complete!"
echo "  - Restart your DAW / plug-in manager and rescan VST3/AU."
echo " ============================================================"
echo ""
read -r -p "Press ENTER to close..." _x
exit 0