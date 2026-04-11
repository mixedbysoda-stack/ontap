#!/bin/bash
set -euo pipefail

# ============================================================================
# On Tap v1 — Build, Sign, Notarize & Package
# ============================================================================
# Usage: ./distribute.sh [version]
# Example: ./distribute.sh 1.0.0
# ============================================================================

VERSION="${1:-1.0.0}"
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
ARTEFACTS="$BUILD_DIR/TapOn_artefacts/Release"
DIST_DIR="$PROJECT_DIR/dist/ontap-v${VERSION}"

# Signing identity (SHA-1 hash)
SIGN_ID="45C26CF1655F48EBC8A21802BDA053514719E1F0"

# Notarization keychain profile (set up with: xcrun notarytool store-credentials)
NOTARY_PROFILE="carbonator-notary"

# PACE wraptool
WRAPTOOL="/Applications/PACEAntiPiracy/Eden/Fusion/Versions/5/bin/wraptool"
PACE_ACCOUNT="sodanswishers"
PACE_WCGUID="D90D90C0-10A1-11F1-AFA6-005056920FF7"

# Plugin paths (after build)
VST3="$ARTEFACTS/VST3/On Tap.vst3"
AU="$ARTEFACTS/AU/On Tap.component"
AAX="$ARTEFACTS/AAX/On Tap.aaxplugin"
AAX_SIGNED="$ARTEFACTS/AAX/On Tap_signed.aaxplugin"
STANDALONE="$ARTEFACTS/Standalone/On Tap.app"

# ============================================================================
echo "=== On Tap v${VERSION} — Distribution Build ==="
echo ""

# --- Step 1: Rebuild as Universal Binary ---
echo "[1/6] Building universal binary (arm64 + x86_64)..."
cd "$PROJECT_DIR"
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" -DCMAKE_OSX_DEPLOYMENT_TARGET="10.13" > /dev/null 2>&1
cmake --build build --config Release --clean-first 2>&1 | tail -1
echo "  Verifying architectures..."
for bin in "$VST3/Contents/MacOS/On Tap" "$AU/Contents/MacOS/On Tap" \
           "$STANDALONE/Contents/MacOS/On Tap" "$AAX/Contents/MacOS/On Tap"; do
    ARCH=$(lipo -info "$bin" 2>/dev/null | grep -o "x86_64 arm64" || true)
    if [ "$ARCH" != "x86_64 arm64" ]; then
        echo "  ERROR: $bin is not universal!"
        exit 1
    fi
done
echo "  All binaries are universal."
echo ""

# --- Step 1b: Inject LSMinimumSystemVersion into plists ---
echo "  Setting LSMinimumSystemVersion=10.13 in all Info.plists..."
for plist in "$VST3/Contents/Info.plist" "$AU/Contents/Info.plist" \
             "$AAX/Contents/Info.plist" "$STANDALONE/Contents/Info.plist"; do
    /usr/libexec/PlistBuddy -c "Delete :LSMinimumSystemVersion" "$plist" 2>/dev/null || true
    /usr/libexec/PlistBuddy -c "Add :LSMinimumSystemVersion string 10.13" "$plist"
done
echo "  Done."
echo ""

# --- Step 2: Code Sign VST3, AU, Standalone ---
echo "[2/6] Code signing VST3, AU, Standalone..."
for plugin in "$VST3" "$AU" "$STANDALONE"; do
    codesign --force --deep --options runtime \
        --sign "$SIGN_ID" "$plugin"
    NAME=$(basename "$plugin")
    echo "  Signed: $NAME"
done
echo ""

# --- Step 3: Notarize VST3, AU, Standalone ---
echo "[3/6] Submitting for Apple notarization..."
NOTARY_PIDS=()
NOTARY_ZIPS=()
for plugin in "$VST3" "$AU" "$STANDALONE"; do
    NAME=$(basename "$plugin" | sed 's/\..*//')
    FORMAT=$(basename "$plugin" | sed 's/.*\.//')
    ZIP="/tmp/ontap_${NAME}_${FORMAT}.zip"
    ditto -c -k --keepParent "$plugin" "$ZIP"
    echo "  Submitting $(basename "$plugin")..."
    xcrun notarytool submit "$ZIP" \
        --keychain-profile "$NOTARY_PROFILE" --wait 2>&1 | tail -2 &
    NOTARY_PIDS+=($!)
    NOTARY_ZIPS+=("$ZIP")
