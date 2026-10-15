# 01 — Zephyr Project Structure

## 1. Cilj vježbe

Upoznati osnovnu strukturu Zephyr aplikacije, namjenu pojedinih datoteka i postupak izgradnje te učitavanja programa na mikroupravljač.

## 2. Potrebna oprema i razvojni alati

**Hardver:**
- ESP32 DevKitC V4
- USB kabel
- PC

**Softver:**
- Visual Studio Code
- Zephyr RTOS (verzija instalirana u lokalnom workspaceu)
- West
- Zephyr SDK i potrebni razvojni alati
- Serijski monitor

U ovoj vježbi koristi se programski jezik C.

## 3. Struktura Zephyr projekta

Osnovna struktura aplikacije:

```text
01_project_structure/
├── CMakeLists.txt
├── prj.conf
├── README.md
└── src/
    └── main.c
```

| Datoteka         | Namjena |
|------------------|---|
| `main.c`         | Izvorni C kod aplikacije. |
| `CMakeLists.txt` | Povezuje aplikaciju sa Zephyrovim sustavom izgradnje i navodi izvorne datoteke. |
| `prj.conf`       | Definira konfiguracijske opcije Zephyr aplikacije. |
| `README.md`      | Sadrži dokumentaciju, upute i zadatke. |

U složenijim projektima koriste se i dodatne datoteke, primjerice `app.overlay` za prilagodbu opisa hardvera te `sample.yaml` za automatizirano testiranje pomoću Twistera.

U ovom primjeru nisu potrebne.

## 4. Izvorni kod aplikacije

Datoteka `src/main.c`:

```c
#include <zephyr/sys/printk.h>

int main(void)
{
    printk("Hello from Zephyr!\n");
    printk("My first Zephyr application.\n");

    return 0;
}
```

**Objašnjenje:**

- `#include <zephyr/sys/printk.h>` — uključuje deklaraciju Zephyrove funkcije `printk()`.
- `int main(void)` — početna funkcija aplikacije.
- `printk()` — ispisuje tekst na Zephyrovu konzolu.
- `\n` — prelazak u novi red.
- `return 0;` — završava funkciju `main()`.

Funkcija `printk()` dio je Zephyrova API-ja i nije isto što i standardna C funkcija `printf()`.

## 5. Konfiguracija izgradnje

### 5.1. CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(project_structure)
target_sources(app PRIVATE src/main.c)
```
**Objašnjenje:**

| Naredba | Namjena |
|---|---|
| `cmake_minimum_required()` | Definira minimalnu potrebnu verziju CMakea. |
| `find_package(Zephyr ...)` | Pronalazi Zephyr i učitava njegov sustav izgradnje. |
| `project()` | Definira naziv projekta. |
| `target_sources()` | Dodaje `main.c` u izgradnju aplikacije. |

### 5.2. prj.conf

```ini
CONFIG_CONSOLE=y
CONFIG_PRINTK=y
```

**Objašnjenje:**

- `CONFIG_CONSOLE=y` — omogućuje konzolni podsustav.
- `CONFIG_PRINTK=y` — omogućuje funkcionalnost ispisa pomoću `printk()`.

Konfiguracija se obrađuje tijekom izgradnje aplikacije.

## 6. Izgradnja aplikacije

Otvoriti PowerShell terminal u direktoriju `zephyr_workspace`, u kojem je prethodno pripremljeno Zephyr razvojno okruženje.

Izvršiti:

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/01_foundations/01_project_structure -d zephyr_course/01_foundations/01_project_structure/build -p always
```

**Objašnjenje parametara:**

| Parametar | Namjena |
|---|---|
| `west build` | Pokreće izgradnju aplikacije. |
| `-b esp32_devkitc/esp32/procpu` | Odabire ciljnu pločicu i procesorsku konfiguraciju. |
| Putanja aplikacije | Određuje projekt koji se izgrađuje. |
| `-d .../build` | Određuje direktorij za generirane datoteke. |
| `-p always` | Pokreće čistu izgradnju. |

Nakon uspješne izgradnje u direktoriju `build/` nastaju generirane konfiguracijske datoteke i programske datoteke potrebne za učitavanje aplikacije.

## 7. Učitavanje programa na mikroupravljač

Spojiti ESP32 DevKitC V4 na računalo pomoću USB kabela.

Izvršiti:

```powershell
west flash -d zephyr_course/01_foundations/01_project_structure/build
```

Naredba `west flash` učitava izgrađeni program na mikroupravljač.

Ako je serijski monitor otvoren i zauzima isti COM port, potrebno ga je zatvoriti prije učitavanja.

## 8. Provjera rada

Otvoriti serijski monitor, odabrati odgovarajući COM port i postaviti brzinu na 115200 baud (ako je tako konfigurirana konzola).

Očekivani ispis aplikacije:

```text
Hello from Zephyr!
My first Zephyr application.
```

Ovisno o konfiguraciji sustava, prije ovog ispisa mogu se pojaviti i Zephyrove početne poruke.

## 9. Zadaci

1. Promijeniti tekst `Hello from Zephyr!` u vlastitu poruku.
2. Dodati treći poziv funkcije `printk()`.
3. Izgraditi aplikaciju.
4. Učitati je na ESP32.
5. Provjeriti novi ispis u serijskom monitoru.

## 10. Pitanja

1. Koja je uloga datoteke `main.c`?
2. Čemu služi `CMakeLists.txt`?
3. Što se konfigurira datotekom `prj.conf`?
4. Koja je razlika između `west build` i `west flash`?
5. Čemu služi funkcija `printk()`?
6. Zašto se koristi `#include <zephyr/sys/printk.h>`?
7. Što se nalazi u direktoriju `build/`?
8. Treba li direktorij `build/` spremati u Git repozitorij? Objasni.


Alat `west` omogućuje izgradnju aplikacije i njezino učitavanje na mikroupravljač.
