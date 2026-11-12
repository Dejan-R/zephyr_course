# 02_two_threads_led

## 1. Cilj

Pokrenuti dvije neovisne dretve (*threads*) koje upravljaju dvjema LED diodama različitim intervalima treptanja pomoću `K_THREAD_DEFINE()` i `k_msleep()`.

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| `K_THREAD_DEFINE()` | Statički definira dretvu, njezin stack i kontrolni blok. |
| `gpio_pin_toggle_dt()` | Mijenja trenutno stanje GPIO pina (ON/OFF). |
| `k_msleep(ms)` | Uspavljuje samo pozivajuću dretvu na određeno vrijeme. |
| Scheduler | Odlučuje koja će se dretva izvršavati na CPU-u. |
| Konkurentnost | Više dretvi napreduje neovisno, dijeleći procesorsko vrijeme. |

**Važno:** Obje dretve imaju isti prioritet (`7`). Time-slicing nije potreban jer se dretve redovito uspavljuju pomoću `k_msleep()`.

## 3. Struktura

```text
02_two_threads_led/
├── src/
│   └── main.c
├── boards/
│   └── esp32_devkitc_procpu.overlay
├── CMakeLists.txt
├── prj.conf
└── README.md
```

**Hardver:** ESP32 DevKitC V4, dvije LED diode i dva otpornika (npr. 330 Ω).

**Spajanje:**
- LED1: GPIO18 -> otpornik -> anoda LED1; katoda -> GND.
- LED2: GPIO19 -> otpornik -> anoda LED2; katoda -> GND.

## 4. Kako radi

Zephyr raspoređuje dvije neovisne dretve:

- `blink_fast()` — mijenja stanje LED1 svakih približno 200 ms.
- `blink_slow()` — mijenja stanje LED2 svakih približno 1000 ms.

Obje dretve statički su definirane pomoću:

```c
K_THREAD_DEFINE(thread1_id, STACK_SIZE, blink_fast,
                NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);

K_THREAD_DEFINE(thread2_id, STACK_SIZE, blink_slow,
                NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);
```

Svaka dretva prvo konfigurira vlastitu LED diodu, a zatim ulazi u beskonačnu petlju.

Dok jedna dretva spava pomoću `k_msleep()`, druga se može izvršavati.

`main()` ispisuje početnu poruku i završava, ali obje dretve nastavljaju raditi.

Dretve se izvršavaju konkurentno i nije im potrebna međusobna sinkronizacija jer upravljaju različitim GPIO pinovima.

Izvorni kod: `src/main.c`.

## 5. Izgradnja i provjera

Iz direktorija `zephyr_workspace` (aktivirano Zephyr okruženje):

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/04_rtos_fundamentals/02_two_threads_led -d zephyr_course/04_rtos_fundamentals/02_two_threads_led/build -p always -- -DDTC_OVERLAY_FILE=boards/esp32_devkitc_procpu.overlay

west flash -d zephyr_course/04_rtos_fundamentals/02_two_threads_led/build
```

**Očekivano ponašanje:**

- LED1 brzo treperi — promjena stanja svakih 200 ms.
- LED2 sporo treperi — promjena stanja svakih 1000 ms.

Puni ciklus uključivanja i isključivanja traje približno 400 ms za LED1 i 2000 ms za LED2.

## 6. Zadaci

1. Promijeni `FAST_INTERVAL_MS` s `200` na `500`. Što primjećuješ?
2. Postavi oba intervala na `500` ms. Moraju li LED diode mijenjati stanje istodobno?