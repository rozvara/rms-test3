// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

void changeModuleState(ModuleState newState) {
  entryTime = exactMillis();
  moduleState = newState;
  firstEntry = true;
  delayedFirstEntry = false; // stav si musí sám nastavit v entry init

  inStart.triggered = false; // zahazujeme nezpracované eventy,
  inReset.triggered = false; // když je minulý stav nekontroloval
  inLeft.triggered  = false;
  inRight.triggered = false;
  #ifdef HW_IN5
    inSplit.triggered = false;
  #endif
}


ISR(TIMER1_COMPA_vect) {
  _exact_millis++;
  if (++_fps_cnt >= FPS_MS) {
    _fps_cnt = 0;
    _fps_tick = true;
  }
  #ifdef INT_CLOCK
    if (++_sec_cnt >= 1000) {
      _sec_cnt = 0;
      _sec_tick = true;
    }
  #endif
}


uint32_t exactMillis() {
  uint32_t ms;
  uint8_t oldSREG = SREG;
  cli();
  ms = _exact_millis;
  SREG = oldSREG;
  return ms;
}


// fps tikání od "nuly" (se startem závodu)
void restartTimer() {
  uint8_t oldSREG = SREG;
  cli();
  _fps_cnt = 0;
  _fps_tick = false;
  fpsTickCount = 0;
  fpsToHalfSec = FPS_SEC/2;
  everyOther = false;
  SREG = oldSREG;
}


#ifdef INT_CLOCK
  void incrementInternalClock() {
    if (internalClockRunning) {
      const uint8_t daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

      now.sec++;
      if (now.sec < 60) return;
      now.sec = 0;

      now.min++;
      if (now.min < 60) return;
      now.min = 0;

      now.hour++;
      if (now.hour < 24) return;
      now.hour = 0;

      now.day++;
      uint8_t monthDays = daysInMonth[now.month - 1];
      if (now.month == 2 && isLeapYear(now.year)) monthDays = 29;
      if (now.day <= monthDays) { updateWeekday(); return; }

      now.day = 1;
      now.month++;
      if (now.month <= 12) { updateWeekday(); return; }

      now.month = 1;
      now.year++;
      updateWeekday();
    }
  }

  bool isLeapYear(uint8_t year) {
    return (year % 4) == 0; // stačí i pro 00–99
  }

  void currentTimeToBuf(uint8_t* buf) {
    if (internalClockRunning) {
      buf[DOW]  = now.weekday;
      buf[DAY]  = now.day;
      buf[MON]  = now.month;
      buf[YEAR] = now.year; // uint8
      buf[HOUR] = now.hour;
      buf[MIN]  = now.min;
      buf[SEC]  = now.sec;
    } else {
      buf[DOW]  = 0;
      buf[DAY]  = 0;
      buf[MON]  = 0;
      buf[YEAR] = 0;
      buf[HOUR] = 0;
      buf[MIN]  = 0;
      buf[SEC]  = 0;
    }
  }

  void updateWeekday() {
    // Tomohiko Sakamoto algorithm
    static const uint8_t t[12] = {0,3,2,5,0,3,5,1,4,6,2,4};
    uint16_t y = 2000 + now.year;
    if (now.month < 3) y -= 1;
    now.weekday = (y + y/4 - y/100 + y/400 + t[now.month-1] + now.day) % 7;
  }
#endif


#if defined(HW_RTC_DS3231)
  void currentTimeToBuf(uint8_t* buf) {
    DateTime rtcNow = rtc.now();
    buf[DOW]  = rtcNow.dayOfTheWeek();
    buf[DAY]  = rtcNow.day();
    buf[MON]  = rtcNow.month();
    buf[YEAR] = (uint8_t)(rtcNow.year()-2000);
    buf[HOUR] = rtcNow.hour();
    buf[MIN]  = rtcNow.minute();
    buf[SEC]  = rtcNow.second();
  }
#endif


// rozloží časový interval na komponenty
void calcTimeParts(const uint32_t *start, const uint32_t *end) {
  uint32_t elapsed = *end - *start;
  uint32_t totalSeconds = elapsed / 1000UL;

  tp.milliseconds = elapsed % 1000UL;
  tp.seconds = totalSeconds % 60UL;
  tp.minutes = (totalSeconds / 60UL) % 60UL;
  tp.hours   = (totalSeconds / 3600UL);

  tp.hrThousands = tp.hours / 1000;
  tp.hrHundreds  = (tp.hours / 100) % 10;
  tp.hrTens      = (tp.hours / 10) % 10;
  tp.hrUnits     = tp.hours % 10;

  tp.minTens    = tp.minutes / 10;
  tp.minUnits   = tp.minutes % 10;
  tp.secTens    = tp.seconds / 10;
  tp.secUnits   = tp.seconds % 10;
  tp.msHundreds = tp.milliseconds / 100;
  tp.msTens     = (tp.milliseconds / 10) % 10;
  tp.msUnits    = tp.milliseconds % 10;
}


