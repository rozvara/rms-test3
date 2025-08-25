// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

#if defined(TARGET_MCU_ATMEGA644P)
  #if !defined(__AVR_ATmega644P__)
    #error "Zvolená konfigurace vyžaduje (MightyCore) ATmega644P"
  #endif
#elif defined(TARGET_MCU_ATMEGA1284P)
  #if !defined(__AVR_ATmega1284P__)
    #error "Zvolená konfigurace vyžaduje (MightyCore) ATmega1284P"
  #endif
#elif defined(TARGET_MCU_MEGA)
  #if !defined(__AVR_ATmega2560__)
    #error "Zvolená konfigurace vyžaduje Arduino Mega 2560"
  #endif
#elif defined(TARGET_MCU_LEONARDO)
  #if !defined(__AVR_ATmega32U4__)
    #error "Zvolená konfigurace vyžaduje Arduino Leonardo"
  #endif
  #define MINIMIZE_FLASH
#else
  #error "Chybí specifikace cílové platformy."
#endif


#ifdef HW_VER_50
  #include "hw/sakul_stopky_v50.h"
#endif
#ifdef HW_VER_41
  #include "hw/sakul_stopky_v41.h"
#endif
#ifdef HW_MEGA2560
  #include "hw/arduino_mega2560.h"
#endif
#ifdef HW_MEGA2560_v41
  #define MINIMIZE_FLASH
  #include "hw/arduino_mega2560_v41.h"
#endif
