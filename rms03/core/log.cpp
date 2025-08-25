// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

#ifndef MAX_LOG_ENTRIES
  #define MAX_LOG_ENTRIES 16
#endif

static LogEntry logBuffer[MAX_LOG_ENTRIES];
static uint8_t logIndex = 0;


bool logEvent(LogEventType type, uint32_t time) {

  if ((type != LOG_START) && (type != LOG_RESET) && (type != LOG_FINISH) &&
      (type != LOG_FINISH_LEFT) && (type != LOG_FINISH_RIGHT)) {
    if (logIndex > MAX_LOG_ENTRIES - 3) { // musí zůstat místo pro konec závodu
      #ifdef MINIMIZE_FLASH
        systemEcho(F("NEULOZENO"));
      #else
        systemEcho(F("Naplneno max zaznamu. NEULOZENO"));
      #endif  
      return false;
    }
  }

  logBuffer[logIndex].timestamp_ms = time;
  logBuffer[logIndex].type = type;

  systemEcho(F("Log typ#"), false); SERIAL_PRINT((uint8_t)type);
  SERIAL_PRINTLN(F(" ulozen"));

  logIndex++;
  return true;
}


void printTime(Stream& out, uint32_t ms, bool hours, bool stamp) {

    uint32_t zero = 0;
    calcTimeParts(&zero, &ms);
    tpToChars(&timeStr[0], '0');

    char charbuf[10];

    if (stamp) out.print("[+");
    if (hours) {
      charbuf[0] = timeStr[1];
      charbuf[1] = timeStr[2];
      charbuf[2] = timeStr[3];
      charbuf[3] = ':';
      charbuf[4] = '\0';
      out.print(charbuf);
    }
    charbuf[0] = timeStr[5];
    charbuf[1] = timeStr[6];
    charbuf[2] = ':';
    charbuf[3] = timeStr[8];
    charbuf[4] = timeStr[9];
    charbuf[5] = '.';
    charbuf[6] = timeStr[11];
    charbuf[7] = timeStr[12];;
    charbuf[8] = timeStr[13];;
    charbuf[9] = '\0';
    out.print(charbuf);
    if (stamp) out.print("]");
}


void logDump(Stream& out) {

  #ifdef TARGET_MCU_LEONARDO
    if (&out == &Serial) {
      if (!Serial) return;
    }
  #endif

  out.println(F("\n=== Zaznam zavodu ==="));

  // Závěrečné vyhodnocení
  uint32_t tStart = 0xFFFFFFFF;
  uint32_t tLeft = 0;
  uint32_t tRight = 0;
  uint32_t tEnd = 0;

  bool hours = false;

  for (uint8_t i = 0; i < logIndex; i++) {  // najdi
    if (logBuffer[i].type == LOG_START) tStart = logBuffer[i].timestamp_ms;
    else if (logBuffer[i].type == LOG_FINISH_LEFT) tLeft = logBuffer[i].timestamp_ms;
    else if (logBuffer[i].type == LOG_FINISH_RIGHT) tRight = logBuffer[i].timestamp_ms;
  }

  tEnd = (tLeft > tRight) ? tLeft : tRight;

  if ((tStart != 0xFFFFFFFF) && (tEnd > 0)) {
    if (tEnd - tStart >= 3600000-1 ) {
      hours = true;
    }
  }

  for (uint16_t i = 0; i < logIndex; i++) {
    uint32_t rel = logBuffer[i].timestamp_ms - tStart;
    printTime(out, rel, hours, true);
    out.print(" ");
    switch (logBuffer[i].type) {
      case LOG_START:        out.print(F("START")); break;
      #ifdef HW_IN5
        case LOG_SPLIT:      out.print(F("MEZICAS")); break;
      #endif
      case LOG_FINISH_LEFT:  out.print(F("CIL LEVA DRAHA")); break;
      case LOG_FINISH_RIGHT: out.print(F("CIL PRAVA DRAHA")); break;
      case LOG_FINISH:       out.print(F("CIL")); break;
      case LOG_RESET:        out.print(F("RESET")); break;
    }
    out.println();
  }

  if (tStart != 0xFFFFFFFF && tLeft > 0 && tRight > 0) {
    uint32_t dLeft = tLeft - tStart;
    uint32_t dRight = tRight - tStart;

    out.println();
    out.println(F("--- Souhrn zavodu ---"));

    out.print(F("Levy cas:  ")); printTime(out, dLeft, hours, false); out.println();
    out.print(F("Pravy cas: ")); printTime(out, dRight, hours, false); out.println();

    if (dLeft < dRight) {
        out.println(F("Vitez:     LEVA DRAHA"));
        out.print(  F("Rozdil:    "));
        out.print((dRight - dLeft) / 1000.0, 3);
        out.println(" s");
    } else if (dRight < dLeft) {
        out.println(F("Vitez:     PRAVA DRAHA"));
        out.print(  F("Rozdil:    "));
        out.print((dLeft - dRight) / 1000.0, 3);
        out.println(" s");
    } else {
        out.println(F("Vysledek:  NEROZHODNE"));
    }
  }

  #ifdef CLOCK_AVAILABLE
    #ifdef INT_CLOCK
      bool printDateTime = internalClockRunning;
    #endif
    #ifdef HW_RTC_DS3231
      // bool printDateTime = !rtc.lostPower();    TODO ověřit chování modulu/knihovny
      bool printDateTime = true;
    #endif
    if (printDateTime) {
      out.println(F("---------------------"));
      uint8_t buf[7];
      char charbuf[11];
      currentTimeToBuf(&buf[0]);
      charbuf[0] = '0' + buf[DAY] / 10;
      charbuf[1] = '0' + buf[DAY] % 10;
      charbuf[2] = '.';
      charbuf[3] = '0' + buf[MON] / 10;
      charbuf[4] = '0' + buf[MON] % 10;
      charbuf[5] = '.';
      charbuf[6] = '0' + ((uint16_t)(2000+buf[YEAR]) / 1000) % 10;
      charbuf[7] = '0' + ((uint16_t)(2000+buf[YEAR]) / 100)  % 10;
      charbuf[8] = '0' + ((uint16_t)(2000+buf[YEAR]) / 10)   % 10;
      charbuf[9] = '0' +  (uint16_t)(2000+buf[YEAR]) % 10;
      charbuf[10] = '\0';
      out.print(  F("Datum:     "));
      out.println(charbuf);

      charbuf[0] = '0' + buf[HOUR] / 10;
      charbuf[1] = '0' + buf[HOUR] % 10;
      charbuf[2] = ':';
      charbuf[3] = '0' + buf[MIN] / 10;
      charbuf[4] = '0' + buf[MIN] % 10;
      charbuf[5] = ':';
      charbuf[6] = '0' + buf[SEC] / 10;
      charbuf[7] = '0' + buf[SEC] % 10;
      charbuf[8] = '\0';
      out.print(  F("Cas:       "));
      out.println(charbuf);
    }
  #endif

  out.println(F("=== Konec zaznamu ===\n"));
}


void logClear() {
  for (uint16_t i = 0; i < MAX_LOG_ENTRIES; i++) {
    logBuffer[i].timestamp_ms = 0xFFFFFFFF;
    logBuffer[i].type = LOG_NONE;
  }
  logIndex = 0;

  systemEcho(F("Log pripraven"));
}
