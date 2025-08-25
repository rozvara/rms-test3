# RMS - firmware pro modulární stopky
# (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

# params:  build.h  main.ino  logfile

compile() {
   arduino-builder \
      -compile \
      -logger=machine \
      -hardware /usr/share/arduino/hardware \
      -hardware $HOME/.arduino15/packages \
      -tools /usr/share/arduino/hardware/tools/avr \
      -tools $HOME/.arduino15/packages \
      -libraries $HOME/Arduino/libraries \
      "$param1" \
      -ide-version=10819 \
      -build-path "$temppath" \
      -warnings=none \
      -build-cache "$temppath" \
      -prefs=build.warn_data_percentage=75 \
      -prefs=runtime.tools.avr-gcc.path=$HOME/.arduino15/packages/arduino/tools/avr-gcc/7.3.0-atmel3.6.1-arduino7 \
      -prefs=runtime.tools.avr-gcc-7.3.0-atmel3.6.1-arduino7.path=$HOME/.arduino15/packages/arduino/tools/avr-gcc/7.3.0-atmel3.6.1-arduino7 \
      -prefs=runtime.tools.arduinoOTA.path=$HOME/.arduino15/packages/arduino/tools/arduinoOTA/1.3.0 \
      -prefs=runtime.tools.arduinoOTA-1.3.0.path=$HOME/.arduino15/packages/arduino/tools/arduinoOTA/1.3.0 \
      "$param2" \
      "$param3" \
      -verbose "$inofile"
}

header="$1"
inofile="$2"
logfile="$3"

temppath="/tmp/rmsbuild" # je i v build_all

FW_VERSION=$(grep -E '^#define[[:space:]]+FW_VERSION[[:space:]]' ${inofile} | sed -E 's/.*"([^"]+)".*/\1/')
FW_TAG=$(grep -E '^#define[[:space:]]+FW_TAG[[:space:]]+"' $header | sed -E 's/.*"([^"]+)".*/\1/')
MCU=$(grep -E '^#define[[:space:]]+TARGET_MCU_' "$header" | sed -E 's/.*TARGET_MCU_//')

echo "                $FW_VERSION$FW_TAG for $MCU" | tee -a "$logfile"

if [[ -z "$FW_VERSION" || -z "$FW_TAG" || -z "$MCU" ]]; then
        echo "CHYBA nastavení $header (chybí FW_VERSION nebo FW_TAG nebo MCU)" | tee -a "$logfile"
    exit 1
fi

param1=""
param2="-prefs=runtime.tools.avrdude.path=$HOME/.arduino15/packages/MightyCore/tools/avrdude/8.0-arduino.1"
param3="-prefs=runtime.tools.avrdude-8.0-arduino.1.path=$HOME/.arduino15/packages/MightyCore/tools/avrdude/8.0-arduino.1"

case "$MCU" in
  MEGA)
    param1="-fqbn=arduino:avr:mega:cpu=atmega2560"
    ;;
  ATMEGA644P)
    param1="-fqbn=MightyCore:avr:644:bootloader=uart0,eeprom=keep,baudrate=default,pinout=standard,variant=modelP,BOD=2v7,LTO=Os_flto,clock=16MHz_external"
    ;;
  ATMEGA1284P)
    param1="-fqbn=MightyCore:avr:1284:bootloader=uart0,eeprom=keep,baudrate=default,pinout=standard,variant=modelP,BOD=2v7,LTO=Os_flto,clock=16MHz_external"
    ;;
  LEONARDO)
    param1="-fqbn=arduino:avr:leonardo"
    ;;
esac

compile
result=$?
if [[ "$result" == "0" ]]; then
    echo "OK              $header" | tee -a "$logfile"
else
    echo "CHYBA build     $header" | tee -a "$logfile"
    exit 1
fi
