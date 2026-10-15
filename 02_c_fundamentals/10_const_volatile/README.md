# 10 — Const and Volatile

## 1. Cilj

Razumjeti značenje kvalifikatora **`const`** i **`volatile`** u jeziku C, razlikovati pokazivač na nepromjenjive podatke od nepromjenjivog pokazivača te vidjeti kako se može čitati vrijednost s `volatile` kvalifikatorom. Primjer ne koristi stvarne registre ni periferni hardver.

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| **`const`** | Ograničava mijenjanje objekta kroz deklarirani izraz ili pristupni put. |
| **`volatile`** | Označava pristup objektu koji prevoditelj mora tretirati kao promjenjiv izvan uobičajenog slijeda programa; pristupi se ne smiju proizvoljno izostaviti ili spojiti. |
| **`const int *p`** | Pokazivač kroz koji nije dopušteno mijenjati vrijednost; sam pokazivač može pokazivati drugdje. |
| **`int * const p`** | Pokazivač čija se spremljena adresa ne može mijenjati nakon inicijalizacije; podatak se može mijenjati. |
| **`const volatile`** | Omogućuje čitanje vrijednosti s `volatile` semantikom, ali zabranjuje pisanje kroz taj pristupni put. |
| **Memorijski mapirani registar (MMIO)** | Hardverski registar kojem se pristupa preko memorijske adrese; u Zephyru se uobičajeno koristi odgovarajući driver ili sistemski I/O API. |

**Važno:** `const` ne znači da objekt fizički nikada ne može promijeniti vrijednost. `volatile` ne osigurava atomarnost, međusobno isključivanje ni sigurnu sinkronizaciju threadova.

## 3. Struktura projekta

```text
10_const_volatile/
├── CMakeLists.txt
├── prj.conf
├── README.md
└── src/
    └── main.c
```

Projekt koristi jedan `main.c`. `CMakeLists.txt` uključuje ga pomoću `target_sources(app PRIVATE src/main.c)`, dok `prj.conf` sadrži `CONFIG_CONSOLE=y` i `CONFIG_PRINTK=y`. Nisu potrebni `app.overlay`, stvarni GPIO pinovi ni dodatne biblioteke.

## 4. Primjer — `const` i `volatile`

**Konstantna vrijednost i pokazivač na `const` podatak:**

```c
const int limit = 50;
int value = 10;
int other_value = 20;

const int *ptr_to_const = &value;
printk("Pointer to const: %d\n", *ptr_to_const);

ptr_to_const = &other_value;
printk("Pointer reassigned: %d\n", *ptr_to_const);
```

Kod `const int *ptr_to_const` nije dopušteno pisati `*ptr_to_const = 30;`, ali pokazivač smijemo preusmjeriti na `other_value`. Vrijednost `limit` deklarirana je s `const`, pa joj se u programu ne može dodijeliti druga vrijednost.

**Nepromjenjiv pokazivač na promjenjiv podatak:**

```c
int *const const_ptr = &value;
*const_ptr = 15;
printk("Value after const pointer write: %d\n", value);
```

Kod `int *const const_ptr` ne smijemo ponovno dodijeliti adresu samom pokazivaču (`const_ptr = &other_value;`), ali možemo mijenjati vrijednost na koju pokazuje.

**Simulirani status i `volatile`:**

```c
volatile uint32_t simulated_status = 0U;
const volatile uint32_t *status_view = &simulated_status;

printk("Simulated status before: %u\n", (unsigned int)*status_view);
simulated_status = 1U;
printk("Simulated status after: %u\n", (unsigned int)*status_view);
```

`status_view` omogućuje čitanje, ali ne i pisanje preko tog pokazivača. Kvalifikator `volatile` nalaže prevoditelju da poštuje pristupe označenom objektu. **Ovdje se vrijednost mijenja običnim C kodom samo radi demonstracije** — nije riječ o stvarnom hardverskom registru, prekidu ni dijeljenju podataka između threadova.

Kod pristupa pravim memorijski mapiranim registrima u Zephyru se koriste odgovarajući driveri ili API-ji poput `sys_read32()` i `sys_write32()`. Sama uporaba `volatile` nije zamjena za te API-je niti za sinkronizaciju.

Cijeli primjer nalazi se u `src/main.c`.

## 5. Izgradnja i provjera

Naredbe pokreni u **PowerShellu**, iz direktorija `zephyr_workspace`, uz aktivno Zephyrovo razvojno okruženje.

**Čista izgradnja:**

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/02_c_fundamentals/10_const_volatile -d zephyr_course/02_c_fundamentals/10_const_volatile/build -p always
```

**Učitavanje na ESP32** (prethodno zatvori serijski monitor ako koristi isti COM port):

```powershell
west flash -d zephyr_course/02_c_fundamentals/10_const_volatile/build
```

Očekivani **aplikacijski** ispis (moguće su i Zephyrove početne poruke):

```text
Const and Volatile
Limit: 50
Pointer to const: 10
Pointer reassigned: 20
Value after const pointer write: 15
Simulated status before: 0
Simulated status after: 1
```

Projekt je pripremljen za ESP32 DevKitC V4; stvarni `west build` i izvođenje na pločici nisu ovdje provjereni.

## 6. Zadaci

**A — Promjena konstantne vrijednosti.** Pokušaj nakon deklaracije dodati `limit = 60;`. Pokreni build, prouči pogrešku prevoditelja i ukloni nedopuštenu dodjelu.

**B — Pokazivač na `const`.** Pokušaj dodati `*ptr_to_const = 30;`. Objasni zašto nije dopušteno mijenjati podatak kroz taj pokazivač, iako `value` i `other_value` nisu `const` varijable. Zatim vrati kod.

**C — Konstantan pokazivač.** Pokušaj dodati `const_ptr = &other_value;`. Prouči pogrešku i usporedi je sa zadatkom B. Vrati kod.

**D — Promjena simuliranog statusa.** Umjesto `simulated_status = 1U;` postavi `simulated_status = 2U;` i predvidi novi ispis. Objasni zašto se ovim primjerom ne demonstrira stvarni hardverski događaj.

**E — Dva kvalifikatora.** Prouči deklaraciju `const volatile uint32_t *status_view`. Objasni zašto se vrijednost smije čitati kroz `status_view`, a ne smije mijenjati izrazom `*status_view = 3U;`.

Nakon zadataka vrati početni kod kako bi očekivani ispis odgovarao primjeru.

## 7. Pitanja

1. Koje ograničenje uvodi `const` kod obične varijable?
2. Po čemu se razlikuju `const int *p` i `int *const p`?
3. Može li se `ptr_to_const` preusmjeriti na drugu varijablu? Zašto?
4. Zašto izraz `*const_ptr = 15;` može promijeniti `value`?
5. Što kvalifikator `volatile` govori prevoditelju?
6. Zašto `volatile` nije zamjena za mutex, atomske operacije ili druge mehanizme sinkronizacije?
7. Što znači kombinacija `const volatile`?
8. Zašto u ovom primjeru ne koristimo stvarni memorijski mapirani registar?

## 8. Zaključak

`const` ograničava izmjene kroz određeni pristupni put, a `volatile` utječe na način na koji prevoditelj obrađuje pristupe objektu. Kod pokazivača je važno razlikovati nepromjenjiv **podatak** od nepromjenjive **adrese spremljene u pokazivaču**.

Primjer simulira status radi učenja sintakse. U stvarnim Zephyr aplikacijama hardverskim registrima treba pristupati preko odgovarajućih API-ja, a za sinkronizaciju threadova koristiti namjenske mehanizme.
