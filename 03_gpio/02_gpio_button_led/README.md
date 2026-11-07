# 02 — GPIO Button LED (Polling)

## 1. Cilj

Upoznati očitavanje GPIO ulaza i upravljanje GPIO izlazom u Zephyr RTOS-u na razvojnoj pločici **ESP32 DevKitC V4**. Vanjsko tipkalo očitava se periodički u glavnoj dretvi (*polling*), a LED dioda svijetli dok je tipkalo pritisnuto.

**Arhitektura:** `button -> polling (main loop) -> LED`

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| **GPIO ulaz** | Digitalni pin s kojeg se očitava logičko stanje. |
| **Polling** | Periodičko očitavanje ulaza u programskoj petlji. |
| **`gpio-keys`** | Devicetree binding za tipkala povezana GPIO pinovima. Ovdje se ulaz očitava izravno preko GPIO API-ja, bez Input API-ja. |
| **`GPIO_ACTIVE_LOW`** | Fizička niska razina predstavlja logički aktivno stanje (`1`). |
| **`GPIO_PULL_UP`** | Uključuje pull-up otpornik koji drži ulaz na visokoj razini kada tipkalo nije pritisnuto. |
| **`GPIO_INPUT`** | Konfigurira pin kao digitalni ulaz. |
| **`gpio_pin_get_dt()`** | Čita **logičko** stanje GPIO ulaza prema polaritetu iz Devicetreea. |
| **`gpio_pin_set_dt()`** | Postavlja **logičko** stanje GPIO izlaza. |
| **`k_sleep()`** | Uspavljuje trenutačnu dretvu; ovdje određuje približan interval očitavanja. |
| **Latency** | Vrijeme od promjene ulaznog signala do reakcije programa. |
| **Debounce** | Uklanjanje lažnih promjena uzrokovanih mehaničkim odskakivanjem kontakata. U ovom primjeru nije implementiran. |

## 3. Struktura projekta

```text
02_gpio_button_led/
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

Potrebni su ESP32 DevKitC V4, vanjska LED dioda, otpornik od **330 Ω**, tipkalo, spojne žice i eksperimentalna pločica.

| Komponenta | Spoj |
|---|---|
| LED — anoda (dulji izvod) | GPIO18 → otpornik 330 Ω → anoda LED diode |
| LED — katoda (kraći izvod) | GND |
| Tipkalo — prvi kontakt | GPIO27 |
| Tipkalo — drugi kontakt | GND |

Tipkalo spajati **između GPIO27 i GND**. Interni pull-up otpornik postavlja ulaz u visoko stanje kada je tipkalo otpušteno. Pri pritisku tipkala ulaz se spaja na GND. Kod tipkala s četiri izvoda provjeriti koji kontakti čine sklopku.

**Ne spajati LED diodu izravno bez otpornika.** U ovom spoju nije potreban vanjski pull-up otpornik za tipkalo.

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

`#include <zephyr/dt-bindings/gpio/gpio.h>` uključuje definicije `GPIO_ACTIVE_HIGH`, `GPIO_ACTIVE_LOW` i `GPIO_PULL_UP`.

- `led1` i `button1` su aliasi koje aplikacija koristi putem `DT_ALIAS()`.
- `my_led:` i `my_button:` su oznake čvorova (*node labels*), a `led_0` i `button_0` nazivi čvorova.
- `gpio-leds` opisuje GPIO LED diode, a `gpio-keys` GPIO tipkala. Odgovarajući bindingi određuju dopuštena svojstva čvorova.
- `gpios = <&gpio0 18 GPIO_ACTIVE_HIGH>` opisuje LED na GPIO18, aktivnu visokom fizičkom razinom.
- `gpios = <&gpio0 27 (GPIO_PULL_UP | GPIO_ACTIVE_LOW)>` opisuje tipkalo na GPIO27 s pull-up otpornikom i aktivnom niskom razinom.
- Svojstva `label = "LED0"` i `label = "BUTTON1"` služe kao tekstualne oznake; program ih ne koristi.

**Važno:** iako se za tipkalo koristi `compatible = "gpio-keys"`, u ovom projektu **ne koristi se Zephyrov Input API niti se uključuje upravljački program za događaje tipkala**. `main.c` sam očitava GPIO u petlji. Nije implementiran debounce; postavke debouncinga vezane uz Input API ne primjenjuju se na ovo izravno očitavanje.

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
project(gpio_button_led)

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
#define BUTTON_NODE DT_ALIAS(button1)
#define POLLING_INTERVAL_MS 10

