# 06 — Struct Pointers

## 1. Cilj

Razumjeti kako funkciji predati **pokazivač na strukturu**, pristupiti njezinim članovima operatorom `->` te razlikovati **čitanje** od **promjene izvorne strukture** preko pokazivača.

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| **Pokazivač na strukturu** | Pokazivač koji sadrži adresu objekta tipa `struct`. |
| **`typedef`** | Omogućuje korištenje naziva `sum_data_t` za definirani tip strukture. |
| **`&s1`** | Dohvaća adresu strukture `s1`. |
| **`data->a`** | Pristupa članu `a` strukture preko pokazivača `data`. |
| **`(*data).a`** | Drugi zapis za `data->a`; zagrade su obvezne. |
| **`const sum_data_t *data`** | Funkcija može čitati strukturu, ali je ne smije mijenjati preko tog pokazivača. |
| **`sum_data_t *data`** | Funkcija preko pokazivača može i mijenjati članove izvorne strukture. |

## 3. Struktura projekta

```text
06_struct_pointers/
├── CMakeLists.txt
├── prj.conf
├── README.md
└── src/
    └── main.c
```

`CMakeLists.txt` uključuje `src/main.c` naredbom `target_sources(app PRIVATE src/main.c)`. Datoteka `prj.conf` uključuje konzolni ispis (`CONFIG_CONSOLE=y` i `CONFIG_PRINTK=y`). Nisu potrebni `app.overlay`, dodatni moduli ni spojeni periferni uređaji.

## 4. Pokazivači na strukture u C programu

**Definicija tipa strukture:**

```c
typedef struct {
    uint8_t a;
    uint8_t b;
} sum_data_t;
```

`uint8_t` označava 8-bitni nenegativni cijeli broj. U `main()` stvaramo dvije strukture: `s1 = {3, 4}` i `s2 = {5, 5}`.

**Čitanje članova strukture preko pokazivača:**

```c
static int calculate_sum(const sum_data_t *data)
{
    return data->a + data->b;
}
```

Operator `->` omogućuje pristup članovima strukture na koju `data` pokazuje. Izraz `data->a` znači isto što i `(*data).a`. Zagrade su potrebne jer bi se bez njih operator `.` primijenio prije `*`. Parametar je `const` jer funkcija samo čita članove i vraća njihov zbroj.

**Promjena izvorne strukture preko pokazivača:**

```c
static void update_values(sum_data_t *data, uint8_t a, uint8_t b)
{
    data->a = a;
    data->b = b;
}
```

Ova funkcija prima pokazivač bez `const` jer mijenja članove strukture. Promjene ostaju vidljive u `main()`, jer se mijenja **isti objekt** na čiju smo adresu proslijedili pokazivač.

**Pozivanje funkcija u `main()`:**

```c
printk("Sum 1: %d\n", calculate_sum(&s1));
printk("Sum 2: %d\n", calculate_sum(&s2));

update_values(&s1, 10, 20);
printk("Updated s1: a = %d, b = %d\n", s1.a, s1.b);
printk("New sum 1: %d\n", calculate_sum(&s1));
```

| Izraz | Značenje |
|---|---|
| `s1.a` | Čita član `a` izravno iz strukture. |
| `calculate_sum(&s1)` | Predaje adresu strukture funkciji koja čita njezine članove. |
| `data->a` | Pristupa članu `a` preko pokazivača. |
| `update_values(&s1, 10, 20)` | Mijenja članove **izvorne** strukture `s1`. |

Cijeli program i komentari nalaze se u `src/main.c`.

## 5. Izgradnja i provjera

Naredbe pokreni u **PowerShellu**, iz direktorija `zephyr_workspace`, uz postavljeno Zephyrovo razvojno okruženje.

**Čista izgradnja:**

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/02_c_fundamentals/06_struct_pointers -d zephyr_course/02_c_fundamentals/06_struct_pointers/build -p always
```

**Učitavanje na ESP32** (zatvori serijski monitor ako koristi isti COM port):

```powershell
west flash -d zephyr_course/02_c_fundamentals/06_struct_pointers/build
```

Očekivani **aplikacijski** ispis (moguće su i Zephyrove početne poruke):

```text
Struct Pointers
Sum 1: 7
Sum 2: 10
Updated s1: a = 10, b = 20
New sum 1: 30
```

Stvarni `west build` i izvođenje na ESP32 pločici nisu ovdje provjereni.

## 6. Zadaci

**A — Promjena početnih vrijednosti.** Postavi `s1 = {2, 8}` i `s2 = {6, 7}`. Prije pokretanja predvidi oba početna zbroja, a zatim provjeri ispis.

**B — Promjena druge strukture.** Nakon postojećeg primjera pozovi `update_values(&s2, 15, 25)` te ispiši nove članove `s2` i njezin novi zbroj. Provjeri da promjena strukture `s2` ne utječe na `s1`.

**C — Operator `->` i operator `.`.** U `calculate_sum()` zamijeni `data->a + data->b` izrazom `(*data).a + (*data).b`. Provjeri daje li program isti rezultat i objasni zašto su zagrade potrebne.

**D — Uloga `const`.** Privremeno pokušaj dodati naredbu `data->a = 10;` u `calculate_sum()`. Pokreni build i prouči dijagnostiku prevoditelja. Zatim ukloni naredbu.

**E — Još jedan objekt iste strukture.** U `main()` dodaj `sum_data_t s3 = {1, 9};`, pozovi `calculate_sum(&s3)` i ispiši rezultat. Objasni zašto ista funkcija može raditi s različitim objektima strukture.

Nakon zadataka vrati početni kod kako bi očekivani ispis odgovarao primjeru.

## 7. Pitanja

1. Što sadrži pokazivač tipa `sum_data_t *`?
2. Koja je razlika između `s1.a` i `data->a`?
3. Zašto su `data->a` i `(*data).a` ekvivalentni izrazi?
4. Što se predaje funkciji pri pozivu `calculate_sum(&s1)`?
5. Zašto `calculate_sum()` može koristiti `const sum_data_t *data`?
6. Zašto `update_values()` ne koristi `const` u parametru?
7. Zašto su novi članovi `s1` vidljivi u `main()` nakon poziva `update_values()`?
8. Koja je razlika između ove vježbe i prethodne `05_pointers_functions`?

## 8. Zaključak

Pokazivač na strukturu omogućuje funkciji pristup članovima izvornog objekta. Operator `->` pojednostavljuje zapis `(*data).member`. U ovom primjeru `calculate_sum()` **čita** članove strukture, a `update_values()` ih **mijenja**, pa su nove vrijednosti dostupne i nakon povratka iz funkcije.
