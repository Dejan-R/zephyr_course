# 03 — Kconfig Basics

## 1. Cilj

Razumjeti kako se u Zephyru definiraju vlastiti Kconfig simboli, kako im se vrijednosti zadaju u `prj.conf` te kako konačna konfiguracija utječe na prevođenje i ponašanje aplikacije.

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| **Kconfig** | Sustav za definiranje konfiguracijskih simbola, njihovih tipova i ovisnosti. |
| **`config`** | Definira konfiguracijski simbol. |
| **`bool` / `int`** | Logički odnosno cjelobrojni tip simbola. |
| **`default`** | Zadana vrijednost simbola. |
| **`range`** | Dopušteni raspon cjelobrojne vrijednosti. |
| **`depends on`** | Ovisnost koja određuje kada je simbol dostupan. |
| **`menu`** | Grupira povezane konfiguracijske opcije. |
| **`prj.conf`** | Zadaje željene vrijednosti konfiguracijskih simbola aplikacije. |
| **`menuconfig`** | Interaktivni alat za pregled i promjenu konfiguracije. |

**Važno:** u datoteci `Kconfig` simbol se definira bez prefiksa `CONFIG_` (npr. `COURSE_FEATURE`), a u `prj.conf` i C kodu koristi se `CONFIG_COURSE_FEATURE`.

## 3. Struktura projekta

```text
03_kconfig_basics/
├── CMakeLists.txt
├── Kconfig
├── prj.conf
├── README.md
└── src/
    └── main.c
```

### `Kconfig`

```kconfig
mainmenu "Kconfig Basics"

menu "Course settings"

config COURSE_FEATURE
    bool "Enable course feature"
    default y
    help
      Enables the example application feature.

config COURSE_REPEAT_COUNT
    int "Repeat count"
    range 1 5
    default 3
    depends on COURSE_FEATURE
    help
      Controls the number of messages printed when the feature is enabled.

endmenu

source "Kconfig.zephyr"
```

- `COURSE_FEATURE` uključuje ili isključuje demonstracijsku funkcionalnost.
- `COURSE_REPEAT_COUNT` određuje broj ispisa od **1 do 5**, a dostupan je samo kada je `COURSE_FEATURE` uključen.
- `source "Kconfig.zephyr"` uključuje Zephyrove standardne Kconfig definicije.
- Datoteka `Kconfig` nalazi se u korijenu aplikacije.

### `prj.conf`

```ini
CONFIG_CONSOLE=y
CONFIG_PRINTK=y
CONFIG_COURSE_FEATURE=y
CONFIG_COURSE_REPEAT_COUNT=3
```

**`Kconfig` definira koje opcije postoje i njihova pravila, a `prj.conf` postavlja željene vrijednosti za aplikaciju.**

### Odakle dolazi konačna konfiguracija?

Konačna konfiguracija ne nastaje samo iz datoteke `prj.conf`. Kconfig obrađuje definicije i ovisnosti simbola, zadane vrijednosti te konfiguracijske postavke odabrane pločice i aplikacije.

| Izvor | Uloga |
|---|---|
| `Kconfig` | Definira simbole, njihove tipove, zadane vrijednosti i ovisnosti. |
| Konfiguracija pločice | Doprinosi početnim postavkama potrebnima za odabranu pločicu. |
| `prj.conf` | Zadaje željene vrijednosti opcija aplikacije. |
| `build/zephyr/.config` | Sadrži konačnu, razriješenu konfiguraciju. |
| `build/zephyr/include/generated/zephyr/autoconf.h` | Sadrži C makroe generirane iz konfiguracije. |

Vrijednost zapisana u `prj.conf` ne mora uvijek postati konačna vrijednost: na nju mogu utjecati Kconfig ovisnosti i druga konfiguracijska pravila.

`CMakeLists.txt` ostaje jednostavan kao u prethodnim vježbama. `app.overlay` nije potreban.

## 4. Kconfig u C programu

Potpuni kod nalazi se u [`src/main.c`](src/main.c). Ključni dio je:

```c
#if defined(CONFIG_COURSE_FEATURE)
    printk("Feature enabled: yes\n");

    for (int i = 1; i <= CONFIG_COURSE_REPEAT_COUNT; i++) {
        printk("Message: %d\n", i);
    }
#else
    printk("Feature enabled: no\n");
#endif
```

