# 05_msgq_example

## 1. Cilj

Razmijeniti podatke između dvije dretve pomoću reda poruka (*message queue*) `k_msgq`. Producer šalje brojeve, a consumer ih prima.

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| Message queue | FIFO red poruka fiksne veličine i kapaciteta. |
| `K_MSGQ_DEFINE()` | Statički definira red poruka i njegov spremnik. |
| `k_msgq_put()` | Šalje kopiju podatka u red. |
| `k_msgq_get()` | Preuzima podatak iz reda kopiranjem. |
| `K_FOREVER` | Čekanje bez vremenskog ograničenja kada se ne može poslati ili primiti poruka. |

**Važno:** `k_msgq` kopira vrijednost (`int`), a ne adresu lokalne varijable. Dinamička memorija nije potrebna. Red može sadržavati najviše 10 poruka.

## 3. Struktura

```text
05_msgq_example/
├── src/
│   └── main.c
├── CMakeLists.txt
├── prj.conf
└── README.md
```

**Hardver:** ESP32 DevKitC V4 i USB serijska konzola. Vanjsko ožičenje nije potrebno.

## 4. Kako radi

1. `producer_thread()` svake sekunde šalje broj pomoću `k_msgq_put()`.
2. `consumer_thread()` čeka poruku pozivom `k_msgq_get()`.
3. Nakon primitka consumer ispisuje primljenu vrijednost.
4. Ako je red prazan, consumer čeka; ako je pun, producer čeka (`K_FOREVER`).

Obje dretve imaju prioritet `7`. U osnovnoj verziji consumer je dovoljno brz pa red obično ostaje prazan. Redoslijed ispisa može varirati zbog schedulera.

**Razlika u odnosu na semafor:** semafor signalizira događaj, a `k_msgq` prenosi i podatak. `k_fifo` (kasnija lekcija) prenosi pokazivače na stavke, umjesto da kopira njihovu vrijednost.

Izvorni kod: `src/main.c`.

## 5. Izgradnja i provjera

Iz direktorija `zephyr_workspace` (aktivirano Zephyr okruženje):

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/04_rtos_fundamentals/05_msgq_example -d zephyr_course/04_rtos_fundamentals/05_msgq_example/build -p always
west flash -d zephyr_course/04_rtos_fundamentals/05_msgq_example/build
```

Primjer serijskog ispisa (redoslijed prvih poruka može odstupati):

```text
Message Queue Example
[producer] sent: 0
[consumer] received: 0
[producer] sent: 1
[consumer] received: 1
...
```

## 6. Zadaci

1. Promijeni `PRODUCER_INTERVAL_MS` s `1000` na `500`. Što se mijenja?
2. U consumer dodaj `k_msleep(2000)` nakon ispisa. Što se dogodi kad se red napuni? Zašto `K_FOREVER` sprječava odbacivanje poruka u ovom primjeru?
