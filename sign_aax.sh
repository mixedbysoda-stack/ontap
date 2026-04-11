#!/bin/bash
# On Tap AAX Plugin Signing Script

if [ -z "$1" ]; then
    echo "Usage: ./sign_aax.sh YOUR_PACE_PASSWORD"
    exit 1
fi

PASSWORD="$1"
AAX_IN="/Users/soda/Desktop/TapOn/build/TapOn_artefacts/Release/AAX/On Tap.aaxplugin"
AAX_OUT="/Users/soda/Desktop/TapOn/build/TapOn_artefacts/Release/AAX/On Tap_signed.aaxplugin"

echo "Signing On Tap AAX..."

/Applications/PACEAntiPiracy/Eden/Fusion/Versions/5/bin/wraptool sign \
    --account sodanswishers \
    --wcguid D90D90C0-10A1-11F1-AFA6-00505692FF7 \
    --password "$PASSWORD" \
    --signid 45C26CF1655F48EBC8A21802BDA053514719E1F0 \
    --in "$AAX_IN" \
    --out "$AAX_OUT" \
    --verbose

if [ $? -eq 0 ]; then
    echo "Signing successful!"
    echo "Signed plugin: $AAX_OUT"
else
    echo "Signing failed. Check the error above."
fi