#if !DT_NODE_HAS_STATUS(LED_NODE, okay)
#error "LED node is missing or disabled in Devicetree"
#endif

#if !DT_NODE_HAS_STATUS(BUTTON_NODE, okay)
#error "Button node is missing or disabled in Devicetree"
#endif

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

int main(void)
{
    int ret;
    int previous_state = -1;

    printk("GPIO Button LED (Polling)\n");

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

    printk("LED GPIO pin: %u\n", (unsigned int)led.pin);
    printk("Button GPIO pin: %u\n", (unsigned int)button.pin);
    printk("Polling interval: %d ms\n", POLLING_INTERVAL_MS);

    while (true) {
        int state = gpio_pin_get_dt(&button);

        if (state < 0) {
            printk("Failed to read button GPIO (error %d)\n", state);
            return state;
        }

        ret = gpio_pin_set_dt(&led, state);
        if (ret < 0) {
            printk("Failed to set LED GPIO (error %d)\n", ret);
            return ret;
        }

        if (state != previous_state) {
            printk("Button: %s | LED: %s\n",
                   state ? "PRESSED" : "RELEASED",
                   state ? "ON" : "OFF");
            previous_state = state;
        }

        k_sleep(K_MSEC(POLLING_INTERVAL_MS));
    }
}
```

**Od Devicetree opisa do izvršavanja programa:**

1. `DT_ALIAS(led1)` i `DT_ALIAS(button1)` povezuju C kod s čvorovima u Devicetreeu. `DT_NODE_HAS_STATUS()` tijekom izgradnje provjerava jesu li čvorovi prisutni i omogućeni.
2. `GPIO_DT_SPEC_GET()` za oba čvora priprema `gpio_dt_spec` s GPIO kontrolerom, brojem pina i oznakama iz Devicetreea.
3. `gpio_is_ready_dt()` provjerava jesu li kontroleri spremni. `gpio_pin_configure_dt()` postavlja GPIO18 kao izlaz, a GPIO27 kao ulaz s konfiguracijom pull-up iz Devicetreea. Povratne vrijednosti konfiguracije provjeravaju se.
4. `gpio_pin_get_dt(&button)` očitava **logičko**, a ne sirovo fizičko stanje tipkala. Zbog `GPIO_ACTIVE_LOW` pritisnuto tipkalo daje `1`, a otpušteno `0`.
5. `gpio_pin_set_dt(&led, state)` postavlja logičko stanje LED izlaza prema očitanoj vrijednosti. Zato LED svijetli dok je tipkalo pritisnuto.
6. `previous_state` omogućuje ispis samo kada se očitano stanje promijeni, bez nepotrebnog ispisa svakih 10 ms. Ne uklanja odskakivanje kontakata.
7. `k_sleep(K_MSEC(POLLING_INTERVAL_MS))` uspavljuje glavnu Zephyrovu dretvu između očitavanja. Interval je približno 10 ms, ali stvarno vrijeme ovisi i o izvršavanju koda te raspoređivanju dretvi.

**Stanja ulaza i izlaza:**

| Tipkalo | Fizička razina GPIO27 | `gpio_pin_get_dt()` | LED na GPIO18 |
|---|---|---|---|
| Otpušteno | HIGH (pull-up) | `0` — neaktivno | OFF |
| Pritisnuto | LOW (GND) | `1` — aktivno | ON |

**Tijek:** `main thread → pročitaj GPIO27 → postavi GPIO18 → provjeri promjenu → k_sleep(10 ms) → ponavljanje`.

**Polling i kompromis između odziva i opterećenja:** kraći interval omogućuje brže otkrivanje promjene ulaza, ali uzrokuje češća buđenja dretve i očitavanja. Dulji interval smanjuje učestalost provjera, ali može povećati kašnjenje reakcije i propustiti kratke pritiske. `k_sleep()` nije aktivno čekanje (*busy-wait*): dretva je tijekom odgode uspavana pa procesor može izvršavati druge poslove ili prijeći u stanje mirovanja kada to sustav podržava.

**Ograničenja:** mehaničko tipkalo može tijekom pritiska ili otpuštanja kratko promijeniti stanje više puta (*contact bounce*). Polling od 10 ms **nije debounce** i ne jamči da će svaka promjena biti stabilna. Ovaj primjer nema GPIO prekide ni ISR. Pristup s prekidima obrađuje se u sljedećoj vježbi.

## 5. Izgradnja i provjera

Iz direktorija `zephyr_workspace` u PowerShellu, s aktiviranim Zephyrovim okruženjem:

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/03_gpio/02_gpio_button_led -d zephyr_course/03_gpio/02_gpio_button_led/build -p always
west flash -d zephyr_course/03_gpio/02_gpio_button_led/build
```

