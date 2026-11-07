# 01 — GPIO Toggle LED

## 1. Cilj

Upoznati osnovno upravljanje GPIO izlazom u Zephyr RTOS-u na razvojnoj pločici **ESP32 DevKitC V4**. Povezati vanjsku LED diodu, definirati je u Devicetree overlayu, konfigurirati GPIO izlaz i periodički mijenjati stanje diode pomoću `gpio_pin_toggle_dt()` i `k_sleep()`.

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| **GPIO izlaz** | Digitalni izlazni pin kojim program upravlja. |
| **`gpio_dt_spec`** | Struktura koja sadrži GPIO kontroler, broj pina i konfiguracijske oznake iz Devicetreea. |
| **`DT_ALIAS()`** | Dohvaća Devicetree čvor prema aliasu. |
| **`GPIO_DT_SPEC_GET()`** | Iz Devicetree svojstva `gpios` priprema specifikaciju GPIO pina. |
| **`gpio_is_ready_dt()`** | Provjerava je li GPIO kontroler inicijaliziran i spreman. |
| **`GPIO_OUTPUT_INACTIVE`** | Postavlja pin kao izlaz u početno neaktivno *logičko* stanje. |
| **`gpio_pin_toggle_dt()`** | Mijenja trenutačno logičko stanje GPIO izlaza. |
| **`k_sleep()`** | Uspavljuje trenutačnu dretvu na zadano vrijeme. |
| **`printk()`** | Ispisuje dijagnostičke poruke na Zephyrovu konzolu. |

## 3. Struktura projekta

```text
01_gpio_toggle_led/
├── boards/
│   └── esp32_devkitc_procpu.overlay
├── src/
│   └── main.c
├── CMakeLists.txt
├── prj.conf
└── README.md
```

Direktorij `build/` nastaje tijekom izgradnje i ne objavljuje se u Git repozitoriju.

### Hardversko povezivanje

Potrebni su ESP32 DevKitC V4, vanjska LED dioda, otpornik od **330 Ω**, spojne žice i eksperimentalna pločica.

| ESP32 / komponenta | Spoj |
|---|---|
| GPIO18 | Otpornik 330 Ω, zatim anoda LED diode (dulji izvod) |
| Katoda LED diode (kraći izvod) | GND na ESP32 |

LED dioda ovdje je spojena kao **aktivna na visokoj logičkoj razini** (`GPIO_ACTIVE_HIGH`). **Ne spajati LED diodu izravno bez otpornika.**

### `boards/esp32_devkitc_procpu.overlay`

```dts
#include <zephyr/dt-bindings/gpio/gpio.h>

/ {
    aliases {
        led1 = &my_led;
    };

    leds {
        compatible = "gpio-leds";

        my_led: led_0 {
            gpios = <&gpio0 18 GPIO_ACTIVE_HIGH>;
            label = "LED0";
        };
    };
};
```

`#include <zephyr/dt-bindings/gpio/gpio.h>` uključuje definicije GPIO oznaka, među kojima je i `GPIO_ACTIVE_HIGH`.

Svojstvo `compatible = "gpio-leds"` označava čvor koji sadrži LED diode upravljane GPIO pinovima. Odgovarajući Devicetree binding definira svojstva tih čvorova, uključujući svojstvo `gpios` koje je obvezno za pojedinu LED diodu.

`led1` je alias, `my_led:` oznaka čvora, `led_0` naziv čvora, a `label = "LED0"` neobvezno tekstualno svojstvo. U izrazu `gpios = <&gpio0 18 GPIO_ACTIVE_HIGH>` oznaka `&gpio0` referencira GPIO kontroler, `18` označava broj pina, a `GPIO_ACTIVE_HIGH` određuje da aktivnom logičkom stanju odgovara visoka fizička razina.

Devicetree opisuje GPIO vezu, a C program određuje ponašanje izlaza. Zbog toga je moguće promijeniti GPIO18 u GPIO26 u overlayu bez promjene programskog koda.

U ovom projektu **ne koristi se `app.overlay`** jer je projekt već organiziran s overlay datotekom unutar `boards/`, namijenjenom odabranoj ESP32 pločici. Pri izgradnji potrebno je potvrditi da je Zephyr pronašao upravo ovu datoteku.

### `prj.conf`

```conf
# GPIO support
CONFIG_GPIO=y

# Console output
CONFIG_CONSOLE=y
CONFIG_PRINTK=y
```

### `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(gpio_toggle_led)

target_sources(app PRIVATE src/main.c)
```

## 4. Objašnjenje programskog koda

Datoteka `src/main.c`:

```c
#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define LED_NODE DT_ALIAS(led1)
#define BLINK_INTERVAL_MS 1000

#if !DT_NODE_HAS_STATUS(LED_NODE, okay)
#error "LED node is missing or disabled in Devicetree"
#endif

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