done

# Wait for all notarization submissions
NOTARY_FAILED=0
for pid in "${NOTARY_PIDS[@]}"; do
    if ! wait "$pid"; then
        NOTARY_FAILED=1
    fi
done
if [ "$NOTARY_FAILED" -eq 1 ]; then
    echo "  ERROR: One or more notarization submissions failed!"
    echo "  Run: xcrun notarytool log <submission-id> --keychain-profile $NOTARY_PROFILE"
    exit 1
fi
echo "  All notarization submissions accepted."
echo ""

# --- Step 4: Staple notarization tickets ---
echo "[4/6] Stapling notarization tickets..."
for plugin in "$VST3" "$AU" "$STANDALONE"; do
    xcrun stapler staple "$plugin" > /dev/null 2>&1
    echo "  Stapled: $(basename "$plugin")"
done
echo ""

# --- Step 5: Sign AAX with PACE wraptool ---
echo "[5/6] Signing AAX with PACE wraptool..."
read -sp "  PACE password for '$PACE_ACCOUNT': " PACE_PASSWORD
echo ""

rm -rf "$AAX_SIGNED"
"$WRAPTOOL" sign \
    --verbose \
    --account "$PACE_ACCOUNT" \
    --password "$PACE_PASSWORD" \
    --wcguid "$PACE_WCGUID" \
    --signid "$SIGN_ID" \
    --in "$AAX" \
    --out "$AAX_SIGNED" 2>&1 | grep -E "^(Successfully|Error)" || true
echo "  AAX signed: $(basename "$AAX_SIGNED")"
echo ""

# --- Step 6: Package distribution ---
echo "[6/6] Packaging distribution..."
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"/{VST3,AU,AAX,Standalone}

cp -R "$VST3"        "$DIST_DIR/VST3/"
cp -R "$AU"          "$DIST_DIR/AU/"
cp -R "$AAX_SIGNED"  "$DIST_DIR/AAX/On Tap.aaxplugin"
cp -R "$STANDALONE"  "$DIST_DIR/Standalone/"

echo "  Distribution folder: $DIST_DIR"
echo ""

# --- Final verification ---
echo "=== Verification ==="
echo ""
echo "Architectures:"
for bin in "$DIST_DIR/VST3/On Tap.vst3/Contents/MacOS/On Tap" \
           "$DIST_DIR/AU/On Tap.component/Contents/MacOS/On Tap" \
           "$DIST_DIR/Standalone/On Tap.app/Contents/MacOS/On Tap" \
           "$DIST_DIR/AAX/On Tap.aaxplugin/Contents/MacOS/On Tap"; do
    printf "  %-50s %s\n" "$(basename "$(dirname "$(dirname "$(dirname "$bin")")")")" "$(lipo -info "$bin" 2>/dev/null | grep -o 'x86_64 arm64')"
done
echo ""

echo "Code signatures:"
for plugin in "$DIST_DIR/VST3/On Tap.vst3" \
              "$DIST_DIR/AU/On Tap.component" \
              "$DIST_DIR/Standalone/On Tap.app" \
              "$DIST_DIR/AAX/On Tap.aaxplugin"; do
    AUTHORITY=$(codesign -dvv "$plugin" 2>&1 | grep "Authority=Developer" | head -1 | sed 's/Authority=//')
    printf "  %-50s %s\n" "$(basename "$plugin")" "$AUTHORITY"
done
echo ""

echo "Notarization:"
for plugin in "$DIST_DIR/VST3/On Tap.vst3" \
              "$DIST_DIR/AU/On Tap.component" \
              "$DIST_DIR/Standalone/On Tap.app"; do
    RESULT=$(xcrun stapler validate "$plugin" 2>&1 | grep -o "worked" || echo "FAILED")
    printf "  %-50s %s\n" "$(basename "$plugin")" "$RESULT"
done
echo ""

echo "Gatekeeper (Standalone):"
if spctl --assess --type execute "$DIST_DIR/Standalone/On Tap.app" 2>&1; then
    echo "  PASSED"
else
    echo "  FAILED"
fi
echo ""

echo "=== Done! On Tap v${VERSION} ready for distribution ==="
echo "  $DIST_DIR"
