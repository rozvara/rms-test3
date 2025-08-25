// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

// Modul se přidává na 2 místech: tady a v module_code.h
// - délka názvu max 12, mezera nelze

#ifdef MODULE_01
  MODULE(MODULE_ID_01, Stopky_HU, module01Loop)
#endif

#ifdef MODULE_02
  MODULE(MODULE_ID_02, Stopky_S-S, module02Loop)
#endif

#ifdef MODULE_03
  MODULE(MODULE_ID_03, Stopky_S-LP, module03Loop)
#endif

#ifdef MODULE_04
  MODULE(MODULE_ID_04, Hodiny, module04Loop)
#endif

#ifdef MODULE_99
  MODULE(MODULE_ID_99, Demo, moduleDemoLoop)
#endif

#ifdef MODULE_SETUP
  MODULE(MODULE_ID_00, Nastaveni, moduleSetupLoop)
#endif
