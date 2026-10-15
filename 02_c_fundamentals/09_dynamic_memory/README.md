# 09 — Dynamic Memory

## 1. Cilj

Razumjeti dinamičku alokaciju memorije u C-u pomoću `malloc()`, `calloc()`, `realloc()` i `free()`, provjeru neuspjele alokacije te pravilno oslobađanje memorije. Primijeniti standardne C funkcije u samostalnoj Zephyr aplikaciji za ESP32.

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| **Dinamička memorija (heap)** | Memorija koju program tijekom izvođenja zatraži i poslije oslobodi. |
| **`malloc()`** | Alocira zadani broj bajtova; sadržaj nije inicijaliziran. |
| **`calloc()`** | Alocira memoriju za više elemenata i postavlja sve bajtove na nulu. |
| **`realloc()`** | Mijenja veličinu već alociranog bloka; postojeći sadržaj čuva do manje od stare i nove veličine. |
| **`free()`** | Oslobađa prethodno alociranu memoriju. |
| **`NULL`** | Vrijednost pokazivača koja označava da nema valjanu adresu alociranog bloka. |
| **Curenje memorije (*memory leak*)** | Alocirana memorija koja više nije dostupna programu, a nije oslobođena. |

Funkcije vraćaju pokazivač tipa `void *` (osim `free()`). **U jeziku C nije potrebno pretvarati** rezultat `malloc()`, `calloc()` ili `realloc()` pomoću `(int *)`.

## 3. Struktura projekta

```text
09_dynamic_memory/
├── CMakeLists.txt
├── prj.conf
├── README.md
└── src/
    └── main.c
```

U `prj.conf` uključena je implementacija standardnih C funkcija za dinamičku memoriju i dodijeljen je mali, namjenski prostor:

```ini
CONFIG_CONSOLE=y
CONFIG_PRINTK=y
CONFIG_COMMON_LIBC_MALLOC=y
CONFIG_COMMON_LIBC_MALLOC_ARENA_SIZE=2048
CONFIG_COMMON_LIBC_CALLOC=y
```

- `CONFIG_COMMON_LIBC_MALLOC=y` omogućuje Zephyrovu zajedničku implementaciju C funkcija za dinamičku memoriju.
- `CONFIG_COMMON_LIBC_MALLOC_ARENA_SIZE=2048` rezervira **2048 bajtova** za pripadajući C heap; to nije količina memorije koju primjer nužno zauzima.
- `CONFIG_COMMON_LIBC_CALLOC=y` osigurava podršku za `calloc()` i uz minimalnu C biblioteku.

Ovaj primjer koristi **standardni C API** iz `<stdlib.h>`. Zephyr ima i zasebne kernel API-je `k_malloc()` / `k_free()`, koji nisu tema ove C vježbe; `CONFIG_HEAP_MEM_POOL_SIZE` odnosi se na Zephyrov sistemski kernel heap, a ne na ovdje konfigurirani C heap.

## 4. Dinamička memorija u C programu

U `src/main.c` demonstrirana su tri uzastopna koraka.
**`malloc()` — jedan cjelobrojni podatak:**

```c
int *single = malloc(sizeof(*single));

if (single == NULL) {
    printk("malloc failed\n");
    return 0;
}

*single = 55;
printk("malloc value: %d\n", *single);
free(single);
single = NULL;
```

Nakon `malloc()` vrijednost se **mora postaviti prije čitanja**, jer memorija nije inicijalizirana. U slučaju neuspjeha ne smije se dereferencirati pokazivač `NULL`.

**`calloc()` — tri elementa inicijalizirana nulama:**

```c
const size_t initial_count = 3;
int *values = calloc(initial_count, sizeof(*values));
```

Program provjerava `values != NULL`, ispisuje tri početne nule te postavlja vrijednosti `10`, `20` i `30`.

**`realloc()` — proširenje s tri na sedam elemenata:**

```c
const size_t expanded_count = 7;
int *resized = realloc(values, expanded_count * sizeof(*values));

if (resized == NULL) {
    printk("realloc failed\n");
    free(values);
    return 0;
}

values = resized;
```

