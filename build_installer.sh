#!/bin/bash
set -euo pipefail

# ============================================================================
# On Tap — macOS Installer Builder
# Component pkgs -> Distribution -> productbuild -> sign -> notarize -> staple
#
# Same structure as TALLBOY's scripts/build_installer.sh. v1.0.0 shipped as a
# flat pkgbuild pkg with no Distribution file, so Installer.app ran it under
# Rosetta on Apple Silicon (Q-57). The Distribution below declares
# hostArchitectures="arm64,x86_64", and scripts/check_pkg_arch.sh gates both
# the unsigned and the final pkg so a regression cannot ship.
# ============================================================================

VERSION="${1:-1.0.0}"
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
DIST_DIR="$PROJECT_DIR/dist/ontap-v${VERSION}"
INSTALLER_DIR="$PROJECT_DIR/installer"
RESOURCES="$INSTALLER_DIR/Resources"
PKG_SIGNED="$INSTALLER_DIR/OnTap-v${VERSION}-Installer.pkg"
GATE="$PROJECT_DIR/scripts/check_pkg_arch.sh"

SIGN_ID_INSTALLER="Developer ID Installer: Miguel Silverio (K7FRP77ZZK)"
NOTARY_PROFILE="carbonator-notary"
IDENT_BASE="com.carbonatedaudio.ontap"

echo "=== On Tap v${VERSION} — macOS Installer ==="
echo ""

# Verify dist folder exists
if [ ! -d "$DIST_DIR" ]; then
    echo "ERROR: Distribution folder not found: $DIST_DIR"
    echo "Run ./distribute.sh first."
    exit 1
fi

# Branded wizard pages are required by the Shipping Playbook. Build without
# them rather than fail, but say so loudly - it is not release-ready.
HAVE_RESOURCES=1
for f in sidebar.png welcome.html license.html conclusion.html; do
    [ -f "$RESOURCES/$f" ] || { echo "WARNING: missing installer resource $RESOURCES/$f"; HAVE_RESOURCES=0; }
done
[ "$HAVE_RESOURCES" = 1 ] || echo "WARNING: building a plain (unbranded) wizard - not release-ready per Playbook."
echo ""

WORK="$(mktemp -d)"
trap 'rc=$?; rm -rf "$WORK"; exit $rc' EXIT
PKGS=()

component() { # $1 bundle path, $2 install dir, $3 pkg suffix
    local src="$1" dest="$2" suf="$3"
    [ -d "$src" ] || { echo "ERROR: missing $suf bundle: $src"; exit 1; }
    local root="$WORK/root_$suf"
    mkdir -p "$root$dest"
    # ditto, never cp -R: cp corrupts the PACE symlink inside the AAX bundle.
    ditto "$src" "$root$dest/$(basename "$src")"
    pkgbuild --root "$root" --identifier "$IDENT_BASE.$suf" --version "$VERSION" \
             --install-location "/" "$WORK/$suf.pkg" > /dev/null
    PKGS+=("$suf")
    echo "  packaged $suf: $(basename "$src")"
}

# --- Step 1: Component packages ---
echo "[1/6] Building component packages..."
component "$DIST_DIR/VST3/On Tap.vst3"       "/Library/Audio/Plug-Ins/VST3"                     vst3
component "$DIST_DIR/AU/On Tap.component"    "/Library/Audio/Plug-Ins/Components"               au
component "$DIST_DIR/AAX/On Tap.aaxplugin"   "/Library/Application Support/Avid/Audio/Plug-Ins" aax
echo ""

# --- Step 2: Distribution + productbuild ---
echo "[2/6] Building product archive (Distribution, hostArchitectures=arm64,x86_64)..."
DIST="$WORK/distribution.xml"
{
    echo '<?xml version="1.0" encoding="utf-8"?>'
    echo '<installer-gui-script minSpecVersion="1">'
    echo '    <title>On Tap</title>'
    if [ "$HAVE_RESOURCES" = 1 ]; then
        echo '    <welcome    file="welcome.html"/>'
        echo '    <license    file="license.html"/>'
        echo '    <conclusion file="conclusion.html"/>'
        echo '    <background          file="sidebar.png" alignment="bottomleft" scaling="proportional"/>'
        echo '    <background-darkAqua file="sidebar.png" alignment="bottomleft" scaling="proportional"/>'
    fi
    echo '    <options customize="never" require-scripts="false" hostArchitectures="arm64,x86_64"/>'
    echo '    <choices-outline>'
    echo '        <line choice="default">'
    for n in "${PKGS[@]}"; do echo "            <line choice=\"$IDENT_BASE.$n\"/>"; done
    echo '        </line>'
    echo '    </choices-outline>'
    echo '    <choice id="default"/>'
    for n in "${PKGS[@]}"; do
        echo "    <choice id=\"$IDENT_BASE.$n\" visible=\"false\"><pkg-ref id=\"$IDENT_BASE.$n\"/></choice>"
        echo "    <pkg-ref id=\"$IDENT_BASE.$n\" version=\"$VERSION\" onConclusion=\"none\">$n.pkg</pkg-ref>"
    done
    echo '</installer-gui-script>'
} > "$DIST"

RES_ARGS=()
[ "$HAVE_RESOURCES" = 1 ] && RES_ARGS=(--resources "$RESOURCES")
PKG_UNSIGNED="$WORK/OnTap-v${VERSION}-unsigned.pkg"
productbuild --distribution "$DIST" ${RES_ARGS[@]+"${RES_ARGS[@]}"} \
             --package-path "$WORK" "$PKG_UNSIGNED" > /dev/null
echo "  Built: $(basename "$PKG_UNSIGNED")"
echo ""

# --- Step 3: Release gate (before spending a notarization round-trip) ---
echo "[3/6] Release gate..."
"$GATE" "$PKG_UNSIGNED"
echo ""

# --- Step 4: Sign pkg ---
echo "[4/6] Signing package..."
rm -f "$PKG_SIGNED"
productsign \
    --sign "$SIGN_ID_INSTALLER" \
    "$PKG_UNSIGNED" \
    "$PKG_SIGNED"
echo "  Signed: $(basename "$PKG_SIGNED")"
echo ""

# --- Step 5: Notarize ---
echo "[5/6] Submitting for Apple notarization..."
xcrun notarytool submit "$PKG_SIGNED" \
    --keychain-profile "$NOTARY_PROFILE" \
    --wait 2>&1 | tail -4

echo ""

# --- Step 6: Staple ---
echo "[6/6] Stapling notarization ticket..."
xcrun stapler staple "$PKG_SIGNED"
echo ""

# --- Verify ---
echo "=== Verification ==="
echo ""

# Release gate again on what actually ships
"$GATE" "$PKG_SIGNED"
echo ""

# Check signature
pkgutil --check-signature "$PKG_SIGNED" 2>&1 | head -5
echo ""

# Check notarization
STAPLE_RESULT=$(xcrun stapler validate "$PKG_SIGNED" 2>&1 | grep -o "worked" || echo "FAILED")
echo "Notarization staple: $STAPLE_RESULT"
echo ""

# Check Gatekeeper
if spctl --assess --type install "$PKG_SIGNED" 2>&1; then
    echo "Gatekeeper: PASSED"
else
    echo "Gatekeeper: FAILED"
fi
echo ""

# File size
SIZE=$(du -h "$PKG_SIGNED" | cut -f1)
echo "Installer: $PKG_SIGNED"
echo "Size: $SIZE"
echo ""

echo "=== Done! On Tap v${VERSION} installer ready ==="
