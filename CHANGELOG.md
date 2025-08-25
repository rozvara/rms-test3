# Changelog

Změny v projektu RMS jsou evidovány v tomto dokumentu.
(Ty v závorce jsou spíše pro vývojáře, než uživatele.)


## [0.3] - 2025-08-25

### Přidáno

- obsluha vstupu IN5 (jen v5.0, resp. volitelně při buildu) pro mezičasy
- datum a čas ve výpisu logu (musí být modul Hodiny)
- interní hodiny pro systémy, které nemají RTC
- modul HU: délku opočtu lze nastavit tlačítky L/P
- modul HU: chyba koncáku kontroluje také IN5 (jen v5.0)
- modul HU: uložení nastavení (délky odpočtu)
- modul Hodiny: nastavení tlačítky (jen interní hodiny)
- modul Stopky S-S
- možnost vypnout/zapnout zobrazování času pulzů
- možnost uložit nastavení systému (výchozí modul, zobrazení času pulzů)
- možnost uložit nastavení modulu (pokud modul podporuje)
- (další konfigurační soubory pro build)

### Opraveno

- využití plného počtu záznamů logu
- bootloop po aktualizaci FW s menším počtem modulů
- (časování/Timer1)

### Změněno

- mezičas přesunut vstup IN5 (ve v4.1 tedy mezičas není a nebude)
- přepínání modulů; nyní má držení tl. Reset i další funkce
- odlišení běžících stopek na LCD displeji (typ pruhu v záhlaví)
- (modul Demo)
- (značky pro půlvteřinu a vteřinu nyní ladí s časováním FPS)
- (optimalizace kódu pro snížení velikosti pro v4.1)
- (refaktoring tam, kde kód začínal být dlouhý/nepřehledný)


## [0.2] - 2025-07-06

### Přidáno

- přidán nový stav pro chyby dráhy/koncáků v modulu 1 (HU)
- grafické rozlišení režimů na LCD displeji (typ pruhu v záhlaví)
- layouty pro sériové displeje (včetně podpory v nastavení)
- čtení a zápis nastavení z/do eeprom
- (modul Demo pro vizualizaci funkcí jádra)
- (konfigurační soubory pro další předpokládané buildy)
- (skripty pro jednoduchý build systém)

### Změněno

- FPS na 27 (x37ms=999ms, nedělitelné deseti pro hezčí setiny/tisíciny)
- čas pulzu zobrazuje navíc mikrovteřiny (přesnost 4 µs)
- (pípání předěláno na neblokující; lze pípat dlouze či přerušovaně)
- (čas z Arduino frameworku nahrazen časovačem bez softwarové korekce)
- (refaktoring kódu pro sériové displeje; lze je libovolně konfigurovat)
- (struktura složek/kódu projektu)


## [0.1] - 2025-06-13

První verze
- modul 1 (HU+odpočet přípravy)
- modul 3 (Stopky S-LP)
- modul 4 (RTC hodiny)
- modul Nastavení (jen pro RTC)
- jednotný log pro všechny moduly
- přepínání modulů
- (rychlé a precizní jádro)
- (rychlá a úsporná obsluha všech displejů při 40 fps)
- (konfigurační soubory pro základní buildy)
