# Instalacija Zephyr RTOS-a na Windowsima

**Ciljna pločica:** ESP32 DevKitC V4 (klasični ESP32)  
**Zephyr RTOS:** `v4.4.2` (verzija odabrana za školska računala)  
**Alati:** Windows PowerShell, Python 3.12, West, Zephyr SDK, VS Code i IDE for Zephyr

Ove upute opisuju **novu instalaciju** i **svakodnevno korištenje postojećeg workspacea**. Ako je `zephyr_workspace` već postavljen, **nemoj ponovno pokretati `west init`, brisati `.west` niti ponovno instalirati Zephyr**.

## 1. Preduvjeti i alati

Instaliraj Python **3.12.x**, Git, Visual Studio Code te VS Code proširenja **IDE for Zephyr** i **Serial Monitor**. Pri instalaciji Pythona označi *Add Python to PATH*.

Za ostale Windows alate provjeri WinGet:

```powershell
winget --version
```

Ako odgovarajući alati nisu već instalirani, preporučeni postupak iz Zephyrovih službenih uputa jest WinGet (preskoči već instalirane pakete):

```powershell
winget install Kitware.CMake Ninja-build.Ninja oss-winget.gperf Python.Python.3.12 Git.Git oss-winget.dtc wget 7zip.7zip
```

Provjeri u **novom** PowerShell prozoru:

```powershell
cmake --version
ninja --version
where.exe wget
wget.exe --version
```

Na školskim računalima instalaciju pojedinih alata mogu ograničiti administratorske postavke.

**Ako `wget.exe` nedostaje:** u postojećim školskim uputama koristi se MSYS2. To je **alternativni postupak**, a ne obvezan dio svake instalacije:

```powershell
winget install --id MSYS2.MSYS2
```

Zatim u zasebnom terminalu **MSYS2 MSYS**:

```bash
pacman -Syu
# Ako ažuriranje zatraži ponovno pokretanje, zatvori MSYS2,
# otvori ga ponovno i ponovi pacman -Syu.
pacman -S wget
wget --version
```

Ako je potrebno da Windows pronađe MSYS2 `wget`, dodaj `C:\msys64\usr\bin` u korisnički `PATH` pa ponovno otvori VS Code. Ne dodaj MSYS2 u `PATH` ako `wget.exe` već radi jer time možeš promijeniti odabir drugih alata. Ako se pojavi pogreška `msys-nettle-9.dll`, ažuriraj MSYS2 naredbom `pacman -Syu` i po potrebi ponovno instaliraj paket `wget`. Ako se pojavi pogreška `msys-nettle-9.dll`, ažuriraj MSYS2 naredbom `pacman -Syu` i po potrebi ponovno instaliraj paket `wget`. Ako se pojavi pogreška `msys-nettle-9.dll`, ažuriraj MSYS2 naredbom `pacman -Syu` i po potrebi ponovno instaliraj paket `wget`.

## 2. Stvaranje Zephyr workspacea (samo prvi put)

U **PowerShellu** pokreni:

```powershell
cd $HOME\Documents
mkdir zephyr_workspace
cd zephyr_workspace
py -3.12 -m venv .venv
```

### Aktivacija `.venv` — važan korak

Na školskim Windows računalima skripte mogu biti blokirane. U istoj PowerShell sesiji pokreni:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy RemoteSigned -Force
.\.venv\Scripts\Activate.ps1
```

U naredbenom retku treba pisati **`(.venv)`**. Postavka `-Scope Process` vrijedi samo za **trenutačnu sesiju**; ne mijenja trajno izvršnu politiku računala.

Provjeri interpreter:

```powershell
python -c "import sys; print(sys.executable)"
```

Putanja treba završavati na `zephyr_workspace\.venv\Scripts\python.exe`, a ne na sistemskom `Python312\python.exe`.

Ako *Group Policy* škole izričito zabranjuje aktivaciju, nemoj mijenjati pravila ustanove. Python iz okruženja možeš pozvati izravno, npr. `& .\.venv\Scripts\python.exe -m pip --version`.

### Instalacija Westa, Zephyra i Python ovisnosti

**Tek nakon aktivacije** `.venv`:

```powershell
python -m pip install --upgrade pip west
west init -m https://github.com/zephyrproject-rtos/zephyr --mr v4.4.2 .
west update
python -m pip install @((west packages pip) -split ' ')
west zephyr-export
```

`west init` izvršava se **samo pri prvoj instalaciji**. Ako dobiješ `already initialized`, workspace postoji: nemoj ga brisati i ne ponavljaj inicijalizaciju.

Naredba za Python ovisnosti važna je za ispravan rad `west boards`, Devicetree alata i ESP32 podrške. Ne instaliraj pakete samo u globalni Python ako IDE i Zephyr koriste `.venv`.

Provjeri stanje:

```powershell
west topdir
git -C zephyr describe --tags --exact-match
west boards | Select-String "esp32_devkitc"
```

Očekivana Git oznaka je **`v4.4.2`**. Za primjere u ovom repozitoriju koristi se ciljna pločica **`esp32_devkitc/esp32/procpu`**.

## 3. VS Code, IDE for Zephyr i SDK

1. U VS Codeu otvori **cijeli direktorij `zephyr_workspace`**, a ne samo pojedinačnu vježbu.
2. U terminalu aktiviraj `.venv` ako terminal nije već aktiviran.
3. U proširenju **IDE for Zephyr** dovrši inicijalno postavljanje i provjeri *Host Tools*.
4. U dijelu *Zephyr SDK* instaliraj potrebni SDK ako ga još nemaš. Ako instalaciju radije provodiš iz terminala, službena alternativa jest `west sdk install` uz aktivirani `.venv`. **Zephyr SDK i Zephyr RTOS imaju zasebne brojeve verzija.**
5. Pri izgradnji odaberi `esp32_devkitc/esp32/procpu`.

**Napomena o IDE-u:** proširenje može pokretati West s **drugim Python interpreterom** nego terminal. Ako naredba `west boards` u aktiviranom PowerShellu radi, ali *Board Picker* ne prikazuje pločice, provjeri postavku Python interpretera u proširenju i odaberi `zephyr_workspace\.venv\Scripts\python.exe` ako je ponuđeno. Pomaže i ponovno otvaranje VS Codea iz aktiviranog terminala pomoću `code .` (nakon zatvaranja prethodne VS Code sesije).

## 4. Provjera instalacije — Hello World na ESP32-u

Iz direktorija `zephyr_workspace`, s aktiviranim `.venv`:

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr/samples/hello_world -d build/hello_world_esp32 -p always
west flash -d build/hello_world_esp32
```