Koristimo **privremeni pokazivač** `resized`: ako realokacija ne uspije, originalni blok `values` i dalje je valjan i treba ga osloboditi. Nakon uspješne realokacije stari pokazivač ne koristimo; blok je mogao biti premješten.

Prva tri elementa ostaju sačuvana. Novi elementi **nisu automatski postavljeni na nulu**, zato im program prije čitanja dodjeljuje vrijednosti `40`, `50`, `60` i `70`. Na kraju poziva `free(values)`.

> Potpuni izvorni kod, uključujući provjere i petlje za ispis, nalazi se u `src/main.c`.

## 5. Izgradnja i provjera

Naredbe se izvršavaju u **PowerShellu**, iz direktorija `zephyr_workspace`, uz aktivno Zephyrovo razvojno okruženje.

**Izgradnja:**

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/02_c_fundamentals/09_dynamic_memory -d zephyr_course/02_c_fundamentals/09_dynamic_memory/build -p always
```

**Učitavanje na ESP32:**

```powershell
west flash -d zephyr_course/02_c_fundamentals/09_dynamic_memory/build
```

Očekivani **aplikacijski** ispis, kada su sve alokacije uspješne (moguće su i početne Zephyrove poruke):

```text
malloc value: 55
calloc initial: 0 0 0
calloc filled: 10 20 30
realloc expanded: 10 20 30 40 50 60 70
Memory released
```

Ako neka alokacija ne uspije, program ispisuje `malloc failed`, `calloc failed` ili `realloc failed` i sigurno prekida primjer.

U `build/zephyr/.config` provjeri vrijednosti `CONFIG_COMMON_LIBC_MALLOC`, `CONFIG_COMMON_LIBC_MALLOC_ARENA_SIZE` i `CONFIG_COMMON_LIBC_CALLOC`. Generirane konfiguracijske datoteke ne uređuju se ručno.

## 6. Zadaci

**A — Promjena vrijednosti.** Promijeni vrijednost spremljenu pomoću `malloc()` s `55` na `99`. Izgradi i provjeri ispis.

**B — Inicijalizacija s `calloc()`.** Pronađi dio programa koji prvo ispisuje nule, a zatim elementima dodjeljuje `10`, `20` i `30`. Objasni razliku između početnog i naknadno postavljenog sadržaja.

**C — Promjena veličine niza.** Promijeni `expanded_count` sa `7` na `5`. Provjeri da program čuva prva tri elementa i inicijalizira samo dva nova elementa.

**D — Neuspjela realokacija.** Prouči što bi se dogodilo kada bi `realloc()` vratio `NULL`. Objasni zašto se rezultat najprije sprema u `resized`, a ne izravno u `values`.

**E — Oslobađanje memorije.** Pronađi sve pozive `free()` u `main.c`. Objasni zašto se `free(values)` poziva i u grani neuspjele realokacije. Nakon zadataka vrati početne vrijednosti.

## 7. Pitanja

1. Koja je razlika između `malloc()` i `calloc()`?
2. Zašto nakon `malloc()` ne čitamo sadržaj prije inicijalizacije?
3. Koje vrijednosti `realloc()` mora sačuvati nakon uspješnog proširenja bloka?
4. Zašto novododane elemente nakon `realloc()` treba inicijalizirati?
5. Zašto se rezultat `realloc()` najprije sprema u privremeni pokazivač?
6. Što znači povratna vrijednost `NULL` i kako je program obrađuje?
7. Zašto je važan `free()` i što je curenje memorije?
8. Čemu služi `CONFIG_COMMON_LIBC_MALLOC_ARENA_SIZE` u ovoj Zephyr aplikaciji?

## 8. Zaključak

Standardne C funkcije `malloc()`, `calloc()`, `realloc()` i `free()` omogućuju dinamičko upravljanje memorijom i u Zephyr aplikacijama, uz odgovarajuću konfiguraciju C biblioteke. Svaku alokaciju treba provjeriti, novododanu memoriju inicijalizirati prije čitanja i svaki uspješno alocirani blok osloboditi kada više nije potreban.

Dinamička memorija povećava fleksibilnost, ali zahtijeva disciplinirano upravljanje resursima. U vremenski kritičnim dijelovima embedded sustava često je prikladnije koristiti unaprijed dodijeljenu memoriju zbog predvidljivosti izvođenja.
