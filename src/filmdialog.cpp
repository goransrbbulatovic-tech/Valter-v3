#include "filmdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QFrame>

FilmDialog::FilmDialog(QWidget *parent, const Film *film)
    : QDialog(parent)
{
    setWindowTitle(film ? "Uredi Film" : "Dodaj Film");
    setFixedWidth(520);
    setModal(true);
    setupUI(film);
}

void FilmDialog::setupUI(const Film *film) {
    QVBoxLayout *main = new QVBoxLayout(this);
    main->setSpacing(12);
    main->setContentsMargins(20, 18, 20, 16);

    // Naslov
    QLabel *lblTitle = new QLabel(film ? "  Uredi podatke o filmu" : "  Unos novog filma");
    lblTitle->setStyleSheet("font-size:14px; font-weight:bold; color:#60a5fa;");
    main->addWidget(lblTitle);

    QFrame *sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background:#2a3040; border:none; max-height:1px;");
    main->addWidget(sep);

    QString inputStyle =
        "QLineEdit,QComboBox,QDateEdit,QTextEdit {"
        "  background:#0f1318; border:1px solid #3d4a62;"
        "  border-radius:4px; padding:5px 10px; color:#dce1eb; font-size:12px;}"
        "QLineEdit:focus,QComboBox:focus,QDateEdit:focus,QTextEdit:focus {border-color:#3b82f6;}"
        "QComboBox::drop-down {border:none;}"
        "QComboBox QAbstractItemView {background:#1c2028; color:#dce1eb;"
        "  selection-background-color:#1e40af; border:1px solid #3d4a62;}"
        "QDateEdit::drop-down {border:none;}";

    QString chkStyle =
        "QCheckBox {color:#dce1eb; font-size:12px;}"
        "QCheckBox::indicator {width:16px; height:16px; border:1px solid #3d4a62;"
        "  border-radius:3px; background:#0f1318;}"
        "QCheckBox::indicator:checked {background:#1d4ed8; border-color:#3b82f6;}";

    // ── Film ────────────────────────────────────────────────
    QGroupBox *grpFilm = new QGroupBox("Informacije o filmu");
    QFormLayout *fFilm = new QFormLayout(grpFilm);
    fFilm->setSpacing(9); fFilm->setContentsMargins(14,18,14,12);
    fFilm->setLabelAlignment(Qt::AlignRight|Qt::AlignVCenter);

    edNaziv = new QLineEdit();
    edNaziv->setPlaceholderText("Npr. Mission: Impossible");
    edNaziv->setStyleSheet(inputStyle);
    fFilm->addRow("Naziv filma:", edNaziv);

    cbFormat = new QComboBox();
    cbFormat->addItems({"2K Scope","4K Scope","2K Flat","4K Flat"});
    cbFormat->setStyleSheet(inputStyle);
    fFilm->addRow("Format:", cbFormat);

    chkSkinut = new QCheckBox("Film je skinut (preuzet)");
    chkSkinut->setStyleSheet(chkStyle);
    fFilm->addRow("Film skinut:", chkSkinut);

    chkInjestovan = new QCheckBox("Film je injestovan (ubacen u projektor)");
    chkInjestovan->setStyleSheet(chkStyle);
    fFilm->addRow("Injestovan:", chkInjestovan);

    main->addWidget(grpFilm);

    // ── Kljuc ───────────────────────────────────────────────
    QGroupBox *grpK = new QGroupBox("Informacije o kljucu");
    QFormLayout *fK = new QFormLayout(grpK);
    fK->setSpacing(9); fK->setContentsMargins(14,18,14,12);
    fK->setLabelAlignment(Qt::AlignRight|Qt::AlignVCenter);

    chkOtkljucan = new QCheckBox("Kljuc je otkljucan (primljen)");
    chkOtkljucan->setStyleSheet(
        "QCheckBox {color:#22c55e; font-size:12px; font-weight:bold;}"
        "QCheckBox::indicator {width:16px; height:16px; border:1px solid #3d4a62;"
        "  border-radius:3px; background:#0f1318;}"
        "QCheckBox::indicator:checked {background:#166534; border-color:#22c55e;}");
    fK->addRow("Status kljuca:", chkOtkljucan);

    chkBezOgranicenja = new QCheckBox("Otkljucan film — vrijedi NEOGRANICENO");
    chkBezOgranicenja->setStyleSheet(
        "QCheckBox {color:#a78bfa; font-size:12px; font-weight:bold;}"
        "QCheckBox::indicator {width:16px; height:16px; border:1px solid #3d4a62;"
        "  border-radius:3px; background:#0f1318;}"
        "QCheckBox::indicator:checked {background:#4c1d95; border-color:#a78bfa;}");
    fK->addRow("", chkBezOgranicenja);

    // Od datum
    deOd = new QDateEdit(QDate::currentDate());
    deOd->setCalendarPopup(true);
    deOd->setDisplayFormat("dd.MM.yyyy");
    deOd->setStyleSheet(inputStyle);
    fK->addRow("Kljuc vazi od:", deOd);

    // Do datum
    deDo = new QDateEdit(QDate::currentDate().addDays(30));
    deDo->setCalendarPopup(true);
    deDo->setDisplayFormat("dd.MM.yyyy");
    deDo->setStyleSheet(inputStyle);
    fK->addRow("Kljuc vazi do:", deDo);

    // Trajanje prikaz
    lblTrajanje = new QLabel();
    lblTrajanje->setStyleSheet("color:#94a3b8; font-size:11px; font-style:italic;");
    fK->addRow("Trajanje:", lblTrajanje);

    main->addWidget(grpK);

    // ── Napomena ────────────────────────────────────────────
    QGroupBox *grpN = new QGroupBox("Napomena (opcionalno)");
    QVBoxLayout *lN = new QVBoxLayout(grpN);
    lN->setContentsMargins(14,18,14,12);
    edNapomena = new QTextEdit();
    edNapomena->setMaximumHeight(60);
    edNapomena->setPlaceholderText("Unesi napomenu...");
    edNapomena->setStyleSheet(inputStyle);
    lN->addWidget(edNapomena);
    main->addWidget(grpN);

    // ── Dugmad ──────────────────────────────────────────────
    QHBoxLayout *bLayout = new QHBoxLayout();
    bLayout->addStretch();
    btnOtkazi = new QPushButton("Otkazi");
    btnOk     = new QPushButton("  Sacuvaj  ");
    btnOk->setObjectName("btnDodaj");
    btnOk->setDefault(true);
    bLayout->addWidget(btnOtkazi);
    bLayout->addWidget(btnOk);
    main->addLayout(bLayout);

    // Popuni ako edit
    if (film) {
        edNaziv->setText(film->naziv);
        int idx = cbFormat->findText(film->format);
        if (idx >= 0) cbFormat->setCurrentIndex(idx);
        chkOtkljucan->setChecked(film->kljucOtkljucan);
        chkBezOgranicenja->setChecked(film->bezOgranicenja);
        if (film->datumOd.isValid())     deOd->setDate(film->datumOd);
        if (film->isticaKljuca.isValid()) deDo->setDate(film->isticaKljuca);
        chkSkinut->setChecked(film->filmSkinut);
        chkInjestovan->setChecked(film->filmInjestovan);
        edNapomena->setPlainText(film->napomena);
    }

    // Connections
    connect(btnOk,             &QPushButton::clicked,    this, &FilmDialog::provjeriFormu);
    connect(btnOtkazi,         &QPushButton::clicked,    this, &QDialog::reject);
    connect(chkBezOgranicenja, &QCheckBox::toggled,      this, &FilmDialog::bezOgranicenjaToggled);
    connect(deOd,  &QDateEdit::dateChanged, this, &FilmDialog::datumOdPromijenjen);
    connect(deDo,  &QDateEdit::dateChanged, this, &FilmDialog::datumDoPromijenjen);
    connect(edNaziv, &QLineEdit::textChanged, this, [this](){
        btnOk->setEnabled(!edNaziv->text().trimmed().isEmpty());
    });

    // Inicijalni stanje
    bezOgranicenjaToggled(chkBezOgranicenja->isChecked());
    azurirajTrajanje();
}

