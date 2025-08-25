// RMS - firmware pro modulární stopky
// Modul "Demo"
// (c) 2025 Rozvářa – GNU GPL v3. Bez záruky.
  
/*
DEMO
- kontroly přerušení na vstupech a vyzvednutí času přerušení
- výpočtu času pro zobrazení na LCD i externích displejích
- použití flagů časování
- chování dvojtečky podle stavu dráhy
- neblokujícího pípáku
- výkonostního rozdílu při nepoužívání časovacích flagů
  (Reset přepíná zobrazení; lze zapnout i v nastavení `perf 1`)
*/


void moduleDemoLoop() {

  // proměnné modulu, které se přenáší z loopu do loopu
  // deklaruj jako static pro loop modulu
  static bool notThisWay = false;

  // navrhni FSM předem (viz ostatní moduly a dokumentace)
  // a dodržuj ho - tedy switch/case je to jediné, co v loopu je
  switch (moduleState) {

    // MOD_SHUTDOWN = "skonči" > udělej co třeba a nastav
    // MOD_EXITING, který netřeba implementovat (byl by jen `return`)
    case MOD_SHUTDOWN:
      moduleEcho(F("Demo konci"));
      // tady lze něco udělat (uložit, zobrazit, počítat)
      // (dokonce i na víc loopů) a potom nastav:
      changeModuleState(MOD_EXITING); // "skončil jsem"
      return;

    // MOD_STARTING je init (vstupní) stav modulu
    // udělej, co třeba a nastav stav, do kterého pak modul přejde,
    // což by měl být vždy MOD_IDLE
    case MOD_STARTING:

      // vymaže LCD, resp. jeho framebuffer
      lcdClear();

      // vykreslí název modulu v jednotném stylu
      lcdModuleTitle(F("Demo"));

      // na velkém displeji je prostor i pro na nápovědu
      #ifdef LCD_20X4
        lcdWrite(4, 3, F("--> terminal"));
      #endif

      // nastav šablonu sériovým displejům
      memcpy(intDisp.layout, intDispLayout, intDisp.len);
      memcpy(extDis1.layout, extDis1Layout, extDis1.len);
      memcpy(extDis2.layout, extDis2Layout, extDis2.len);

      // pro demo se hodí se startem časomíry zarovnat překreslování
      restartTimer();

      // levou dráhu použijeme pro čas na displejích
      leftTrack.finishTime = exactMillis(); 

      // při spuštění modulu zobrazujeme Uptime, tedy start je 0
      leftTrack.startTime = 0;

      // stav dráhy ovlivňuje chování dvojteček na displejích
      // při uptime neblikáme
      leftTrack.changeState(TRACK_READY);

      // přejdi do hlavního stavu, který obsluhuje tlačítka a zobrazení
      changeModuleState(MOD_IDLE);
      break;


    // MOD_IDLE je výchozí stav, který jediný říká hlavnímu loopu
    // "teď mě uživatel může přepnout"
    // resp. v jiném stavu přepnutí modulu nejde udělat -
    // hlavní loop si nevšímá tlačítka Reset, když není tento stav
    case MOD_IDLE:

      // flag firstEntry slouží k tomu, aby stav veděl, jestli
      // už běžel, nebo má něco udělat "poprvé a jednou"
      if (firstEntry) {
        firstEntry = false;
        SERIAL_PRINTLN();
        SERIAL_PRINTLN(F("[      27x37ms=999ms      ]    Cas pri '|'"));
      }

      // vstupy (signál od přerušení) kontrolujeme v každým loopu
      if (checkInput(&inStart)) {
        // Start -> jakoby odstartoval závod - stačí uložit čas pulzu,
        // přepnout stav modulu nebo dráhy apod.
        leftTrack.startTime = inStart.time;
        leftTrack.changeState(TRACK_RUNNING);
        restartTimer(); // pro demo se hodí zarovnat časování k startu
        buzzWakeUp(3, 1500, 80, 920); // demo beep - krátce, tři vteřiny
        firstEntry = true;
        return;
      }

      // (build s úsporou místa nemá perfomance overlay)
      #ifndef MINIMIZE_FLASH
        if (checkInput(&inReset)) {
          // Reset -> on/off performance overlay
          // Přepíná zobrazení počtu smyček programu za sekundu
          // 1.000.000/počet za sekundu je průměrná rychlost loopu v µs
          showPerfOverlay = !showPerfOverlay;
          lcdModuleTitle(F("Demo"));
          buzzer(INFO_BEEP);
        }
      #else
        // Reset v MOD_IDLE má i další funkce - je třeba ho konzumovat,
        // i když ho stav nepotřebuje
        checkInput(&inReset); 
      #endif

      // Levá -> ukázka dopadu, když vše běží v každým loopu
      // (což je stejně k ničemu, jádro vykreslí až s flagem)
      if (checkInput(&inLeft)) {
        notThisWay = true;
        buzzer(WARN_BEEP);
      }

      // Pravá -> výpočty a aktualizace jen při flagu timerFPS
      // a hned jak loop modulu skončí, jádro vykreslí změny
      if (checkInput(&inRight)) {
        notThisWay = false;
        buzzer(INFO_BEEP);
      }

      // při flagu časování
      // je správný čas aktualizovat všechny displeje
      if (timerFPS || notThisWay) {

        // 1. krok: uložit aktuální čas do proměnné (protože pointer)
        leftTrack.finishTime = exactMillis();

        // 2. krok: vytvoř časové komponenty z rozdílu dvou časů
        calcTimeParts(&leftTrack.startTime, &leftTrack.finishTime);

        // 3. krok: z časových komponent udělej text HHHH:MM:SS.sss
        tpToChars(&timeStr[0]);

        // HHHH:MM:SS.sss má 14 znaků -> sloupec podle displeje
        #ifdef LCD_16X2
           enum { c = 2 };  // na malém LCD:   __HHHH:MM:SS.sss
                            //                 Uptime
        #else
           enum { c = 6 };  // na velkém LCD:  ______HHHH:MM:SS.sss
                            //                 Uptime 
        #endif

        // text je přímo použitelný pro LCD, včetně blikající dvojtečky
        lcdWrite(c, 1, timeStr);  

        // protože má čas 14 znaků (zpočátku úvodní mezery místo ___H:)
        // titulek vykreslíme až potom
        if (leftTrack.state == TRACK_READY)
          // od zapnutí
          lcdWrite(0, 1, F("Uptime"));
        else
          // od startu tlačítkem
          lcdWrite(0, 1, F("Stopky"));

        // jednou vygenerovaný text se použije i pro sériové displeje.
        // ale výstup formátuje použitá formátovací šablona
        // viz ".layout" výše
        extDis1.applyLayout(&timeStr[0], leftTrack.colon);
        extDis2.applyLayout(&timeStr[0], leftTrack.colon);
        intDisp.applyLayout(&timeStr[0], leftTrack.colon);
      }

      // vizualizace překreslování aneb modul kontroluje události pořád,
      // ale o displeje se stará jen s flagem "timerFPS"
      //   .  událost fps => budou se překreslovat všechny displeje
      //   :  frame s příznakem "půlvteřina" (resp. 481/518ms)
      //   |  frame s příznakem "vteřina" (resp. každých 999ms)

      // 27 fps protože
      // - dostatečně svižné
      // - setiny a tisíciny nejsou stále stejné číslice
      // - 27x37ms=999ms, vhodné i pro orientační flagy (půl)vteřin

      // NEPLÉST SI S ČASOMÍROU
      // PŘÍZNAKY ŘÍDÍ JEN PŘEKRESLOVÁNÍ

      if (timerOneSec) {
        // levá dráha je už spočtená, můžeme rovnou zobrazit
        timeStr[7] = ':'; // do terminálu bez "blikání"
        SERIAL_PRINT("|");
        SERIAL_PRINTLN(timeStr);
      } else if (timerHalfSec) {
        SERIAL_PRINT(":");
      } else if (timerFPS) {
        SERIAL_PRINT(".");
      }
      break;

    // MOD_SAVESETT jednorázově nastaví systém a zavolá loop modulu, když
    // v systémovém menu uživatel zvolí "Uložit nastavení aktuálního modulu"
    case MOD_SAVESETT:
      SERIAL_PRINTLN(F("Demo dostalo pokyn ulozit sve nastaveni."));
      SERIAL_PRINTLN(F("Je to ale jen demo a do pameti nezapisuje."));
      buzzer(INFO_BEEP);
      break;
  }
}
