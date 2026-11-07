# 03 — GPIO Button Interrupt (Direct ISR)

## 1. Cilj

Upoznati konfiguriranje GPIO prekida i registraciju callback funkcije u **Zephyr RTOS-u** na razvojnoj pločici **ESP32 DevKitC V4**. Svaki detektirani prijelaz tipkala u aktivno stanje poziva GPIO callback koji **izravno mijenja stanje LED diode** (*toggle*).


**Arhitektura:** `button -> GPIO interrupt -> callback (ISR context) -> LED toggle`

**Očekivano ponašanje:** LED počinje ugašena. Pritisak tipkala mijenja njezino stanje (OFF -> ON ili ON -> OFF). Otpuštanje ne bi trebalo promijeniti LED. Međutim, zbog istitravanja mehaničkog kontakta pojedini pritisak može izazvati više prekida i više promjena LED diode.

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| **Interrupt / prekid** | Događaj kojim hardver pokreće obradu bez stalnog programskog očitavanja ulaza. |
| **ISR (*Interrupt Service Routine*)** | Kod koji se izvršava u kontekstu prekida. Zephyrov GPIO driver obično obrađuje hardverski prekid i poziva registrirani aplikacijski callback. |
| **GPIO callback** | Funkcija koju GPIO driver poziva kada se dogodi prijavljeni prekid. Ovdje se izvršava u interrupt contextu. |
| **`gpio_callback`** | Struktura koja driveru čuva handler i masku GPIO pinova. Mora ostati valjana dok je registrirana. |
| **`gpio_init_callback()`** | Priprema callback strukturu, povezuje funkciju i masku GPIO pinova. |
| **`BIT(button.pin)`** | Stvara masku pina: bit na položaju `button.pin` postavlja na `1`. |
| **`gpio_add_callback()`** | Registrira pripremljeni callback na GPIO kontroleru. |
| **`gpio_pin_interrupt_configure_dt()`** | Konfigurira prekid za GPIO pin opisan u Devicetreeu. |
| **`GPIO_INT_EDGE_TO_ACTIVE`** | Okida prekid pri prijelazu u *logički aktivno* stanje; uz `GPIO_ACTIVE_LOW` to je fizički HIGH → LOW. |
| **`gpio_pin_toggle_dt()`** | Mijenja stanje GPIO izlaza (ON ↔ OFF). |
| **`ARG_UNUSED()`** | Označava parametar koji funkcija namjerno ne koristi. |
| **`k_sleep(K_FOREVER)`** | Uspavljuje glavnu dretvu bez periodičkog buđenja i bez pollinga tipkala. |
| **Debounce** | Uklanjanje istitravanja mehaničkog kontakta. U ovoj vježbi namjerno nije implementiran. |

## 3. Struktura projekta

```text
03_gpio_button_interrupt/
├── boards/
│   └── esp32_devkitc.overlay
├── src/
│   └── main.c
├── CMakeLists.txt
├── prj.conf
└── README.md
```

### Hardversko povezivanje

Potrebni su ESP32 DevKitC V4, LED dioda, otpornik **330 Ω**, tipkalo, spojne žice i eksperimentalna pločica.

| Komponenta | Spoj |
|---|---|
| LED — anoda (dulji izvod) | GPIO18 → otpornik 330 Ω → anoda |
| LED — katoda (kraći izvod) | GND |
| Tipkalo — prvi kontakt | GPIO27 |
| Tipkalo — drugi kontakt | GND |

**Tipkalo se spaja između GPIO27 i GND.** Interni `GPIO_PULL_UP` drži ulaz visoko kada je tipkalo otpušteno. Pritiskom se ulaz spaja na GND. Za četveronožno tipkalo provjeriti parove međusobno povezanih kontakata.

**Ne spajati LED bez serijskog otpornika.** Nije potreban dodatni vanjski pull-up otpornik.

### `boards/esp32_devkitc_procpu.overlay`

```dts
#include <zephyr/dt-bindings/gpio/gpio.h>

/ {
    aliases {
        led1 = &my_led;
        button1 = &my_button;
    };

    leds {
        compatible = "gpio-leds";

        my_led: led_0 {
            gpios = <&gpio0 18 GPIO_ACTIVE_HIGH>;
            label = "LED0";
        };
    };

    buttons {
        compatible = "gpio-keys";

        my_button: button_0 {
            gpios = <&gpio0 27 (GPIO_PULL_UP | GPIO_ACTIVE_LOW)>;
            label = "BUTTON1";
        };
    };
};
```

- `led1` i `button1` su Devicetree aliasi; C kod ih referencira s `DT_ALIAS()`.
- LED izlaz na GPIO18 fizički je aktivan u stanju HIGH.
- Tipkalo na GPIO27 logički je aktivno kada ulaz padne na LOW. Za logički `ACTIVE` upotrebljava se `GPIO_ACTIVE_LOW`.
- `gpio-leds` i `gpio-keys` daju standardan opis GPIO potrošača kroz Devicetree; ovdje **ne koristimo Zephyrov Input API** i ne oslanjamo se na njegov debounce.

### `prj.conf`

