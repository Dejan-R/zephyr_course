# 01 — Functions

## 1. Cilj

Razumjeti deklaraciju, definiciju i pozivanje C funkcije, parametre i povratnu vrijednost te ulogu `.c` i `.h` datoteka u Zephyr aplikaciji. Primjer zbraja dva cijela broja funkcijom `my_function()`.

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| Funkcija | Imenovani dio programa koji izvršava određeni zadatak. |
| Deklaracija (prototip) | Opisuje naziv funkcije, tip povratne vrijednosti i parametre. |
| Definicija | Sadrži tijelo funkcije, odnosno njezinu implementaciju. |
| Argument | Vrijednost koja se predaje funkciji prilikom poziva. |
| Povratna vrijednost | Rezultat koji funkcija vraća naredbom `return`. |
| `.c` datoteka | Izvorna datoteka s implementacijama funkcija i drugim C kodom. |
| `.h` datoteka | Zaglavlje s deklaracijama dostupnima drugim izvornim datotekama. |
| Include guard | Sprječava višestruku obradu sadržaja istog zaglavlja unutar jedne prijevodne jedinice. |

U ESP-IDF-u početna funkcija aplikacije često je `app_main()`, a u ovom Zephyr primjeru koristimo `main()`.

## 3. Struktura projekta

```text
01_functions/
├── CMakeLists.txt
├── prj.conf
├── README.md
└── src/
    ├── main.c
    ├── my_function.c
    └── my_function.h
```

### `src/my_function.h`

```c
#ifndef MY_FUNCTION_H
#define MY_FUNCTION_H

/* Function declaration shared between C source files. */
int my_function(int a, int b);

#endif /* MY_FUNCTION_H */
```

Zaglavlje sadrži **deklaraciju**. `#ifndef`, `#define` i `#endif` čine *include guard*.

### `src/my_function.c`

```c
#include "my_function.h"

/* Function definition: add two integers and return the result. */
int my_function(int a, int b)
{
    return a + b;
}
```

Ova datoteka sadrži **definiciju** funkcije. Parametri `a` i `b` su tipa `int`, a `return a + b;` vraća njihov zbroj.

### `src/main.c`

```c
/*
 * C source files (.c) contain implementations of functions and application code.
 * Header files (.h) contain declarations shared between source files.
 */
#include <zephyr/sys/printk.h>
#include "my_function.h"

int main(void)
{
    int a = 4;
    int b = 5;
    int result = my_function(a, b);

    printk("Functions\n");
    printk("%d + %d = %d\n", a, b, result);

    return 0;
}
```

`main.c` uključuje zaglavlje i poziva funkciju. Za korištenje funkcije nije potrebno kopirati njezinu implementaciju u `main.c`.

### `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(functions)

target_sources(app PRIVATE
    src/main.c
    src/my_function.c
)
```

U `target_sources()` navedene su **obje `.c` datoteke**. Zaglavlje se ne navodi kao izvorna datoteka za prevođenje; `#include "my_function.h"` pronalazi ga jer se nalazi u istom direktoriju kao i izvorna datoteka koja ga uključuje.

### `prj.conf`

```ini
CONFIG_CONSOLE=y
CONFIG_PRINTK=y
```

Ova vježba ne treba `app.overlay`, dodatni hardver ni posebne Kconfig simbole.

## 4. Izgradnja i provjera

U PowerShellu, iz direktorija `zephyr_workspace`, uz aktivno Zephyrovo okruženje:

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/02_c_fundamentals/01_functions -d zephyr_course/02_c_fundamentals/01_functions/build -p always
west flash -d zephyr_course/02_c_fundamentals/01_functions/build
```

Očekivani aplikacijski ispis na serijskom monitoru:

```text
Functions
4 + 5 = 9
```

Ispis je očekivan prema izvornom kodu; konkretna izgradnja i izvođenje na ESP32-u nisu ovdje provjereni.

## 5. Zadaci

**A — Promjena argumenata.** U `main.c` postavi `a = 10` i `b = 7`. Ponovno izgradi aplikaciju i provjeri rezultat.

**B — Više poziva.** Pozovi `my_function()` još jednom s drugim brojevima i ispiši dobiveni rezultat.

**C — Promjena implementacije.** U `my_function.c` promijeni izraz `a + b` u `a - b`. Provjeri što se mijenja i zatim vrati zbrajanje.

**D — Pogreška povezivanja.** Privremeno ukloni `src/my_function.c` iz `target_sources()`. Pokreni izgradnju i pronađi poruku o nedostajućoj definiciji funkcije. Vrati datoteku u CMake i ponovno izgradi projekt.

**E — Zaglavlje.** Objasni čemu služi `my_function.h` i zašto sadrži deklaraciju, a ne implementaciju funkcije.

## 6. Pitanja

1. Koja je razlika između deklaracije i definicije funkcije?
2. Što su parametri, a što argumenti?
3. Što radi naredba `return a + b;`?
4. Zašto `main.c` uključuje `my_function.h`?
5. Zašto je `my_function.c` naveden u `target_sources()`?
6. Čemu služi *include guard*?
7. Koja je razlika između pogreške pri prevođenju i pogreške pri povezivanju?

## 7. Analiza primjera

Pri pozivu `my_function(4, 5)` argumenti se prosljeđuju parametrima `a` i `b`. Funkcija računa njihov zbroj te vraća `9`. Pozivatelj sprema povratnu vrijednost u varijablu `result` i ispisuje je pomoću `printk()`.

U ovom primjeru datoteke `main.c` i `my_function.c` prevode se odvojeno, a povezivač spaja njihove rezultate u aplikaciju. Funkcijska deklaracija u zaglavlju omogućuje provjeru ispravnosti poziva tijekom prevođenja.

## 8. Zaključak

Funkciju je moguće deklarirati u `.h` datoteci, definirati u zasebnoj `.c` datoteci i pozvati iz `main.c`. Zephyrov `CMakeLists.txt` mora uključiti sve `.c` datoteke čija je implementacija potrebna aplikaciji. Tako program ostaje pregledan i jednostavan za proširivanje.
