# 06_workqueue_example

## 1. Cilj

Upoznati odgođeno izvršavanje (*deferred execution*) u Zephyru: GPIO prekid predaje posao sistemskoj workqueue dretvi, koja mijenja stanje LED diode.

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| Workqueue | Red poslova koje obrađuje posebna Zephyrova dretva. |
| Work item (`k_work`) | Objekt koji predstavlja posao i njegovu funkciju obrade. |
| `K_WORK_DEFINE()` | Statički definira i inicijalizira posao s pripadnim handlerom. |

| Work handler | Funkcija koja se izvršava u kontekstu workqueue dretve, ne u ISR-u. |

## 3. Struktura

```text
06_workqueue_example/
├── src/
│   └── main.c
├── boards/
│   └── esp32_devkitc_procpu.overlay
├── CMakeLists.txt
├── prj.conf
└── README.md
```

**Hardver:** ESP32 DevKitC V4, tipkalo između GPIO27 i GND (interni pull-up), LED dioda na GPIO18 preko otpornika od npr. 330 Ω (GPIO18 -> otpornik -> anoda LED; katoda -> GND).

## 4. Kako radi

1. `main()` konfigurira LED i tipkalo, registrira GPIO callback te uključuje prekid.
2. Pritisak tipkala pokreće ISR; on samo poziva `k_work_submit(&button_work)`.
3. Zephyrova sistemska workqueue preuzima posao i izvršava `button_work_handler()`.
4. Handler mijenja stanje LED diode i ispisuje poruku.

**Važno:** Ne stvaramo vlastitu dretvu. Ako je isti work item već na čekanju, ponovno slanje ne dodaje drugu kopiju posla. Primjer ne koristi debounce, pa jedan pritisak može prouzročiti više prekida ili promjena LED diode (vidi `03_gpio/04_gpio_button_interrupt_debounce`).

Izvorni kod: `src/main.c`.

## 5. Izgradnja i provjera

Iz `zephyr_workspace` (aktivirano Zephyr okruženje):

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/04_rtos_fundamentals/06_workqueue_example -d zephyr_course/04_rtos_fundamentals/06_workqueue_example/build -p always -- -DDTC_OVERLAY_FILE=boards/esp32_devkitc_procpu.overlay
west flash -d zephyr_course/04_rtos_fundamentals/06_workqueue_example/build
```

Očekivani ispis nakon pritiska (broj poruka može odstupati):

```text
Button -> ISR -> Workqueue -> LED
Press the button on GPIO27
[workqueue] LED toggled
```

## 6. Zadaci

1. Privremeno ukloni `k_work_submit()` iz ISR-a. Zašto se LED više ne mijenja?
2. Zamijeni `gpio_pin_toggle_dt(&led)` s `gpio_pin_set_dt(&led, 1)`. Što se mijenja pri ponovljenim pritiscima?
