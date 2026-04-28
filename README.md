# 🎬 Evidencija Ključeva i Filmova — KC Bar

> Desktop aplikacija za vođenje evidencije filmskih ključeva i filmova u bioskopu KC Bar.

![Build Status](https://github.com/YOUR_USERNAME/EvidencijaKCBar/actions/workflows/build.yml/badge.svg)

---

## ✨ Funkcionalnosti

| Funkcija | Opis |
|---|---|
| **Naziv filma** | Puni naziv filma |
| **Format** | 2K Scope · 4K Scope · 2K Flat · 4K Flat |
| **Status ključa** | Otkljucan / Zakljucan |
| **Datum isteka** | Datum do kada važi ključ |
| **Upozorenje** | Automatski detektuje ključeve koji ističu ≤7 dana |
| **Film skinut** | Indikator da li je film preuzet |
| **Napomnena** | Slobodan tekst uz svaki unos |

### Statusne boje
- 🟢 **Zeleno** — ključ aktivan, sve u redu
- 🟡 **Žuto** — ključ ističe za ≤7 dana ⚠
- 🔴 **Crveno** — ključ istekao ❌
- ⚫ **Sivo** — ključ zakljucan 🔒

---

## 📥 Preuzimanje

### Brza instalacija (bez kompajliranja)
1. Idi na **[Releases](../../releases/latest)**
2. Preuzmi `EvidencijaKCBar_Setup_vX.X.X.exe`
3. Instaliraj i pokreni — to je sve!

### Portabilna verzija (bez instalacije)
1. Idi na **[Actions](../../actions)** → zadnji uspješan build
2. Preuzmi artifact `EvidencijaKCBar-Windows-Portable`
3. Raspakuj ZIP i pokreni `EvidencijaKCBar.exe`

---

## 🔧 Kompajliranje iz koda

### Preduslovi
- [Qt 5.15](https://www.qt.io/download-open-source) (MinGW ili MSVC)
- Qt Creator (dolazi uz Qt)

### Koraci
```bash
# Kloniraj repozitorij
git clone https://github.com/YOUR_USERNAME/EvidencijaKCBar.git
cd EvidencijaKCBar

# Otvori EvidencijaKCBar.pro u Qt Creator
# ili komandnom linijom:
mkdir build && cd build
qmake ../EvidencijaKCBar.pro CONFIG+=release
make         # Linux/MinGW
nmake        # Windows MSVC
```

---

## 📁 Struktura projekta

```
EvidencijaKCBar/
├── .github/
│   └── workflows/
│       └── build.yml        ← GitHub Actions (auto build)
├── src/
│   ├── main.cpp
│   ├── mainwindow.h / .cpp  ← Glavni prozor
│   └── filmdialog.h / .cpp  ← Dijalog za unos
├── EvidencijaKCBar.pro       ← Qt projekt
├── CMakeLists.txt            ← CMake alternativa
├── installer.iss             ← Inno Setup installer
└── README.md
```

---

## 💾 Podaci

Evidencija se automatski čuva u JSON formatu:
- **Windows**: `%APPDATA%\KC Bar\EvidencijaKCBar\evidencija.json`
- **Linux**: `~/.local/share/KC Bar/EvidencijaKCBar/evidencija.json`

---

## 🚀 GitHub Actions — Automatski Build

Svaki `push` na `main` granu automatski:
1. Kompajlira aplikaciju na **Windows** (Qt 5.15 + MSVC)
2. Pakuje sve Qt DLL-ove (`windeployqt`)
3. Pravi **Windows Installer** (Inno Setup)
4. Upload artifakata dostupnih za preuzimanje

Svaki novi **GitHub Release** automatski prilaže installer uz release.

---

*KC Bar — Bioskop*
