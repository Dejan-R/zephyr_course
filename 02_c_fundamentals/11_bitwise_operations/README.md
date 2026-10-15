# 11 — Bitwise Operations and Preprocessor Macros

## 1. Cilj

Razumjeti **bitovne operatore** u jeziku C i njihovu primjenu za postavljanje, provjeru, promjenu i brisanje pojedinih bitova. Naučiti razliku između **običnih i funkcijskih makronaredbi** definiranih pomoću `#define` te koristiti Zephyrov makro `BIT()` za izradu bitnih maski.

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| **`&` (AND)** | Bit rezultata je `1` samo ako je taj bit postavljen u oba operanda; koristi se za provjeru i maskiranje. |
| **`\|` (OR)** | Postavlja bit ako je postavljen u barem jednom operandu. |
| **`^` (XOR)** | Postavlja bit kada se odgovarajući bitovi razlikuju; koristi se za promjenu (*toggle*) bita. |
| **`~` (NOT)** | Obrće sve bitove operanda; s operatorom `&` omogućuje brisanje odabranih bitova. |
| **`<<` / `>>`** | Pomaci bitova ulijevo i udesno. |
| **Bitna maska** | Vrijednost u kojoj su odabrani bitovi postavljeni radi rada s pojedinim zastavicama. |
| **`#define`** | Definira preprocesorsku makronaredbu, koja se proširuje prije prevođenja C koda. |
| **Obični makro** | Simbolički naziv za vrijednost ili izraz, npr. `STATUS_READY`. |
| **Funkcijski makro** | Makronaredba s parametrima, npr. `HAS_FLAG(value, flag)`; nije C funkcija. |
| **`BIT(n)`** | Zephyrov makro koji stvara masku s postavljenim bitom na poziciji `n` (brojenje od nule). |

**Važno:** `&` i `|` su bitovni operatori; `&&` i `||` su logički operatori. Njihova namjena nije ista.

## 3. Struktura projekta

```text
11_bitwise_operations/
├── CMakeLists.txt
├── prj.conf
├── README.md
└── src/
    └── main.c
```

`CMakeLists.txt` uključuje `src/main.c` naredbom `target_sources(app PRIVATE src/main.c)`. U `prj.conf` uključeni su `CONFIG_CONSOLE=y` i `CONFIG_PRINTK=y`. Nisu potrebni `app.overlay`, dodatne datoteke ili fizički povezani uređaji.

## 4. Bitovne operacije i makronaredbe u programu

U `src/main.c` uključuje se `<zephyr/sys/util.h>`, koji omogućuje uporabu Zephyrova makroa `BIT()`:

```c
#define STATUS_READY BIT(0)
#define STATUS_ERROR BIT(1)
#define STATUS_BUSY  BIT(2)

#define HAS_FLAG(value, flag) (((value) & (flag)) != 0U)
```

- `STATUS_READY`, `STATUS_ERROR` i `STATUS_BUSY` **obični su makroi** koji označavaju bitove 0, 1 i 2.
- `HAS_FLAG(value, flag)` **funkcijski je makro**: provjerava je li bit određen maskom postavljen. Poziva se slično funkciji, ali preprocesor umjesto poziva funkcije proširuje njegov izraz.
- Zagrade u definiciji funkcijskog makroa važne su kako bi se izbjegle pogreške zbog prioriteta operatora.

Program koristi varijablu `uint32_t status = 0U;` kao **simulirani statusni registar**. Redom izvodi sljedeće operacije:

| Operacija u `main.c` | Rezultat | Objašnjenje |
|---|---|---|
| `status \|= STATUS_READY \| STATUS_ERROR;` | `0x03` | Postavlja bitove 0 i 1. |
| `HAS_FLAG(status, STATUS_READY)` | `yes` | Provjerava bit 0 operatorom `&`. |
| `status ^= STATUS_ERROR;` | `0x01` | Mijenja bit 1 iz `1` u `0`. |
| `status \|= STATUS_BUSY;` | `0x05` | Postavlja bit 2. |
| `status &= ~STATUS_BUSY;` | `0x01` | Briše bit 2, a ostale ostavlja netaknutima. |
| `1U << 3` | `8` | Pomak ulijevo. |
| `8U >> 2` | `2` | Pomak udesno. |

Vrijednosti su zapisane heksadekadski jer je tako jednostavnije prepoznati bitne maske. Primjerice, `0x05` odgovara binarnom zapisu `0000 0101` — postavljeni su bitovi 0 i 2.

