# 12_scheduler_full_demo

## 1. Cilj

Povezati prioritete, preotimanje procesora (*preemption*) i stanja dretvi u jednom primjeru: timer periodički budi dretvu višeg prioriteta, koja preuzima CPU od dretve nižeg prioriteta.

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| Scheduler (raspoređivač) | Odabire dretvu najvišeg prioriteta među spremnima (READY). |
| Preemption (preotimanje procesora) | Dretva višeg prioriteta može prekinuti dretvu nižeg prioriteta. |
| `K_TIMER_DEFINE()` | Definira timer koji periodički generira događaj. |
| `k_sem_give()` | Timer handler signalizira HIGH dretvu. |
| `k_sem_take(..., K_FOREVER)` | HIGH čeka događaj i ne zauzima CPU dok čeka. |
| `k_busy_wait()` | Simulira aktivni rad LOW dretve; ne uspavljuje je. |

**Važno:** `k_busy_wait()` ne sprječava preotimanje procesora. Koristimo ga samo za demonstraciju jer nepotrebno troši CPU vrijeme.

## 3. Struktura

```text
12_scheduler_full_demo/
├── src/
│   └── main.c
├── CMakeLists.txt
├── prj.conf
└── README.md
```

**Hardver:** ESP32 DevKitC V4 i USB serijska konzola; vanjsko ožičenje nije potrebno. Primjer je namijenjen jednoj procesorskoj jezgri (`esp32_devkitc/esp32/procpu`).

## 4. Kako radi

1. `main()` pokreće periodični timer (prvi događaj nakon 2000 ms, zatim svakih 2000 ms).
2. `HIGH` (prioritet `1`) čeka semafor (`BLOCKED`), a `LOW` (prioritet `10`) stalno radi.
3. Timer handler u kontekstu prekida poziva `k_sem_give()`.
4. `HIGH` postaje READY i, nakon obrade prekida, preotima CPU od `LOW` te ispisuje poruku.
5. `HIGH` ponovno čeka semafor, pa se `LOW` nastavlja izvršavati.

Za razliku od `11_scheduler_priority`, ovdje HIGH **nije stalno READY**: većinu vremena čeka događaj, zato LOW redovito dobiva CPU.

**Napomena:** Binarni semafor (maksimum `1`) može spojiti više brzih događaja u jedan signal; ovaj primjer obrađuje događaje vrlo brzo.

Izvorni kod: `src/main.c`.

## 5. Izgradnja i provjera

Iz direktorija `zephyr_workspace` (aktivirano Zephyr okruženje):

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/04_rtos_fundamentals/12_scheduler_full_demo -d zephyr_course/04_rtos_fundamentals/12_scheduler_full_demo/build -p always
west flash -d zephyr_course/04_rtos_fundamentals/12_scheduler_full_demo/build
```

Primjer ispisa (redoslijed početnih poruka i razmaci mogu odstupati):

```text
Scheduler Full Demo
[HIGH] waiting for timer
[LOW] working...
[LOW] working...
...
[HIGH] timer event - preempts LOW
[HIGH] waiting for timer
[LOW] working...
...
```

## 6. Zadaci

1. Promijeni `TIMER_PERIOD_MS` s `2000` na `1000`. Kako se mijenja učestalost HIGH poruka?
2. Zamijeni `k_busy_wait(LOW_WORK_US)` u LOW dretvi s `k_msleep(300)`. Zašto HIGH i dalje može preuzeti CPU kada timer istekne?
