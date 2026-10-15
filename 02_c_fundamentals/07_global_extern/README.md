# 07 — Global and Extern

## 1. Cilj

Razumjeti razliku između **definicije globalne varijable** i njezine **`extern` deklaracije**, omogućiti pristup istoj varijabli iz više `.c` datoteka te vidjeti kako povezivač povezuje njihove reference.

Primjer se nadovezuje na `01_functions`, gdje smo već koristili više izvornih datoteka. Ovdje je nova tema **dijeljenje jedne globalne varijable između datoteka**; lokalne i globalne `static` varijable obrađujemo u sljedećoj vježbi `08_static`.

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| **Globalna varijabla** | Varijabla definirana izvan funkcija, s trajanjem tijekom cijelog izvođenja programa. |
| **`extern`** | U ovom primjeru označava deklaraciju varijable definirane u drugoj `.c` datoteci. |
| **Deklaracija** | Obavještava prevoditelj o nazivu i tipu varijable, bez nove definicije objekta. |
| **Definicija** | Stvara objekt; `int shared_value = 55;` ujedno mu zadaje početnu vrijednost. |
| **Vanjsko povezivanje (*external linkage*)** | Omogućuje da se naziv iste globalne varijable poveže između različitih `.c` datoteka. |
| **Zaglavna datoteka (`.h`)** | Sadrži zajedničku deklaraciju dostupnu izvornim datotekama koje je uključuju. |
| **Povezivač (*linker*)** | Povezuje reference na `shared_value` s jednom odgovarajućom definicijom. |

**Važno:** `extern` ne znači da je varijabla automatski vidljiva u svakom dijelu programa. Datoteka koja je koristi mora imati odgovarajuću deklaraciju, ovdje preko `#include "shared_data.h"`. Ne stvaramo zasebnu kopiju varijable za svaku datoteku.

## 3. Struktura projekta

```text
07_global_extern/
├── CMakeLists.txt
├── prj.conf
├── README.md
└── src/
    ├── main.c
    ├── shared_data.c
    └── shared_data.h
```

- `src/shared_data.c` definira globalnu varijablu i funkciju koja je mijenja.
- `src/shared_data.h` deklarira varijablu i funkciju za korištenje u drugim izvornim datotekama.
- `src/main.c` pristupa globalnoj varijabli i poziva funkciju iz druge datoteke.

U `CMakeLists.txt` potrebno je navesti **obje** izvorne datoteke:

```cmake
target_sources(app PRIVATE
    src/main.c
    src/shared_data.c
)
```

Budući da se `shared_data.h` nalazi u istom direktoriju kao `.c` datoteke, nije potreban `target_include_directories()`. U `prj.conf` uključeni su `CONFIG_CONSOLE=y` i `CONFIG_PRINTK=y`. Hardverski priključci nisu potrebni.

## 4. Globalna varijabla i `extern` u C programu

**`src/shared_data.h` — zajednička deklaracija:**

```c
#ifndef SHARED_DATA_H
#define SHARED_DATA_H

/* Declare the global variable defined in shared_data.c. */
extern int shared_value;

/* Declare a function that modifies the same global variable. */
void increment_shared_value(void);

#endif /* SHARED_DATA_H */
```

Naredba `extern int shared_value;` **deklarira** varijablu, ali u ovom obliku ne stvara novu varijablu. Include guard sprječava ponovnu obradu istog zaglavlja unutar jedne jedinice prevođenja.

**`src/shared_data.c` — jedna definicija varijable:**

```c
#include "shared_data.h"

/* Define and initialize the global variable exactly once. */
int shared_value = 55;

void increment_shared_value(void)
{
    shared_value++;
}
```

Redak `int shared_value = 55;` je **definicija**. Funkcija `increment_shared_value()` mijenja upravo tu varijablu.

**`src/main.c` — korištenje varijable iz druge datoteke:**

```c
#include <zephyr/sys/printk.h>
#include "shared_data.h"

int main(void)
{
    printk("Global and Extern\n");
    printk("Initial shared value: %d\n", shared_value);

    /* Modify the same variable from another source file. */
    increment_shared_value();

    printk("Updated shared value: %d\n", shared_value);

    return 0;
}
```

