# 03_thread_states

## 1. Cilj

Upoznati stanja dretvi (READY, RUNNING, SLEEPING) i promatrati kako ih Zephyrov scheduler raspoređuje.

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| READY | Dretva je spremna za rad i čeka CPU. |
| RUNNING | Dretva se trenutačno izvršava (stanje raspoređivanja). |
| SLEEPING | Dretva čeka istek vremena, primjerice nakon `k_msleep()`. |
| `k_busy_wait()` | Aktivno čeka; ne uspavljuje dretvu. |
| `k_thread_state_str()` | Dijagnostički prikazuje trenutačno stanje druge dretve. |
| Preemption | Dretva višeg prioriteta može prekinuti dretvu nižeg prioriteta. |

**Napomena:** Dijagnostički ispis `queued` označava dretvu spremnu za CPU (READY). `RUNNING` nije zasebna vrijednost koju ova funkcija mora ispisati.

## 3. Struktura

```text
03_thread_states/
├── src/
│   └── main.c
├── CMakeLists.txt
├── prj.conf
└── README.md
```

**Hardver:** ESP32 DevKitC V4 i USB serijska konzola; bez vanjskog ožičenja.

## 4. Kako radi

- `worker_thread()` aktivno radi oko 600 ms, zatim spava oko 2000 ms.
- `observer_thread()` svakih 250 ms ispisuje stanje workera.
- Worker ima prioritet `7`, a observer `5` (manji broj = viši preemptivni prioritet).
- Dok worker radi, observer ga može prekinuti; tada je worker READY (`queued`). Dok worker spava, prikazuje se `sleeping`.

`k_busy_wait()` služi samo za ovu demonstraciju i inače nepotrebno troši CPU. Ispis je dijagnostički; redoslijed i broj poruka mogu se razlikovati.

Izvorni kod: `src/main.c`.

## 5. Izgradnja i provjera

Iz direktorija `zephyr_workspace`, uz aktivirano Zephyr okruženje:

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/04_rtos_fundamentals/03_thread_states -d zephyr_course/04_rtos_fundamentals/03_thread_states/build -p always
west flash -d zephyr_course/04_rtos_fundamentals/03_thread_states/build
```

Primjer dijela ispisa (redoslijed može odstupati):

```text
Thread States
[worker] RUNNING - doing work
[observer] worker state: queued
[worker] going to sleep
[observer] worker state: sleeping
...
```

## 6. Zadaci

1. Promijeni `OBSERVER_INTERVAL_MS` s `250` na `1000`. Koja stanja uspijevaš uočiti?
2. Promijeni `WORK_TIME_US` sa `600000` na `100000`. Kako se mijenja vjerojatnost opažanja stanja READY (`queued`)?
