# 08 — Static Variables

## 1. Cilj

Razumjeti razliku između obične lokalne varijable i lokalne `static` varijable te objasniti što `static` znači na razini `.c` datoteke. Povezati novu temu s prethodnom vježbom `07_global_extern`, bez ponavljanja cijele teorije o globalnim varijablama.

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| **Automatska lokalna varijabla** | Varijabla deklarirana unutar funkcije bez `static`; njezino trajanje vezano je uz pojedino izvršavanje bloka. |
| **Lokalna `static` varijabla** | Vidljiva je samo u svojem bloku, ali zadržava vrijednost između poziva funkcije. |
| **`static` na razini datoteke** | Varijabli daje *internal linkage*: njezino se ime ne može povezati iz druge `.c` datoteke pomoću `extern`. |
| **Opseg (*scope*)** | Dio izvornog koda u kojem je identifikator vidljiv. |
| **Vijek trajanja (*storage duration*)** | Razdoblje tijekom kojeg objekt postoji. |
| **Povezivanje imena (*linkage*)** | Određuje mogu li deklaracije iz različitih dijelova programa označavati isti objekt. |

**Važno:** `static` ne znači uvijek isto. Unutar funkcije utječe na trajanje lokalne varijable, a na razini datoteke dodatno ograničava povezivanje njezina imena na tu izvornu datoteku. Statičke varijable postoje tijekom cijelog izvođenja programa, ali njihovo konkretno mjesto u memoriji ovisi o implementaciji i povezivaču.

## 3. Struktura projekta

```text
08_static/
├── CMakeLists.txt
├── prj.conf
├── README.md
└── src/
    ├── main.c
    ├── counter.c
    └── counter.h
```

- `main.c` tri puta poziva istu funkciju.
- `counter.c` sadrži demonstraciju automatske, lokalne statičke i statičke varijable na razini datoteke.
- `counter.h` sadrži deklaraciju funkcije `demonstrate_counters()`.
- `CMakeLists.txt` uključuje obje `.c` datoteke pomoću `target_sources()`.

Nije potreban `app.overlay` ni dodatni hardver.

## 4. Primjer — što ostaje između poziva?

U `src/counter.c` nalazi se varijabla na razini datoteke:

```c
/* File-scope static variable: accessible only within this source file. */
static int file_counter = 0;
```

Unutar funkcije `demonstrate_counters()` uspoređujemo dvije lokalne varijable:

```c
int automatic_counter = 0;
static int local_static_counter = 0;

automatic_counter++;
local_static_counter++;
file_counter++;
```

- `automatic_counter` pri svakom pozivu ponovno počinje od `0`, pa nakon povećanja uvijek iznosi `1`.
- `local_static_counter` pri idućem pozivu zadržava prethodnu vrijednost; redom iznosi `1`, `2`, `3`.
- `file_counter` također zadržava vrijednost, ali je deklariran izvan funkcije i njegovo je ime ograničeno na `counter.c`.

U `src/main.c` pozivamo funkciju tri puta:

```c
demonstrate_counters();
demonstrate_counters();
demonstrate_counters();
```

**Razlika u odnosu na `07_global_extern`:** ondje je globalna varijabla dostupna drugoj `.c` datoteci putem `extern` deklaracije. Ovdje `static int file_counter` ostaje skrivena unutar `counter.c`. Funkcija `demonstrate_counters()` i dalje je dostupna iz `main.c` jer sama nije deklarirana kao `static`.

## 5. Izgradnja i provjera

Naredbe se izvršavaju u **PowerShellu**, iz direktorija `zephyr_workspace`, uz aktivno Zephyrovo razvojno okruženje.

**Izgradnja:**

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/02_c_fundamentals/08_static -d zephyr_course/02_c_fundamentals/08_static/build -p always
```

**Učitavanje na ESP32** (zatvori serijski monitor ako koristi isti COM port):

```powershell
west flash -d zephyr_course/02_c_fundamentals/08_static/build
```

Očekivani **aplikacijski** ispis (moguće su i početne Zephyrove poruke):

```text
Static Variables
Auto: 1, Local static: 1, File static: 1
Auto: 1, Local static: 2, File static: 2
Auto: 1, Local static: 3, File static: 3
```

U ovom primjeru ne ispisujemo memorijske adrese jer je cilj usporediti trajanje i vidljivost varijabli.

## 6. Zadaci

**A — Broj poziva.** U `main.c` pozovi `demonstrate_counters()` pet puta. Predvidi ispis prije izgradnje i provjeri rezultat.

**B — Početna vrijednost.** U `counter.c` postavi početnu vrijednost `local_static_counter` na `10`. Pokreni program i objasni zašto vrijednost raste kroz više poziva.

**C — Uklanjanje `static`.** Privremeno promijeni `static int local_static_counter = 0;` u `int local_static_counter = 0;`. Usporedi ispis i vrati `static`.

**D — Statička varijabla na razini datoteke.** Privremeno ukloni `static` ispred `file_counter`, pa prouči razliku u mogućnosti povezivanja tog imena iz drugih `.c` datoteka. Nakon zadatka vrati izvornu deklaraciju.

**E — Vidljivost između datoteka.** Kao zaseban eksperiment pokušaj iz `main.c` pristupiti `file_counter` putem `extern int file_counter;` i ispisati ga. Pokreni čisti build i prouči pogrešku povezivača: simbol je definiran sa `static` u `counter.c` i zato nije dostupan drugim datotekama. Zatim ukloni privremeni kod.

Nakon zadataka vrati početne vrijednosti i izvorni kod projekta.

## 7. Pitanja

1. Zašto `automatic_counter` svaki put ispisuje `1`?
2. Zašto `local_static_counter` zadržava vrijednost između poziva funkcije?
3. Koja je razlika između opsega varijable i njezina vijeka trajanja?
4. Što `static` znači kada se koristi unutar funkcije?
5. Što `static` znači kada se koristi na razini `.c` datoteke?
6. Zašto `main.c` može pozvati `demonstrate_counters()`, ali ne može pomoću `extern` pristupiti `file_counter`?
7. Po čemu se `file_counter` razlikuje od globalne varijable `shared_value` iz `07_global_extern`?
8. Zašto nije ispravno tvrditi da se sve lokalne varijable nužno nalaze na stogu?

## 8. Zaključak

Obična lokalna varijabla stvara se za pojedino izvršavanje bloka, dok lokalna `static` varijabla zadržava svoju vrijednost između poziva funkcije. Na razini datoteke `static` ograničava povezivanje imena na tu `.c` datoteku, što omogućuje skrivanje internih podataka modula.
