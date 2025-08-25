// RMS - firmware pro modulární stopky
// Modul "Hodiny"
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

#ifndef CLOCK_AVAILABLE
  #error "Modul Hodiny potřebuje zdroj času"
#endif

void m04DisplayDateTime() {
  #ifdef LCD_16X2
    enum { r = 0 };
  #else
    enum { r = 2 };
  #endif

  #ifdef INT_CLOCK
    if (!internalClockRunning) {
      #ifdef LCD_16X2
        lcdWrite(0, 1, F("R>Nastav dR>Menu"));
      #else
        lcdWrite(1, 2, F("Reset > Nastaveni"));
        lcdWrite(0, 3, F("Dlouze Reset > Menu"));
      #endif
      return;
    }
  #endif

  uint8_t buf[7];
  char charbuf[15];
  char reversed[7];
  currentTimeToBuf(&buf[0]);

  //lcd čas
  charbuf[0] = (buf[HOUR] > 9) ? '0' + buf[HOUR] / 10 : ' ';
  charbuf[1] = '0' + buf[HOUR] % 10;
  charbuf[2] = ':';
  charbuf[3] = '0' + buf[MIN] / 10;
  charbuf[4] = '0' + buf[MIN] % 10;
  charbuf[5] = ':';
  charbuf[6] = '0' + buf[SEC] / 10;
  charbuf[7] = '0' + buf[SEC] % 10;
  charbuf[8] = '\0';
  enum { c0 = (LCD_COLS - 8) / 2 };
  lcdWrite(c0, 0+r, charbuf);

  // lcd datum
  charbuf[ 0] = dayOfWeek[buf[DOW]][0];
  charbuf[ 1] = dayOfWeek[buf[DOW]][1];
  charbuf[ 2] = ' ';
  charbuf[ 3] = ' ';
  charbuf[ 4] = (buf[DAY] > 9) ? '0' + buf[DAY] / 10 : ' ';
  charbuf[ 5] = '0' + buf[DAY] % 10;
  charbuf[ 6] = '.';
  charbuf[ 7] = (buf[MON] > 9) ? '0' + buf[MON] / 10 : ' ';
  charbuf[ 8] = '0' + buf[MON] % 10;
  charbuf[ 9] = '.';
  charbuf[10] = '2';
  charbuf[11] = '0';
  charbuf[12] = '0' + (buf[YEAR] / 10) % 10;
  charbuf[13] = '0' + buf[YEAR] % 10;
  charbuf[14] = '\0';
  enum { c1 = (LCD_COLS - 14) / 2 };
  lcdWrite(c1, 1+r, charbuf);

  // interní HHMMSS:
  charbuf[0] = (buf[HOUR] > 9) ? '0' + buf[HOUR] / 10 : ' ';
  charbuf[1] = '0' + buf[HOUR] % 10;
  charbuf[2] = '0' + buf[MIN] / 10;
  charbuf[3] = '0' + buf[MIN] % 10;
  charbuf[4] = '0' + buf[SEC] / 10;
  charbuf[5] = '0' + buf[SEC] % 10;
  charbuf[6] = ':';
  reverse(&charbuf[0], &reversed[0], 7);
  intDisp.write(reversed);

  // externí HHMM:
  charbuf[0] = ' ';
  charbuf[1] = ' ';
  charbuf[2] = (buf[HOUR] > 9) ? '0' + buf[HOUR] / 10 : ' ';
  charbuf[3] = '0' + buf[HOUR] % 10;
  charbuf[4] = '0' + buf[MIN] / 10;
  charbuf[5] = '0' + buf[MIN] % 10;
  charbuf[6] = ':';
  reverse(&charbuf[0], &reversed[0], 7);
  extDis1.write(reversed);
}