```conf
# GPIO support
CONFIG_GPIO=y

# Console output during initialization (not inside ISR)
CONFIG_CONSOLE=y
CONFIG_PRINTK=y
```

### `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(gpio_button_interrupt)

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
#include <zephyr/sys/util.h>

#define LED_NODE DT_ALIAS(led1)
#define BUTTON_NODE DT_ALIAS(button1)

#if !DT_NODE_HAS_STATUS(LED_NODE, okay)
#error "LED node is missing or disabled in Devicetree"
#endif

#if !DT_NODE_HAS_STATUS(BUTTON_NODE, okay)
#error "Button node is missing or disabled in Devicetree"
#endif

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

/* Callback data must remain valid for the lifetime of the interrupt. */
static struct gpio_callback button_cb_data;

/* GPIO callback: on ESP32 it runs in interrupt context. */
static void button_pressed(const struct device *port,
                           struct gpio_callback *cb,
                           gpio_port_pins_t pins)
{
    ARG_UNUSED(port);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    /* Keep the callback short. No sleep, logging, or debounce here. */
    (void)gpio_pin_toggle_dt(&led);
}

int main(void)
{
    int ret;

    printk("GPIO Button Interrupt (Direct ISR)\n");

    if (!gpio_is_ready_dt(&led)) {
        printk("LED GPIO controller is not ready\n");
        return -1;
    }

    if (!gpio_is_ready_dt(&button)) {
        printk("Button GPIO controller is not ready\n");
        return -1;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("Failed to configure LED GPIO (error %d)\n", ret);
        return ret;
    }

    ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret < 0) {
        printk("Failed to configure button GPIO (error %d)\n", ret);
        return ret;
    }

    /* Register the callback before enabling the GPIO interrupt. */
    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));

    ret = gpio_add_callback(button.port, &button_cb_data);
    if (ret < 0) {
        printk("Failed to add button callback (error %d)\n", ret);
        return ret;
    }

    ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret < 0) {
        printk("Failed to configure button interrupt (error %d)\n", ret);
        return ret;
    }

    printk("LED GPIO pin: %u\n", (unsigned int)led.pin);
    printk("Button GPIO pin: %u\n", (unsigned int)button.pin);
    printk("Interrupt: edge to ACTIVE (button press)\n");
    printk("Press the button to toggle the LED\n");

    /* The main thread does not poll the button. */
    while (true) {
        k_sleep(K_FOREVER);
    }
}
```

### Od Devicetree opisa do izvođenja

1. `DT_ALIAS()` nalazi `led1` i `button1` iz overlaya. `DT_NODE_HAS_STATUS()` provjerava tijekom prevođenja jesu li definirani i omogućeni.
2. `GPIO_DT_SPEC_GET()` priprema GPIO opise s kontrolerom, brojem pina i DT oznakama.
3. `gpio_is_ready_dt()` provjerava GPIO kontrolere. `gpio_pin_configure_dt()` postavlja LED kao izlaz (početno OFF), a tipkalo kao ulaz s internim pull-up otpornikom.
4. `gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin))` priprema callback i postavlja masku samo za GPIO27.
5. `gpio_add_callback(button.port, &button_cb_data)` registrira callback u popisu GPIO kontrolera.
6. **Tek nakon registracije** `gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE)` omogućuje prekid na prijelaz u logički aktivno stanje. Zbog `GPIO_ACTIVE_LOW` to znači fizički **HIGH -> LOW**, odnosno pritiskanje tipkala.
7. Pri prekidu GPIO driver poziva `button_pressed()`. Taj se callback za ugrađeni ESP32 GPIO izvršava u interrupt contextu, a ne kao zasebna dretva. `gpio_pin_toggle_dt()` mijenja stanje LED izlaza.
8. Glavna dretva ulazi u `k_sleep(K_FOREVER)`. Ne očitava tipkalo, ne troši CPU aktivnim čekanjem i ne upravlja LED-om.

### Što su parametri callbacka?

```c
static void button_pressed(const struct device *port,
                           struct gpio_callback *cb,
                           gpio_port_pins_t pins)
