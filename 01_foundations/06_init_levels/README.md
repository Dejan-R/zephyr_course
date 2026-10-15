# 06 — Initialization Levels

## 1. Cilj

Razumjeti kako Zephyr pokreće inicijalizacijske funkcije **prije `main()`**, kako ih registriramo makroom `SYS_INIT()` te kako inicijalizacijska razina i prioritet određuju njihov redoslijed. Vježba koristi ESP32 DevKitC V4 i ne zahtijeva dodatni hardver.

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| **`SYS_INIT()`** | Registrira funkciju koju Zephyr poziva tijekom pokretanja sustava. |
| **Initialization level** | Faza pokretanja kojoj funkcija pripada. |
| **`PRE_KERNEL_1`** | Rana inicijalizacija bez dostupnih kernel servisa. |
| **`PRE_KERNEL_2`** | Sljedeća rana faza; kernel servisi još nisu dostupni. |
| **`POST_KERNEL`** | Faza nakon pokretanja kernela, kada su kernel servisi dostupni. |
| **`APPLICATION`** | Inicijalizacija neposredno prije aplikacijskog `main()`. |
| **Priority** | Broj koji određuje redoslijed unutar iste razine; manji broj izvršava se ranije. |

Zephyr ima i druge razine, primjerice `EARLY`, a `SMP` je dostupna u odgovarajućoj SMP konfiguraciji. Ovdje istražujemo četiri razine važne za razumijevanje tipičnog početka aplikacije.

## 3. Struktura projekta

```text
06_init_levels/
├── CMakeLists.txt
├── prj.conf
├── README.md
└── src/
    └── main.c
```

`app.overlay` nije potreban jer ne opisujemo dodatni hardver. U `prj.conf` nalaze se `CONFIG_CONSOLE=y` i `CONFIG_PRINTK=y`; `CMakeLists.txt` uključuje `src/main.c` kao i u prethodnim vježbama.

## 4. Redoslijed inicijalizacije

Potpuni kod nalazi se u [`src/main.c`](src/main.c). Funkcije registriramo ovako:

```c
SYS_INIT(init_pre_kernel_1, PRE_KERNEL_1, 80);
SYS_INIT(init_pre_kernel_2, PRE_KERNEL_2, 80);
SYS_INIT(init_post_kernel_first, POST_KERNEL, 80);
SYS_INIT(init_post_kernel_second, POST_KERNEL, 90);
SYS_INIT(init_application, APPLICATION, 80);
```

Svaka funkcija ima potpis `static int function_name(void)` i vraća `0`. U primjeru poziva `record_init()` kako bi zabilježila svoj redoslijed u polje `init_order`.

```c
static int init_post_kernel_first(void)
{
    record_init(2);
    return 0;
}
```

**Zašto ne koristimo `printk()` iz ranih inicijalizacijskih funkcija?** Konzola tada možda još nije spremna, a u fazama `PRE_KERNEL_1` i `PRE_KERNEL_2` nije dopušteno oslanjati se na uobičajene kernel servise. Zato inicijalizacijske funkcije samo bilježe redoslijed u memoriju, a `main()` ga kasnije ispisuje.

U dvije funkcije na razini `POST_KERNEL` namjerno koristimo prioritete `80` i `90`. Time demonstriramo da se **unutar iste razine prvo izvršava funkcija s manjim prioritetnim brojem**. Prioriteti različitih razina ne uspoređuju se međusobno: cijela razina `PRE_KERNEL_1` prethodi `PRE_KERNEL_2`, bez obzira na brojeve prioriteta.

Ova vježba koristi `SYS_INIT()`, a ne registraciju novog drivera. Driveri koriste povezanu infrastrukturu inicijalizacije, ali njihova registracija i API nisu predmet ovog primjera.

## 5. Izgradnja i provjera

U PowerShellu, iz direktorija `zephyr_workspace`, s aktivnim Zephyrovim okruženjem:

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/01_foundations/06_init_levels -d zephyr_course/01_foundations/06_init_levels/build -p always
west flash -d zephyr_course/01_foundations/06_init_levels/build
```

Očekivani aplikacijski ispis:

```text
Initialization Levels
1: PRE_KERNEL_1 (priority 80)
2: PRE_KERNEL_2 (priority 80)
3: POST_KERNEL (priority 80)
4: POST_KERNEL (priority 90)
5: APPLICATION (priority 80)
main() started
```

Moguće su Zephyrove i ESP32 početne poruke prije aplikacijskog ispisa. Redoslijed je zabilježen tijekom pokretanja, a prikazan tek iz `main()`.

## 6. Zadaci

**A — Promijeni prioritete.** Zamijeni prioritete `80` i `90` dviju `POST_KERNEL` funkcija. Ponovno izgradi aplikaciju i provjeri promjenu redoslijeda njihova izvršavanja.

**B — Dodaj inicijalizacijsku funkciju.** Proširi `EVENT_COUNT`, popis naziva i dodaj novu funkciju na razinu `APPLICATION` s prioritetom `90`. Provjeri da se izvršava nakon postojeće `APPLICATION` funkcije, ali prije `main()`.

**C — Promijeni razinu.** Premjesti `init_post_kernel_second` na `APPLICATION` i promatraj redoslijed. Objasni zašto je razina važnija od vrijednosti prioriteta druge razine.

**D — Istraži Zephyrov API.** U `include/zephyr/init.h` pronađi definiciju `SYS_INIT()` i utvrdi koja tri argumenta prima.

## 7. Pitanja

1. Koja je svrha makroa `SYS_INIT()`?
2. Koja je razlika između `PRE_KERNEL_1` i `POST_KERNEL`?
3. Zašto u ranim inicijalizacijskim funkcijama ne koristimo kernel servise?
4. Kako prioritet utječe na redoslijed funkcija unutar iste razine?
5. Zašto `POST_KERNEL` s prioritetom `90` prethodi `APPLICATION` s prioritetom `80`?
6. Zašto redoslijed bilježimo u memoriju, a ispisujemo u `main()`?
7. Po čemu se `SYS_INIT()` razlikuje od uobičajenog pozivanja funkcije iz `main()`?

## 8. Zaključak

Zephyr izvršava registrirane inicijalizacijske funkcije prije `main()`, prema zadanoj razini i prioritetu. `SYS_INIT()` omogućuje uključivanje vlastitog inicijalizacijskog koraka u pokretanje sustava bez izmjena aplikacijskog `main()`. Rana inicijalizacija ima ograničenja, pa se kernel servisi smiju koristiti tek u odgovarajućim kasnijim fazama.
