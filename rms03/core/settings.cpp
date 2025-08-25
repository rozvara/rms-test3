// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.


bool delEeprom(uint8_t idModul, uint8_t idKey) {
  Record r;
  for (uint8_t i = 0; i < BLOCK_COUNT; i++) {
    readBlock(i, r);
    if (!validCrc(r)) continue;
    if ((r.idModul == idModul) && (r.idKey & 0x7F) == idKey) {
      uint8_t buf[12] = {0};
      memcpy(r.data, buf, DATA_LEN);
      r.idModul = idModul | 0x80; // 7.bit=smazaný záznam
      r.crc = crc16((uint8_t*)&r, 2 + DATA_LEN);
      writeBlock(i, r);
      return true;
    }
  }
  return false;
}


void showLcdSetupInstructions() {
  lcdClear();
  #ifdef LCD_16X2
    lcdModuleTitle(F("->Terminal"));
    lcdWrite(0, 1, F("115200bit 'help'"));
  #endif
  #ifdef LCD_20X4
    lcdModuleTitle(F("Nastaveni"));
    lcdWrite(0, 2, F("Serial 115200/8/1bit"));
    lcdWrite(2, 3, F("Terminal: 'help'"));
  #endif
  intDisp.write(F("--55-- ")); // jako System Setup :)
  extDis1.fill(' ');
  extDis2.fill(' ');  
}


void showSystemInfo() {

  SERIAL_PRINT(F("HW: ")); SERIAL_PRINTLN(F(HW_NAME)); 

  #define BUILD_DATE __DATE__
  #define BUILD_TIME __TIME__
  SERIAL_PRINT(F("FW: ")); SERIAL_PRINT(F(FW_VERSION)); SERIAL_PRINT(F(FW_TAG));
  SERIAL_PRINT(F(" (")); SERIAL_PRINT(F(BUILD_DATE)); Serial.write(' '); SERIAL_PRINT(F(BUILD_TIME)); SERIAL_PRINTLN(F(")"));
  SERIAL_PRINT(F("    Pocet modulu: ")); SERIAL_PRINTLN(modulesInstalled);
  SERIAL_PRINT(F("    Log zaznamu: ")); SERIAL_PRINTLN(MAX_LOG_ENTRIES);
  // TODO EEPROM records
}


void showHelp() {
  SERIAL_PRINTLN(F("\nPrikazy:"));
  SERIAL_PRINTLN(F("  list [m] [k]             - vypise zaznamy (vse | jeden modul | jen modul+klic"));
  SERIAL_PRINTLN(F("  set <m> <k> \"text\"       - nastavi pro <modul> <klic> az 12 znaku"));
  SERIAL_PRINTLN(F("  set <m> <k> H [,H,H...]  - nastavi pro <modul> <klic> az 12 hodnot 0-255"));
  SERIAL_PRINTLN(F("  del <m> <k>              - smaze zaznam <modul> <klic>"));

#ifdef HW_RTC_DS3231
  SERIAL_PRINTLN(F("  rtc YYYY-MM-DD HH:MM:SS  - nastavi cas v RTC"));
#endif

//TODO interní hodiny???
//SERIAL_PRINTLN(F("  time HH MM SS"));
//SERIAL_PRINTLN(F("  date DD MM YYYY"));

  #ifndef MINIMIZE_FLASH
  SERIAL_PRINTLN(F("  perf <1|0>               - zap/vyp zobrazovani rychlosti programu"));
  #endif
  SERIAL_PRINTLN(F("  info                     - zobrazi udaje o hardware/firmware"));
  SERIAL_PRINTLN(F("  help                     - tato napoveda"));
}


