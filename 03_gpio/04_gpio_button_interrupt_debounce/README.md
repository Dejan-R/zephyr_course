# 04 — GPIO Button Interrupt + Timestamp Debounce

## 1. Cilj

Upoznati jednostavan **software debounce** temeljen na vremenu (timestamp) u Zephyr RTOS-u.


**Očekivano ponašanje:** LED je na početku ugašena. Prvi prekid pritiska mijenja stanje LED-a. Ostali prekidi unutar 50 ms od zadnjeg prihvaćenog pritiska ignoriraju se. Otpuštanje tipkala ne prebacuje LED (koristi se `GPIO_INT_EDGE_TO_ACTIVE`).

> **Važno:** Ova tehnika jest vremenski filtar događaja, **nije potvrda da se signal stabilizirao**. Jedan rub uzimamo kao valjan odmah, pa tijekom zadanog intervala ignoriramo sljedeće rubove. Vrlo dugo istitravanje ili vrlo brzi namjerni pritisci mogu biti pogrešno obrađeni.

## 2. Novi pojmovi

| Pojam | Objašnjenje |
|---|---|
| Contact bounce | Mehanički kontakti pri pritisku mogu brzo mijenjati stanje prije nego što se ustale. |
| Debounce interval | Vremenski prozor tijekom kojeg ignoriramo nove prekide nakon prihvaćenog pritiska. |
| `k_uptime_get()` | Vraća vrijeme od pokretanja sustava u milisekundama, kao `int64_t`. Ne uspavljuje niti čeka. |
| `last_press_time` | Vrijeme zadnjeg **prihvaćenog** prekida. |
| `DEBOUNCE_MS` | Odabrani vremenski prag; |
| `GPIO_INT_EDGE_TO_ACTIVE` | Okida samo prijelaz prema logički aktivnom stanju. Uz `GPIO_ACTIVE_LOW` to je fizički HIGH → LOW (pritisak). |
| ISR context | Callback ESP32 GPIO drivera izvršava se u kontekstu prekida; izbjegavati blokiranje i dugačke operacije. |

## 3. Struktura projekta i hardver

```text
04_gpio_button_interrupt_debounce/
├── boards/
│   └── esp32_devkitc.overlay
├── src/
│   └── main.c
├── CMakeLists.txt
├── prj.conf
└── README.md
```

Ne uključuj generirani `build/` direktorij u verzioniranje.

**Spojevi:**

| Komponenta | Povezivanje |
|---|---|
| LED anoda | GPIO18 → serijski otpornik 330 Ω → anoda |
| LED katoda | GND |
| Tipkalo (jedna strana) | GPIO27 |
| Tipkalo (druga strana) | GND |

Interni pull-up održava pin GPIO27 HIGH dok tipkalo nije pritisnuto. Pritiskom ga spušta na LOW; Devicetree označava takav pritisak kao `GPIO_ACTIVE_LOW`. LED ima `GPIO_ACTIVE_HIGH`.

Za četveronožno tipkalo provjeri koji su izvodi povezani u paru. LED uvijek koristi serijski otpornik.

### `boards/esp32_devkitc.overlay`

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

`led1` i `button1` povezuju Devicetree i `DT_ALIAS()` pozive u kodu. `gpio_pin_configure_dt()` kombinira zastavice iz DT-a s `GPIO_INPUT` / `GPIO_OUTPUT_INACTIVE`.

### `prj.conf`

```conf
CONFIG_GPIO=y
CONFIG_CONSOLE=y
CONFIG_PRINTK=y
```

### `CMakeLists.txt`

```cmake
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(gpio_button_interrupt_debounce)

target_sources(app PRIVATE src/main.c)
```

## 4. Objašnjenje `main.c`

Glavni program nalazi se u `src/main.c` — projekt je kompletan u ZIP-u.

### A) Zephyr GPIO i Devicetree