- `#if defined(...)` uvjetno uključuje dio koda tijekom prevođenja, na temelju konačne konfiguracije.
- `CONFIG_COURSE_REPEAT_COUNT` cjelobrojna je vrijednost generirana iz Kconfiga.
- Kada je funkcionalnost isključena, grana koja koristi `CONFIG_COURSE_REPEAT_COUNT` ne prevodi se.

**Ovo nije runtime uvjet `if`: odluka se donosi pri prevođenju.**

## 5. Izgradnja i provjera

U PowerShellu, iz direktorija `zephyr_workspace`, s aktivnim Zephyrovim okruženjem:

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/01_foundations/03_kconfig_basics -d zephyr_course/01_foundations/03_kconfig_basics/build -p always
west flash -d zephyr_course/01_foundations/03_kconfig_basics/build
```

Očekivani **aplikacijski** ispis:

```text
Kconfig Basics
Feature enabled: yes
Message: 1
Message: 2
Message: 3
```

Nakon izgradnje pregledaj:

- `build/zephyr/.config` — konačne vrijednosti konfiguracijskih simbola.
- `build/zephyr/include/generated/zephyr/autoconf.h` — generirani C makroi.

U `.config` potraži `CONFIG_COURSE_FEATURE=y` i `CONFIG_COURSE_REPEAT_COUNT=3`.

Za interaktivni pregled konfiguracije:

```powershell
west build -d zephyr_course/01_foundations/03_kconfig_basics/build -t menuconfig
```

U izborniku potraži **Course settings**. Ako `menuconfig` nije dostupan u terminalu, zadatke možeš izvršiti izmjenom `prj.conf`.

## 6. Zadaci

**A — Promjena cjelobrojne vrijednosti.** U `prj.conf` postavi `CONFIG_COURSE_REPEAT_COUNT=5`. Napravi čisti build i provjeri da se ispisuje pet poruka.

**B — Isključivanje funkcionalnosti.** Postavi `CONFIG_COURSE_FEATURE=n` i ukloni redak `CONFIG_COURSE_REPEAT_COUNT=3`, jer taj simbol ovisi o prvome. Ponovno izgradi aplikaciju. Očekivani aplikacijski ispis:

```text
Kconfig Basics
Feature enabled: no
```

**C — Istraživanje ovisnosti.** U `Kconfig` pronađi `depends on COURSE_FEATURE`. Objasni zašto `COURSE_REPEAT_COUNT` nije dostupan kada je funkcionalnost isključena.

**D — Zadane vrijednosti.** Vrati `CONFIG_COURSE_FEATURE=y`, ukloni `CONFIG_COURSE_REPEAT_COUNT` iz `prj.conf` i napravi čisti build. U `.config` provjeri vrijednost nastalu zbog `default 3`.

**E — Pregled konfiguracije.** Pronađi vlastite simbole u `.config` i `autoconf.h` te usporedi njihov zapis.

**F — Konačna vrijednost i ovisnosti.** Usporedi postavke iz `prj.conf` s vrijednostima u `.config`. Objasni zašto postavka iz `prj.conf` ne mora uvijek postati konačna konfiguracija.

Nakon promjene datoteka `Kconfig` ili `prj.conf` u ovoj vježbi koristi `-p always` za pouzdanu ponovnu konfiguraciju. Nakon zadataka vrati početne vrijednosti.

## 7. Pitanja

1. Koja je razlika između `Kconfig` i `prj.conf`?
2. Zašto u definiciji pišemo `COURSE_FEATURE`, a u C kodu `CONFIG_COURSE_FEATURE`?
3. Što znače `bool`, `int`, `default` i `range`?
4. Čemu služi `depends on COURSE_FEATURE`?
5. Što omogućuje `source "Kconfig.zephyr"`?
6. Što radi `#if defined(CONFIG_COURSE_FEATURE)`?
7. Koja je razlika između `.config` i `autoconf.h`?
8. Zašto Kconfig opcija može ukloniti dio koda iz firmwarea?
9. Zašto vrijednost iz `prj.conf` ne mora biti konačna vrijednost u `.config`?

## 8. Zaključak

Kconfig definira konfiguracijske simbole, njihove tipove, zadane vrijednosti i ovisnosti. `prj.conf` postavlja željene vrijednosti aplikacije, dok Kconfig sustav određuje konačnu konfiguraciju uzimajući u obzir pravila simbola i postavke pločice.

Zephyr zatim generira konfiguracijske datoteke i C makroe, pa možemo uključivati značajke firmwarea bez izmjena izvornog programskog koda.
