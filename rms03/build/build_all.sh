#!/bin/bash

# RMS - firmware pro modulární stopky
# (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

# složku fw/ vždy smaže, aby tam bylo jen co se povedlo

inofile="rms03.ino" # hlavní soubor projektu

path="$(pwd)"
logfile="$path/build.log"
fw_path="$path/fw"
buildscript="$path/build_one.sh"
temppath="/tmp/rmsbuild" # je i v build_one

mkdir -p "$temppath/build"

rm -rf "$fw_path"
mkdir -p "$fw_path"

rm "$logfile"

hfile="$(dirname "$logfile")/../build.h"
cp "$hfile" "$temppath/build.h"

cd ../config/versions
for header in *.h; do
    echo "Zpracovávám $header..."

    pathname="$(pwd)/$header"
    projectpath="$(dirname "$pathname")/../../"

    # KVŮLI ARDUINO IDE/BUILDER PŘEPISUJE build.h
    rm "$hfile"
    cp "$header" "$hfile"

    FW_VERSION=$(grep -E '^#define[[:space:]]+FW_VERSION[[:space:]]' "${projectpath}${inofile}" | sed -E 's/.*"([^"]+)".*/\1/')
    FW_TAG=$(grep -E '^#define[[:space:]]+FW_TAG[[:space:]]+"' $header | sed -E 's/.*"([^"]+)".*/\1/')
    MCU=$(grep -E '^#define[[:space:]]+TARGET_MCU_' "$header" | sed -E 's/.*TARGET_MCU_//')

    "$buildscript" "$pathname" "${projectpath}${inofile}" "$logfile"
    result=$?

    if [[ "$result" == "0" ]]; then
        cp "${temppath}/${inofile}.hex" "${fw_path}/${FW_VERSION}${FW_TAG}-$(echo "$MCU" | tr 'A-Z' 'a-z').hex"
    fi
    echo | tee -a "$logfile"
done

rm "$hfile"
cp "$temppath/build.h" "$hfile"

cd "${fw_path}/"
detox .

cd "$path"
