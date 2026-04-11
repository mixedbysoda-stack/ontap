#!/bin/bash
# On Tap AAX Plugin Wrapping Script

if [ -z "$1" ]; then
    echo "Usage: ./wrap_aax.sh YOUR_PACE_PASSWORD"
    exit 1
fi

PASSWORD="$1"
AAX_IN="/Users/soda/Desktop/TapOn/build/TapOn_artefacts/Release/AAX/On Tap.aaxplugin"
AAX_OUT="/Users/soda/Desktop/TapOn/build/TapOn_artefacts/Release/AAX/On Tap_wrapped.aaxplugin"
PT_PLUGINS="/Library/Application Support/Avid/Audio/Plug-Ins"

echo "Wrapping On Tap AAX..."

/Applications/PACEAntiPiracy/Eden/Fusion/Versions/5/bin/wraptool wrap \
    --verbose \
    --account sodanswishers \
    --password "$PASSWORD" \
    --wcguid D90D90C0-10A1-11F1-AFA6-00505692FF7 \
    --in "$AAX_IN" \
    --out "$AAX_OUT"

if [ $? -eq 0 ]; then
    echo "Wrapping successful!"
    echo "Installing to Pro Tools..."
    sudo rm -rf "$PT_PLUGINS/On Tap.aaxplugin"
    sudo cp -R "$AAX_OUT" "$PT_PLUGINS/On Tap.aaxplugin"
    echo "Done! Restart Pro Tools to load On Tap."
else
    echo "Wrapping failed."
fi