void moduleSetupLoop() {

  if (moduleState == MOD_EXITING) return;
  if (moduleState == MOD_SHUTDOWN) { 
    intDisp.fill(' ');
    moduleState = MOD_EXITING;
    return;
  }
  if (moduleState == MOD_STARTING) {
    showLcdSetupInstructions();
    while (Serial.available()) Serial.read(); // zahoď buffer před spuštěním
    moduleState = MOD_IDLE;
    return;
  }

  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  moduleEcho(F("Prijato '"), false); SERIAL_PRINT(cmd); SERIAL_PRINTLN("'");

  bool cmdDone = false;
  bool cmdError = false;

  if (cmd.startsWith("list")) {
    int modFilter = -1, keyFilter = -1;
    sscanf(cmd.c_str(), "list %d %d", &modFilter, &keyFilter);

    SERIAL_PRINTLN(F("Modul Klic Data"));
    SERIAL_PRINTLN(F("---------------"));
    uint8_t count = 0;
    for (int i = 0; i < BLOCK_COUNT; i++) {
      Record r;
      readBlock(i, r);
      if (!validCrc(r) || (r.idModul & 0x80)) continue; // vadný nebo smazaný

      uint8_t mod = r.idModul & 0x7F;
      uint8_t key = r.idKey & 0x7F;
      bool isText = r.idKey & 0x80;

      if ((modFilter >= 0 && mod != modFilter) ||
          (keyFilter >= 0 && key != keyFilter)) continue;

      SERIAL_PRINT("  ");
      SERIAL_PRINT(mod < 100 ? (mod < 10 ? F("  ") : F(" ")) : F(""));
      SERIAL_PRINT(mod); SERIAL_PRINT("  ");
      SERIAL_PRINT(key < 100 ? (key < 10 ? F("  ") : F(" ")) : F(""));
      SERIAL_PRINT(key); Serial.write(' ');
//            SERIAL_PRINT(isText ? F("znaky ") : F("cisla "));

      if (isText) {
        SERIAL_PRINT('"');
        for (uint8_t j = 0; j < DATA_LEN; j++) {
          char c = (char)r.data[j];
          if (c == 0) break;
          SERIAL_PRINT(c);
        }
        SERIAL_PRINTLN('"');
      } else {
        for (uint8_t j = 0; j < DATA_LEN; j++) {
          if (j > 0) Serial.write(',');
          SERIAL_PRINT(r.data[j] < 100 ? (r.data[j] < 10 ? F("  ") : F(" ")) : F(""));
          SERIAL_PRINT(r.data[j]);
        }
        SERIAL_PRINTLN();
      }
      count++;
    }
    if (count > 0) SERIAL_PRINTLN(F("---------------"));
    SERIAL_PRINT(F("Nalezeno: ")); SERIAL_PRINTLN(count);
    cmdDone = true;
  }

  if (cmd.startsWith("set ")) {
    int m, k;
    String msg;
    msg = F("Chybny vstup.");
    const char* src = cmd.c_str();
    if (sscanf(src, "set %d %d", &m, &k) == 2) {
      // set
      if (m>=0 || m<=127 || k>=0 || k<=127) {
        const char* p = strchr(src, ' ');
        if (!p) goto fail;
        p = strchr(p + 1, ' ');
        if (!p) goto fail;
        p = strchr(p + 1, ' ');
        if (!p) goto fail;
        while (*p == ' ') p++;

        if (*p == '"') { // text
          p++;
          const char* end = strchr(p, '"');
          if (!end) goto fail;
          char buf[12] = {0};
          strncpy(buf, p, min(12, (int)(end - p)));
          if (setEeprom(m, k | 0x80, buf))
            msg = F("OK. Text ulozen.");
          else 
            // FIXME err do core
            msg = F("Chyba. (Text pres hodnoty?)");
        } else { // hodnoty oddělené čárkami
          uint8_t buf[12] = {0};
          uint8_t count = 0;
          while (*p && count < 12) {
            while (*p == ' ') p++;
            int val = atoi(p);
            if (val < 0 || val > 255) goto fail;
            buf[count++] = val;
            p = strchr(p, ',');
            if (!p) break;
            p++;
          }
          if (setEeprom(m, k & 0x7F, buf))
            msg = ("OK. Hodnoty ulozeny.");
          else 
            msg = F("Chyba. (Hodnoty pres text?)");
        }
      }
      fail: {}
    }
    SERIAL_PRINTLN(msg);
    return;
  }

  if (cmd.startsWith("del ")) {
    // FIXME maže smazaný/odsmaže
    int m, k;
    if (sscanf(cmd.c_str(), "del %d %d", &m, &k) == 2) {
      if (m>=0 || m<=127 || k>=0 || k<=127) {
        if (delEeprom(m, k)) {
          SERIAL_PRINTLN(F("Zaznam smazan."));
          cmdDone = true;
        }
        else {
          SERIAL_PRINT(F("Nenalezeno. "));
          cmdError = true; cmdDone = true;
        }
      } else {
        cmdError = true; cmdDone = true;
      }
    }
  }

  #ifdef HW_RTC_DS3231
    if (cmd.startsWith("rtc ")) {
      String rest = cmd.substring(4);
      rest.trim();

      const char* raw = rest.c_str();
      int y, mo, d, h, mi, s;
      if (6 == sscanf(raw, "%d-%d-%d %d:%d:%d", &y, &mo, &d, &h, &mi, &s)) {
        rtc.adjust(DateTime(y, mo, d, h, mi, s));
        SERIAL_PRINT(F("Cas v RTC: "));
        DateTime dt = rtc.now();
        char buffer[20];
        sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d", 
          dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());
        SERIAL_PRINTLN(buffer);
      } else {
        cmdError = true;
      }
      cmdDone = true; 
    }
  #endif

  #ifndef MINIMIZE_FLASH
    if (cmd.startsWith("perf ")) {
      String rest = cmd.substring(5);
      int val = rest.toInt();
      if (val == 0) { showPerfOverlay = false; showLcdSetupInstructions(); }
      else if (val == 1) { showPerfOverlay = true; }
      else { cmdError = true; }
      cmdDone = true;
    }
  #endif

  if (cmd == "help") {
    showHelp();
    cmdDone = true;
  }

  if (cmd == "info") {
    showSystemInfo();
    cmdDone = true;
  }

  if (!cmdDone) {
    SERIAL_PRINT(F("Neznamy prikaz."));
  }
  if (cmdError) {
    SERIAL_PRINT(F("Chyba."));
  }
  if (!cmdDone || cmdError) {
    SERIAL_PRINTLN(F(" Zadej 'help'.")); buzzer(WARN_BEEP);
  } else {
    SERIAL_PRINTLN(); buzzer(INFO_BEEP);
  }
}
