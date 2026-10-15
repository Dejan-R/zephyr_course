# 02 — Zephyr Build System

## 1. Cilj

Razumjeti kako Zephyr iz C izvornog koda izrađuje firmware, razlikovati uloge alata West, CMake, Ninja, prevoditelja i povezivača te usporediti inkrementalnu i *pristine* izgradnju. Upoznati i ulogu West manifesta u upravljanju repozitorijima razvojnog okruženja.

## 2. Kako nastaje firmware

Pojednostavljeni tijek izgradnje:

```text
west build
    |
    v
CMake + Zephyrova konfiguracija (Kconfig, Devicetree)
    |
    v
Ninja -> C compiler -> objektne datoteke -> linker
    |
    v
build/zephyr/zephyr.elf (+ ostali izlazi, ovisno o pločici)
    |
    v
west flash -> ESP32
```

- **West** pokreće izgradnju i učitavanje aplikacije te upravlja repozitorijima workspacea.
- **CMake** konfigurira projekt i generira pravila izgradnje; Zephyr pritom obrađuje Kconfig i Devicetree.
- **Ninja** izvršava zadatke izgradnje u uobičajenoj Zephyrovoj konfiguraciji.
- **C compiler** prevodi C datoteke u objektne datoteke.
- **Linker** povezuje objektne datoteke i biblioteke u izvršnu ELF datoteku.

Prikaz je pojednostavljen; stvarni build uključuje dodatne korake i može proizvesti više izlaznih datoteka.

## 3. Primjer — CMake utječe na aplikaciju

```text
02_build_system/
├── CMakeLists.txt
├── prj.conf
├── README.md
└── src/
    └── main.c
```

**`src/main.c`:**

```c
#include <zephyr/sys/printk.h>

#ifndef APP_VERSION
#define APP_VERSION "undefined"
#endif

int main(void)
{
    printk("Zephyr Build System\n");
    printk("Application version: %s\n", APP_VERSION);
    printk("Build completed successfully!\n");

    return 0;
}
```

**`CMakeLists.txt`:**

```cmake
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(build_system)
target_sources(app PRIVATE src/main.c)
target_compile_definitions(app PRIVATE
    APP_VERSION=\"1.0\"
)
```

**`prj.conf`:**

```ini
CONFIG_CONSOLE=y
CONFIG_PRINTK=y
```

Nova je naredba `target_compile_definitions()`: prevoditelju prosljeđuje makro `APP_VERSION` kao tekstni literal `"1.0"`. Ako nije definiran, `#ifndef` u programu postavlja vrijednost `"undefined"`.

> `APP_VERSION` je nastavni primjer kompilacijske definicije, a ne službeni Zephyrov sustav za verzioniranje firmwarea. `app.overlay` i `sample.yaml` nisu potrebni.

## 4. Izgradnja, pokretanje i analiza rezultata

Naredbe se izvršavaju u **PowerShellu**, iz direktorija `zephyr_workspace`, uz aktivno Zephyrovo razvojno okruženje.

**Prva, čista izgradnja:**

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/01_foundations/02_build_system -d zephyr_course/01_foundations/02_build_system/build -p always
```

**Učitavanje na ESP32** (zatvori serijski monitor ako zauzima isti COM port):

```powershell
west flash -d zephyr_course/01_foundations/02_build_system/build
```

Očekivani aplikacijski ispis:

```text
Zephyr Build System
Application version: 1.0
Build completed successfully!
```

Moguće su prethodne sistemske poruke. Serijski monitor obično je postavljen na 115200 baud.

**Generirane datoteke:**

| Datoteka | Tipična lokacija unutar `build/` | Uloga |
|---|---|---|
| `zephyr.elf` | `zephyr/zephyr.elf` | Povezani izvršni program, koristan i za debugiranje. |
| `zephyr.dts` | `zephyr/zephyr.dts` | Objedinjeni Devicetree opis. |
| `autoconf.h` | `zephyr/include/generated/zephyr/autoconf.h` | Makroi izvedeni iz Kconfig konfiguracije. |
| `.config` | `zephyr/.config` | Konačna Kconfig konfiguracija. |
| `build.ninja` | `build.ninja` | Pravila izgradnje koja izvršava Ninja. |

Generirane datoteke **ne uređuju se ručno**, a direktorij `build/` ne pohranjuje se u Git.

**Inkrementalna izgradnja** koristi postojeće stanje i ponovno izgrađuje ono što se promijenilo:

```powershell
west build -d zephyr_course/01_foundations/02_build_system/build
```

**Pristine izgradnja** uklanja prethodno generirano stanje te ponovno konfigurira i izgrađuje projekt:

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/01_foundations/02_build_system -d zephyr_course/01_foundations/02_build_system/build -p always
```

