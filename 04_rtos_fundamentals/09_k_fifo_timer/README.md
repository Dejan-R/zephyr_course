# 09_k_fifo_timer

## 1. Cilj

Povezati periodički kernel timer, semafor i FIFO u modelu *producer–consumer*. Naučiti kako `k_fifo` prenosi pokazivače na podatke, za razliku od `k_msgq` koji kopira poruke.

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| `K_FIFO_DEFINE()` | Statički definira i inicijalizira prazan FIFO red. |
| `struct data_item` | Struktura podatka; prvo polje rezervirano je za FIFO. |
| `k_fifo_put()` | Predaje pokazivač na objekt u FIFO red. |
| `k_fifo_get()` | Preuzima pokazivač ili čeka ako je red prazan. |
| `k_malloc()` / `k_free()` | Alociraju i oslobađaju memoriju objekta. |
| `CONFIG_HEAP_MEM_POOL_SIZE` | Veličina sistemskog heapa za `k_malloc()`. |

**Važno:** FIFO ne kopira sadržaj objekta. Nakon `k_fifo_put()` producer više ne smije koristiti predani objekt; consumer ga nakon obrade oslobađa pomoću `k_free()`. Dinamička alokacija ovdje je odabrana radi učenja, ali nije uvjet za korištenje FIFO-a.

## 3. Struktura

```text
09_k_fifo_timer/
├── src/
│   └── main.c
├── CMakeLists.txt
├── prj.conf
└── README.md
```

**Hardver:** ESP32 DevKitC V4 i USB serijska konzola. Vanjsko ožičenje nije potrebno.

## 4. Kako radi

1. `main()` pokreće timer; prvi događaj dolazi nakon 1000 ms, zatim svakih 1000 ms.
2. Timer handler poziva `k_sem_give()` i budi producer dretvu.
3. Producer pomoću `k_malloc()` stvara objekt, upisuje broj i predaje ga pomoću `k_fifo_put()`.
4. Consumer čeka na `k_fifo_get(..., K_FOREVER)`, čita broj i poziva `k_free()`.
5. Ciklus se ponavlja bez aktivnog čekanja.

**Razlika:** `k_msgq` kopira vrijednost u red, a `k_fifo` prenosi pokazivač na postojeći objekt. Binarni semafor (`0–1`) signalizira događaj timera i ne mora sačuvati svaki istek ako producer kasni.

Izvorni kod: `src/main.c`.

## 5. Izgradnja i provjera

Iz direktorija `zephyr_workspace` (aktivirano Zephyr okruženje):

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/04_rtos_fundamentals/09_k_fifo_timer -d zephyr_course/04_rtos_fundamentals/09_k_fifo_timer/build -p always
west flash -d zephyr_course/04_rtos_fundamentals/09_k_fifo_timer/build
```

Primjer ispisa (redoslijed pojedinih poruka može odstupati):

```text
Timer + FIFO Example
[producer] sent: 0
[consumer] received: 0
[producer] sent: 1
[consumer] received: 1
...
```

## 6. Zadaci

1. Promijeni `TIMER_PERIOD_MS` s `1000` na `500`. Kako se mijenja učestalost generiranja podataka?
2. Dodaj `k_msleep(1500)` u consumer nakon ispisa, a prije `k_free()`. Što se događa s redom kada producer stvara podatke brže nego što ih consumer obrađuje? Zašto se može pojaviti `allocation failed`?