**Prije `west flash` zatvori serijski monitor** ako zauzima isti COM port. Nakon učitavanja otvori Serial Monitor, odaberi odgovarajući COM port i brzinu **115200 baud**, pa provjeri ispis `Hello World` (uz moguće početne poruke sustava).

## 5. Pokretanje vježbi iz `zephyr_course`

Ako repozitorij još nije na računalu, iz `zephyr_workspace` pokreni:

```powershell
git clone https://github.com/Dejan-R/zephyr_course.git
```

Prva samostalna vježba:

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/01_foundations/01_project_structure -d zephyr_course/01_foundations/01_project_structure/build -p always
west flash -d zephyr_course/01_foundations/01_project_structure/build
```

Za druge vježbe koristi naredbe iz njihovih README-ova. **Ne kopiraj `build/` u Git ni u ZIP-ove za učenike**; svako računalo generira vlastite build datoteke.

## 6. Svakodnevni rad (nakon instalacije)

Za **svaki novi PowerShell terminal** u postojećem workspaceu dovoljno je:

```powershell
cd $HOME\Documents\zephyr_workspace
Set-ExecutionPolicy -Scope Process -ExecutionPolicy RemoteSigned -Force
.\.venv\Scripts\Activate.ps1
```

Ako VS Code otvara terminal već s aktiviranim `(.venv)`, aktivacija nije potrebna drugi put. Nakon toga otvori odgovarajući projekt, izgradi ga i učitaj na pločicu. **Ne ponavljaj `west init`, `west update` ni instalaciju SDK-a pri svakom otvaranju.**

## 7. Rješavanje čestih problema

| Simptom | Što provjeriti |
|---|---|
| `Activate.ps1 cannot be loaded` | U istoj sesiji pokreni `Set-ExecutionPolicy -Scope Process -ExecutionPolicy RemoteSigned -Force`, pa aktiviraj `.venv`. |
| `Running pip install outside of a virtual environment` | Aktiviraj `.venv` i provjeri `python -c "import sys; print(sys.executable)"`. |
| `No module named 'jsonschema'` | U aktiviranom `.venv` instaliraj Python ovisnosti naredbom iz odjeljka 2. |
| `esptool>=5.0.2 not found in PATH` | Provjeri instalaciju Python ovisnosti i koristi li IDE isti Python kao `.venv`. |
| IDE ne nudi pločice | Pokreni `west boards` iz aktiviranog terminala; ako radi, provjeri Python interpreter u IDE-u. |
| `west init: already initialized` | Workspace je već stvoren. Nemoj ponovno pokretati `west init`. |
| `Configuring incomplete, errors occurred!` | Potraži **prvu konkretnu pogrešku** nekoliko redaka iznad završne CMake poruke. |
| `west flash` ne može otvoriti port | Zatvori Serial Monitor ili drugi program koji koristi isti COM port. |

### Korisna naredba — potpuno brisanje ESP32 flash memorije

Samo ako je stvarno potrebno i uz odabrani COM port; **briše sav sadržaj flash memorije**:

```powershell
python -m esptool --port COM5 erase-flash
```

## 8. Reference

- [Zephyr — Getting Started Guide](https://docs.zephyrproject.org/latest/develop/getting_started/)
- [Zephyr — ESP32 DevKitC](https://docs.zephyrproject.org/latest/boards/espressif/esp32_devkitc/doc/index.html)
- [Microsoft — PowerShell Set-ExecutionPolicy](https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.security/set-executionpolicy)

**Napomena:** upute opisuju konkretno okruženje sa Zephyrom `v4.4.2`. Kod drugih verzija nazivi pločica, paketi i neke naredbe mogu se razlikovati.
