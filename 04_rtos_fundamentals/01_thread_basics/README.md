# 01_thread_basics

## 1. Cilj

Upoznati osnovni rad s dretvama (*threads*) u Zephyr RTOS-u, statičko stvaranje dretve pomoću `K_THREAD_DEFINE()` i uspavljivanje pomoću `k_msleep()`.

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| `main()` | Već se izvršava kao Zephyrova dretva. |
| `K_THREAD_DEFINE()` | Statički definira dretvu, njezin stack i kontrolni blok. Zadnji argument `0` znači pokretanje bez odgode. |
| `k_msleep(ms)` | Uspavljuje samo dretvu koja je poziva, ne cijeli procesor. |
| Prioritet | Kod preemptivnih dretvi manji broj označava viši prioritet. `worker` ovdje ima prioritet 5. |

**Dva osnovna načina stvaranja dretve u Zephyru:**

1. `K_THREAD_DEFINE()` — statičko definiranje dretve pomoću makroa.
2. `k_thread_create()` — stvaranje dretve tijekom izvođenja programa.

Kod `k_thread_create()` potrebno je prethodno osigurati stack i kontrolni blok (`struct k_thread`). To ne znači nužno korištenje dinamičke memorije (*heap*).

## 3. Struktura

```text
01_thread_basics/
├── src/
│   └── main.c
├── CMakeLists.txt
├── prj.conf
└── README.md
```

**Hardver:** ESP32 DevKitC V4 i USB serijska konzola. Vanjsko ožičenje nije potrebno.

## 4. Kako radi

Zephyr raspoređuje dvije dretve:

- `worker_thread()` — ispisuje poruku svakih približno 1000 ms.
- `main()` — ispisuje poruku svakih približno 1500 ms.

Dretva `worker_thread()` statički je definirana pomoću:

```c
K_THREAD_DEFINE(worker_id, STACK_SIZE, worker_thread,
                NULL, NULL, NULL, WORKER_PRIORITY, 0, 0);
```

Dok jedna dretva spava pomoću `k_msleep()`, druga se može izvršavati.

Redoslijed prvih ispisa nije strogo određen.

Izvorni kod: `src/main.c`.

## 5. Izgradnja i provjera

Iz direktorija `zephyr_workspace` (aktivirano Zephyr okruženje):

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/04_rtos_fundamentals/01_thread_basics -d zephyr_course/04_rtos_fundamentals/01_thread_basics/build -p always

west flash -d zephyr_course/04_rtos_fundamentals/01_thread_basics/build
```

Primjer ispisa na serijskoj konzoli (redoslijed i vremenski razmaci mogu odstupati):

```text
Thread Basics
Main thread running
Worker thread running
Worker thread running
Main thread running
Worker thread running
Main thread running
...
```

## 6. Zadaci

1. Promijeni `WORKER_INTERVAL_MS` s `1000` na `500`. Što primjećuješ u serijskom ispisu?
2. Promijeni `WORKER_PRIORITY` s `5` na `7`. Mijenja li se učestalost ispisa dok se obje dretve uglavnom uspavljuju?