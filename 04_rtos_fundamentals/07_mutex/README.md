# 07_mutex

## 1. Cilj

Zaštititi zajedničku varijablu (`counter`) kojoj pristupaju dvije dretve pomoću mutexa (`k_mutex`).

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| Mutex | Osigurava isključiv pristup zajedničkom resursu. |
| `K_MUTEX_DEFINE()` | Statički definira i inicijalizira mutex. |
| `k_mutex_lock()` | Zaključava mutex; uz `K_FOREVER` čeka ako je zauzet. |
| `k_mutex_unlock()` | Otključava mutex; poziva ga dretva koja ga je zaključala. |
| Kritična sekcija | Dio koda u kojem se pristupa zaštićenom podatku. |
| Priority inheritance | Mehanizam koji može privremeno povisiti prioritet vlasnika mutexa ako ga čeka dretva višeg prioriteta. |

**Važno:** Mutex se koristi u kontekstu dretve, ne iz ISR-a. Kritičnu sekciju treba držati kratkom; `k_msleep()` i `printk()` su izvan mutexa.

## 3. Struktura

```text
07_mutex/
├── src/
│   └── main.c
├── CMakeLists.txt
├── prj.conf
└── README.md
```

**Hardver:** ESP32 DevKitC V4 i USB serijska konzola. Vanjsko ožičenje nije potrebno.

## 4. Kako radi

- `producer_thread()` svakih približno 500 ms zaključa mutex, poveća `counter` i otključa ga.
- `consumer_thread()` svakih približno 700 ms zaključa mutex, pročita `counter` u lokalnu varijablu, otključa mutex i ispiše očitanu vrijednost.
- Ako jedna dretva drži mutex, druga na `k_mutex_lock()` čeka dok mutex ne bude slobodan.
- Obje dretve imaju prioritet `7`; mutex štiti zajedničku varijablu, a ne redoslijed ispisa.

**Razlika u odnosu na semafor:** semafor smo koristili za dojavu događaja, a mutex ovdje za zaštitu zajedničkog podatka. Primjer pokazuje ispravan obrazac zaštite; uklanjanje mutexa ne mora uvijek dati vidljivu pogrešku na ovom hardveru.

Izvorni kod: `src/main.c`.

## 5. Izgradnja i provjera

Iz direktorija `zephyr_workspace` (aktivirano Zephyr okruženje):

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/04_rtos_fundamentals/07_mutex -d zephyr_course/04_rtos_fundamentals/07_mutex/build -p always
west flash -d zephyr_course/04_rtos_fundamentals/07_mutex/build
```

Primjer ispisa (redoslijed i vrijednosti mogu odstupati):

```text
Mutex Example
[consumer] counter: 1
[consumer] counter: 2
[consumer] counter: 3
...
```

## 6. Zadaci

1. Promijeni `PRODUCER_INTERVAL_MS` s `500` na `200`. Kako se mijenja ispis?
2. Premjesti `printk()` unutar kritične sekcije i usporedi. Zašto je bolje ispis raditi nakon otključavanja?