Pristine build koristan je pri promjeni pločice, važnijim promjenama konfiguracije ili rješavanju problema s build direktorijem; nije potreban nakon svake izmjene C koda.

### West manifest i verzije repozitorija

West ima još jednu važnu ulogu: upravlja repozitorijima u Zephyrovu workspaceu. Datoteka **`west.yml`** naziva se *manifest* i određuje projekte te njihove revizije. Manifest može uključivati i druge manifeste.

| Naredba | Namjena |
|---|---|
| `west init` | Inicijalizira novi West workspace. |
| `west update` | Usklađuje repozitorije s manifestom. |
| `west build` | Izgrađuje aplikaciju. |
| `west topdir` | Ispisuje korijenski direktorij workspacea. |
| `west manifest --path` | Ispisuje putanju aktivnog manifesta. |
| `west list` | Prikazuje projekte iz razriješenog manifesta. |

Za pregled **postojećeg** workspacea pokreni:

```powershell
west topdir
west manifest --path
west list
```

Pronađi aktivni `west.yml` i pregledaj `projects` i `revision`. Revizija može biti grana, oznaka verzije (*tag*) ili Git commit; za preciznu ponovljivost projekta korisno je zabilježiti konkretne commitove.

**Ne pokreći `west init` ni `west update` samo radi ovog zadatka.** Oni nisu potrebni za pregled i mogu promijeniti stanje razvojnog okruženja.

## 5. Zadaci

**A — Promjena verzije.** U `CMakeLists.txt` promijeni `APP_VERSION=\"1.0\"` u `APP_VERSION=\"2.0\"`. Pokreni inkrementalni build i `west flash`. Provjeri ispis `Application version: 2.0` i objasni zašto nisi mijenjao `main.c`.

**B — Analiza direktorija `build/`.** Pronađi `zephyr.elf`, `zephyr.dts`, `autoconf.h` i `build.ninja`. Opiši namjenu svake datoteke i objasni zašto se generirane datoteke ne uređuju ručno.

**C — Usporedba izgradnji.** Pokreni inkrementalni build bez izmjene koda, a zatim pristine build. Usporedi terminalski izlaz i pronađi korake ponovne konfiguracije i prevođenja.

**D — West manifest.** Pronađi aktivni `west.yml` pomoću `west manifest --path`. Pomoću `west list` identificiraj najmanje tri projekta i njihove revizije. Objasni razliku između `west update` i `west build` te zašto je odabir poznatih verzija važan za reproducibilan firmware.

## 6. Pitanja

1. Koja je razlika između uloga Westa, CMakea i Ninje?
2. Što rade prevoditelj i povezivač?
3. Čemu služi `target_compile_definitions()` u ovom primjeru?
4. Zašto je `APP_VERSION` dostupan u `main.c` iako ondje nije bezuvjetno definiran?
5. Koja je razlika između inkrementalnog i pristine builda?
6. Gdje se nalaze `zephyr.elf`, `zephyr.dts` i `autoconf.h`?
7. Čemu služi `west.yml` i koja je razlika između `west update` i `west build`?

**Zaključak:** West upravlja izgradnjom i repozitorijima workspacea, CMake priprema projekt, Ninja izvršava zadatke, a prevoditelj i povezivač stvaraju firmware. Manifest omogućuje kontrolu korištenih repozitorija i njihovih verzija.
