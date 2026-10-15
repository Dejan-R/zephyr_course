# 04 — Devicetree Basics

## 1. Cilj

Razumjeti strukturu Zephyrova Devicetreea, ulogu datoteke `app.overlay`, pristup generiranim podacima iz C programa i povezivanje DeviceTree čvorova sa Zephyrovim modelom uređaja.

Primjer prikazuje i pronalaženje već inicijaliziranog uređaja prema nazivu tijekom rada. **Nije potrebno spojiti LED diodu**: program ne konfigurira GPIO pin niti mijenja njegovo stanje.

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| **Devicetree** | Hijerarhijski opis hardvera koji Zephyr obrađuje tijekom izgradnje. |
| **Root node** `/` | Korijenski čvor stabla. |
| **Node** | Čvor, primjerice `led_0`. |
| **Parent / child node** | Odnos roditeljskog čvora `course_leds` i podređenog čvora `led_0`. |
| **Property** | Svojstvo čvora, primjerice `gpios` ili `label`. |
| **Node name / node label** | Ime `led_0` i zasebna oznaka `course_led:`. |
| **Phandle** | Referenca na drugi čvor, primjerice `&gpio0` ili `&course_led`. |
| **Alias** | Alternativni naziv u `/aliases`, primjerice `course-led`. |
| **`compatible`** | Identifikator za pronalazak Devicetree bindinga; sam po sebi ne stvara driver. |
| **Overlay** | Datoteka koja dopunjuje ili mijenja opis pločice. |
| **`struct device`** | Zephyrova instanca uređaja, povezana s odgovarajućim driverom. |
| **`struct gpio_dt_spec`** | Struktura s GPIO kontrolerom (`port`), pinom (`pin`) i oznakama (`dt_flags`). |
| **Runtime lookup** | Pronalaženje već registriranog uređaja prema nazivu tijekom izvođenja. |

**Važno:** `app.overlay` nije datoteka koju firmware čita tijekom rada. Zephyr iz opisa pločice i overlaya tijekom izgradnje generira C makroe.

## 3. Struktura projekta

```text
04_devicetree_basics/
├── CMakeLists.txt
├── prj.conf
├── app.overlay
├── README.md
└── src/
    └── main.c
```

### `app.overlay`

```dts
#include <zephyr/dt-bindings/gpio/gpio.h>

/ {
    course_leds {
        compatible = "gpio-leds";

        course_led: led_0 {
            gpios = <&gpio0 18 GPIO_ACTIVE_HIGH>;
            label = "Course LED";
        };
    };

    aliases {
        course-led = &course_led;
    };
};
```

- `/` je korijenski čvor; `course_leds` je roditeljski, a `led_0` podređeni čvor.
- `compatible = "gpio-leds"` koristi Zephyrov binding za opis LED dioda na GPIO pinovima.
- `course_led:` je **node label**, `led_0` **node name**, a `label = "Course LED"` tekstno **svojstvo**.
- `gpios = <&gpio0 18 GPIO_ACTIVE_HIGH>` opisuje kontroler, broj pina i oznake. `&gpio0` je phandle referenca na GPIO kontroler.
- `#include <zephyr/dt-bindings/gpio/gpio.h>` uključuje konstante poput `GPIO_ACTIVE_HIGH`, a ne GPIO driver.
- `course-led = &course_led` definira alias; u C makrou `DT_ALIAS(course_led)` crtica se piše podvlakom.

GPIO18 služi samo za demonstraciju. **Nije potrebno spojiti LED diodu.**

### Zašto `app.overlay`, a ne `boards/<board>.overlay`?

## 4. Pristup podacima i modelu uređaja iz C programa

### Čitanje DeviceTree podataka

```c
#define LED_NODE DT_ALIAS(course_led)

#if !DT_NODE_EXISTS(LED_NODE)
#error "Devicetree alias 'course-led' is missing"
#endif
```

`DT_ALIAS()` dohvaća čvor prema aliasu, a `DT_NODE_EXISTS()` provjerava njegovo postojanje tijekom prevođenja. Makroi `DT_PROP()`, `DT_GPIO_PIN()` i `DT_GPIO_FLAGS()` čitaju generirana svojstva bez pristupanja hardveru.

### Dohvat GPIO kontrolera

```c
#define GPIO_NODE DT_GPIO_CTLR(LED_NODE, gpios)

#if !DT_NODE_HAS_STATUS(GPIO_NODE, okay)
#error "GPIO controller is disabled in Devicetree"
#endif

static const struct gpio_dt_spec led =
    GPIO_DT_SPEC_GET(LED_NODE, gpios);

static const struct device *const gpio_dev =
    DEVICE_DT_GET(GPIO_NODE);
```

| Element | Uloga |
|---|---|
| `DT_GPIO_CTLR()` | Određuje čvor GPIO kontrolera iz svojstva `gpios`. |
| `DT_NODE_HAS_STATUS()` | Tijekom prevođenja provjerava status DeviceTree čvora (`okay`). |
| `GPIO_DT_SPEC_GET()` | Stvara `gpio_dt_spec` s članovima `port`, `pin` i `dt_flags`. |
| `DEVICE_DT_GET()` | Dohvaća pokazivač na Zephyrovu instancu uređaja. |
| `device_is_ready()` | Provjerava je li uređaj inicijaliziran i spreman za korištenje. |

