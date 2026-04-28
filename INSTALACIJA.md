# Evidencija Kljuceva i Filmova — KC Bar
## Uputstvo za Instalaciju i Pokretanje

---

## Sadrzaj paketa

```
EvidencijaKCBar/
├── src/
│   ├── main.cpp          — Ulazna tačka aplikacije
│   ├── mainwindow.h/cpp  — Glavni prozor
│   └── filmdialog.h/cpp  — Dijalog za unos/izmjenu
├── EvidencijaKCBar.pro   — Qt qmake projektni fajl
├── CMakeLists.txt        — CMake build (alternativa)
├── installer.iss         — Inno Setup installer skripta
└── INSTALACIJA.md        — Ovo uputstvo
```

---

## Kompajlovanje na Windows (Qt Creator)

### Preduslovi
1. Instalirajte **Qt 5.15** (besplatno): https://www.qt.io/download-open-source
   - U Qt Installer-u odaberite: `Qt 5.15.x` → `MSVC 2019 64-bit` ili `MinGW 64-bit`
2. Instalirajte **Qt Creator** (dolazi uz Qt)

### Koraci
1. Otvorite **Qt Creator**
2. `File` → `Open File or Project` → odaberite `EvidencijaKCBar.pro`
3. Kliknite `Configure Project`
4. Prebacite na `Release` mod (u donjem lijevom uglu)
5. `Build` → `Build Project` (Ctrl+B)
6. Executable se nalazi u `build/` folderu

---

## Pravljenje Installera (Windows .exe)

### Preduslovi
- **Inno Setup 6**: https://jrsoftware.org/isdl.php
- Buildovana aplikacija u `Release` modu

### Koraci
1. Nakon builda, pokrenite **windeployqt** da sakupite Qt DLL-ove:
   ```cmd
   cd release\
   "C:\Qt\5.15.x\msvc2019_64\bin\windeployqt.exe" EvidencijaKCBar.exe
   ```
2. Otvorite `installer.iss` u **Inno Setup Compileru**
3. `Build` → `Compile` (F9)
4. Installer se generiše u `installer_output\EvidencijaKCBar_Setup_v1.0.0.exe`

---

## Podatke čuva automatski

Aplikacija čuva podatke u:
- **Windows**: `C:\Users\<korisnik>\AppData\Roaming\KC Bar\EvidencijaKCBar\evidencija.json`
- **Linux**:   `~/.local/share/KC Bar/EvidencijaKCBar/evidencija.json`

Podaci se čuvaju automatski pri svakoj promjeni i učitavaju pri pokretanju.

---

## Funkcionalnosti

| Funkcija              | Opis                                                  |
|-----------------------|-------------------------------------------------------|
| Dodaj film            | Unos novog filma s formatom, ključem i statusom       |
| Uredi film            | Izmjena svih podataka o odabranom filmu               |
| Obriši film           | Brisanje filma s potvrdom                             |
| Statusna boja         | 🟢 Aktivan / 🟡 Uskoro ističe (≤7 dana) / 🔴 Istekao |
| Upozorenje            | Banner upozorenje na vrhu ako postoje kritični ključevi|
| Automatska provjera   | Svaku minutu provjerava stanje ključeva               |
| Formati               | 2K Scope, 4K Scope, 2K Flat, 4K Flat                 |
| Film skinut           | ✔ Da / ✘ Ne prikaz                                   |

---

## Podrška

KC Bar — Bioskop
