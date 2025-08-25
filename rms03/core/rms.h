// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

#pragma once

// MODUL JE FSM
enum ModuleState {
  MOD_IDLE      = 0, // NUTNÝ stav pro přepnutí modulu
  MOD_READY     = 1, // připraven ke startu
  MOD_RUNNING   = 2, // běží závod
  MOD_PAUSED    = 3, // měření pozastaveno
  MOD_STOPPED   = 4, // závod skončil
  MOD_PREPARE   = 5, // příprava (na countdown)
  MOD_COUNTDOWN = 6, // odpočítává
  MOD_BUSY      = 7,
  MOD_ERROR     = 8,
  // MOD_ ...
  MOD_STARTING, // vstupní stav (init)
  MOD_SHUTDOWN, // pokyn "ukonči se"
  MOD_SAVESETT, // pokyn "ulož si nastavení"
  MOD_EXITING,  // stav "skončil jsem"
  //
  SYSTEM        // ne modul, ale přepínač modulů
};
ModuleState moduleState ;

void (*moduleLoop)() = nullptr;
uint8_t currentModule = 0xFF;
uint8_t futureModule = 0;
uint8_t modulesInstalled ;

void changeModuleState(ModuleState newState);
bool firstEntry = true; // FSM helpers
bool delayedFirstEntry = false;
uint32_t entryTime = 0;


// DRÁHA
// stav dráhy je současně index do trackColon[] - co se použije z layoutu v daném stavu
enum TrackState { 
  TRACK_READY   = 0,
  TRACK_RUNNING = 1,
  TRACK_STOPPED = 2,
  TRACK_PAUSED  = 3,
  TRACK_OFF     = 4,
  TRACK_ERROR   = 5
};

// šablona dvojteček (pro TrackState)  o=nic  k=svítí  h=bliká
const char trackColon[6] = {'k','h','k','h','o','o'};

struct Track {
  TrackState state;
  uint32_t startTime;
  uint32_t finishTime;
  char colon;

  void changeState(TrackState newState) {
    state = newState;
    colon = trackColon[newState];
  }
};
Track leftTrack;
Track rightTrack;



// ČASOVÁNÍ
const uint8_t FPS_SEC = 27;
const uint8_t FPS_MS = 37;

// pro ISR
volatile uint32_t _exact_millis = 0;
volatile uint8_t _fps_cnt = 0;
volatile bool _fps_tick = false;

// pro loop
uint8_t fpsTickCount = 0;
uint8_t fpsToHalfSec = FPS_SEC/2;  // 27: 13<-->14
bool everyOther = false;

// pro loop/modul
bool timerOneSec = false;
bool timerHalfSec = false;
bool timerFPS = false;


#if defined(HW_RTC_DS3231) && defined(INT_CLOCK)
  #error "Nemůže být víc zdrojů času"
#endif


#ifdef HW_RTC_DS3231
  #include <RTClib.h>
  RTC_DS3231 rtc;
#endif


// interní hodiny potřebují sekundový signál a hodnoty
#ifdef INT_CLOCK
  volatile uint16_t _sec_cnt = 0;
  volatile bool _sec_tick = false;
  struct InternalTime {
    uint8_t weekday = 0, day = 24, month = 8, year = 25,
            hour = 12, min = 0, sec = 0;
  };
  InternalTime now;
  bool internalClockRunning = false;
  void incrementInternalClock();
  bool isLeapYear(uint8_t year);
  void updateWeekday();
#endif


// obě verze hodin pro zobrazení použijí uint8_t buf[7]
#if defined(HW_RTC_DS3231) || defined(INT_CLOCK)
  #define CLOCK_AVAILABLE
  enum { DOW = 0, DAY = 1, MON = 2, YEAR = 3, HOUR = 4, MIN = 5, SEC = 6 };
  const char dayOfWeek[7][3] = {"Ne", "Po", "Ut", "St", "Ct", "Pa", "So"};
  void currentTimeToBuf(uint8_t* buf);
#endif



// volatilní flagy z přerušení na flagy pro právě jeden loop
inline void timingFlags() {
  timerFPS = false;
  timerHalfSec = false;
  timerOneSec = false;
  if (_fps_tick) {
    _fps_tick = false;
    timerFPS = true;
    if (++fpsTickCount >= fpsToHalfSec) {
      timerHalfSec = true;
      timerOneSec = everyOther;
      everyOther = !everyOther; 
      fpsTickCount = 0;
      fpsToHalfSec = FPS_SEC - fpsToHalfSec; // 27 -> 13<-->14
    }
  }
  #ifdef INT_CLOCK
    if (_sec_tick) {
      _sec_tick = false;
      incrementInternalClock();
    }
  #endif
}


