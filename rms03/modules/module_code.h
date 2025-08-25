// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.
   
// Modul se přidává na 2 místech: tady a v module_registration.h
// Tento soubor je centrální evidence modulů a jejich id pro eeprom nastavení

#ifdef MODULE_01
  #define MODULE_ID_01  1 // Stopky 1: Hasičský útok s odpočtem
  #include "01hu.h"
  #include "01hu.cpp"
#endif

#ifdef MODULE_02
  #define MODULE_ID_02  2 // Stopky 2: Ukončuje S
  #include "02ss.h"
  #include "02ss.cpp"
#endif

#ifdef MODULE_03
  #define MODULE_ID_03  3 // Stopky 3: Ukončuje L nebo P
  #include "03slp.h"
  #include "03slp.cpp"
#endif

#ifdef MODULE_04
  #define MODULE_ID_04  4 // Hodiny
  #include "04clock.h"
  #include "04clock.cpp"
#endif

#ifdef MODULE_99
  #define MODULE_ID_99  99  // FPS demo
  #include "99demo.h"
  #include "99demo.cpp"
#endif

#ifdef MODULE_SETUP
  #define MODULE_ID_00  0  // 0 je pro system
  #include "../core/settings.h"
  #include "../core/settings.cpp"
#endif
