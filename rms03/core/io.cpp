// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

#include <PinChangeInterrupt.h>

void handlePinInterrupt(InputPin *input, uint32_t *input_ms, uint16_t *input_tcnt) {
  uint32_t new_ms;
  uint16_t new_tcnt;
  uint8_t sreg = SREG;
  cli();
  new_ms = *input_ms;
  new_tcnt = *input_tcnt;
  SREG = sreg;
  if (new_ms - input->time >= input->debounce) {
    input->time = new_ms;
    input->us   = new_tcnt * 4; // hodnota TCNT z Timeru á 4us
    input->triggered = true;
    if (showPulseTime)  {
      char buf[8];
      systemEcho(F("Pulz "), false); 
      buf[0] = input->name;
      buf[1] = (input == &inReset && moduleState == MOD_IDLE) ? '+' : ' ';
      buf[2] = '@';
      buf[3] = ' ';
      buf[4] = '\0';
      SERIAL_PRINT(buf);
      SERIAL_PRINT(input->time);
      buf[0] = '.';
      buf[1] = ('0' + (input->us / 100));
      buf[2] = ('0' + (input->us / 10 % 10));
      buf[3] = ('0' + (input->us % 10));
      buf[4] = ' ';
      buf[5] = 'm';
      buf[6] = 's';
      buf[7] = '\0';
      SERIAL_PRINTLN(buf);
    }
  }
}


bool checkInput(InputPin *input) {
  if (input->triggered) {
    input->triggered = false;  // zkonzumovat před vrácením
    return true;
  } else {
    return false;
  }
}


void ISR_inStart() {
  _time_inStart = _exact_millis;
  _tcnt_inStart = TCNT1;
  _isr_inStart  = true;
}


void ISR_inReset() {
  _time_inReset = _exact_millis;
  _tcnt_inReset = TCNT1;
  _isr_inReset  = true;
}


void ISR_inLeft() {
  _time_inLeft = _exact_millis;
  _tcnt_inLeft = TCNT1;
  _isr_inLeft  = true;
}


void ISR_inRight() {
  _time_inRight = _exact_millis;
  _tcnt_inRight = TCNT1;
  _isr_inRight  = true;
}

#ifdef HW_IN5
  void ISR_inSplit() {
    _time_inSplit = _exact_millis;
    _tcnt_inSplit = TCNT1;
    _isr_inSplit  = true;
  }
#endif


void attachPinInterrupts() {
  attachPCINT(digitalPinToPCINT(IN1), ISR_inStart, FALLING);
  attachPCINT(digitalPinToPCINT(IN2), ISR_inReset, FALLING);
  attachPCINT(digitalPinToPCINT(IN3), ISR_inLeft,  FALLING);
  attachPCINT(digitalPinToPCINT(IN4), ISR_inRight, FALLING);
  #ifdef HW_IN5
    attachPCINT(digitalPinToPCINT(IN5), ISR_inSplit, FALLING);
  #endif
}


void dettachPinInterrupts() {
  detachPCINT(digitalPinToPCINT(IN1));
  detachPCINT(digitalPinToPCINT(IN2));
  detachPCINT(digitalPinToPCINT(IN3));
  detachPCINT(digitalPinToPCINT(IN4));
  #ifdef HW_IN5
    detachPCINT(digitalPinToPCINT(IN5));
  #endif
}


void turnOnLeftSignal() {
  #ifdef HW_OUT3
    digitalWrite(OUT3, HIGH);
  #else
    // v4.1 nemá výstupy
  #endif  
}


void turnOnRightSignal() {
  #ifdef HW_OUT4
    digitalWrite(OUT4, HIGH);
  #else
    // v4.1 nemá výstupy
  #endif  
}


void turnOffLeftSignal() {
  #ifdef HW_OUT3
    digitalWrite(OUT3, LOW);
  #else
    // v4.1 nemá výstupy
  #endif  
}


void turnOffRightSignal() {
  #ifdef HW_OUT4
    digitalWrite(OUT4, LOW);
  #else
    // v4.1 nemá výstupy
  #endif  
}


void turnOffSignals() {
	turnOffLeftSignal();
	turnOffRightSignal();
}
