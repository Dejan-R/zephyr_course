# 05 — Driver API Pattern

## 1. Cilj

Razumjeti kako Zephyr odvaja **javni API podsustava** od **implementacije drivera**. Primjer koristi postojeći GPIO kontroler na ESP32 DevKitC V4 i pregledava njegovu tablicu funkcija.

**Program ne konfigurira GPIO pinove i ne zahtijeva dodatni hardver.**

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| **Subsystem API** | Zajedničko sučelje koje aplikacija koristi neovisno o proizvođaču kontrolera, primjerice GPIO API. |
| **Driver backend** | Implementacija operacija za konkretan hardverski kontroler. |
| **Function pointer** | Pokazivač na funkciju koji omogućuje pozivanje odgovarajuće operacije. |
| **`struct gpio_driver_api`** | Zephyrova tablica funkcijskih pokazivača za GPIO operacije. |
| **`device->api`** | Pokazivač na API tablicu povezanu s instancom `struct device`. |
| **Dispatch** | Usmjeravanje poziva javnog API-ja prema odgovarajućoj implementaciji drivera. |

`struct device`, `DEVICE_DT_GET()` i `device_is_ready()` već su obrađeni u `04_devicetree_basics`;

## 3. Struktura projekta

```text
05_driver_api_pattern/
├── CMakeLists.txt
├── prj.conf
├── README.md
└── src/
    └── main.c
```

Nije potreban `app.overlay` jer koristimo GPIO kontroler definiran u osnovnom Devicetree opisu ESP32 pločice.

U `prj.conf` uključene su opcije:

```ini
CONFIG_CONSOLE=y
CONFIG_PRINTK=y
CONFIG_GPIO=y
```

## 4. Driver API obrazac

### Dohvat GPIO uređaja

```c
#define GPIO_NODE DT_NODELABEL(gpio0)

static const struct device *const gpio_dev =
    DEVICE_DT_GET(GPIO_NODE);
```

Na temelju DeviceTree opisa dohvaćamo postojeću instancu GPIO kontrolera.

### Pristup tablici operacija

```c
const struct gpio_driver_api *api =
    (const struct gpio_driver_api *)gpio_dev->api;
```

Pokazivač `gpio_dev->api` povezuje instancu uređaja s tablicom funkcija koju osigurava konkretni GPIO driver.

| Član API tablice | Namjena |
|---|---|
| `api->pin_configure` | Implementacija konfiguriranja GPIO pina. |
| `api->port_get_raw` | Implementacija čitanja stanja GPIO porta. |

Program samo provjerava jesu li pokazivači na te operacije različiti od `NULL`.

**Ne pozivamo funkcije drivera, ne konfiguriramo pinove i ne čitamo električna stanja ulaza.**

### Kako javni API dolazi do drivera?

U stvarnoj aplikaciji koristi se javni Zephyrov GPIO API, primjerice `gpio_pin_configure_dt()` ili `gpio_port_get_raw()`, a ne izravni pozivi preko `api->...`.

Pojednostavljeni slijed:

```text
Application
    |
    v
Zephyr GPIO API
    |
    v
gpio_driver_api
    |
    v
ESP32 GPIO driver implementation
```

Zahvaljujući tom obrascu, aplikacija može koristiti iste javne GPIO funkcije na različitim mikroupravljačima, dok Zephyr pozive usmjerava prema odgovarajućim driverima.

**Izravni pristup `device->api` u ovoj je vježbi namijenjen isključivo proučavanju Zephyrove arhitekture.**

## 5. Izgradnja i provjera

U PowerShellu, iz direktorija `zephyr_workspace`, s aktivnim Zephyrovim okruženjem:

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/01_foundations/05_driver_api_pattern -d zephyr_course/01_foundations/05_driver_api_pattern/build -p always
west flash -d zephyr_course/01_foundations/05_driver_api_pattern/build
```

Aplikacijski ispis potvrđen na ESP32 DevKitC V4:

```text
Driver API Pattern
Device: gpio@3ff44000
Device ready: yes
Driver API available: yes
Pin configure operation: available
Port read operation: available
```

Primjer je uspješno izgrađen i pokrenut na ESP32 DevKitC V4. Naziv GPIO uređaja može se razlikovati na drugim pločicama.

Vrijednost `available` znači da je funkcijski pokazivač prisutan u API tablici. **Ne znači da je pripadajuća operacija izvršena.**

## 6. Zadaci

**A — Istraži API.** U Zephyrovoj datoteci `include/zephyr/drivers/gpio.h` pronađi `struct gpio_driver_api` i identificiraj još dvije operacije osim `pin_configure` i `port_get_raw`.

**B — Proširi ispis.** U `main.c` dodaj provjeru pokazivača `api->port_toggle_bits` i ispiši je li operacija dostupna. Nemoj pozivati samu operaciju.

**C — Pronađi implementaciju.** U Zephyrovu izvornom kodu pronađi GPIO driver za ESP32. Identificiraj gdje se funkcije konkretnog drivera pridružuju članovima GPIO API tablice.

**D — Analiziraj arhitekturu.** Objasni zašto aplikacija treba pozivati javnu funkciju `gpio_pin_configure_dt()`, a ne izravno `api->pin_configure`.

**E — Prati poziv kroz driver API.** U datoteci `include/zephyr/drivers/gpio.h` pronađi javnu funkciju `gpio_port_get_raw()` i njezinu implementacijsku funkciju `z_impl_gpio_port_get_raw()`.

Prouči kako funkcija pomoću `DEVICE_API_GET(gpio, port)` pristupa tablici operacija uređaja i poziva `api->port_get_raw(port, value)`.

Objasni razliku između javne GPIO funkcije i funkcijskog pokazivača u driver API tablici. Zašto aplikacija treba koristiti `gpio_port_get_raw()`, a ne izravno `api->port_get_raw()`?

## 7. Pitanja

1. Koja je razlika između javnog GPIO API-ja i implementacije drivera?
2. Čemu služi `gpio_dev->api`?
3. Što je funkcijski pokazivač?
4. Što predstavlja `struct gpio_driver_api`?
5. Zašto različite pločice mogu koristiti isti javni GPIO API?
6. Zašto u ovoj vježbi nije potrebno spajati LED diodu?
7. Kako poziv javne funkcije `gpio_port_get_raw()` dolazi do implementacije konkretnog GPIO drivera?

## 8. Zaključak

Zephyrov javni API odvaja aplikacijski kod od implementacije pojedinog hardverskog drivera. `struct device` povezuje instancu uređaja s tablicom operacija, a konkretni driver osigurava implementacije tih operacija.

Pristup `device->api` u ovoj je vježbi **edukacijski prikaz unutarnje arhitekture**. Za stvarno korištenje GPIO kontrolera aplikacija treba pozivati javni Zephyrov GPIO API.