# 11_scheduler_priority

## 1. Cilj

Demonstrirati kako Zephyrov raspoređivač (*scheduler*) bira dretvu s najvišim prioritetom među spremnima (READY) te pokazati preotimanje procesora (*preemption*) i izgladnjivanje dretvi (*starvation*).

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| Prioritet | Manji broj označava viši prioritet. Prioritet `1` viši je od `5`, a `5` od `10`. |
| Preemption (preotimanje procesora) | Dretva višeg prioriteta može prekinuti izvršavanje dretve nižeg prioriteta i preuzeti CPU. |
| Starvation (izgladnjivanje dretve) | Dretva nižeg prioriteta može ostati bez CPU vremena dok je dretva višeg prioriteta stalno READY. |
| `k_busy_wait()` | Aktivno čeka bez uspavljivanja dretve; tijekom čekanja troši CPU vrijeme. |
| `K_THREAD_DEFINE()` — delay | Zadnji argument određuje početnu odgodu pokretanja dretve u milisekundama. |

**Važno:** `k_busy_wait()` ne sprječava preotimanje procesora (*preemption*). Dretva višeg prioriteta i dalje može preuzeti CPU.

Vremensko dijeljenje procesorskog vremena (*time-slicing*) ne rješava izgladnjivanje dretvi različitih prioriteta.

## 3. Struktura

```text
11_scheduler_priority/
├── src/
│   └── main.c
├── CMakeLists.txt
├── prj.conf
└── README.md
```

**Hardver:** ESP32 DevKitC V4 i USB serijska konzola. Vanjsko ožičenje nije potrebno. Primjer koristi jednu procesorsku jezgru (`esp32_devkitc/esp32/procpu`).

## 4. Kako radi

Zephyr raspoređuje tri dretve različitih prioriteta. Sve imaju beskonačnu petlju i ne koriste `k_msleep()`.

Dretve se pokreću s različitim početnim odgodama radi lakšeg promatranja:

1. `LOW` (prioritet `10`) pokreće se odmah.
2. Nakon približno 2 s `MID` (prioritet `5`) postaje READY, preuzima CPU i prekida izvršavanje dretve `LOW` (*preemption*).
3. Nakon približno 4 s `HIGH` (prioritet `1`) postaje READY, preuzima CPU i prekida izvršavanje dretve `MID`.
4. `HIGH` ostaje spremna za izvršavanje i zauzima CPU. Dretve nižih prioriteta više ne dobivaju CPU vrijeme (*starvation*).

`k_busy_wait()` troši CPU vrijeme i ovdje služi **samo za demonstraciju**. Hardverski prekidi i dalje se mogu izvršavati.

Izvorni kod: `src/main.c`.

## 5. Izgradnja i provjera

Iz direktorija `zephyr_workspace` (aktivirano Zephyr okruženje):

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/04_rtos_fundamentals/11_scheduler_priority -d zephyr_course/04_rtos_fundamentals/11_scheduler_priority/build -p always

west flash -d zephyr_course/04_rtos_fundamentals/11_scheduler_priority/build
```

Primjer ispisa (prijelazi su približni; `LOW` i `MID` ponavljaju se više puta):

```text
Scheduler Priority Example
LOW starts now, MID after 2 s, HIGH after 4 s

[LOW] running
[LOW] running
...

[MID] running
[MID] running
...

[HIGH] running
[HIGH] running
...
```

## 6. Zadaci

1. Zamijeni `HIGH_PRIORITY` i `LOW_PRIORITY`. Koja će dretva dominirati nakon što se sve pokrenu?
2. U `high_thread()` zamijeni `k_busy_wait(BUSY_WAIT_US)` s `k_msleep(200)`. Zašto `MID` sada dobiva CPU, a `LOW` i dalje može ostati bez CPU vremena?