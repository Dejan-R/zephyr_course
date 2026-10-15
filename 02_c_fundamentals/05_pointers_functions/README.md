# 05 — Pointers and Functions

## 1. Cilj

Razumjeti kako se **adrese varijabli predaju funkciji kao argumenti**, kako funkcija čita vrijednosti preko pokazivača te kako može **promijeniti izvorne varijable** bez vraćanja njihove vrijednosti naredbom `return`. Upoznati i osnovnu sintaksu **pokazivača na funkciju**, preko kojega se funkcija može pozvati neizravno.

Primjer se nadovezuje na `03_pointers`: najprije zbraja dva broja preko pokazivača, zatim istu funkciju poziva preko pokazivača na funkciju i na kraju zamjenjuje izvorne vrijednosti.

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| **Pokazivač kao parametar** | Parametar poput `int *a_ptr` prima adresu objekta tipa `int`. |
| **`&a`** | Dohvaća adresu varijable `a`, koju možemo predati funkciji. |
| **`*a_ptr`** | Pristupa vrijednosti varijable na koju pokazivač pokazuje. |
| **Čitanje preko pokazivača** | Funkcija koristi izvornu vrijednost bez njezina mijenjanja. |
| **Pisanje preko pokazivača** | Funkcija mijenja izvornu varijablu izrazom poput `*a_ptr = 5;`. |
| **Privremena varijabla** | U `swap_values()` čuva jednu vrijednost dok se dvije vrijednosti zamjenjuju. |
| **Pokazivač na funkciju** | Pokazivač preko kojega možemo pozvati funkciju odgovarajućeg potpisa (povratni tip i parametri). |

**Važno:** C uvijek predaje argumente **po vrijednosti**. Kada predajemo pokazivač na podatak, kopira se **adresa**, a ne objekt na koji ona pokazuje. Zato funkcija može pristupati izvornom objektu. Za dva obična `int` broja pokazivači ne donose nužno uštedu memorije; ovdje ih koristimo radi razumijevanja pristupa i promjene izvornih podataka.

Pokazivač **predan funkciji** i pokazivač **na funkciju** nisu ista stvar: prvi upućuje na podatak, a drugi omogućuje pozivanje funkcije.

## 3. Struktura projekta

```text
05_pointers_functions/
├── CMakeLists.txt
├── prj.conf
├── README.md
└── src/
    └── main.c
```

`CMakeLists.txt` uključuje `src/main.c` pomoću `target_sources(app PRIVATE src/main.c)`, a `prj.conf` omogućuje konzolni ispis (`CONFIG_CONSOLE=y`, `CONFIG_PRINTK=y`). Za ovaj primjer ne trebaju `app.overlay`, dodatne biblioteke ni priključeni periferni uređaji.

## 4. Pokazivači i funkcije

**Funkcija koja čita dvije vrijednosti i vraća njihov zbroj:**

```c
static int add_using_pointers(int *a_ptr, int *b_ptr)
{
    return *a_ptr + *b_ptr;
}
```

Parametri `a_ptr` i `b_ptr` sadrže adrese. Operator `*` dohvaća brojeve na tim adresama; funkcija ih zbraja i rezultat vraća pomoću `return`. Izvorne varijable pritom ostaju nepromijenjene.

**Funkcija koja zamjenjuje izvorne vrijednosti:**

```c
static void swap_values(int *a_ptr, int *b_ptr)
{
    int temp = *a_ptr;

    *a_ptr = *b_ptr;
    *b_ptr = temp;
}
```

Varijabla `temp` privremeno čuva prvu vrijednost. Dodjele preko `*a_ptr` i `*b_ptr` mijenjaju varijable čije su adrese predane. Funkcija vraća `void` jer ne vraća rezultat kao povratnu vrijednost, nego mijenja izvorne podatke.

### Pokazivač na funkciju

```c
int (*operation)(int *, int *) = add_using_pointers;
```

`operation` je **pokazivač na funkciju** koja prima dva argumenta tipa `int *` i vraća `int`. Zagrade oko `*operation` važne su jer razlikuju pokazivač na funkciju od deklaracije funkcije koja vraća pokazivač.

Istu funkciju možemo pozvati izravno ili preko pokazivača:

```c
add_using_pointers(&a, &b);  /* Direct call. */
operation(&a, &b);           /* Call through a function pointer. */
```

Oba poziva daju isti rezultat. Jednakovrijedan je i zapis `(*operation)(&a, &b)`. Pokazivači na funkcije česta su osnova *callback* mehanizama, no njihova detaljnija primjena nije tema ove vježbe.

U `main()` se primjeri izvode ovim redoslijedom:

