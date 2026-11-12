# 08_k_timer

## 1. Cilj

Upoznati Zephyrov kernel timer (`k_timer`) i pokretati periodički posao u dretvi pomoću semafora, bez aktivnog čekanja (*busy waiting*).

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| `K_TIMER_DEFINE()` | Statički definira i inicijalizira kernel timer. |
| `k_timer_start()` | Pokreće timer; prvi argument vremena je početna odgoda, a drugi period ponavljanja. |
| Timer expiry handler | Funkcija koja se poziva pri isteku timera; izvršava se u kontekstu prekida. |
| `K_SEM_DEFINE()` | Definira semafor kojim timer signalizira dretvu. |
| `k_sem_give()` | Šalje signal iz timer handlera. |
| `k_sem_take()` | Dretva čeka signal bez stalnog provjeravanja timera. |

**Važno:** U timer handleru ne smijemo blokirati niti obavljati dugotrajan posao. Semafor ima maksimum `1`, pa se više brzih isteka može spojiti u jedan signal.

## 3. Struktura

```text
08_k_timer/
├── src/
│   └── main.c
├── CMakeLists.txt
├── prj.conf
└── README.md
```

**Hardver:** ESP32 DevKitC V4 i USB serijska konzola. Vanjsko ožičenje nije potrebno.

## 4. Kako radi

1. `main()` pokreće timer: prvi istek nakon 1000 ms, zatim svakih 1000 ms.
2. `worker_thread()` čeka na `k_sem_take(&timer_sem, K_FOREVER)`.
3. Pri isteku timera `timer_handler()` poziva `k_sem_give()`.
4. Dretva se budi, ispisuje broj obrađenih događaja i ponovno čeka.

**Razlika u odnosu na `k_msleep()`:** `k_msleep()` uspavljuje pozivajuću dretvu, a `k_timer` samostalno generira vremenske događaje. Timer i dretva nisu ista stvar.

Izvorni kod: `src/main.c`.

## 5. Izgradnja i provjera

Iz direktorija `zephyr_workspace` (aktivirano Zephyr okruženje):

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/04_rtos_fundamentals/08_k_timer -d zephyr_course/04_rtos_fundamentals/08_k_timer/build -p always
west flash -d zephyr_course/04_rtos_fundamentals/08_k_timer/build
```

Primjer ispisa (približno jednom u sekundi):

```text
Kernel Timer Example
[worker] timer tick: 1
[worker] timer tick: 2
[worker] timer tick: 3
...
```

## 6. Zadaci

1. Promijeni `TIMER_PERIOD_MS` s `1000` na `500`. Kako se mijenja učestalost ispisa?
2. Nakon `printk()` u workeru dodaj `k_msleep(1500)`. Zašto broj obrađenih događaja više ne mora odgovarati broju isteka timera?
