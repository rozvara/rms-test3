// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

void initSerialDisplays() {

  Serial1.begin(19200);    // oba externí na Serial1
  LEDDisplay.begin(19200); // LED display (interní)

  intDisp = {
    .target = &LEDDisplay,
    .addr = 0,
    .offset = 0,
    .len = 7,
    .colonPin = LEDdp
  };

  extDis1 = {
    .target = &Serial1,
    .addr = 0,
    .offset = 0,
    .len = 7,
    .colonPin = 0
  };

 extDis2 = {
    .target = &Serial1,
    .addr = 8,
    .offset = 0,
    .len = 7,
    .colonPin = 0
  };

  uint8_t buf[DATA_LEN];
  char charbuf[DATA_LEN];
  if (getEeprom(0, 0, buf)) { // pokud displeje nejsou defaultní, musí být i jejich layouty

    #ifdef MINIMIZE_FLASH
      systemEcho(F("Disp.init"));
    #else
      systemEcho(F("Konfigurace displeju"));
    #endif  

    extDis1.len = buf[0]; extDis1.addr = buf[1]; extDis1.offset = buf[2];
    extDis2.len = buf[3]; extDis2.addr = buf[4]; extDis2.offset = buf[5];
    if (getEeprom(0, 1, charbuf)) {
      memcpy(extDis1Layout, charbuf, extDis1.len);
      SERIAL_PRINTLN(extDis1Layout);
    } else {
      #ifdef MINIMIZE_FLASH
        systemEcho(F("Chyba Disp1"));
      #else
        systemEcho(F("Chyba konfigurace displeje 1"));
      #endif  
    }

    if (getEeprom(0, 2, charbuf)) { // musí být i jejich layouty
      memcpy(extDis2Layout, charbuf, extDis1.len);
      SERIAL_PRINTLN(extDis2Layout);
    } else {
      #ifdef MINIMIZE_FLASH
        systemEcho(F("Chyba Disp2"));
      #else
        systemEcho(F("Chyba konfigurace displeje 2"));
      #endif  
    }
  }
}

void updateSerialDisplays() {
  static bool oneOrAnother;
  intDisp.updateDisplay();
  extDis1.updateDisplay(timerOneSec && oneOrAnother);  // posílají se změny + zotavení z výpadkku napájení
  extDis2.updateDisplay(timerOneSec && !oneOrAnother); // jednou sekundu jeden nebo druhý
  if (timerOneSec) oneOrAnother = !oneOrAnother;
}

void reverse(const char* src, char* dst, uint8_t len) {
  for (uint8_t i = 0; i < len; i++) {
    dst[i] = src[len - 1 - i];
  }
} 


// void testExternalDisplays() {
//   char testChars[16];
//   testChars[0] = ':';
//   testChars[1] = '1';
//   testChars[2] = '2';
//   testChars[3] = '3';
//   testChars[4] = '4';
//   testChars[5] = 'L';
//   extDis1.write(&testChars[0]);
//   testChars[0] = ' ';
//   testChars[1] = '9';
//   testChars[2] = '8';
//   testChars[3] = '7';
//   testChars[4] = '6';
//   testChars[5] = 'P';
//   extDis2.write(&testChars[0]);
// }