// převede komponenty času na znaky pro zobrazení
void tpToChars(char *buf, char space = ' ') {
  if (tp.hours>0) {
    buf[0] = (tp.hrThousands>0) ? '0' + tp.hrThousands : space;
    buf[1] = (tp.hrHundreds>0) ? '0' + tp.hrHundreds : space;
    buf[2] = (tp.hrTens>0) ? '0' + tp.hrTens : space;
    buf[3] = '0' + tp.hrUnits;
    buf[4] = ':';
  } else {
    buf[0] = space;
    buf[1] = space;
    buf[2] = space;
    buf[3] = space;
    buf[4] = space;
  }
  buf[5] = (tp.minTens>0 || tp.hours>0) ? '0' + tp.minTens : space;
  buf[6] = '0' + tp.minUnits;
  buf[7] = (tp.msHundreds > 4) ? ' ' : ':'; // vstup do šablon bliká vždy
  buf[8]  = '0' + tp.secTens;
  buf[9]  = '0' + tp.secUnits;
  buf[10] = '.';
  buf[11] = '0' + tp.msHundreds;
  buf[12] = '0' + tp.msTens;
  buf[13] = '0' + tp.msUnits;
}



void updateBuzzer() {
  uint32_t now = exactMillis();
  if (!buzzState) {
    if (now - buzzTimer >= buzzOffTime) {
      tone(pinTONE, buzzFreq);
      buzzState = true;
      buzzTimer = now;
    }
  } else {
    if (now - buzzTimer >= buzzOnTime) {
      noTone(pinTONE);
      buzzState = false;
      buzzTimer = now;
      buzzCount--;
    }
  }
}


void buzzWakeUp(uint8_t count, uint16_t freq, uint16_t duration, uint16_t pause = 0) {
  buzzCount = count;
  buzzFreq = freq;
  buzzOnTime = duration;
  buzzOffTime = pause;
  buzzTimer = exactMillis();
  buzzState = false;
  noTone(pinTONE);
}


// TODO: zvuky do Nastavení
void buzzer(uint8_t type) {
  switch (type) {
    case INFO_BEEP:
      buzzWakeUp(1, 2500, 80);
      break;
    case WARN_BEEP:
      buzzWakeUp(3, 2000, 80, 100);
      break;
    case READY_BEEP:
      buzzWakeUp(1, 200, 2000);
      break;
    // case LONG_BEEP:
    //   buzzWakeUp(1, 1500, 2500);
    //   break;
  }
}



// ===== EEPROM
uint16_t crc16(const uint8_t* data, uint8_t len) {
  uint16_t crc = 0xFFFF;
  for (uint8_t i = 0; i < len; i++) {
    crc ^= ((uint16_t)data[i]) << 8;
    for (uint8_t j = 0; j < 8; j++)
      crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : (crc << 1);
  }
  return crc;
}


bool validCrc(const Record& r) {
  uint16_t crc = crc16((uint8_t*)&r, 2 + DATA_LEN);
  return (crc == r.crc);
}


void readBlock(uint8_t index, Record& out) {
  uint16_t addr = index * BLOCK_SIZE;
  for (uint8_t i = 0; i < BLOCK_SIZE; i++) {
    ((uint8_t*)&out)[i] = EEPROM.read(addr + i);
  }
}


void writeBlock(uint8_t index, const Record& rec) {
  uint16_t addr = index * BLOCK_SIZE;
  for (uint8_t i = 0; i < BLOCK_SIZE; i++) {
    EEPROM.update(addr + i, ((const uint8_t*)&rec)[i]);
  }
}


bool getEeprom(uint8_t idModul, uint8_t idKey, uint8_t* buf) {
  bool found = false;
  uint8_t i = 0;
  while (!found && i < BLOCK_COUNT-1) {
    Record r;
    readBlock(i, r);
    if (validCrc(r) && (r.idModul == idModul) && (r.idKey & 0x7F) == idKey) {
      memcpy(buf, r.data, DATA_LEN);
      found = true;
    }
    i++;
  }
  return found;
}


bool getEeprom(uint8_t idModul, uint8_t idKey, char* buf) {
  return getEeprom(idModul, idKey, (uint8_t*)buf);
}


bool setEeprom(uint8_t idModul, uint8_t idKey, const uint8_t* buf) {
  Record r;

  // existuje? -> aktualizovat
  for (uint8_t i = 0; i < BLOCK_COUNT; i++) {
    readBlock(i, r);
    if (!validCrc(r)) continue;
    if ((r.idModul == idModul) && (r.idKey & 0x7F) == idKey) {
      if (r.idKey != idKey) return false; // text/uint mišmaš
      memcpy(r.data, buf, DATA_LEN);
      r.crc = crc16((uint8_t*)&r, 2 + DATA_LEN);
      writeBlock(i, r);
      return true;
    }
  }

  int8_t unused = -1;
  int8_t deleted = -1;

  // neexistuje? -> první nepoužitý/vadný blok (nemá CRC)
  for (int8_t i = 0; i < BLOCK_COUNT; i++) {
    readBlock(i, r);
    if (!validCrc(r)) {
      unused = i;
      break;
    }
  }

  // není žádný volný? -> první smazaný
  if (unused == -1) {
    for (int8_t i = 0; i < BLOCK_COUNT; i++) {
      readBlock(i, r);
      if (validCrc(r) && (r.idModul & 0x80)) {
        deleted = i;
        break;
      }
    }
  }

  r.idModul = idModul;
  r.idKey = idKey;
  memcpy(r.data, buf, DATA_LEN);
  r.crc = crc16((uint8_t*)&r, 2 + DATA_LEN);

  if (unused != -1) {
    writeBlock((uint8_t)unused, r);
    return true;
  } else if (deleted != -1) {
    writeBlock((uint8_t)deleted, r);
    return true;
  } else {
    systemEcho(F("EEPROM plna!"));
    return false;
  }
}


bool setEeprom(uint8_t idModul, uint8_t idKey, const char* buf) {
  return setEeprom(idModul, idKey | 0x80, (const uint8_t*)buf);
}


