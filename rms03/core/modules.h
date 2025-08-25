// RMS - firmware pro modulární stopky
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.

#pragma once

struct ModuleInstalled {
  uint8_t id;
  char name[13];
  void (*loop)();
};

#define MODULE(id, name, loopFn) { id, #name, loopFn },
const ModuleInstalled modules[] = {
  #include "../modules/module_registration.h"
};
#undef MODULE
