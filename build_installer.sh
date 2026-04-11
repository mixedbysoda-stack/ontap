#!/bin/bash
set -euo pipefail

# ============================================================================
# On Tap — macOS Installer Builder
# Signs, notarizes, and staples a .pkg installer
# ============================================================================

VERSION="${1:-1.0.0}"
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
DIST_DIR="$PROJECT_DIR/dist/ontap-v${VERSION}"
INSTALLER_DIR="$PROJECT_DIR/installer"
PAYLOAD="$INSTALLER_DIR/payload"
PKG_UNSIGNED="$INSTALLER_DIR/OnTap-v${VERSION}-unsigned.pkg"
PKG_SIGNED="$INSTALLER_DIR/OnTap-v${VERSION}-Installer.pkg"

SIGN_ID_INSTALLER="Developer ID Installer: Miguel Silverio (K7FRP77ZZK)"
NOTARY_PROFILE="carbonator-notary"

echo "=== On Tap v${VERSION} — macOS Installer ==="
echo ""

# Verify dist folder exists
if [ ! -d "$DIST_DIR" ]; then
    echo "ERROR: Distribution folder not found: $DIST_DIR"
    echo "Run ./distribute.sh first."
    exit 1
fi

# --- Step 1: Stage payload ---
echo "[1/5] Staging payload..."
rm -rf "$PAYLOAD"
mkdir -p "$PAYLOAD/Library/Audio/Plug-Ins/VST3"
mkdir -p "$PAYLOAD/Library/Audio/Plug-Ins/Components"
mkdir -p "$PAYLOAD/Library/Application Support/Avid/Audio/Plug-Ins"

cp -R "$DIST_DIR/VST3/On Tap.vst3"        "$PAYLOAD/Library/Audio/Plug-Ins/VST3/"
cp -R "$DIST_DIR/AU/On Tap.component"      "$PAYLOAD/Library/Audio/Plug-Ins/Components/"
cp -R "$DIST_DIR/AAX/On Tap.aaxplugin"     "$PAYLOAD/Library/Application Support/Avid/Audio/Plug-Ins/"

echo "  VST3:  On Tap.vst3"
echo "  AU:    On Tap.component"
echo "  AAX:   On Tap.aaxplugin"
echo ""

# --- Step 2: Build unsigned pkg ---
echo "[2/5] Building unsigned package..."
pkgbuild \
    --root "$PAYLOAD" \
    --identifier "com.carbonatedaudio.ontap.pkg" \
    --version "$VERSION" \
    --install-location "/" \
    "$PKG_UNSIGNED" > /dev/null 2>&1

echo "  Built: $(basename "$PKG_UNSIGNED")"
echo ""

# --- Step 3: Sign pkg ---
echo "[3/5] Signing package..."
rm -f "$PKG_SIGNED"
productsign \
    --sign "$SIGN_ID_INSTALLER" \
    "$PKG_UNSIGNED" \
    "$PKG_SIGNED"

rm -f "$PKG_UNSIGNED"
echo "  Signed: $(basename "$PKG_SIGNED")"
echo ""

# --- Step 4: Notarize ---
echo "[4/5] Submitting for Apple notarization..."
xcrun notarytool submit "$PKG_SIGNED" \
    --keychain-profile "$NOTARY_PROFILE" \
    --wait 2>&1 | tail -4

echo ""

# --- Step 5: Staple ---
echo "[5/5] Stapling notarization ticket..."
xcrun stapler staple "$PKG_SIGNED"
echo ""

# --- Verify ---
echo "=== Verification ==="
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