```c
int a = 3;
int b = 5;

int result = add_using_pointers(&a, &b);
printk("Sum: %d + %d = %d\n", a, b, result);

int (*operation)(int *, int *) = add_using_pointers;
result = operation(&a, &b);
printk("Sum through function pointer: %d\n", result);

swap_values(&a, &b);
printk("After swap: a = %d, b = %d\n", a, b);
```

| Izraz | Što se događa |
|---|---|
| `add_using_pointers(&a, &b)` | Funkcija prima adrese, čita `3` i `5` te vraća `8`. |
| `operation(&a, &b)` | Preko pokazivača na funkciju poziva se `add_using_pointers()`. |
| `swap_values(&a, &b)` | Funkcija preko pokazivača mijenja `a` na `5` i `b` na `3`. |
| `&a`, `&b` | Predaju adrese varijabli, a ne vrijednosti `3` i `5`. |
| `*a_ptr`, `*b_ptr` | Pristupaju vrijednostima preko primljenih adresa. |

Cijeli program nalazi se u `src/main.c`.

## 5. Izgradnja i provjera

Naredbe pokreni u **PowerShellu**, iz direktorija `zephyr_workspace`, uz prethodno postavljeno Zephyrovo razvojno okruženje.

**Čista izgradnja:**

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/02_c_fundamentals/05_pointers_functions -d zephyr_course/02_c_fundamentals/05_pointers_functions/build -p always
```

**Učitavanje na ESP32** (prethodno zatvori serijski monitor ako koristi isti COM port):

```powershell
west flash -d zephyr_course/02_c_fundamentals/05_pointers_functions/build
```

Očekivani **aplikacijski** ispis (moguće su i Zephyrove početne poruke):

```text
Sum: 3 + 5 = 8
Sum through function pointer: 8
After swap: a = 5, b = 3
```

Projekt je pripremljen za ESP32 DevKitC V4; stvarni `west build` i izvođenje na pločici nisu ovdje provjereni.

## 6. Zadaci

**A — Promjena početnih vrijednosti.** U `main.c` postavi `a = 10` i `b = 20`. Prije pokretanja predvidi sva tri retka ispisa, zatim provjeri rezultat.

**B — Provjera promjene izvornih varijabli.** Dodaj ispis vrijednosti `a` i `b` neposredno prije poziva `swap_values()` i usporedi ih s vrijednostima nakon poziva.

**C — Dvostruka zamjena.** Pozovi `swap_values(&a, &b)` još jednom. Objasni zašto se nakon drugog poziva vraćaju početne vrijednosti.

**D — Nova funkcija.** Napiši `static void increment(int *number)` koja povećava izvornu varijablu za `1`. Pozovi je s `increment(&a)` i ispiši novu vrijednost `a`.

**E — Usporedba pristupa.** Napiši zasebnu funkciju `static int add_by_value(int a, int b)` koja vraća `a + b`. Usporedi njezin rezultat s `add_using_pointers()` i objasni zašto je za jednostavno zbrajanje dvaju `int` brojeva predaja po vrijednosti obično prirodnija.

**F — Pokazivač na funkciju.** Umjesto `operation(&a, &b)` upotrijebi `(*operation)(&a, &b)`. Provjeri dobivaš li isti rezultat. Objasni što označuje `int (*operation)(int *, int *)`.

Nakon zadataka vrati početni kod kako bi očekivani ispis odgovarao primjeru.

## 7. Pitanja

1. Što znači parametar `int *a_ptr`?
2. Zašto pri pozivu koristimo `&a`, a unutar funkcije `*a_ptr`?
3. Što vraća `add_using_pointers()` i mijenja li varijable `a` i `b`?
4. Zašto je u `swap_values()` potrebna privremena varijabla `temp`?
5. Kako `swap_values()` mijenja `a` i `b` iako ima povratni tip `void`?
6. Što se u jeziku C kopira pri predaji pokazivača funkciji?
7. Zašto korištenje pokazivača ovdje nije automatski ušteda memorije?
8. Koja je razlika između ovog primjera i lekcije `03_pointers`?
9. Koja je razlika između pokazivača kao argumenta funkcije i pokazivača na samu funkciju?

## 8. Zaključak

Pokazivače možemo koristiti kao argumente funkcija kako bismo pristupili izvornim podacima. `add_using_pointers()` pokazuje **čitanje vrijednosti**, a `swap_values()` **njihovu promjenu preko pokazivača**.

Pokazivač na funkciju `operation` omogućuje **neizravan poziv funkcije** `add_using_pointers()`. C pritom argumente uvijek predaje po vrijednosti: kada argument predstavlja adresu, kopira se adresa, a ne cijeli objekt na koji pokazuje.