void module04Loop() {
  #ifdef INT_CLOCK
    static uint8_t pos = 0; // seřizovací kurzor
    int8_t changed = 0;
  #endif

  switch (moduleState) {

    case MOD_SHUTDOWN:
      changeModuleState(MOD_EXITING);
      return;

    case MOD_STARTING:
      #ifdef INT_CLOCK
        if (!internalClockRunning) {
          uint8_t buf[DATA_LEN];
          if (getEeprom(MODULE_ID_04, 0, buf)) {
            now.day   = buf[0];
            now.month = buf[1];
            now.year  = buf[2];
            updateWeekday();
          }
        }
      #endif
      changeModuleState(MOD_IDLE);
      break;

    case MOD_IDLE:
      if (firstEntry) {
        firstEntry = false;
        lcdClear();
        #ifdef LCD_16X2
          lcdModuleTitle(F(" HODINY "));
        #else
          lcdModuleTitle(F("HODINY"));
        #endif
        extDis1.fill(' ');
        extDis2.fill(' ');
        intDisp.fill(' ');
      }
      #ifdef INT_CLOCK
        // interní hodiny lze nastavit tlačítky
        // (RTC hodiny nemají žádné možnosti - seřizování přes nastavení)
        if (checkInput(&inReset)) {
          changeModuleState(MOD_PREPARE);
          return;
        }
      #endif
      if (timerFPS) m04DisplayDateTime();
      break;

    // stav nastavení mají pouze interní hodiny
    case MOD_PREPARE:
      #ifdef INT_CLOCK
        if (firstEntry) {
          firstEntry = false;
          pos = 0;
          lcdClear();
          #ifdef LCD_20X4
            lcdStateTitle(F("Datum & cas"));
            lcdWrite(0, 3, F("S>Posun LP>-+ R>Zpet"));
          #endif
          extDis1.fill(' ');
          extDis2.fill(' ');
          intDisp.fill(' ');
        }
        if (checkInput(&inReset)) {
          changeModuleState(MOD_IDLE);
          return;
        }
        if (checkInput(&inStart)) { // co se seřizuje
          if (++pos > 6) pos = 0;
        }
        changed = 0;
        if (checkInput(&inLeft)) { // L/P - změna - +
          changed = -1;
          delay(INPUT_DEBOUNCE); // workaround, aby šlo držet
          if (digitalRead(IN3) == LOW) inLeft.triggered = true;
        }
        if (checkInput(&inRight)) {
          changed = +1;
          delay(INPUT_DEBOUNCE);
          if (digitalRead(IN4) == LOW) inRight.triggered = true;
        }
        if (changed) {
          uint8_t oldSREG;
          switch (pos) {
            case 0:
              now.day += changed;
              if (now.day > 31) now.day = 1;
              if (now.day <  1) now.day = 31;
              break;
            case 1:
              now.month += changed;
              if (now.month > 12) now.month = 1;
              if (now.month <  1) now.month = 12;
              break;
            case 2:
              now.year += changed;
              if (now.year > 99) now.year = 25;
              if (now.year < 25) now.year = 99;
              break;
            case 3:
              now.hour += changed;
              if (now.hour == 255) now.hour = 23;
              if (now.hour > 23) now.hour = 0;
              break;
            case 4:
              now.min += changed;
              if (now.min == 255) now.min = 59;
              if (now.min > 59) now.min = 0;
              break;
            case 5:
              oldSREG = SREG;
              cli();
              _sec_cnt = 0;
              _sec_tick = false;
              SREG = oldSREG;
              now.sec = (now.sec/10)*10 + 10*changed;
              if (now.sec >240) now.sec = 50;
              if (now.sec > 50) now.sec = 0;
              break;
            case 6:
              if (!internalClockRunning) {
                oldSREG = SREG;
                cli();
                _sec_cnt = 0;
                _sec_tick = false;
                SREG = oldSREG;
              }
              internalClockRunning = !internalClockRunning;
              break;
          }
          updateWeekday();
        }
        if (timerFPS) {
          char buf[17];
          buf[16] = '\0';

          #ifdef LCD_16X2
            enum { r = 0 };
          #else
            enum { r = 1 };
          #endif

          buf[ 0] = ' ';
          buf[ 1] = '0' + now.day / 10;
          buf[ 2] = '0' + now.day % 10;
          buf[ 3] = ' ';
          buf[ 4] = ' ';
          buf[ 5] = '0' + now.month / 10;
          buf[ 6] = '0' + now.month % 10;
          buf[ 7] = ' ';
          buf[ 8] = ' ';  // TODO: 2099+ :)
          buf[ 9] = '2';
          buf[10] = '0';
          buf[11] = '0' + (now.year / 10) % 10;
          buf[12] = '0' + now.year % 10;
          buf[13] = ' ';
          buf[14] = dayOfWeek[now.weekday][0];
          buf[15] = dayOfWeek[now.weekday][1];
          lcdWrite(0, r, buf);

          buf[ 0] = ' ';
          buf[ 1] = '0' + now.hour / 10;
          buf[ 2] = '0' + now.hour % 10;
          buf[ 3] = ' ';
          buf[ 4] = ' ';
          buf[ 5] = '0' + now.min / 10;
          buf[ 6] = '0' + now.min % 10;
          buf[ 7] = ' ';
          buf[ 8] = ' ';
          buf[ 9] = '0' + now.sec / 10;
          buf[10] = '0' + now.sec % 10;
          buf[11] = ' ';
          buf[12] = ' ';
          if (internalClockRunning) {
            buf[13] = 'Z'; buf[14] = 'a'; 
          } else {
            buf[13] = 'V'; buf[14] = 'y';
          }
          buf[15] = 'p';
          lcdWrite(0, r+1, buf);

          const uint8_t lcdPos[7][2] = {0,0,4,0,8,0,0,1,4,1,8,1,12,1};
          lcdWrite(lcdPos[pos][0], r+lcdPos[pos][1], 7); // custom char
        }
      #endif
      break;


    case MOD_SAVESETT:
      #ifdef INT_CLOCK
        uint8_t buf[DATA_LEN];
        buf[0] = now.day;
        buf[1] = now.month;
        buf[2] = now.year;
        if (setEeprom(MODULE_ID_04, 0, buf)) {
          buzzer(INFO_BEEP);
          moduleEcho("Datum ulozen.");
        }
      #endif
      break;
  }
}
