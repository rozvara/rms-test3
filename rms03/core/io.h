// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

#pragma once

struct InputPin {
  bool triggered;
  uint32_t time;
  uint16_t us;
  uint8_t debounce;
  char name;
};
InputPin inStart = { false, 0, 0, INPUT_DEBOUNCE, 'S' };
InputPin inLeft  = { false, 0, 0, INPUT_DEBOUNCE, 'L' };
InputPin inRight = { false, 0, 0, INPUT_DEBOUNCE, 'P' };
InputPin inReset = { false, 0, 0, INPUT_DEBOUNCE, 'R' };
#ifdef HW_IN5
  InputPin inSplit = { false, 0, 0, INPUT_DEBOUNCE, 'M' };
#endif

// pro modul
bool checkInput(InputPin *input);
void turnOnLeftSignal();
void turnOnRightSignal();
void turnOffLeftSignal();
void turnOffRightSignal();
void turnOffSignals();


// ISR
volatile bool _isr_inStart;
volatile uint32_t _time_inStart;
volatile uint16_t _tcnt_inStart;

volatile bool _isr_inReset;
volatile uint32_t _time_inReset;
volatile uint16_t _tcnt_inReset;

volatile bool _isr_inLeft;
volatile uint32_t _time_inLeft;
volatile uint16_t _tcnt_inLeft;

volatile bool _isr_inRight;
volatile uint32_t _time_inRight;
volatile uint16_t _tcnt_inRight;

#ifdef HW_IN5
  volatile bool _isr_inSplit;
  volatile uint32_t _time_inSplit;
  volatile uint16_t _tcnt_inSplit;
#endif

void handlePinInterrupt(InputPin *input, uint32_t *input_ms, uint16_t *input_tcnt);


// v každém loopu volatilní flagy+časy z přerušení do objektů InputPin
inline void interruptFlags() {
  if (_isr_inStart) { _isr_inStart = false; handlePinInterrupt(&inStart, &_time_inStart, &_tcnt_inStart); }
  if (_isr_inReset) { _isr_inReset = false; handlePinInterrupt(&inReset, &_time_inReset, &_tcnt_inReset); }
  if (_isr_inLeft ) { _isr_inLeft  = false; handlePinInterrupt(&inLeft,  &_time_inLeft,  &_tcnt_inLeft ); }
  if (_isr_inRight) { _isr_inRight = false; handlePinInterrupt(&inRight, &_time_inRight, &_tcnt_inRight); }
  #ifdef HW_IN5
    if (_isr_inSplit) { _isr_inSplit = false; handlePinInterrupt(&inSplit, &_time_inSplit, &_tcnt_inSplit); }
  #endif
}

void ISR_inStart();
void ISR_inReset();
void ISR_inLeft();
void ISR_inRight();
#ifdef HW_IN5
  void ISR_inSplit();
#endif


void attachPinInterrupts();
void dettachPinInterrupts();

bool showPulseTime = true;