```

- `port` je pokazivač na GPIO kontroler na kojem se dogodio prekid.
- `cb` je pokazivač na strukturu callbacka koja je registrirana.
- `pins` je maska pinova za koje je driver prijavio događaj.
- `ARG_UNUSED()` izričito označava da se tri parametra u ovom jednostavnom primjeru ne koriste. Potpis funkcije ostaje usklađen sa Zephyr API-jem.

`button_cb_data` je `static`, a ne lokalna varijabla unutar `main()`. GPIO driver mora zadržati valjan pokazivač na callback strukturu i nakon završetka inicijalizacije. U ovom primjeru ona ima životni vijek cijele aplikacije.

### Zašto ne ispisujemo iz callbacka?

Callback se izvršava u kontekstu prekida. ISR mora imati kratko i predvidljivo trajanje. Funkcije poput `k_sleep()` ne smiju se pozivati iz ISR-a, a sporo ispisivanje preko serijske konzole nepotrebno produžuje obradu. Zato se `printk()` koristi **samo tijekom inicijalizacije**.

### Tijek i stanja

| Događaj | Fizički GPIO27 | Logički događaj | LED |
|---|---|---|---|
| Tipkalo otpušteno | HIGH (pull-up) | Nema novog prekida | Ostaje u prethodnom stanju |
| Tipkalo pritisnuto | LOW (GND) | `EDGE_TO_ACTIVE` | Mijenja stanje (toggle) |
| Tipkalo pušteno | HIGH | `EDGE_TO_INACTIVE` (nije konfiguriran) | Ostaje u prethodnom stanju |

**Važno:** `GPIO_INT_EDGE_TO_ACTIVE` ne znači 'prekid cijelo vrijeme dok je tipkalo pritisnuto', nego prekid zbog prijelaza u aktivno stanje. Jedan fizički pritisak *bez istitravanja* daje jednu promjenu LED diode. Ako je tipkalo pritisnuto već tijekom uključivanja prekida, možda neće nastati novi rub do sljedećeg otpuštanja i pritiska.


## 5. Izgradnja i provjera

Iz direktorija `zephyr_workspace` u PowerShellu, s aktiviranim Zephyrovim okruženjem:

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/03_gpio/03_gpio_button_interrupt -d zephyr_course/03_gpio/03_gpio_button_interrupt/build -p always
west flash -d zephyr_course/03_gpio/03_gpio_button_interrupt/build
```

Tijekom konfiguracije provjeriti da je pronađen overlay:

```text
-- Found devicetree overlay: .../boards/esp32_devkitc.overlay
```

Ako iz nekog razloga nije pronađen, može se navesti izričito:

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/03_gpio/03_gpio_button_interrupt -d zephyr_course/03_gpio/03_gpio_button_interrupt/build -p always -- -DDTC_OVERLAY_FILE=boards/esp32_devkitc.overlay
```

U `build/zephyr/zephyr.dts` provjeriti da su `led1` i `button1` povezani s GPIO18 i GPIO27.

Nakon učitavanja otvoriti serijsku konzolu na **115200 baud**. Očekivani aplikacijski ispis:

```text
GPIO Button Interrupt (Direct ISR)
LED GPIO pin: 18
Button GPIO pin: 27
Interrupt: edge to ACTIVE (button press)
Press the button to toggle the LED
```

## 6. Zadaci

**A — Razlika polling / interrupt.** Pokrenuti prethodnu i novu vježbu. Objasniti kada se u svakom primjeru izvršava kod koji reagira na tipkalo.

**B — Drugi rub prekida.** Privremeno zamijeniti `GPIO_INT_EDGE_TO_ACTIVE` s `GPIO_INT_EDGE_TO_INACTIVE`. Promatrati LED pri pritisku i otpuštanju te objasniti promjenu.

**C — Oba ruba.** Konfigurirati `GPIO_INT_EDGE_BOTH` i usporediti rezultat. Objasniti zašto LED može mijenjati stanje i na pritisak i na otpuštanje (uz dodatne promjene zbog bouncea).

**D — Callback maska.** Objasniti što predstavlja `BIT(button.pin)` i zašto callback ne treba primati svaki događaj sa svih pinova GPIO kontrolera.

**E — Redoslijed inicijalizacije.** Pronaći `gpio_init_callback()`, `gpio_add_callback()` i `gpio_pin_interrupt_configure_dt()`. Objasniti zašto se prekid uključuje **nakon** registracije callbacka.

**F — Istitravanje kontakata.** Više puta pritisnuti tipkalo i zabilježiti može li jedan fizički pritisak izazvati neočekivano stanje LED diode. Ne dodavati debounce u ovu vježbu.

**G — Životni vijek callback strukture.** Objasniti zašto je `button_cb_data` `static` i što bi bilo problematično kada bi se callback registrirao pomoću kratkotrajne lokalne strukture iz pomoćne funkcije.

## 7. Pitanja

1. Koja je razlika između pollinga i GPIO prekida?
2. Što je hardverski prekid, a što Zephyrov GPIO callback?
3. U kojem se izvršnom kontekstu poziva `button_pressed()` u ovom primjeru?
4. Zašto callback nije isto što i dodatna Zephyrova dretva?
5. Što znači `GPIO_INT_EDGE_TO_ACTIVE` u kombinaciji s `GPIO_ACTIVE_LOW`?
6. Zašto otpuštanje tipkala ne bi trebalo promijeniti LED?
7. Čemu služe `gpio_init_callback()` i `gpio_add_callback()`?
8. Što predstavlja `BIT(button.pin)`?
9. Zašto je `button_cb_data` deklariran s `static`?
10. Zašto ISR ne smije pozivati `k_sleep()`?
11. Zašto u ovom primjeru nema `printk()` u callbacku?
12. Zašto se glavna dretva može uspavati s `K_FOREVER`?
13. Kako mehanički *bounce* utječe na broj GPIO prekida i stanje LED diode?
14. Zašto izravno upravljanje LED-om u ISR-u nije uvijek primjereno za složenije aplikacije?
15. Kako bismo u sljedećoj vježbi razlikovali stvarni pritisak od kratkih oscilacija kontakta?