`led.port` i `gpio_dev` pokazuju na isti GPIO kontroler. Status `okay` znači da je čvor omogućen u DeviceTreeu, **ali ne jamči** da je driver uspješno inicijaliziran; zato dodatno provjeravamo `device_is_ready()`.

### Pronalaženje uređaja tijekom izvođenja

```c
const struct device *runtime_dev = device_get_binding(gpio_dev->name);
```

`device_get_binding()` traži već registrirani uređaj prema nazivu. Za razliku od `DEVICE_DT_GET()`, poziva se tijekom rada programa i može vratiti `NULL` ako uređaj nije pronađen ili nije uspješno inicijaliziran.

```c
printk("Runtime lookup: %s\n",
       runtime_dev != NULL ? "found" : "not found");
printk("Runtime same instance: %s\n",
       runtime_dev == gpio_dev ? "yes" : "no");
```

Time uspoređujemo dohvat instance preko DeviceTree čvora i dohvat prema njezinu nazivu. Nijedna od ovih operacija ne konfigurira GPIO pin; upravljanje pinovima obrađuje se u `03_gpio`.

## 5. Izgradnja i provjera

U PowerShellu, iz direktorija `zephyr_workspace`, s aktivnim Zephyrovim okruženjem:

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/01_foundations/04_devicetree_basics -d zephyr_course/01_foundations/04_devicetree_basics/build -p always
west flash -d zephyr_course/01_foundations/04_devicetree_basics/build
```

Očekivani **aplikacijski** ispis (naziv kontrolera ovisi o pločici):

```text
Devicetree Basics
LED label: Course LED
GPIO controller: <controller name>
LED GPIO pin: 18
LED flags: 0
GPIO controller ready: yes
Same device instance: yes
Runtime lookup: found
Runtime same instance: yes
```

`<controller name>` označava stvarnu vrijednost `gpio_dev->name`, **nije doslovan ispis**. Vrijednost `0` za flags odgovara `GPIO_ACTIVE_HIGH`. Moguće su dodatne Zephyrove početne poruke.

U `build/zephyr/zephyr.dts` pronađi `course_leds`, `led_0`, `course-led` i GPIO kontroler. To je **spojeni Devicetree**, a ne samo izvorni `app.overlay`.

## 6. Zadaci

**A — Promjena svojstva.** U `app.overlay` promijeni `label = "Course LED";` u `label = "Status LED";`. Ponovno izgradi aplikaciju i provjeri promjenu ispisa bez izmjene `main.c`.

**B — Promjena pina.** Promijeni `18` u `19` i provjeri `LED GPIO pin: 19`. Nije potrebno spajati LED diodu.

**C — Generirani opis.** U `build/zephyr/zephyr.dts` pronađi čvor, svojstva i alias te razlikuj `course_led`, `led_0`, `label` i `course-led`.

**D — Pogrešan alias.** Preimenuj `course-led` samo u overlayu, pokreni build, protumači pogrešku i vrati ispravan naziv.

**E — Aktivna razina.** Promijeni `GPIO_ACTIVE_HIGH` u `GPIO_ACTIVE_LOW` i usporedi `LED flags`.

**F — Struktura.** Privremeno ispiši `led.pin` i `led.dt_flags` te ih usporedi s rezultatima `DT_GPIO_PIN()` i `DT_GPIO_FLAGS()`.

**G — Model uređaja.** Objasni zašto je `led.port == gpio_dev` i zašto status `okay` ne zamjenjuje `device_is_ready()`.

**H — Runtime lookup.** Privremeno pozovi `device_get_binding("invalid-device")` i provjeri vraća li `NULL`. Potom vrati `gpio_dev->name` i usporedi pokazivače.

Nakon promjena overlaya koristi čisti build (`-p always`) kako bi se konfiguracija ponovno generirala.

## 7. Pitanja

1. Koja je razlika između root, parent i child čvora?
2. Koja je razlika između node name, node label i `label` svojstva?
3. Što predstavljaju `&gpio0` i `&course_led`?
4. Čemu služi `compatible = "gpio-leds"`?
5. Zašto overlay uključuje `zephyr/dt-bindings/gpio/gpio.h`?
6. Koja je razlika između `app.overlay` i overlaya u `boards/`?
7. Što rade `DT_ALIAS()`, `DT_PROP()` i `DT_GPIO_PIN()`?
8. Što sadrži `build/zephyr/zephyr.dts`?
9. Što sadrži `gpio_dt_spec` i čemu služi `GPIO_DT_SPEC_GET()`?
10. Koja je razlika između `DT_NODE_HAS_STATUS()` i `device_is_ready()`?
11. Koja je razlika između `DEVICE_DT_GET()` i `device_get_binding()`?
12. Zašto nije potrebno spojiti LED diodu?

## 8. Zaključak

Devicetree odvaja opis hardvera od aplikacijske logike. Iz opisa se generiraju makroi, a `GPIO_DT_SPEC_GET()` i `DEVICE_DT_GET()` omogućuju pristup instanci GPIO kontrolera. `device_is_ready()` provjerava spremnost uređaja, dok `device_get_binding()` demonstrira njegovo pronalaženje prema nazivu tijekom rada. Samo upravljanje GPIO pinom obrađuje se u kasnijim vježbama.
