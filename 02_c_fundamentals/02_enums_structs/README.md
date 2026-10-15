# 02 — Enums and Structs

## 1. Cilj

Razumjeti kako `enum` definira skup imenovanih vrijednosti, kako `struct` objedinjuje povezane podatke te kako se članovima strukture pristupa operatorom `.`. Na jednostavnom primjeru simuliranog očitanja senzora povezati `enum`, `struct` i naredbu `switch` bez korištenja stvarnog hardvera.

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| **`enum`** | Korisnički definirani cjelobrojni tip s imenovanim vrijednostima (enumeratorima). |
| **Enumerator** | Imenovana konstanta, primjerice `SENSOR_OK`. Bez eksplicitnih vrijednosti počinje od `0`, a sljedeći enumeratori rastu za `1`. |
| **`struct`** | Korisnički definirani tip koji grupira podatke različitih tipova u jednu cjelinu. |
| **Član strukture** | Pojedini podatak unutar strukture, primjerice `temperature_c`. |
| **Operator `.`** | Pristupa članu konkretne varijable strukture. |
| **Designated initializer** | Inicijalizira odabrane članove njihovim nazivima, primjerice `.status = SENSOR_OK`. |
| **`switch` / `case`** | Odabire dio programa prema vrijednosti izraza; ovdje prema statusu senzora. |

**Važno:** `enum` olakšava čitljivost koda, a `struct` organizaciju podataka. U ovoj vježbi vrijednosti senzora su zadane u programu; ne koristi se pravi senzor.

## 3. Struktura projekta

```text
02_enums_structs/
├── CMakeLists.txt
├── prj.conf
├── README.md
└── src/
    └── main.c
```

**`src/main.c` — definicije tipova:**

```c
/* Enum: named values describing the sensor status. */
enum sensor_status {
    SENSOR_OK,
    SENSOR_WARNING,
    SENSOR_ERROR
};

/* Struct: related sensor data stored in one object. */
struct sensor_reading {
    int temperature_c;
    int humidity_percent;
    enum sensor_status status;
};
```

`enum sensor_status` određuje dopuštene nazive statusa u primjeru, a `struct sensor_reading` grupira temperaturu, vlagu i status. `enum` u C-u nije stroga runtime provjera valjanosti vrijednosti: i dalje treba voditi računa o mogućim neočekivanim vrijednostima.

**`CMakeLists.txt`** dodaje jedinu izvornu datoteku naredbom `target_sources(app PRIVATE src/main.c)`. **`prj.conf`** uključuje `CONFIG_CONSOLE=y` i `CONFIG_PRINTK=y`. Dodatne datoteke `Kconfig` i `app.overlay` ovdje nisu potrebne.

## 4. Korištenje `enum` i `struct` u programu

U `main()` stvaramo jednu varijablu strukture i inicijaliziramo njezine članove:

```c
struct sensor_reading reading = {
    .temperature_c = 26,
    .humidity_percent = 58,
    .status = SENSOR_OK
};
```

Vrijednosti čitamo operatorom `.`:

```c
printk("Temperature: %d C\n", reading.temperature_c);
printk("Humidity: %d %%\n", reading.humidity_percent);
```

Za status koristimo `switch (reading.status)`. Grane `case SENSOR_OK`, `case SENSOR_WARNING` i `case SENSOR_ERROR` ispisuju odgovarajuću poruku, a `default` obrađuje neočekivanu vrijednost. `break` završava odabranu granu.

**Napomena:** `%%` u formatu `printk()` ispisuje znak `%`. U ovom primjeru nema pokazivača ni pristupa članovima preko operatora `->`; to se uči u kasnijim vježbama.

## 5. Izgradnja i provjera

Naredbe se izvršavaju u **PowerShellu**, iz direktorija `zephyr_workspace`, uz aktivno Zephyrovo okruženje.

**Izgradnja:**

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/02_c_fundamentals/02_enums_structs -d zephyr_course/02_c_fundamentals/02_enums_structs/build -p always
```

**Učitavanje na ESP32** (zatvori serijski monitor ako zauzima COM port):

```powershell
west flash -d zephyr_course/02_c_fundamentals/02_enums_structs/build
```

Očekivani **aplikacijski** ispis (moguće su i Zephyrove početne poruke):

```text
Enums and Structs
Temperature: 26 C
Humidity: 58 %
Status: OK
```

Projekt je pripremljen za navedenu pločicu, ali stvarni `west build` i `west flash` nisu provedeni u ovom okruženju.

## 6. Zadaci

**A — Promjena članova strukture.** Promijeni `.temperature_c` na `30`, a `.humidity_percent` na `45`. Ponovno izgradi aplikaciju i provjeri ispis.

**B — Promjena statusa.** Postavi `.status = SENSOR_WARNING`, a zatim `.status = SENSOR_ERROR`. Provjeri odgovarajuće grane naredbe `switch`.

**C — Novi član strukture.** Dodaj član `int sensor_id;` u `struct sensor_reading`, inicijaliziraj ga na `1` i ispiši `Sensor ID: 1`.

**D — Novi enum status.** Dodaj `SENSOR_OFFLINE` u `enum sensor_status` i pripadajuću granu u `switch` koja ispisuje `Status: OFFLINE`. Isprobaj novi status.

**E — Imenovane vrijednosti.** Privremeno dodijeli eksplicitne vrijednosti enumeratorima (primjerice `SENSOR_OK = 0`, `SENSOR_WARNING = 10`, `SENSOR_ERROR = 20`). Dodaj ispis numeričke vrijednosti statusa pomoću `(int)reading.status` i objasni rezultat. Zatim vrati početnu verziju.

Nakon zadataka vrati početne vrijednosti primjera.

## 7. Pitanja

1. Koja je razlika između `enum` i `struct`?
2. Koje su početne numeričke vrijednosti `SENSOR_OK`, `SENSOR_WARNING` i `SENSOR_ERROR` ako nisu ručno zadane?
3. Što predstavlja varijabla `reading`, a što njezini članovi?
4. Čemu služi operator `.` u izrazu `reading.temperature_c`?
5. Zašto je zapis `.status = SENSOR_OK` pregledniji od inicijalizacije bez naziva članova?
6. Što radi `switch`, a čemu služe `case`, `break` i `default`?
7. Može li struktura sadržavati člana tipa `enum`?
8. Zašto u ovom primjeru nije potreban stvarni senzor ni DeviceTree overlay?

## 8. Zaključak

`enum` daje razumljiva imena skupovima stanja, a `struct` povezuje podatke koji pripadaju istoj cjelini. Zajedno omogućuju čitljiviji embedded C kod, primjerice pri predstavljanju senzorskih očitanja, statusa uređaja i poruka između dijelova programa. Ovdje smo ih koristili bez pokazivača; njihova upotreba s adresama i pokazivačima slijedi u kasnijim lekcijama.
