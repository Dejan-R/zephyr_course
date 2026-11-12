# 04_button_sem_thread

## 1. Cilj

Povezati GPIO prekid (*interrupt*) i dretvu pomoću binarnog semafora. Dretva čeka pritisak tipkala bez stalnog provjeravanja GPIO pina (*polling*).

Napomena: Ovaj primjer ne koristi debounce. Zbog mehaničkog titranja kontakata jedan fizički pritisak tipkala može izazvati više GPIO prekida. Debounce je zasebno obrađen u 03_gpio/04_gpio_button_interrupt_debounce.

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| Semafor | RTOS mehanizam za signalizaciju događaja i sinkronizaciju. |
| `K_SEM_DEFINE(button_sem, 0, 1)` | Definira semafor s početnim brojem 0 i maksimumom 1 (binarni semafor). |
| `k_sem_take()` | Preuzima signal ili blokira dretvu dok signal ne stigne. |
| `k_sem_give()` | Signalizira semafor; može se pozvati iz GPIO prekida. |
| `K_FOREVER` | Čekanje bez vremenskog ograničenja. |
| ISR | Prekidna rutina; ovdje samo obavještava dretvu. |

## 3. Struktura

```text
04_button_sem_thread/
├── src/
│   └── main.c
├── boards/
│   └── esp32_devkitc_procpu.overlay
├── CMakeLists.txt
├── prj.conf
└── README.md
```

**Hardver:** ESP32 DevKitC V4 i tipkalo spojeno između **GPIO27 i GND**. Koristi se interni pull-up otpornik; vanjski otpornik nije potreban.

## 4. Kako radi

1. `main()` konfigurira GPIO27 kao ulaz i uključuje prekid na pritisak.
2. `button_thread()` čeka na `k_sem_take(&button_sem, K_FOREVER)` (BLOCKED).
3. Pritiskom tipkala GPIO callback poziva `k_sem_give()`.
4. Dretva postaje READY, dobiva CPU, ispisuje poruku i ponovno čeka.

**Važno:** ISR ne ispisuje poruke i ne čeka; samo signalizira semafor. Semafor s maksimumom 1 ne broji svaki uzastopni prekid. Mehaničko tipkalo može proizvesti više poruka zbog odskakivanja kontakta (*bounce*); u ovom primjeru nema debouncea (vidi `03_gpio/04_gpio_button_interrupt_debounce`).

Izvorni kod: `src/main.c`.

## 5. Izgradnja i provjera

Iz `zephyr_workspace` (aktivirano Zephyr okruženje):

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/04_rtos_fundamentals/04_button_sem_thread -d zephyr_course/04_rtos_fundamentals/04_button_sem_thread/build -p always -- -DDTC_OVERLAY_FILE=boards/esp32_devkitc_procpu.overlay
west flash -d zephyr_course/04_rtos_fundamentals/04_button_sem_thread/build
```

Očekivani ispis (pri svakom prihvaćenom pritisku):

```text
Button -> ISR -> Semaphore -> Thread
Press the button on GPIO27
[thread] Button pressed!
[thread] Button pressed!
...
```

## 6. Zadaci

1. U ISR-u privremeno ukloni `k_sem_give()`. Zašto dretva više ne ispisuje poruke?
2. Dodaj `k_msleep(500)` nakon ispisa u dretvi. Što primjećuješ pri više brzih pritisaka i zašto?

