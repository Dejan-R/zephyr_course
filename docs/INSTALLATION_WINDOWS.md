# Instalacija Zephyr RTOS-a na Windowsima

**Razvojna pločica:** ESP32 DevKitC V4 (klasični ESP32)  
**Zephyr RTOS:** `v4.4.2`  
**Alati:** Windows PowerShell, Python 3.12, West, Zephyr SDK, VS Code i IDE for Zephyr

Ove upute obuhvaćaju prvu instalaciju Zephyra, pripremu alata i pokretanje primjera iz repozitorija `zephyr_course`. Ako je `zephyr_workspace` već postavljen, prijeći na [odjeljak 6](#6-svakodnevni-rad-nakon-instalacije). Nije potrebno ponovno pokretati `west init` ni instalirati Zephyr.

## 1. Preduvjeti i alati

Instalirati **Python 3.12.x**, Git i Visual Studio Code te proširenja **IDE for Zephyr** i **Serial Monitor** za VS Code. Pri instalaciji Pythona označiti opciju *Add Python to PATH*.

Provjeriti je li dostupan WinGet:

```powershell
winget --version
```

Za instalaciju potrebnih Windows alata može se koristiti naredba iz [službenih Zephyrovih uputa](https://docs.zephyrproject.org/latest/develop/getting_started/index.html). Već instalirane pakete nije potrebno ponovno instalirati:

```powershell
winget install Kitware.CMake Ninja-build.Ninja oss-winget.gperf Python.Python.3.12 Git.Git oss-winget.dtc wget 7zip.7zip
```

Zatvoriti terminal, otvoriti **novi PowerShell** i provjeriti dostupnost alata:

```powershell
python --version
cmake --version
ninja --version
where.exe wget
wget.exe --version
```

Ako `wget.exe` nije dostupan, može se instalirati i putem **MSYS2**. Taj korak nije potreban ako postojeći `wget.exe` radi.

U PowerShellu:

```powershell
winget install --id MSYS2.MSYS2
```

U zasebnom terminalu **MSYS2 MSYS**:

```bash
pacman -Syu
# Ako MSYS2 traži ponovno pokretanje, zatvoriti ga,
# ponovno otvoriti i ponoviti pacman -Syu.
pacman -S --needed wget
wget --version
```

Ako Windows ne pronalazi MSYS2 `wget.exe`, dodati `C:\msys64\usr\bin` u korisničku varijablu `PATH` i otvoriti novi terminal. Ne dodavati taj direktorij ako `wget.exe` već radi jer se time može promijeniti odabir drugih programa. Ako se pojavi pogreška `msys-nettle-9.dll`, ažurirati MSYS2 naredbom `pacman -Syu` i provjeriti instalaciju paketa `wget`.

## 2. Stvaranje Zephyr workspacea (samo prvi put)

U **PowerShellu** stvoriti direktorij `zephyr_workspace` i Python virtualno okruženje:

```powershell
cd $HOME\Documents
mkdir zephyr_workspace
cd zephyr_workspace
py -3.12 -m venv .venv
```

### Aktivacija `.venv`

Ako PowerShell blokira pokretanje aktivacijske skripte, dopustiti njezino izvršavanje samo u trenutačnoj sesiji:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy RemoteSigned -Force
.\.venv\Scripts\Activate.ps1
```

Nakon aktivacije na početku naredbenog retka treba pisati `(.venv)`. Postavka `-Scope Process` ne mijenja trajno izvršnu politiku sustava.

Provjeriti koristi li se Python iz virtualnog okruženja:

```powershell
python -c "import sys; print(sys.executable)"
```

Putanja treba završavati s `zephyr_workspace\.venv\Scripts\python.exe`.

Ako aktivacija nije dopuštena pravilima sustava, Python iz virtualnog okruženja može se pozvati izravno, primjerice:

```powershell
& .\.venv\Scripts\python.exe -m pip --version
```

### Instalacija Westa, Zephyra i Python ovisnosti

U **aktiviranom** `.venv` pokrenuti:

```powershell
python -m pip install --upgrade pip west
west init -m https://github.com/zephyrproject-rtos/zephyr --mr v4.4.2 .
west update
python -m pip install @((west packages pip) -split ' ')
west zephyr-export
```

`west init` pokreće se samo pri stvaranju novog workspacea. Ako se pojavi poruka `already initialized`, ne brisati `.west` niti ponovno inicijalizirati postojeći workspace.

Python ovisnosti instaliraju se u `.venv`, kako bi West, Zephyr i razvojni alati koristili odgovarajuće pakete.

Provjeriti instalaciju:

```powershell
west topdir
git -C zephyr describe --tags --exact-match
west boards | Select-String "esp32_devkitc"
```

Očekivana Git oznaka je `v4.4.2`, a ciljna pločica za primjere u ovom repozitoriju `esp32_devkitc/esp32/procpu`.

## 3. VS Code, IDE for Zephyr i Zephyr SDK

1. U VS Codeu otvoriti **cijeli direktorij `zephyr_workspace`**.
2. Otvoriti terminal i aktivirati `.venv` ako već nije aktiviran.
3. U proširenju **IDE for Zephyr** dovršiti početnu konfiguraciju i provjeriti *Host Tools*.
4. U dijelu *Zephyr SDK* instalirati SDK ako već nije instaliran. Zephyr SDK i Zephyr RTOS imaju **odvojene brojeve verzija**.
5. Za izgradnju primjera odabrati pločicu `esp32_devkitc/esp32/procpu`.

SDK se može instalirati i iz aktiviranog terminala, iz direktorija izvornog koda Zephyra:

```powershell
cd zephyr
west sdk install
cd ..
```

**IDE i Python:** proširenje može koristiti drugi Python interpreter od terminala. Ako `west boards` u aktiviranom PowerShellu radi, a *Board Picker* ne prikazuje pločice, provjeriti Python interpreter u postavkama proširenja. Ako je moguće, odabrati `zephyr_workspace\.venv\Scripts\python.exe`. VS Code može se ponovno otvoriti naredbom `code .` iz aktiviranog terminala.

## 4. Provjera instalacije — Hello World na ESP32-u

Iz direktorija `zephyr_workspace`, s aktiviranim `.venv`, pokrenuti:

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr/samples/hello_world -d build/hello_world_esp32 -p always
west flash -d build/hello_world_esp32
```

Prije `west flash` **zatvoriti Serial Monitor** ako koristi isti COM port. Nakon učitavanja otvoriti serijski monitor u VS Codeu, odabrati odgovarajući COM port i brzinu **115200 baud**.

Očekivani aplikacijski ispis:

```text
Hello World!
```

Mogu se pojaviti i početne Zephyrove poruke, a stvarni ispis može sadržavati naziv pločice.

## 5. Pokretanje vježbi iz `zephyr_course`

Ako repozitorij još nije preuzet, iz direktorija `zephyr_workspace` preuzeti ga naredbom:

```powershell
git clone https://github.com/Dejan-R/zephyr_course.git
```

Izgraditi i učitati prvu vježbu:

```powershell
west build -b esp32_devkitc/esp32/procpu zephyr_course/01_foundations/01_project_structure -d zephyr_course/01_foundations/01_project_structure/build -p always
west flash -d zephyr_course/01_foundations/01_project_structure/build
```

Za ostale primjere koristiti naredbe navedene u njihovim README datotekama. Direktorije `build/` ne uključivati u Git repozitorij; stvaraju se lokalno tijekom izgradnje.

## 6. Svakodnevni rad (nakon instalacije)

U novom PowerShell terminalu aktivirati postojeće okruženje:

```powershell
cd $HOME\Documents\zephyr_workspace
Set-ExecutionPolicy -Scope Process -ExecutionPolicy RemoteSigned -Force
.\.venv\Scripts\Activate.ps1
```

Ako terminal već prikazuje `(.venv)`, nije potrebno ponovno aktivirati okruženje. Nakon toga moguće je izgrađivati i učitavati aplikacije.

**Ne ponavljati `west init`, `west update` ni instalaciju SDK-a pri svakom pokretanju VS Codea.**

## 7. Rješavanje čestih problema

| Simptom | Što provjeriti |
|---|---|
| `Activate.ps1 cannot be loaded` | U trenutačnom PowerShell prozoru pokrenuti `Set-ExecutionPolicy -Scope Process -ExecutionPolicy RemoteSigned -Force` pa aktivirati `.venv`. Ako to nije dopušteno pravilima sustava, ne zaobilaziti ograničenja. |
| `Running pip install outside of a virtual environment` | Aktivirati `.venv` i provjeriti `python -c "import sys; print(sys.executable)"`. |
| `No module named 'jsonschema'` | U aktiviranom `.venv` ponoviti instalaciju Python ovisnosti iz odjeljka 2. |
| `esptool>=5.0.2 not found in PATH` | Provjeriti Python ovisnosti i koristi li IDE interpreter iz `.venv`. |
| IDE ne prikazuje pločice | Pokrenuti `west boards` u aktiviranom terminalu i provjeriti postavku Python interpretera u proširenju. |
| `west init: already initialized` | Workspace već postoji. Ne pokretati ponovno `west init`. |
| `Configuring incomplete, errors occurred!` | Pronaći prvu konkretnu pogrešku iznad završne CMake poruke. |
| `west flash` ne može otvoriti COM port | Zatvoriti Serial Monitor i druge programe koji koriste taj port te provjeriti odabrani port. |

### Potpuno brisanje ESP32 flash memorije

Ovu naredbu koristiti **samo ako je stvarno potrebna**. Briše **sav sadržaj flash memorije**, uključujući postojeći firmware i spremljene podatke. Zamijeniti `COM5` stvarnim portom:

```powershell
python -m esptool --port COM5 erase-flash
```

## 8. Reference

- [Zephyr — Getting Started Guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html)
- [Zephyr — ESP32 DevKitC](https://docs.zephyrproject.org/latest/boards/espressif/esp32_devkitc/doc/index.html)
- [Zephyr — izdanje v4.4.2](https://github.com/zephyrproject-rtos/zephyr/releases/tag/v4.4.2)
- [Microsoft — PowerShell Set-ExecutionPolicy](https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.security/set-executionpolicy)

**Napomena:** upute su pripremljene za Zephyr `v4.4.2`. Službene poveznice iznad vode na aktualnu dokumentaciju, koja se može razlikovati od te verzije.