void FilmDialog::bezOgranicenjaToggled(bool checked) {
    deOd->setEnabled(!checked);
    deDo->setEnabled(!checked);
    lblTrajanje->setText(checked ? "Neograniceno trajanje" : "");
    if (!checked) azurirajTrajanje();
}

void FilmDialog::datumOdPromijenjen(const QDate &d) {
    if (deDo->date() < d) deDo->setDate(d.addDays(1));
    azurirajTrajanje();
}

void FilmDialog::datumDoPromijenjen(const QDate &) {
    azurirajTrajanje();
}

void FilmDialog::azurirajTrajanje() {
    if (chkBezOgranicenja->isChecked()) return;
    int dana = deOd->date().daysTo(deDo->date());
    if (dana < 0) {
        lblTrajanje->setText("  Datum 'Do' mora biti nakon datuma 'Od'!");
        lblTrajanje->setStyleSheet("color:#ef4444; font-size:11px;");
    } else {
        lblTrajanje->setText(QString("Trajanje: %1 dan(a)  [%2 — %3]")
            .arg(dana)
            .arg(deOd->date().toString("dd.MM.yyyy"))
            .arg(deDo->date().toString("dd.MM.yyyy")));
        lblTrajanje->setStyleSheet("color:#22c55e; font-size:11px; font-weight:bold;");
    }
}

void FilmDialog::provjeriFormu() {
    if (edNaziv->text().trimmed().isEmpty()) return;
    if (!chkBezOgranicenja->isChecked() && deOd->date() > deDo->date()) return;
    accept();
}

Film FilmDialog::getFilm() const {
    Film f;
    f.naziv           = edNaziv->text().trimmed();
    f.format          = cbFormat->currentText();
    f.kljucOtkljucan  = chkOtkljucan->isChecked();
    f.bezOgranicenja  = chkBezOgranicenja->isChecked();
    f.datumOd         = deOd->date();
    f.isticaKljuca    = deDo->date();
    f.filmSkinut      = chkSkinut->isChecked();
    f.filmInjestovan  = chkInjestovan->isChecked();
    f.uArhivi         = false;
    f.napomena        = edNapomena->toPlainText().trimmed();
    return f;
}
