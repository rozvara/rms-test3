// RMS - firmware pro modulární stopky
// Modul "Stopky 2: jedna dráha, start i ukončení S"
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.


void mod02UpdateDisplays(bool colon = false) {
  tpToChars(&timeStr[0]);
  #ifdef LCD_16X2
    enum { c = 2, r = 1 };
  #else
    enum { c = 6, r = 3 };
  #endif
  if (colon) timeStr[7] = ':';
  lcdWrite(c, r, timeStr);
  intDisp.applyLayout(&timeStr[0], rightTrack.colon);
  extDis2.applyLayout(&timeStr[0], rightTrack.colon);
}


void mod02Finished() {
  rightTrack.changeState(TRACK_STOPPED);
  calcTimeParts(&rightTrack.startTime, &rightTrack.finishTime);
  mod02UpdateDisplays(true);
  lcdStateTitle(F("Ukonceno"));
  changeModuleState(MOD_STOPPED);
}


void module02Loop() {

  switch (moduleState) {

    // case MOD_EXITING:
    //   return;

    case MOD_SHUTDOWN:
      changeModuleState(MOD_EXITING);
      return;

    case MOD_STARTING:
      intDisp.fill(' ');
      extDis1.fill(' ');
      extDis2.fill(' ');
      turnOffSignals();

      // TODO zjistit, zda nemá raději běžet v prvním displeji
      memcpy(intDisp.layout, intDispLayout, intDisp.len);
      memcpy(extDis2.layout, extDis2Layout, extDis2.len);

      leftTrack.changeState(TRACK_OFF);
      rightTrack.changeState(TRACK_OFF);
      
      changeModuleState(MOD_IDLE);
      break;

    case MOD_IDLE:
      if (firstEntry) {
        firstEntry = false;
        lcdClear();
        lcdModuleTitle(F("STOPKY S-S"));
        #ifdef LCD_16X2
          lcdWrite(0, 1, F("R>Funkce dR>Menu"));
        #else
          lcdWrite(3, 2, F("Reset > Funkce"));
          lcdWrite(0, 3, F("Dlouze Reset > Menu"));
        #endif
      }
      if (checkInput(&inReset)) {
        changeModuleState(MOD_READY);
      }
      break;

    case MOD_READY:
      if (firstEntry) {
        firstEntry = false;
        lcdClear();
        lcdStateTitle(F("STOPKY"));
        #ifdef LCD_16X2
          lcdWrite(0, 1, F("S>Start   R>Zpet"));
        #else
          lcdWrite(0, 2, F("Start > Zahaji zavod"));
          lcdWrite(3, 3, F("Reset > Zpatky"));
        #endif

        rightTrack.startTime = 0;
        rightTrack.finishTime = 0;

        rightTrack.changeState(TRACK_READY);
        calcTimeParts(&rightTrack.startTime, &rightTrack.finishTime);
        tpToChars(&timeStr[0]);

        intDisp.applyLayout(&timeStr[0], rightTrack.colon);
        extDis2.applyLayout(&timeStr[0], rightTrack.colon);
      }
      if (checkInput(&inReset)) {
        changeModuleState(MOD_IDLE);
        return;
      }
      if (checkInput(&inStart)) {
        restartTimer();
        moduleEcho(F("Stopky spusteny"));
        logClear();
        logEvent(LOG_START, inStart.time);
        rightTrack.startTime = inStart.time;
        rightTrack.changeState(TRACK_RUNNING);
        changeModuleState(MOD_RUNNING);
      }
      break;

    case MOD_RUNNING:
      if (firstEntry) {
        firstEntry = false;
        lcdClear();
        lcdBusyTitle(F("Stopky"));
        #ifdef LCD_20X4
          lcdWrite(0, 1, F("S>Ukonci    R>Zrusit"));
        #endif
      }
      #ifdef HW_IN5
        if (checkInput(&inSplit)) {
          moduleEcho(F("Mezicas"));
          logEvent(LOG_SPLIT, inSplit.time);
          calcTimeParts(&rightTrack.startTime, &inSplit.time);
          tpToChars(&timeStr[0]);
          timeStr[7] = ':';
          #ifdef LCD_16X2
            enum { c = 2, r = 0 };
            lcdWrite(c, r, timeStr);
            lcdWrite(0, r, "M-cas");
          #else
            enum { c = 6, r = 2 };
            lcdWrite(c, r, timeStr);
            lcdWrite(0, r, "Mezicas");
          #endif
        }
      #endif
      if (checkInput(&inStart)) {
        rightTrack.finishTime = inStart.time;
        logEvent(LOG_FINISH, rightTrack.finishTime);
        mod02Finished();
        return;
      }
      if (timerFPS) {
        uint32_t currTime = exactMillis();
        calcTimeParts(&rightTrack.startTime, &currTime);
        mod02UpdateDisplays();
      }
      if (checkInput(&inReset)) {
        moduleEcho(F("Mereni zruseno"));
        logEvent(LOG_RESET, inReset.time);
        calcTimeParts(&rightTrack.startTime, &inReset.time);
        mod02UpdateDisplays(true);

        lcdStateTitle(F("Zruseno"));

        rightTrack.changeState(TRACK_STOPPED);
        changeModuleState(MOD_STOPPED);
      }
      break;

    case MOD_STOPPED:
      if (firstEntry) {
        firstEntry = false;
        #ifdef LCD_20X4
          lcdClearRow(1);
        #endif
        moduleEcho(F("Zadost o vypis"));
        logDump(Serial);
        // TODO SD karta
      }
      if (checkInput(&inReset)) {
        changeModuleState(MOD_IDLE);
      }
      break;
  
  }
}
