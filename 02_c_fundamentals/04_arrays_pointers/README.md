# 04 — Arrays and Pointers

## 1. Cilj

Razumjeti odnos između C nizova i pokazivača, pristup elementima indeksiranjem i aritmetikom pokazivača te zašto izrazi `data[i]` i `*(p + i)` daju istu vrijednost kada `p` pokazuje na prvi element niza.

## 2. Novi pojmovi

| Pojam | Značenje |
|---|---|
| **Niz (array)** | Skup elemenata istog tipa smještenih uzastopno u memoriji. |
| **Indeks** | Položaj elementa u nizu; prvi element ima indeks `0`. |
| **Pokazivač na element** | Varijabla koja čuva adresu elementa niza. |
| **Aritmetika pokazivača** | Pomicanje pokazivača za određeni broj elemenata, a ne bajtova. |
| **Dereferenciranje (`*`)** | Dohvaćanje ili promjena vrijednosti na adresi na koju pokazivač pokazuje. |
| **`sizeof`** | Operator koji daje veličinu tipa ili objekta u bajtovima. |

**Važno:** naziv niza u većini izraza pretvara se u pokazivač na prvi element, ali **niz i pokazivač nisu ista vrsta objekta**.

## 3. Struktura projekta

```text
04_arrays_pointers/
├── CMakeLists.txt
├── prj.conf
├── README.md
└── src/
    └── main.c
```
U `src/main.c` definiramo niz i pokazivač:

```c
int data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
int *p = data;
```

Pokazivač `p` sadrži adresu prvog elementa, jednaku `&data[0]`.

`CMakeLists.txt` uključuje `src/main.c` putem `target_sources(app PRIVATE src/main.c)`, a `prj.conf` omogućuje ispis preko konzole (`CONFIG_CONSOLE=y` i `CONFIG_PRINTK=y`). Za ovaj projekt nisu potrebni `Kconfig` ni `app.overlay`.

## 4. Pristup elementima niza

Unutar petlje uspoređujemo dva načina pristupa istom elementu:

```c
for (int i = 0; i < 10; i++) {
    printk("data[%d] = %d, *(p + %d) = %d\n",
           i, data[i], i, *(p + i));
}
```

| Izraz | Značenje |
|---|---|
| `data[0]` | Prvi element niza. |
| `&data[0]` | Adresa prvog elementa. |
| `p` | Adresa spremljena u pokazivaču, jednaka `&data[0]`. |
| `p + i` | Adresa elementa s indeksom `i`. |
| `*(p + i)` | Vrijednost elementa s indeksom `i`. |
| `data[i]` | Ista vrijednost dobivena indeksiranjem. |

Za ovaj primjer vrijedi **`data[i] == *(p + i)`** za svaki valjani indeks `i` od `0` do `9`.

Pomak `p + 1` znači prelazak na **sljedeći `int` element**, odnosno pomak za `sizeof(int)` bajtova, a ne nužno za jedan bajt. Izrazi koji pristupaju elementima izvan granica niza nisu dopušteni.

## 5. Izgradnja i provjera

Naredbe se izvršavaju u **PowerShellu**, iz direktorija `zephyr_workspace`, s aktivnim Zephyrovim okruženjem.

**Izgradnja:**

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/02_c_fundamentals/04_arrays_pointers -d zephyr_course/02_c_fundamentals/04_arrays_pointers/build -p always
```

**Učitavanje na ESP32** (prethodno zatvori serijski monitor ako koristi isti COM port):

```powershell
west flash -d zephyr_course/02_c_fundamentals/04_arrays_pointers/build
```

Očekivani **aplikacijski** ispis (moguće su i početne Zephyrove poruke):

```text
Arrays and Pointers
data[0] = 0, *(p + 0) = 0
data[1] = 1, *(p + 1) = 1
data[2] = 2, *(p + 2) = 2
data[3] = 3, *(p + 3) = 3
data[4] = 4, *(p + 4) = 4
data[5] = 5, *(p + 5) = 5
data[6] = 6, *(p + 6) = 6
data[7] = 7, *(p + 7) = 7
data[8] = 8, *(p + 8) = 8
data[9] = 9, *(p + 9) = 9
```

## 6. Zadaci

**A — Promjena elemenata.** Promijeni početne vrijednosti niza, ponovno izgradi aplikaciju i provjeri da oba načina pristupa ispisuju jednake vrijednosti.

**B — Upis preko pokazivača.** Prije petlje dodaj `*(p + 3) = 30;`. Provjeri da `data[3]` i `*(p + 3)` sada daju `30`.

**C — Usporedba adresa.** Ispiši `p` i `&data[0]` pomoću formata `%p` (uz pretvorbu u `(void *)`). Provjeri jesu li adrese jednake.

**D — Veličina niza i pokazivača.** Usporedi `sizeof(data)`, `sizeof(p)` i `sizeof(data[0])`. Objasni zašto se veličina cijelog niza razlikuje od veličine pokazivača. Nemoj pretpostavljati da svi uređaji imaju jednaku veličinu pokazivača.

**E — Granice niza.** Objasni zašto petlja koristi uvjet `i < 10`, a ne `i <= 10`. **Nemoj izvršavati** pristup `data[10]` jer se nalazi izvan granica niza.

Nakon zadataka vrati izvorne vrijednosti kako bi ispis odgovarao dokumentaciji.

## 7. Pitanja

1. Koja je razlika između niza i pokazivača?
2. Zašto `int *p = data;` pokazuje na prvi element niza?
3. Što predstavlja izraz `&data[0]`?
4. Zašto su `data[i]` i `*(p + i)` jednaki u ovom primjeru?
5. Što se događa s adresom kada izračunamo `p + 1`?
6. Kako preko pokazivača promijeniti četvrti element niza?
7. Zašto `sizeof(data)` i `sizeof(p)` ne moraju dati isti rezultat?
8. Koji su valjani indeksi niza `int data[10]` i zašto je pristup izvan njih opasan?

## 8. Zaključak

Elementima niza možemo pristupiti indeksiranjem (`data[i]`) ili preko pokazivača (`*(p + i)`). Aritmetika pokazivača uzima u obzir veličinu tipa elementa, a pristup mora ostati unutar granica niza. Iako naziv niza u većini izraza daje adresu prvog elementa, **niz nije isto što i pokazivačka varijabla**.
