// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.


void setup() {
  initTimer();
  Serial.begin(115200);
  #ifdef TARGET_MCU_LEONARDO
    while (!Serial && exactMillis() < 2000); // čekání na USB CDC, max 2s
  #endif
  systemEcho(F("Start "), false);
  SERIAL_PRINT(FW_VERSION); SERIAL_PRINTLN(FW_TAG);

  initLCD();
  initPins();
  initSerialDisplays();

  #ifdef HW_RTC_DS3231
    if (!rtc.begin()) systemEcho(F("RTC nereaguje"));
  #endif

  modulesInstalled = sizeof(modules) / sizeof(modules[0]);

  #ifdef DEFAULT_MODULE
    futureModule = DEFAULT_MODULE;
  #endif

  uint8_t buf[DATA_LEN];
  if (getEeprom(0, 3, buf)) {
    futureModule = buf[0];
    showPulseTime = (bool)(buf[1]);
  }
  if (futureModule > modulesInstalled-1) futureModule = 0;
  switchModule();
  attachPinInterrupts();  
}


void loop() {
  timingFlags();
  interruptFlags();

  if (inReset.triggered && moduleState == MOD_IDLE) checkResetLongPress();
  if (switchModuleNow) switchModule();
  moduleLoop();

  #ifndef MINIMIZE_FLASH
    performanceOverlay();
  #endif
  if (timerFPS) { updateLCD(); updateSerialDisplays(); }
  if (buzzCount > 0) updateBuzzer();
}


void switchModule() {
  if (moduleLoop != nullptr) {
    changeModuleState(MOD_SHUTDOWN);
    uint32_t now = exactMillis();
    while ((exactMillis() < now+50) && moduleState != MOD_EXITING) moduleLoop();
  }
  switchModuleNow = false;
  currentModule = futureModule;
  moduleLoop = modules[currentModule].loop;
  changeModuleState(MOD_STARTING);

  systemEcho(F("Modul '"), false);
  SERIAL_PRINT(modules[currentModule].name);
  SERIAL_PRINTLN(F("' aktivni"));
}


#ifndef LONG_PRESS
  #define LONG_PRESS 1000
#endif

void checkResetLongPress() {
  delay(INPUT_DEBOUNCE);
  while ((exactMillis() < inReset.time + LONG_PRESS) && (digitalRead(IN2) == LOW));
  if (digitalRead(IN2) == LOW) {
    lcdClear();
    #if defined(LCD_20X4) && !defined(MINIMIZE_FLASH)
      lcdWrite(0, 3, F("S>Moznosti L/P>Volba"));
    #endif
    switchMode = 0;
    futureModule = currentModule;
    moduleLoop = loopWhileResetHeld;
    changeModuleState(SYSTEM);
  }
}


void loopWhileResetHeld() {
  if (digitalRead(IN2) == HIGH) { // uvolnění resetu
    switchModuleNow = true;
    return;
  }

  #ifdef LCD_16X2
    enum { c = 0 };
  #else
    enum { c = 2 };
  #endif
  const String arrowText = F("-->");

  if (checkInput(&inStart)) {
    if (++switchMode > 2) switchMode = 0;
    lcdClearRow(0);
    lcdClearRow(1);
  }

  switch (switchMode) {

    case 0:
      // přepínání modulů
      if (checkInput(&inLeft)) {
        futureModule = (futureModule == 0) ? modulesInstalled - 1 : futureModule - 1;
        lcdClearRow(1);
      }
      if (checkInput(&inRight)) {
        futureModule = (futureModule == modulesInstalled - 1) ? 0 : futureModule + 1;
        lcdClearRow(1);
      }
      if (timerFPS) {
        #ifdef MINIMIZE_FLASH
          lcdWrite(c, 0, F("Modul"));
        #else
          lcdWrite(c, 0, F("Prepnuti modulu"));
        #endif  
        lcdWrite(c, 1, arrowText);
        lcdWrite(c+4, 1, modules[futureModule].name);
      }
      break;

    case 1:
      // zobrazování časů pulzů
      if (checkInput(&inLeft) || checkInput(&inRight)) {
        showPulseTime = !showPulseTime;
      }
      if (timerFPS) {
        #ifdef MINIMIZE_FLASH
          lcdWrite(c, 0, F("Pulzy"));
        #else
          lcdWrite(c, 0, F("Zobraz cas pulzu"));
        #endif  
        lcdWrite(c, 1, arrowText);
        lcdWrite(c+4, 1, showPulseTime ? F("Zap") : F("Vyp"));
      }
      break;

    case 2:
      // uložení preferencí
      if (checkInput(&inLeft)) { // pro systém
        uint8_t buf[DATA_LEN];
        buf[0] = currentModule;
        buf[1] = (uint8_t)showPulseTime;
        setEeprom(0, 3, buf);
        buzzer(INFO_BEEP);
        systemEcho(F("Ulozeno"));
      }
      if (checkInput(&inRight)) { // pro modul
        moduleState = MOD_SAVESETT;
        modules[currentModule].loop();
        moduleState = SYSTEM;
      }
      if (timerFPS) {
        #ifdef MINIMIZE_FLASH
          lcdWrite(c, 0, F("Ulozit"));
        #else
          lcdWrite(c, 0, F("Ulozit nastaveni"));
        #endif  
        lcdWrite(c, 1, F("L>System P>Modul"));
      }
      break;
  }
}



void terminalEcho(const String& who, const String& msg, bool lf = true) {
  SERIAL_PRINT(who);
  SERIAL_PRINT(F("> "));
  if (lf) { SERIAL_PRINTLN(msg); }
  else    { SERIAL_PRINT(msg); }
}


void systemEcho(const String& msg, bool lf = true) {
  terminalEcho(F("System"), msg, lf);
}


void moduleEcho(const String& msg, bool lf = true) {
  terminalEcho(modules[currentModule].name, msg, lf);
}