// Timer1 na 1ms (prescaler 64)
// 4us přesnost - více než dostatečné pro PCINT
inline void initTimer() {
  #if F_CPU == 16000000L
    cli();
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS11) | (1 << CS10);
    OCR1A = 249;
    TIMSK1 |= (1 << OCIE1A);
    sei();
  #else
    #error "Chybí nastavení pro zvolenou frekvenci"
  #endif
}

uint32_t exactMillis();
void restartTimer();



// jedno místo pro výpočet zobrazení
struct TimeParts { // NOTE: plní calcTimeParts()
  uint16_t hours;  //       na charbuf převádí tpToChars()
  uint8_t  minutes;
  uint8_t  seconds;
  uint16_t milliseconds;

  uint8_t hrThousands;
  uint8_t hrHundreds;
  uint8_t hrTens;
  uint8_t hrUnits; 
  uint8_t minTens;
  uint8_t minUnits;
  uint8_t secTens;
  uint8_t secUnits;
  uint8_t msHundreds;
  uint8_t msTens;
  uint8_t msUnits;
};
TimeParts tp;

void calcTimeParts(const uint32_t *start, const uint32_t *end);
void tpToChars(char *buf, char empty = ' ');

// jediný používaný čas pro zobrazování - pro lcd rovnou,
// pro displeje je to vstup do šablon, proto vč. LPUE- 
char timeStr[20] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
                    '\0','L','P','U','E','-'};  // tpToChars() mění 0 až 13


// BUZZER/BEEPER
#define INFO_BEEP 0
#define WARN_BEEP 1
#define READY_BEEP 2
//#define LONG_BEEP 3
//#define CRITICAL_BEEP 4

uint8_t buzzCount;
uint16_t buzzFreq;
uint16_t buzzOnTime; // délka zvuku [ms]
uint16_t buzzOffTime; // pauza
uint32_t buzzTimer;
bool buzzState;

// pro modul - vytvoří zvuk
void buzzWakeUp(uint8_t count, uint16_t freq, uint16_t duration, uint16_t pause = 0);
void buzzer(uint8_t type); // dělá fyzicky zvuk
void updateBuzzer();       // loop agent


// Překryvná vrstva (primárně pro rychlou kontrolu smyčky)
#ifndef MINIMIZE_FLASH
  bool showPerfOverlay = false;

  inline void performanceOverlay() {
    static uint32_t loops = 0;
    static uint32_t last = 0;
    if (showPerfOverlay) {
      loops++;
      if (timerOneSec) { last = loops; loops = 0; }
      if (timerFPS) {
        char perf[10];
        perf[0] = '[';
        perf[1] = '0'+moduleState;
        perf[2] = ':';
        perf[3] = '0' + (last / 10000)  % 10;
        perf[4] = '0' + (last / 1000)   % 10;
        perf[5] = '0' + (last / 100)    % 10;
        perf[6] = '0' + (last / 10)     % 10;
        perf[7] = '0' + (last)          % 10;
        perf[8] = ']';
        perf[9] = '\0';
        lcdWrite(0, 0, perf);
      }
    }
  }
#endif


// === NASTAVENÍ
#include <EEPROM.h>
#define BLOCK_SIZE  16
#define BLOCK_COUNT 64    // 64*16 = 1024; TODO podle procesoru EEPROM.length()
#define DATA_LEN    12   // SIZE-4 (2=id, 2=crc)
// index je uint8_t -> max 255x16 (4kB)

struct Record {
    uint8_t idModul;  // bit 7 = deleted
    uint8_t idKey;    // bit 7 = typ (0=uint8_t, 1=char)
    uint8_t data[DATA_LEN];
    uint16_t crc;
};

bool getEeprom(uint8_t idModul, uint8_t idKey, char* buf);
bool getEeprom(uint8_t idModul, uint8_t idKey, uint8_t* buf);

bool setEeprom(uint8_t idModul, uint8_t idKey, const uint8_t* buf);
bool setEeprom(uint8_t idModul, uint8_t idKey, const char* buf);


// === Makra kvůli Leonardo
#ifdef TARGET_MCU_LEONARDO
  #define SERIAL_PRINT(x)   if (Serial) Serial.print(x)
  #define SERIAL_PRINTLN(x) if (Serial) Serial.println(x)
#else
  #define SERIAL_PRINT(x)   Serial.print(x)
  #define SERIAL_PRINTLN(x) Serial.println(x)
#endif