```c
#define LED_NODE    DT_ALIAS(led1)
#define BUTTON_NODE DT_ALIAS(button1)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);
```

`DT_ALIAS` dolazi iz overlaya; compile-time provjere s `DT_NODE_HAS_STATUS()` daju jasnu poruku ako su aliasi nepostojeći/onemogućeni. `gpio_is_ready_dt()` i povratne vrijednosti GPIO funkcija provjeravamo prije daljnjeg rada.

### B) Callback i trajanje podataka

```c
static struct gpio_callback button_cb_data;
```

Callback struktura mora ostati živa dok je registrirana. `static` na razini datoteke daje joj statički vijek trajanja i internal linkage. Callback funkcija također je `static` jer je ne pozivamo direktno iz drugih `.c` datoteka; Zephyr je poziva preko pohranjenog pokazivača.

### C) Timestamp algoritam

```c
#define DEBOUNCE_MS 50
static int64_t last_press_time = -DEBOUNCE_MS;
```

**Zašto `-DEBOUNCE_MS`, a ne `0`?**
Ako pritisnemo tipkalo npr. 10 ms nakon pokretanja sustava i `last_press_time` je `0`, uvjet `10 - 0 < 50` bio bi zadovoljen, pa bi prvi pritisak bio ignoriran. Početna vrijednost `-DEBOUNCE_MS` (odnosno `-50`) osigurava da je `10 - (-50) >= 50`, pa se prvi pritisak prihvaća.
`int64_t` omogućuje računanje vremena bez praktičnih problema s prelijevanjem 32-bitnog brojača.
```c
int64_t now = k_uptime_get();

if ((now - last_press_time) < DEBOUNCE_MS) {
    return;
}

last_press_time = now;
(void)gpio_pin_toggle_dt(&led);
```

Svi dodatni rubovi prije nego što prođe **50 ms od zadnjeg prihvaćenog ruba** ignoriraju se. Odbijeni događaji NE ažuriraju timestamp, pa se prozor ne produžuje svakim bounceom.

### D) Redoslijed inicijalizacije

1. Provjeri da su kontroleri spremni.
2. Konfiguriraj LED kao izlaz, početno ugašen.
3. Konfiguriraj BUTTON kao ulaz (`GPIO_PULL_UP` dolazi iz DT-a).
4. `gpio_init_callback(...)` priprema strukturu.
5. `gpio_add_callback(...)` registrira callback.
6. **Tek sada** `gpio_pin_interrupt_configure_dt(..., GPIO_INT_EDGE_TO_ACTIVE)` omogućuje prekid.
7. `main` se uspavljuje s `k_sleep(K_FOREVER)`; ISR radi neovisno o stanju glavne dretve.

### E) Što namjerno NE radimo u ISR-u

Ne koristimo `k_sleep()`, `printk()` ili dulju obradu. `printk` u ISR-u nije uvijek strogo zabranjen, ali može trajati dugo, ovisno o konfiguraciji konzole, i nije dobar izbor za ovaj primjer. Ispisujemo samo informacije pri pokretanju iz `main()`.

`gpio_pin_toggle_dt(&led)` je pojednostavljenje za **ugrađeni GPIO ESP32**. Driveri preko sporih sabirnica ili oni koji mogu blokirati nisu nužno pogodni za pozivanje iz ISR-a. Povratnu vrijednost toggle operacije ovdje namjerno ignoriramo; produkcijski kod mora predvidjeti obradu pogrešaka i eventualno premjestiti posao u dretvu.

## 5. Build, flash i provjera

### Zephyr IDE (preporučeno za tvoje okruženje)

Odaberi pločicu `esp32_devkitc/esp32/procpu`, napravi **Pristine Build**, zatim **Flash**. Provjeri je li u konfiguracijskom logu naveden:

```text
-- Found devicetree overlay: .../boards/esp32_devkitc.overlay
```

### `west` iz aktiviranog Zephyr okruženja (opcionalno)

