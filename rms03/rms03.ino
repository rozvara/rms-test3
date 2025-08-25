// RMS - firmware pro modulární stopky; primárně Sakul.cz Stopky v4.1 a v5.0
// (c) 2025 Rozvářa
// Licencováno pod GNU GPL v3. Bez záruky.
// Viz <https://www.gnu.org/licenses/gpl-3.0.html> pro plné znění licence.

#define FW_VERSION "RMS 0.3"

#include "build.h"         // procesor, displej, moduly, předvolby
#include "config/config.h" // PCB/piny

#include "core/lcd.h"
#include "core/rms.h"
#include "core/log.h"
#include "core/io.h"
#include "core/displays.h"
#include "core/main.h"

#include "core/lcd.cpp"
#include "core/rms.cpp"
#include "core/log.cpp"
#include "core/io.cpp"
#include "core/displays.cpp"

#include "modules/module_code.h" // linkuje kód vybraných modulů
#include "core/modules.h"        // registrace modulů

#include "core/main.cpp"