Tijekom konfiguracije provjeriti poruku oblika:

```text
-- Found devicetree overlay: .../boards/esp32_devkitc_procpu.overlay
```

Ako overlay nije pronađen, pokrenuti čistu izgradnju s izričito navedenom datotekom:

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/03_gpio/02_gpio_button_led -d zephyr_course/03_gpio/02_gpio_button_led/build -p always -- -DDTC_OVERLAY_FILE=boards/esp32_devkitc_procpu.overlay
```

Primjenu overlaya provjeriti i u `build/zephyr/zephyr.dts`: pronaći aliase `led1` i `button1` te svojstva `gpios` za GPIO18 i GPIO27.

Nakon učitavanja otvoriti serijsku konzolu na **115200 baud**. Očekivani aplikacijski ispis:

```text
GPIO Button LED (Polling)
LED GPIO pin: 18
Button GPIO pin: 27
Polling interval: 10 ms
Button: RELEASED | LED: OFF
Button: PRESSED | LED: ON
Button: RELEASED | LED: OFF
```

Prvi redak stanja ovisi o tome je li tipkalo pritisnuto pri pokretanju. Daljnji redci pojavljuju se pri promjenama očitanog stanja; kod odskakivanja kontakata može ih biti više za jedan pritisak. Moguće su i Zephyrove početne poruke.

**Provjera:** dok je tipkalo pritisnuto LED treba svijetliti, a nakon otpuštanja ugasiti se. Ako ispis radi, a LED ne reagira, provjeriti polaritet LED diode, otpornik, GPIO18, GPIO27, GND i kontakte tipkala. Program ne može utvrditi je li fizička LED ispravno spojena.

## 6. Zadaci

**A — Interval očitavanja.** Promijeniti `POLLING_INTERVAL_MS` s `10` na `100`. Usporediti odziv tipkala i objasniti posljedice većeg intervala.

**B — Drugi ulazni GPIO.** Premjestiti tipkalo s GPIO27 na GPIO26 i promijeniti odgovarajuće svojstvo `gpios` u overlayu. Provjeriti rad bez izmjene broja pina u `main.c`.

**C — Logičko i fizičko stanje.** Objasniti zašto `gpio_pin_get_dt()` vraća `1` za pritisnuto tipkalo, iako je fizička razina na ulazu LOW. Usporediti s ponašanjem `gpio_pin_get_raw()`.

**D — Obrnuto ponašanje LED diode.** Izmijeniti program tako da LED svijetli kada je tipkalo otpušteno, a gasi se kada je pritisnuto.

**E — Ispis promjena.** Ukloniti provjeru `state != previous_state` i promatrati broj poruka na konzoli. Vratiti provjeru i objasniti njezinu svrhu.

**F — Devicetree.** U `build/zephyr/zephyr.dts` pronaći oba aliasa, oba čvora i njihove GPIO konfiguracije.

**G — Odskakivanje kontakata.** Promatrati ispis pri sporom pritiskanju i otpuštanju tipkala te objasniti zašto interval od 10 ms nije pouzdano uklanjanje odskakivanja kontakata.

Nakon promjene overlaya ponovno izgraditi projekt s `-p always`.

## 7. Pitanja

1. Po čemu se polling razlikuje od obrade tipkala putem GPIO prekida?
2. Zašto se tipkalo spaja između GPIO27 i GND te čemu služi `GPIO_PULL_UP`?
3. Koja je razlika između `GPIO_ACTIVE_LOW` i fizičke niske razine?
4. Zašto `gpio_pin_get_dt(&button)` vraća `1` kada je tipkalo pritisnuto?
5. Što predstavljaju aliasi `led1` i `button1` te što radi `GPIO_DT_SPEC_GET()`?
6. Koju ulogu imaju `gpio-keys` i `gpio-leds` u Devicetree opisu?
7. Zašto se nakon očitavanja poziva `gpio_pin_set_dt(&led, state)`?
8. Što provjeravaju `gpio_is_ready_dt()` i povratne vrijednosti GPIO funkcija?
9. Kako `POLLING_INTERVAL_MS` utječe na odziv, opterećenje i otkrivanje kratkih pritisaka?
10. Zašto polling s intervalom od 10 ms nije isto što i debounce?
11. Što radi `k_sleep()` i izvršava li se program tijekom te odgode kao aktivno čekanje?
12. Zašto je `previous_state` koristan i može li spriječiti odskakivanje kontakata?
