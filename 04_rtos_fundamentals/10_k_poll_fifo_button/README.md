# 10_k_poll_fifo_button

## 1. Cilj

Naučiti kako jedna dretva pomoću `k_poll()` čeka **više vrsta događaja**: podatak u FIFO redu ili pritisak tipkala. Tipkalo uključuje/isključuje periodički timer; primljeni podatak mijenja LED.

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| `k_poll()` | Blokira dretvu dok barem jedan od praćenih događaja ne postane dostupan. |
| `struct k_poll_event` | Opisuje objekt i događaj koji čekamo. |
| `K_POLL_TYPE_FIFO_DATA_AVAILABLE` | FIFO sadrži podatak spreman za preuzimanje. |
| `K_POLL_TYPE_SEM_AVAILABLE` | Semafor je dostupan. |
| `K_POLL_MODE_NOTIFY_ONLY` | `k_poll()` samo obavještava; `k_fifo_get()` / `k_sem_take()` preuzimaju podatak/signal. |
| `K_POLL_STATE_NOT_READY` | Reset stanja događaja prije sljedećeg čekanja. |
| `k_timer_start()` / `k_timer_stop()` | Pokreću i zaustavljaju periodični timer. |

## 3. Struktura

```text
10_k_poll_fifo_button/
├── src/
│   └── main.c
├── boards/
│   └── esp32_devkitc_procpu.overlay
├── CMakeLists.txt
├── prj.conf
└── README.md
```

**Hardver:** ESP32 DevKitC V4; LED na GPIO18 preko otpornika 330 Ω do GND-a; tipkalo između GPIO27 i GND-a (interni pull-up).

## 4. Kako radi

1. `main()` inicijalizira GPIO i uključuje prekid tipkala. Timer je isprva zaustavljen.
2. ISR tipkala poziva `k_sem_give(&button_sem)`; `poll_thread()` prima signal pomoću `k_poll()` i pokreće/zaustavlja timer.
3. Timer svakih 1000 ms signalizira producer preko `timer_sem`.
4. Producer alocira objekt i predaje ga u FIFO pomoću `k_fifo_put()`.
5. `poll_thread()` prepoznaje FIFO događaj, preuzima objekt, ispisuje vrijednost, mijenja LED i oslobađa memoriju.

**Važno:** `k_poll()` **ne preuzima** signal ni podatak: nakon događaja pozivamo `k_sem_take(..., K_NO_WAIT)` ili `k_fifo_get(..., K_NO_WAIT)` i resetiramo `events[].state`. FIFO prenosi pokazivač (ne kopiju), pa nakon `k_fifo_put()` producer više ne smije pristupati objektu. Uključen je jednostavan vremenski filter tipkala od 50 ms (nije potpuni debounce); pri zaustavljanju timera već pripremljen podatak može još stići.

Izvorni kod: `src/main.c`.

## 5. Izgradnja i provjera

Iz `zephyr_workspace` uz aktivirano Zephyr okruženje:

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/04_rtos_fundamentals/10_k_poll_fifo_button -d zephyr_course/04_rtos_fundamentals/10_k_poll_fifo_button/build -p always -- -DDTC_OVERLAY_FILE=boards/esp32_devkitc_procpu.overlay
west flash -d zephyr_course/04_rtos_fundamentals/10_k_poll_fifo_button/build
```

Primjer ispisa (nakon prvog i drugog pritiska tipkala):

```text
k_poll + FIFO + Timer + Button
Press GPIO27 button to start/stop timer
[poll] timer started
[poll] data: 0
[poll] data: 1
[poll] data: 2
[poll] timer stopped
```

## 6. Zadaci

1. Promijeni `TIMER_PERIOD_MS` na `500`. Što se događa s brzinom promjene LED stanja?
2. U `poll_thread()` privremeno komentiraj reset `events[0].state`. Kako to utječe na rad `k_poll()` i zašto je reset važan?