Iz direktorija `zephyr_workspace`:

```bash
west build -b esp32_devkitc/esp32/procpu \
  zephyr_course/03_gpio/04_gpio_button_interrupt_debounce \
  -d zephyr_course/03_gpio/04_gpio_button_interrupt_debounce/build \
  -p always

west flash -d zephyr_course/03_gpio/04_gpio_button_interrupt_debounce/build
```

Ako terminal javlja `west: command not found`, to nije problem ovog primjera: pokrenuti build/flash kroz Zephyr IDE, gdje je okruženje već pripremljeno, ili aktivirati njegov Python/Zephyr environment.

**Očekivani startup ispis:**

```text
GPIO Button Interrupt + Timestamp Debounce
LED GPIO pin: 18
Button GPIO pin: 27
Interrupt: edge to ACTIVE (button press)
Debounce window: 50 ms
Press the button to toggle the LED
```

Kod **ne ispisuje poruku na svaki pritisak**, jer se callback izvršava u interrupt contextu. LED je indikator valjanih događaja.

### Što provjeriti na hardveru

- Pritisak tipkala obično jednom prebacuje LED stanje.
- Kratko držanje i otpuštanje ne bi trebali prebaciti LED zbog release ruba.
- Višestruki brzi rubovi tijekom bouncea trebali bi biti ignorirani.


Ovaj algoritam nije potpun mehanizam potvrde stabilnog stanja: prihvaća **prvi rub odmah**, zatim ograničava brzinu novih prihvaćenih događaja. Za robusno stanje tipkala (`PRESSED/RELEASED`) kasnije je korisno koristiti odgođenu provjeru ulaza, npr. `k_work_delayable`, ali **to nije dio ove lekcije**.

## 6. Praktični zadaci

1. Promijeni `DEBOUNCE_MS` sa `50` na `300`; opažaj razlikuje li se ponašanje.
2. Postavi `DEBOUNCE_MS` na `0` i usporedi s prethodnom vježbom bez debouncea.
3. S `DEBOUNCE_MS = 3000` pokušaj brzo pritisnuti tipkalo dva puta. Zašto drugi pritisak možda neće biti prihvaćen?
4. Zamijeni `GPIO_INT_EDGE_TO_ACTIVE` s `GPIO_INT_EDGE_BOTH` i objasni zašto će i otpuštanje moći mijenjati LED (uz postojeći vremenski filtar).
5. Objasni zašto se timestamp ažurira samo nakon prihvaćenog događaja, a ne za svaki ulazni prekid.

## 7. Pitanja za provjeru znanja

**1. Zašto `last_press_time` počinje s `-DEBOUNCE_MS`?**
Da prvi prekid, čak i odmah nakon pokretanja, ne bude odbijen zbog premalog `now - last_press_time`.

**2. Je li `k_uptime_get()` isto što i `k_sleep()`?**
Ne. `k_uptime_get()` samo očita vrijeme od pokretanja; `k_sleep()` uspavljuje dretvu i ne koristi se u ISR-u.

**3. Što `GPIO_ACTIVE_LOW` znači za tipkalo?**
Fizički LOW (GPIO27 spojen na GND) predstavlja logički aktivno stanje (pritisnuto).

**4. Zašto registriramo callback prije omogućavanja prekida?**
Da ne omogućimo prekid u trenutku kada njegova callback obrada još nije postavljena.

**5. Je li timestamp metoda isto što i provjera stabilnog stanja?**
Ne. Ovdje ograničavamo koliko često priznajemo rub; ne uzorkujemo signal nakon perioda smirivanja.

**6. Zašto nema `printk()` u callbacku?**
Da ISR ostane kratak i ne ovisi o potencijalno sporoj konzoli.

**7. Hoće li main dretva pozivati callback?**
Ne. GPIO driver poziva callback pri prekidu. Main dretva ostaje uspavana.