Prvi ispis čita vrijednost `55`. Nakon poziva `increment_shared_value()` ista varijabla ima vrijednost `56`, što se vidi u `main.c` iako je promjena izvršena u `shared_data.c`.

**Razlika koju treba zapamtiti:**

```c
extern int shared_value; /* Declaration: in shared_data.h */
int shared_value = 55;   /* Definition: in shared_data.c */
```

Globalne varijable mogu biti korisne za zajedničko stanje, ali njihov nekontrolirani pristup otežava održavanje programa. U višedretvenim aplikacijama istodobni pristup zajedničkim podacima zahtijeva i odgovarajuću sinkronizaciju; to ćemo obraditi u RTOS i arhitekturnim vježbama.

## 5. Izgradnja i provjera

Naredbe izvršavaj u **PowerShellu**, iz direktorija `zephyr_workspace`, uz aktivno Zephyrovo razvojno okruženje.

**Čista izgradnja:**

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/02_c_fundamentals/07_global_extern -d zephyr_course/02_c_fundamentals/07_global_extern/build -p always
```

**Učitavanje na ESP32** (zatvori serijski monitor ako koristi isti COM port):

```powershell
west flash -d zephyr_course/02_c_fundamentals/07_global_extern/build
```

Očekivani **aplikacijski** ispis (moguće su i Zephyrove početne poruke):

```text
Global and Extern
Initial shared value: 55
Updated shared value: 56
```

Stvarni Zephyrov `west build` i izvođenje na ESP32 pločici nisu ovdje provjereni.

## 6. Zadaci

**A — Promjena početne vrijednosti.** U `shared_data.c` promijeni definiciju u `int shared_value = 100;`. Prije pokretanja predvidi oba ispisa, a zatim provjeri rezultat.

**B — Promjena vrijednosti iz `main.c`.** Vrati početnu vrijednost `55`. Nakon poziva `increment_shared_value()` dodaj `shared_value += 10;` i provjeri da završna vrijednost iznosi `66`.

**C — Nedostajuća definicija.** Privremeno zamijeni `int shared_value = 55;` u `shared_data.c` retkom `extern int shared_value;`. Pokreni čisti build i prouči pogrešku povezivača zbog nedostajuće definicije. Zatim vrati izvorni redak.

**D — Nedostajuća deklaracija.** Privremeno ukloni `#include "shared_data.h"` iz `main.c`. Pokreni build i prouči dijagnostiku prevoditelja, zatim vrati uključivanje zaglavlja.

**E — Dvostruka definicija.** U `main.c` privremeno dodaj `int shared_value = 42;` izvan funkcije `main()`. Pokreni čisti build i prouči pogrešku povezivača zbog dvije definicije istog globalnog simbola. Nakon toga ukloni dodani redak.

Nakon zadataka vrati izvorni kod kako bi ispis odgovarao primjeru.

## 7. Pitanja

1. Koja je razlika između deklaracije `extern int shared_value;` i definicije `int shared_value = 55;`?
2. Zašto je deklaracija globalne varijable smještena u `.h` datoteku?
3. Koliko stvarnih varijabli `shared_value` postoji u ovom projektu?
4. Zašto `main.c` može pristupiti varijabli definiranoj u `shared_data.c`?
5. Zašto promjena iz funkcije `increment_shared_value()` ostaje vidljiva u `main()`?
6. Zašto `shared_data.c` mora biti uključen u `target_sources()`?
7. Što se događa ako globalna varijabla nema definiciju ili je definirana dvaput?
8. Po čemu se `extern` razlikuje od `static` na razini datoteke i zašto će `static` imati zasebnu vježbu?

## 8. Zaključak

Globalna varijabla **definira se na jednom mjestu**, a iz drugih se `.c` datoteka može koristiti pomoću odgovarajuće **`extern` deklaracije**, najčešće smještene u zajedničko zaglavlje. Prevoditelj provjerava deklaracije, a povezivač povezuje reference s definicijom.