int main(void)
{
    int ret;
    bool led_on = false;

    printk("GPIO Toggle LED\n");

    if (!gpio_is_ready_dt(&led)) {
        printk("GPIO controller is not ready\n");
        return -1;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("Failed to configure GPIO pin (error %d)\n", ret);
        return ret;
    }

    printk("LED GPIO pin: %u\n", (unsigned int)led.pin);

    while (true) {
        ret = gpio_pin_toggle_dt(&led);
        if (ret < 0) {
            printk("Failed to toggle GPIO pin (error %d)\n", ret);
            return ret;
        }

        led_on = !led_on;
        printk("LED: %s\n", led_on ? "ON" : "OFF");

        k_sleep(K_MSEC(BLINK_INTERVAL_MS));
    }
}
```

**Od Devicetree opisa do izvršavanja programa:**

1. `DT_ALIAS(led1)` povezuje C kod s LED čvorom u Devicetreeu. Makro `DT_NODE_HAS_STATUS()` provjerava tijekom izgradnje postoji li čvor i je li omogućen.
2. `GPIO_DT_SPEC_GET()` dohvaća GPIO kontroler, broj pina i oznake bez upisivanja GPIO broja u `main.c`.
3. `gpio_is_ready_dt()` provjerava spremnost GPIO **kontrolera**, ali ne može ustanoviti je li LED fizički priključena.
4. `gpio_pin_configure_dt()` postavlja GPIO18 kao izlaz, s početnim neaktivnim stanjem. Povratna vrijednost omogućuje prepoznavanje pogreške konfiguracije.
5. `gpio_pin_toggle_dt()` mijenja logičko stanje izlaza, a `led_on` prati očekivano stanje koje se ispisuje pomoću `printk()`.
6. `k_sleep(K_MSEC(1000))` uspavljuje **glavnu Zephyrovu dretvu** približno jednu sekundu. Zatim se ponavlja petlja. Dretva se nakon isteka odgode ponovno može rasporediti za izvršavanje; to nije precizno vremensko taktiranje.

**Tijek:** `main thread → while → toggle LED → printk → k_sleep → ponavljanje`.

## 5. Izgradnja i provjera

Iz direktorija `zephyr_workspace` u PowerShellu, s aktiviranim Zephyrovim okruženjem, pokrenuti:

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/03_gpio/01_gpio_toggle_led -d zephyr_course/03_gpio/01_gpio_toggle_led/build -p always
west flash -d zephyr_course/03_gpio/01_gpio_toggle_led/build
```

Tijekom konfiguracije provjeriti da se pojavljuje zapis oblika:

```text
-- Found devicetree overlay: .../boards/esp32_devkitc_procpu.overlay
```

**Ako se overlay ne pojavi**, ne pretpostavljati da je automatski učitan. Pokrenuti čistu izgradnju i izričito ga zadati:

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/03_gpio/01_gpio_toggle_led -d zephyr_course/03_gpio/01_gpio_toggle_led/build -p always -- -DDTC_OVERLAY_FILE=boards/esp32_devkitc_procpu.overlay
```

Primjenu overlaya dodatno provjeriti u `build/zephyr/zephyr.dts` (potražiti `led1` i GPIO18).

Očekivani **aplikacijski** ispis na serijskoj konzoli:

```text
GPIO Toggle LED
LED GPIO pin: 18
LED: ON
LED: OFF
LED: ON
LED: OFF
...
```

LED dioda treba biti uključena približno jednu sekundu, zatim isključena približno jednu sekundu, i tako neprekidno. **Puni ciklus uključivanja i isključivanja traje približno dvije sekunde.** Moguće su dodatne Zephyrove početne poruke. Ako se poruke ispisuju, a LED ne trepće, provjeriti polaritet diode, otpornik, GPIO18, GND i spojeve. Ispis nije dokaz ispravnog fizičkog povezivanja.

## 6. Zadaci

**A — Interval treptanja.** Promijeniti `BLINK_INTERVAL_MS` sa `1000` na `250`. Ponovno izgraditi i provjeriti brzinu treptanja.

**B — Drugi GPIO pin.** Promijeniti u overlayu GPIO18 u GPIO26, premjestiti žicu na GPIO26 i provjeriti rad bez izmjene GPIO broja u C kodu.

**C — Drugi alias.** Preimenovati `led1` u `status-led` u overlayu. Prilagoditi C makro na `DT_ALIAS(status_led)` i provjeriti izgradnju.

**D — Početno stanje.** Umjesto `GPIO_OUTPUT_INACTIVE` postaviti `GPIO_OUTPUT_ACTIVE`. Prilagoditi početnu vrijednost `led_on` tako da tekstualni ispis odgovara stanju diode.

**E — Provjera pogrešaka.** Objasniti što se događa ako `gpio_pin_configure_dt()` vrati negativnu vrijednost.

**F — Devicetree.** Otvoriti `build/zephyr/zephyr.dts` i pronaći alias, LED čvor i vrijednost svojstva `gpios`.

Nakon promjene overlaya koristiti `-p always` kako bi se Devicetree ponovno generirao.

## 7. Pitanja

1. Što predstavlja `led1` u Devicetreeu i zašto u C kodu pišemo `DT_ALIAS(led1)`?
2. Koja je razlika između `my_led:`, `led_0` i `label = "LED0"`?
3. Zašto se `GPIO_ACTIVE_HIGH` navodi u Devicetree overlayu i u kojoj je zaglavnoj datoteci definiran?
4. Koje podatke sadrži `gpio_dt_spec`?
5. Što provjerava `gpio_is_ready_dt()` i što ne može provjeriti?
6. Zašto provjeravamo povratne vrijednosti funkcija `gpio_pin_configure_dt()` i `gpio_pin_toggle_dt()`?
7. Koja je uloga `GPIO_OUTPUT_INACTIVE`?
8. Izvršava li se `main()` u Zephyru kao dretva?
9. Što se događa s dretvom tijekom poziva `k_sleep()`?
10. Zašto `k_sleep(K_MSEC(1000))` ne jamči potpuno precizan period od jedne sekunde?
