#include "keygen.h"
#include "../src/license.h"
#include <QNetworkInterface>
#include <QDesktopServices>
#include <QUrl>
#include <QInputDialog>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QScreen>

KeygenWindow::KeygenWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Valter — Generator Licencnih Kljuceva");
    setMinimumSize(600, 500);
    resize(680, 560);

    setStyleSheet(R"(
        QMainWindow, QWidget { background: #0e1118; color: #c8d4f0; font-size: 13px; font-family: 'Segoe UI'; }
        QLabel { color: #8fa4cc; }
        QListWidget {
            background: #141a28; border: 1px solid #2d3a55; border-radius: 8px;
            font-family: 'Courier New', monospace; font-size: 14px;
            color: #60a5fa; padding: 4px; letter-spacing: 1px;
        }
        QListWidget::item { padding: 6px 12px; border-bottom: 1px solid #1e2535; }
        QListWidget::item:selected { background: #1e3a8a; color: white; border-radius: 4px; }
        QListWidget::item:hover { background: #1a2535; }
        QSpinBox {
            background: #141a28; border: 1px solid #2d3a55; border-radius: 6px;
            padding: 6px 10px; color: #c8d4f0; font-size: 13px;
        }
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #2563eb, stop:1 #1d4ed8);
            color: white; border: 1px solid #3b82f6; border-radius: 7px;
            padding: 9px 20px; font-size: 13px; font-weight: 700; min-width: 120px;
        }
        QPushButton:hover { background: #3b82f6; }
        QPushButton#btnObrisi { background: #7f1d1d; border-color: #ef4444; }
        QPushButton#btnObrisi:hover { background: #991b1b; }
        QPushButton#btnEmail { background: #1e3a8a; border-color: #60a5fa; color: #bfdbfe; }
        QPushButton#btnEmail:hover { background: #1d4ed8; }
        QPushButton#btnKopiraj, QPushButton#btnKopirajSve {
            background: #065f46; border-color: #34d399; color: #a7f3d0;
        }
        QPushButton#btnKopiraj:hover, QPushButton#btnKopirajSve:hover { background: #047857; }
    )");

    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *main = new QVBoxLayout(central);
    main->setContentsMargins(20, 18, 20, 18);
    main->setSpacing(14);

    // Header
    QLabel *lblTitle = new QLabel("🎬  VALTER — Generator Licencnih Kljuceva");
    lblTitle->setStyleSheet("font-size: 18px; font-weight: 800; color: #60a5fa;");
    QLabel *lblInfo = new QLabel("Generirani kljucevi su jedinstveni i validni za Valter program.");
    lblInfo->setStyleSheet("color: #3a5080; font-size: 12px;");
    main->addWidget(lblTitle);
    main->addWidget(lblInfo);

    QFrame *sep = new QFrame(); sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background: #2563eb; border: none; max-height: 2px;");
    main->addWidget(sep);

    // Controls
    QHBoxLayout *ctrl = new QHBoxLayout(); ctrl->setSpacing(12);
    ctrl->addWidget(new QLabel("Broj kljuceva:"));
    spnKolicina = new QSpinBox();
    spnKolicina->setRange(1, 100); spnKolicina->setValue(10);
    spnKolicina->setFixedWidth(80);
    ctrl->addWidget(spnKolicina);
    ctrl->addSpacing(10);
    btnGeneriraj = new QPushButton("⚡  Generiraj");
    ctrl->addWidget(btnGeneriraj);
    ctrl->addStretch();
    lblBroj = new QLabel("0 kljuceva");
    lblBroj->setStyleSheet("color: #4a5e85; font-size: 12px;");
    ctrl->addWidget(lblBroj);
    main->addLayout(ctrl);

    // Lista kljuceva
    lstKljucevi = new QListWidget();
    lstKljucevi->setSelectionMode(QAbstractItemView::ExtendedSelection);
    main->addWidget(lstKljucevi, 1);

    // Akcije
    QHBoxLayout *acts = new QHBoxLayout(); acts->setSpacing(10);
    btnKopiraj    = new QPushButton("📋  Kopiraj selektovani");  btnKopiraj->setObjectName("btnKopiraj");
    btnKopirajSve = new QPushButton("📋  Kopiraj sve");          btnKopirajSve->setObjectName("btnKopirajSve");
    btnEmail      = new QPushButton("📧  Pošalji na Email");     btnEmail->setObjectName("btnEmail");
    btnObrisi     = new QPushButton("✕  Obrisi listu");         btnObrisi->setObjectName("btnObrisi");
    acts->addWidget(btnKopiraj); acts->addWidget(btnKopirajSve); acts->addWidget(btnEmail); acts->addStretch(); acts->addWidget(btnObrisi);
    main->addLayout(acts);

    // Info panel
    QFrame *infoSep = new QFrame(); infoSep->setFrameShape(QFrame::HLine);
    infoSep->setStyleSheet("background: #1e2535; border: none; max-height: 1px;");
    main->addWidget(infoSep);

    QHBoxLayout *infoL = new QHBoxLayout(); infoL->setSpacing(20);
    QLabel *lblFormat = new QLabel("Format: <b style=\"color:#3b82f6\">VALTR-XXXXX-XXXXX-XXXXX-XXXXX</b>");
    lblFormat->setStyleSheet("color: #2a3a55; font-size: 11px;");
    QLabel *lblBinding = new QLabel("Svaki kljuc se veze za <b style=\"color:#f59e0b\">jedan racunar</b> pri prvoj aktivaciji.");
    lblBinding->setStyleSheet("color: #2a3a55; font-size: 11px;");
    infoL->addWidget(lblFormat); infoL->addWidget(lblBinding); infoL->addStretch();
    main->addLayout(infoL);

    connect(btnGeneriraj,    &QPushButton::clicked, this, &KeygenWindow::generiraj);
    connect(btnKopiraj,      &QPushButton::clicked, this, &KeygenWindow::kopirajSelektovani);
    connect(btnKopirajSve,   &QPushButton::clicked, this, &KeygenWindow::kopirajSve);
    connect(btnObrisi,       &QPushButton::clicked, this, &KeygenWindow::obrisi);
    connect(btnEmail,        &QPushButton::clicked, this, &KeygenWindow::posaljiEmail);

    if (auto *screen = QApplication::primaryScreen()) {
        QRect g = screen->availableGeometry();
        move((g.width()-width())/2, (g.height()-height())/2);
    }
}

void KeygenWindow::generiraj() {
    int n = spnKolicina->value();
    for (int i = 0; i < n; i++)
        lstKljucevi->addItem(License::generateKey());
    lblBroj->setText(QString("%1 kljuceva").arg(lstKljucevi->count()));
}
void KeygenWindow::kopirajSelektovani() {
    QStringList selected;
    for (auto *item : lstKljucevi->selectedItems())
        selected << item->text();
    if (selected.isEmpty()) { QMessageBox::information(this,"","Selektujte kljuceve iz liste."); return; }
    QApplication::clipboard()->setText(selected.join("\n"));
    QMessageBox::information(this,"Kopirano", QString("%1 kljuc(a) kopiran u clipboard.").arg(selected.size()));
}
void KeygenWindow::kopirajSve() {
    if (lstKljucevi->count() == 0) return;
    QStringList all;
    for (int i = 0; i < lstKljucevi->count(); i++)
        all << lstKljucevi->item(i)->text();
    QApplication::clipboard()->setText(all.join("\n"));
    QMessageBox::information(this,"Kopirano", QString("%1 kljuc(a) kopiran u clipboard.").arg(all.size()));
}
void KeygenWindow::obrisi() {
    lstKljucevi->clear();
    lblBroj->setText("0 kljuceva");
}

void KeygenWindow::posaljiEmail() {
    // Uzmi selektovani ili zadnji generisani kljuc
    QString kljuc;
    auto selected = lstKljucevi->selectedItems();
    if (!selected.isEmpty()) {
        kljuc = selected.first()->text();
    } else if (lstKljucevi->count() > 0) {
        kljuc = lstKljucevi->item(lstKljucevi->count()-1)->text();
    } else {
        QMessageBox::information(this, "Nema kljuca",
            "Najprijе generišite kljuc, pa ga selektujte iz liste.");
        return;
    }

    // Pitaj za email adresu kupca
    bool ok;
    QString email = QInputDialog::getText(this,
        "Pošalji licencni kljuc",
        "Email adresa kupca:",
        QLineEdit::Normal, "", &ok);

    if (!ok || email.trimmed().isEmpty()) return;
    email = email.trimmed();

    // Pripremi mailto link
    QString subject = "Valter - Licencni kljuc za aktivaciju";
    QString body =
        "Postovani,\n\n"
        "Hvala vam na kupovini programa Valter!\n\n"
        "Vas licencni kljuc za aktivaciju je:\n\n"
        "    " + kljuc + "\n\n"
        "Upute za aktivaciju:\n"
        "1. Pokrenite Valter program\n"
        "2. Pri prvom pokretanju ce se otvoriti prozor za aktivaciju\n"
        "3. Unesite kljuc tacno kako je naveden gore\n"
        "4. Kliknite 'Aktiviraj' - program je spreman za koristenje!\n\n"
        "U slucaju problema, javite se na ovaj email.\n\n"
        "Lijep pozdrav,\n"
        "KC Bar";

    // Enkoduj za URL
    QUrl mailUrl("mailto:" + email);
    QUrlQuery query;
    query.addQueryItem("subject", subject);
    query.addQueryItem("body", body);
    mailUrl.setQuery(query);

    bool opened = QDesktopServices::openUrl(mailUrl);
    if (!opened) {
        // Fallback - samo kopiraj u clipboard
        QApplication::clipboard()->setText(kljuc);
        QMessageBox::information(this, "Email klijent nije pronadjen",
            "Kljuc je kopiran u clipboard:\n\n" + kljuc +
            "\n\nNalijepite ga rucno u email.");
    }
}