**Napomena o Zephyrovim makroima:** `BIT(n)` stvara masku **jednog** bita, dok postojeći Zephyrov `BIT_MASK(n)` stvara masku **nižih `n` bitova**. Zbog toga ne definiramo vlastiti makro pod nazivom `BIT_MASK`. Zephyr već nudi gotove pomoćne makroe, ali vlastiti `HAS_FLAG()` ovdje služi za učenje funkcijskih makronaredbi.

Za makronaredbe vrijede ista upozorenja kao i u običnom C preprocesoru: ne provjeravaju tipove kao funkcije, a neki funkcijski makroi mogu svoje argumente evaluirati više puta. Stoga izbjegavaj izraze s nuspojavama poput `i++` kao argumente složenih makroa. U stvarnom programu, kada je prikladno, prednost može imati funkcija ili `static inline` funkcija.

Uvjetno prevođenje (`#if defined(...)`) već je obrađeno u `01_foundations/03_kconfig_basics`, a include guard (`#ifndef` / `#define` / `#endif`) u `02_c_fundamentals/01_functions`.

## 5. Izgradnja i provjera

Iz direktorija `zephyr_workspace`, u PowerShellu s aktivnim Zephyrovim okruženjem:

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/02_c_fundamentals/11_bitwise_operations -d zephyr_course/02_c_fundamentals/11_bitwise_operations/build -p always

west flash -d zephyr_course/02_c_fundamentals/11_bitwise_operations/build
```

Očekivani **aplikacijski** ispis (moguće su i Zephyrove početne poruke):

```text
Bitwise Operations and Macros
Initial status: 0x00
Set READY and ERROR: 0x03
READY set: yes
Toggle ERROR: 0x01
Set BUSY: 0x05
Clear BUSY: 0x01
Left shift (1 << 3): 8
Right shift (8 >> 2): 2
```

Za detaljnije objašnjenje `west build` i `west flash` pogledaj `01_foundations/02_build_system`.

## 6. Zadaci

**A — Provjera zastavice.** Dodaj ispis koji provjerava je li `STATUS_ERROR` postavljen nakon izvršavanja `status ^= STATUS_ERROR;`. Objasni rezultat.

**B — Ponovljeni XOR.** Izvrši `status ^= STATUS_ERROR;` još jednom i provjeri je li bit pogreške ponovno postavljen. Objasni zašto.

**C — Nova zastavica.** Definiraj `STATUS_ALARM` pomoću `BIT(3)` i dodaj operacije postavljanja, provjere i brisanja tog bita.

**D — Bitovni i logički operatori.** Objasni zašto `status & STATUS_READY` i `status && STATUS_READY` nisu isto, primjerice kada `status` sadrži neki drugi postavljeni bit.

**E — Funkcijski makro i zagrade.** Definiraj `#define DOUBLE(value) ((value) * 2U)` i ispiši `DOUBLE(1U + 2U)`. Zatim privremeno ukloni zagrade oko `value` u definiciji te prouči razliku. Vrati ispravnu definiciju.

**F — Zephyrovi makroi.** U zaglavlju `<zephyr/sys/util_macro.h>` pronađi definicije `BIT(n)` i `BIT_MASK(n)` te objasni zašto nisu zamjenjivi.

Nakon zadataka vrati početni kod kako bi očekivani ispis odgovarao primjeru.

## 7. Pitanja

1. Koja je razlika između operatora `&` i `&&`?
2. Kako se operatorom `|` postavlja određeni bit bez mijenjanja ostalih?
3. Zašto je `status &= ~STATUS_BUSY;` primjer brisanja bita?
4. Čemu služi operator `^` pri radu sa zastavicama?
5. Koja je razlika između `<<` i `>>`?
6. Što označava izraz `BIT(2)` i koja je njegova vrijednost?
7. Koja je razlika između običnog i funkcijskog makroa definiranog pomoću `#define`?
8. Zašto se u funkcijskim makronaredbama koriste zagrade?
9. Zašto funkcijski makro nije isto što i C funkcija?
10. U kojim smo ranijim vježbama već susreli `#include`, include guard i `#if defined(...)`?

## 8. Zaključak

Bitovni operatori omogućuju rad s pojedinim bitovima unutar cjelobrojne vrijednosti: postavljanje, provjeru, promjenu i brisanje zastavica. Preprocesorske makronaredbe mogu dati razumljiva imena maskama i omogućiti kratke izraze s parametrima. Zephyr pritom nudi vlastite pomoćne makroe poput `BIT()`.