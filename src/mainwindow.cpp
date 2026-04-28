#include "mainwindow.h"
#include "filmdialog.h"

#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QTextCodec>
#include <QStandardPaths>
#include <QDir>
#include <QApplication>
#include <QScreen>
#include <QPalette>
#include <QFrame>
#include <QPrinter>
#include <QFontMetrics>
#include <QDateTime>
#include <QStatusBar>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>

// ─────────────────────────────────────────────────────────────────────────────
//  Boje po statusu
// ─────────────────────────────────────────────────────────────────────────────
static QColor bgForStatus(RowStatus s) {
    switch (s) {
        case RS_ZAKLJUCAN:   return QColor(32, 36, 48);
        case RS_NEOGRANICEN: return QColor(35, 28, 58);
        case RS_UREDU:       return QColor(16, 32, 22);
        case RS_UPOZORENJE:  return QColor(42, 32, 10);
        case RS_KRITICNO:    return QColor(52, 24, 8);
        case RS_ISTEKAO:     return QColor(48, 12, 12);
        case RS_ARHIVA:      return QColor(24, 26, 32);
        default:             return QColor(28, 32, 42);
    }
}
static QColor accentForStatus(RowStatus s) {
    switch (s) {
        case RS_ZAKLJUCAN:   return QColor(80, 95, 120);
        case RS_NEOGRANICEN: return QColor(139, 92, 246);
        case RS_UREDU:       return QColor(34, 197, 94);
        case RS_UPOZORENJE:  return QColor(245, 158, 11);
        case RS_KRITICNO:    return QColor(251, 146, 60);
        case RS_ISTEKAO:     return QColor(239, 68, 68);
        case RS_ARHIVA:      return QColor(60, 70, 90);
        default:             return QColor(96, 165, 250);
    }
}
static QColor fgForStatus(RowStatus s) {
    if (s == RS_ZAKLJUCAN || s == RS_ARHIVA) return QColor(110, 125, 150);
    return QColor(220, 228, 240);
}

// ─────────────────────────────────────────────────────────────────────────────
//  RowDelegate — uniformna selekcija cijelog reda
// ─────────────────────────────────────────────────────────────────────────────
void RowDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt,
                        const QModelIndex &idx) const
{
    p->save();
    QRect r = opt.rect;

    RowStatus st = (RowStatus)idx.data(Qt::UserRole).toInt();
    bool selected = opt.state & QStyle::State_Selected;
    bool hover    = opt.state & QStyle::State_MouseOver;

    QColor bg = selected ? QColor(37, 99, 235)
              : hover    ? bgForStatus(st).lighter(130)
                         : bgForStatus(st);

    // Pozadina
    p->fillRect(r, bg);

    // Lijeva akcentna traka (samo na prvoj koloni)
    if (idx.column() == 0) {
        QColor acc = selected ? QColor(96, 165, 250) : accentForStatus(st);
        p->fillRect(r.x(), r.y(), 4, r.height(), acc);
    }

    // Donja linija separatora
    p->setPen(QColor(38, 44, 58));
    p->drawLine(r.left(), r.bottom(), r.right(), r.bottom());

    // Tekst
    QColor fg = selected ? Qt::white : fgForStatus(st);
    p->setPen(fg);

    QFont f = opt.font;
    // Bold za status kolonu (kolona 9)
    if (idx.column() == 9) { f.setBold(true); f.setPointSize(f.pointSize()); }
    p->setFont(f);

    QString txt = idx.data(Qt::DisplayRole).toString();
    int pad = (idx.column() == 0) ? 14 : 6;
    Qt::Alignment al = (idx.column() == 0 || idx.column() == 10)
                     ? (Qt::AlignVCenter | Qt::AlignLeft)
                     : Qt::AlignCenter;
    p->drawText(r.adjusted(pad, 0, -4, 0), al, txt);

    p->restore();
}

// ─────────────────────────────────────────────────────────────────────────────
//  MainWindow
// ─────────────────────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Valter — Program za evidenciju filmova i kljuceva za bioskop");
    setMinimumSize(1200, 700);
    resize(1400, 800);
    setupStyle();
    setupUI();
    ucitajPodatke();
    popuniTabelu();
    popuniArhivu();
    azurirajStatusBar();
    azurirajBrojacTabova();
    provjeriIstekKljuceva();

    timerProvjera = new QTimer(this);
    connect(timerProvjera, &QTimer::timeout, this, &MainWindow::provjeriIstekKljuceva);
    timerProvjera->start(60000);

    if (auto *screen = QApplication::primaryScreen()) {
        QRect g = screen->availableGeometry();
        move((g.width()-width())/2, (g.height()-height())/2);
    }
}
MainWindow::~MainWindow() { sacuvajPodatke(); }

// ─────────────────────────────────────────────────────────────────────────────
//  Style
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupStyle() {
    qApp->setStyle("Fusion");

    QPalette p;
    p.setColor(QPalette::Window,          QColor(18, 22, 32));
    p.setColor(QPalette::WindowText,      QColor(220, 228, 240));
    p.setColor(QPalette::Base,            QColor(14, 18, 26));
    p.setColor(QPalette::AlternateBase,   QColor(24, 28, 40));
    p.setColor(QPalette::Text,            QColor(220, 228, 240));
    p.setColor(QPalette::Button,          QColor(36, 42, 58));
    p.setColor(QPalette::ButtonText,      QColor(220, 228, 240));
    p.setColor(QPalette::Highlight,       QColor(37, 99, 235));
    p.setColor(QPalette::HighlightedText, Qt::white);
    p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(80, 92, 115));
    qApp->setPalette(p);

    qApp->setStyleSheet(R"(
        * { font-family: 'Segoe UI', sans-serif; }

        QMainWindow { background: #12161e; }

        /* ── Tabs ──────────────────────────────────── */
        QTabWidget::pane {
            border: 1px solid #242b3d;
            background: #12161e;
            border-radius: 0 6px 6px 6px;
        }
        QTabBar::tab {
            background: #0e1118;
            color: #6b7fa8;
            padding: 11px 28px;
            border: 1px solid #1e2535;
            border-bottom: none;
            border-top-left-radius: 7px;
            border-top-right-radius: 7px;
            font-size: 13px;
            font-weight: 600;
            margin-right: 3px;
            min-width: 160px;
        }
        QTabBar::tab:selected {
            background: #12161e;
            color: #60a5fa;
            border-bottom: 3px solid #3b82f6;
        }
        QTabBar::tab:hover:!selected { background: #181e2e; color: #93c5fd; }

        /* ── Buttons ───────────────────────────────── */
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #2d3650, stop:1 #222840);
            color: #c8d4f0;
            border: 1px solid #3a4460;
            border-radius: 7px;
            padding: 9px 20px;
            font-size: 13px;
            font-weight: 600;
            min-width: 100px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #3a4870, stop:1 #2a3458);
            border-color: #5a7acc;
            color: #e0eaff;
        }
        QPushButton:pressed { background: #1e2845; }
        QPushButton:disabled { color: #3a4460; border-color: #242b3d; background: #181e2e; }

        QPushButton#btnDodaj {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #2563eb, stop:1 #1d4ed8);
            border-color: #3b82f6; color: #ffffff;
        }
        QPushButton#btnDodaj:hover {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #3b82f6, stop:1 #2563eb);
        }
        QPushButton#btnObrisi, QPushButton#btnObrisiArh {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #991b1b, stop:1 #7f1d1d);
            border-color: #ef4444; color: #fecaca;
        }
        QPushButton#btnObrisi:hover, QPushButton#btnObrisiArh:hover {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #b91c1c, stop:1 #991b1b);
        }
        QPushButton#btnArhiviraj {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #1e40af, stop:1 #1e3a8a);
            border-color: #60a5fa; color: #bfdbfe;
        }
        QPushButton#btnVratiArh {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #166534, stop:1 #14532d);
            border-color: #4ade80; color: #bbf7d0;
        }
        QPushButton#btnPDF {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #7e22ce, stop:1 #6b21a8);
            border-color: #a855f7; color: #e9d5ff;
        }
        QPushButton#btnCSV {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #065f46, stop:1 #064e3b);
            border-color: #34d399; color: #a7f3d0;
        }
        QPushButton#btnUvezi {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #92400e, stop:1 #78350f);
            border-color: #fbbf24; color: #fef3c7;
        }

        /* ── Table ─────────────────────────────────── */
        QTableWidget {
            background: #0e1118;
            gridline-color: transparent;
            border: 1px solid #1e2535;
            border-radius: 8px;
            font-size: 13px;
            outline: none;
        }
        QTableWidget::item { border: none; }
        QTableWidget::item:selected { background: transparent; }

        QHeaderView {
            background: #0a0d14;
            border-radius: 0;
        }
        QHeaderView::section {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #1a2035, stop:1 #131826);
            color: #7b92c4;
            padding: 10px 8px;
            border: none;
            border-right: 1px solid #1e2535;
            border-bottom: 2px solid #2563eb;
            font-weight: 700;
            font-size: 11px;
            letter-spacing: 0.8px;
            text-transform: uppercase;
        }
        QHeaderView::section:first { border-top-left-radius: 8px; }
        QHeaderView::section:last  { border-right: none; border-top-right-radius: 8px; }
        QHeaderView::section:hover { background: #1e2840; color: #93c5fd; }

        QScrollBar:vertical {
            background: #0e1118; width: 12px; border-radius: 6px;
        }
        QScrollBar::handle:vertical {
            background: #2d3a55; border-radius: 6px; min-height: 30px;
        }
        QScrollBar::handle:vertical:hover { background: #3b4e72; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
        QScrollBar:horizontal {
            background: #0e1118; height: 12px; border-radius: 6px;
        }
        QScrollBar::handle:horizontal {
            background: #2d3a55; border-radius: 6px; min-width: 30px;
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }

        /* ── Status Bar ────────────────────────────── */
        QStatusBar {
            background: #090c12;
            color: #4a5e85;
            font-size: 12px;
            border-top: 1px solid #1a2035;
            padding: 3px 8px;
        }

        /* ── Search ────────────────────────────────── */
        QLineEdit {
            background: #141a28;
            border: 1px solid #2d3a55;
            border-radius: 7px;
            padding: 8px 14px;
            color: #c8d4f0;
            font-size: 13px;
        }
        QLineEdit:focus { border-color: #3b82f6; background: #161d2e; }
        QLineEdit::placeholder { color: #3a4a6a; }

        /* ── GroupBox (dialozi) ─────────────────────── */
        QGroupBox {
            border: 1px solid #2d3a55;
            border-radius: 8px;
            margin-top: 14px;
            font-weight: 700;
            font-size: 12px;
            color: #6b82aa;
            padding-top: 6px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 14px; padding: 0 6px;
            background: #12161e;
        }

        /* ── Upozorenje labels ─────────────────────── */
        QLabel#lblUpozorenje {
            color: #fbbf24; font-size: 13px; font-weight: 700;
            padding: 7px 16px; background: #1c1507;
            border: 1px solid #92400e; border-radius: 7px;
        }
        QLabel#lblKriticno {
            color: #fca5a5; font-size: 13px; font-weight: 700;
            padding: 7px 16px; background: #1f0707;
            border: 1px solid #7f1d1d; border-radius: 7px;
        }
    )");
}

// ─────────────────────────────────────────────────────────────────────────────
//  UI Setup
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupUI() {
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *mainL = new QVBoxLayout(central);
    mainL->setSpacing(10);
    mainL->setContentsMargins(16, 14, 16, 10);

    // ── Header ─────────────────────────────────────────────
    QHBoxLayout *hdr = new QHBoxLayout();

    // Logo + title
    QLabel *lblIcon = new QLabel("🎬");
    lblIcon->setStyleSheet("font-size: 32px;");
    QVBoxLayout *ttl = new QVBoxLayout();
    QLabel *lblN = new QLabel("Valter");
    lblN->setStyleSheet("font-size: 24px; font-weight: 900; color: #60a5fa; letter-spacing: 1.5px;");
    QLabel *lblP = new QLabel("Program za evidenciju filmova i kljuceva za bioskop");
    lblP->setStyleSheet("font-size: 12px; color: #3a5080; font-weight: 500;");
    ttl->addWidget(lblN); ttl->addWidget(lblP); ttl->setSpacing(3);
    hdr->addWidget(lblIcon);
    hdr->addSpacing(10);
    hdr->addLayout(ttl);
    hdr->addStretch();

    lblUpozorenje = new QLabel();
    lblUpozorenje->setObjectName("lblUpozorenje");
    lblUpozorenje->setVisible(false);
    lblUpozorenje->setWordWrap(true);
    lblUpozorenje->setMaximumWidth(700);
    hdr->addWidget(lblUpozorenje);
    mainL->addLayout(hdr);

    // Separator
    QFrame *sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
                       "stop:0 #12161e, stop:0.3 #2563eb, stop:0.7 #2563eb, stop:1 #12161e);"
                       "border: none; max-height: 2px; margin: 2px 0;");
    mainL->addWidget(sep);

    // ── Toolbar ────────────────────────────────────────────
    QHBoxLayout *tb = new QHBoxLayout(); tb->setSpacing(8);

    btnDodaj     = new QPushButton("＋  Dodaj Film");   btnDodaj->setObjectName("btnDodaj");
    btnUredi     = new QPushButton("✎  Uredi");
    btnObrisi    = new QPushButton("✕  Obrisi");        btnObrisi->setObjectName("btnObrisi");
    btnArhiviraj = new QPushButton("📁  U Arhivu");     btnArhiviraj->setObjectName("btnArhiviraj");
    btnOsvezi    = new QPushButton("↻  Osvjezi");
    btnPDF       = new QPushButton("📄  PDF");          btnPDF->setObjectName("btnPDF");
    btnCSV       = new QPushButton("📊  Excel/CSV");    btnCSV->setObjectName("btnCSV");
    btnUvezi     = new QPushButton("📥  Uvezi");        btnUvezi->setObjectName("btnUvezi");

    btnUredi->setEnabled(false);
    btnObrisi->setEnabled(false);
    btnArhiviraj->setEnabled(false);

    tb->addWidget(btnDodaj); tb->addWidget(btnUredi); tb->addWidget(btnObrisi);
    tb->addWidget(btnArhiviraj); tb->addSpacing(8); tb->addWidget(btnOsvezi);
    tb->addSpacing(12); tb->addWidget(btnPDF); tb->addWidget(btnCSV); tb->addWidget(btnUvezi);
    tb->addStretch();

    // Pretraga
    QLabel *lblSrch = new QLabel("🔍");
    lblSrch->setStyleSheet("font-size: 15px;");
    edPretraga = new QLineEdit();
    edPretraga->setPlaceholderText("Pretrazi filmove...");
    edPretraga->setMinimumWidth(240);
    edPretraga->setMaximumWidth(300);
    tb->addWidget(lblSrch); tb->addWidget(edPretraga);
    mainL->addLayout(tb);

    // ── Legenda ────────────────────────────────────────────
    QHBoxLayout *leg = new QHBoxLayout(); leg->setSpacing(6);
    leg->addStretch();
    auto mkLeg = [&](const QString &col, const QString &lbl) {
        QLabel *dot = new QLabel();
        dot->setFixedSize(12, 12);
        dot->setStyleSheet(QString("background:%1; border-radius:6px;").arg(col));
        QLabel *txt = new QLabel(lbl);
        txt->setStyleSheet("color: #4a5e85; font-size: 12px;");
        leg->addWidget(dot); leg->addWidget(txt); leg->addSpacing(8);
    };
    mkLeg("#22c55e", "U redu");
    mkLeg("#f59e0b", "Upozorenje (≤7d)");
    mkLeg("#f97316", "Kriticno (≤1d)");
    mkLeg("#ef4444", "Istekao");
    mkLeg("#a855f7", "Neogranicen");
    mkLeg("#5a6f90", "Zakljucan");
    mainL->addLayout(leg);

    // ── Tab widget ─────────────────────────────────────────
    tabWidget = new QTabWidget(this);
    tabWidget->setDocumentMode(false);
    mainL->addWidget(tabWidget, 1);

    // Tab 1 — Aktivni
    QWidget *pgAkt = new QWidget();
    QVBoxLayout *laAkt = new QVBoxLayout(pgAkt);
    laAkt->setContentsMargins(0, 8, 0, 0); laAkt->setSpacing(0);
    tabela = new QTableWidget();
    tabela->setColumnCount(11);
    tabela->setHorizontalHeaderLabels({
        "Naziv Filma","Format","Kljuc","Od","Do",
        "Trajanje","Preostalo","Skinut","Injestovan","Status","Napomena"
    });
    setupTabela(tabela);
    laAkt->addWidget(tabela);
    tabWidget->addTab(pgAkt, "  Aktivni filmovi");

    // Tab 2 — Arhiva
    QWidget *pgArh = new QWidget();
    QVBoxLayout *laArh = new QVBoxLayout(pgArh);
    laArh->setContentsMargins(0, 8, 0, 0); laArh->setSpacing(8);

    QHBoxLayout *arhTb = new QHBoxLayout(); arhTb->setSpacing(8);
    btnVratiIzArhive = new QPushButton("↩  Vrati u aktivne"); btnVratiIzArhive->setObjectName("btnVratiArh");
    btnObrisiArhiva  = new QPushButton("✕  Trajno obrisi");   btnObrisiArhiva->setObjectName("btnObrisiArh");
    btnVratiIzArhive->setEnabled(false); btnObrisiArhiva->setEnabled(false);
    QLabel *lblArhInfo = new QLabel("Filmovi kojima je istekao kljuc ili su rucno premjesteni.");
    lblArhInfo->setStyleSheet("color: #3a4e6a; font-size: 12px; font-style: italic;");
    arhTb->addWidget(btnVratiIzArhive); arhTb->addWidget(btnObrisiArhiva);
    arhTb->addSpacing(16); arhTb->addWidget(lblArhInfo); arhTb->addStretch();
    laArh->addLayout(arhTb);

    tabelaArhiva = new QTableWidget();
    tabelaArhiva->setColumnCount(11);
    tabelaArhiva->setHorizontalHeaderLabels({
        "Naziv Filma","Format","Kljuc","Od","Do",
        "Trajanje","Preostalo","Skinut","Injestovan","Status","Napomena"
    });
    setupTabela(tabelaArhiva);
    laArh->addWidget(tabelaArhiva);
    tabWidget->addTab(pgArh, "  Arhiva");

    statusBar()->showMessage("Spreman  |  KC Bar");

    // Konekcije
    connect(btnDodaj,         &QPushButton::clicked, this, &MainWindow::dodajFilm);
    connect(btnUredi,         &QPushButton::clicked, this, &MainWindow::urediFilm);
    connect(btnObrisi,        &QPushButton::clicked, this, &MainWindow::obrisiFilm);
    connect(btnArhiviraj,     &QPushButton::clicked, this, &MainWindow::posaljiUArhivu);
    connect(btnVratiIzArhive, &QPushButton::clicked, this, &MainWindow::vratiIzArhive);
    connect(btnObrisiArhiva,  &QPushButton::clicked, this, &MainWindow::obrisiIzArhive);
    connect(btnOsvezi,        &QPushButton::clicked, this, &MainWindow::osveziTabelu);
    connect(btnPDF,           &QPushButton::clicked, this, &MainWindow::izvezuPDF);
    connect(btnCSV,           &QPushButton::clicked, this, &MainWindow::izvezuCSV);
    connect(btnUvezi,         &QPushButton::clicked, this, &MainWindow::uvezi);
    connect(edPretraga,       &QLineEdit::textChanged, this, &MainWindow::pretragaPromijenjena);
    connect(tabela,       &QTableWidget::itemSelectionChanged, this, &MainWindow::selekcijaPromijenjena);
    connect(tabela,       &QTableWidget::cellDoubleClicked,    this, &MainWindow::urediFilm);
    connect(tabelaArhiva, &QTableWidget::itemSelectionChanged, this, &MainWindow::selekcijaArhivePromijenjena);
    connect(tabWidget,    &QTabWidget::currentChanged,         this, &MainWindow::tabPromijenjena);
}

void MainWindow::setupTabela(QTableWidget *t) {
    // Delegat za uniformno bojanje
    t->setItemDelegate(new RowDelegate(t));
    t->setMouseTracking(true);

    // Sve kolone su Interactive — korisnik moze da vuce i mijenja sirinu
    for (int i = 0; i < 11; i++)
        t->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Interactive);

    // Naziv i Napomena se istežu, ostale fiksne default sirine
    t->horizontalHeader()->setSectionResizeMode(0,  QHeaderView::Stretch);
    t->horizontalHeader()->setSectionResizeMode(10, QHeaderView::Stretch);

    // Default sirine — dovoljno da se vidi pun tekst
    t->setColumnWidth(1,  92);   // Format
    t->setColumnWidth(2,  118);  // Kljuc — "Otkljucan" = 9 znakova
    t->setColumnWidth(3,  108);  // Od
    t->setColumnWidth(4,  108);  // Do
    t->setColumnWidth(5,  100);  // Trajanje
    t->setColumnWidth(6,  118);  // Preostalo
    t->setColumnWidth(7,   82);  // Skinut
    t->setColumnWidth(8,   96);  // Injestovan
    t->setColumnWidth(9,  118);  // Status

    // Minimalne sirine da se ne može smanjiti ispod čitljivog
    t->horizontalHeader()->setMinimumSectionSize(60);

    t->verticalHeader()->setVisible(false);
    t->setAlternatingRowColors(false); // Delegat preuzima bojanje
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setSelectionMode(QAbstractItemView::SingleSelection);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setSortingEnabled(true);
    t->setShowGrid(false);
    t->verticalHeader()->setDefaultSectionSize(44);
    t->horizontalHeader()->setCursor(Qt::SizeHorCursor);
    t->setStyleSheet("QTableWidget { border-radius: 8px; }"
                     "QHeaderView::section { cursor: col-resize; }");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────────────────────────
RowStatus MainWindow::rowStatus(const Film &f) const {
    if (f.uArhivi)          return RS_ARHIVA;
    if (!f.kljucOtkljucan)  return RS_ZAKLJUCAN;
    if (f.bezOgranicenja)   return RS_NEOGRANICEN;
    int d = danaDoIsteka(f);
    if (d < 0)  return RS_ISTEKAO;
    if (d <= 1) return RS_KRITICNO;
    if (d <= 7) return RS_UPOZORENJE;
    return RS_UREDU;
}

int MainWindow::danaDoIsteka(const Film &f) const {
    if (!f.kljucOtkljucan || f.bezOgranicenja) return 99999;
    return QDate::currentDate().daysTo(f.isticaKljuca);
}
int MainWindow::trajanjeDana(const Film &f) const {
    if (f.bezOgranicenja || !f.datumOd.isValid() || !f.isticaKljuca.isValid()) return 0;
    return f.datumOd.daysTo(f.isticaKljuca);
}
QString MainWindow::statusKljuca(const Film &f) const {
    if (!f.kljucOtkljucan)  return "Zakljucan";
    if (f.bezOgranicenja)   return "Neogranicen";
    int d = danaDoIsteka(f);
    if (d < 0)  return QString("Istekao (%1d)").arg(-d);
    if (d == 0) return "DANAS istice!";
    if (d == 1) return "SUTRA istice!";
    if (d <= 7) return QString("%1 dan(a)").arg(d);
    return QString("U redu (%1d)").arg(d);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Popuni red — samo tekst + UserRole, delegat boji sve
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::popuniRed(QTableWidget *tbl, int row, const Film &f) {
    RowStatus st = rowStatus(f);
    int dana = danaDoIsteka(f);
    int traj = trajanjeDana(f);

    auto mk = [&](const QString &txt) {
        QTableWidgetItem *it = new QTableWidgetItem(txt);
        it->setData(Qt::UserRole, (int)st);
        return it;
    };

    // Format boja (extra info u UserRole+1 nije podrzana, koristimo poseban role)
    auto mkFmt = [&](const QString &txt) {
        QTableWidgetItem *it = new QTableWidgetItem(txt);
        it->setData(Qt::UserRole, (int)st);
        // Sacuvaj format info za delegata
        it->setData(Qt::UserRole + 1, txt.contains("4K") ? 1 : 0);
        return it;
    };

    // 0 - Naziv
    tbl->setItem(row, 0, mk(f.naziv));

    // 1 - Format
    tbl->setItem(row, 1, mkFmt(f.format));

    // 2 - Kljuc — puni naziv uvijek
    QString kTxt = !f.kljucOtkljucan ? "Zakljucan"
                 : f.bezOgranicenja  ? "Neogranicen"
                                     : "Otkljucan";
    tbl->setItem(row, 2, mk(kTxt));

    // 3 - Od
    tbl->setItem(row, 3, mk(
        (!f.kljucOtkljucan || f.bezOgranicenja) ? "—"
        : f.datumOd.toString("dd.MM.yyyy")));

    // 4 - Do
    tbl->setItem(row, 4, mk(
        (!f.kljucOtkljucan || f.bezOgranicenja) ? "—"
        : f.isticaKljuca.toString("dd.MM.yyyy")));

    // 5 - Trajanje
    tbl->setItem(row, 5, mk(
        !f.kljucOtkljucan ? "—"
        : f.bezOgranicenja ? "∞"
        : QString("%1 dana").arg(traj)));

    // 6 - Preostalo
    QString danaTxt;
    if (!f.kljucOtkljucan)    danaTxt = "—";
    else if (f.bezOgranicenja) danaTxt = "∞";
    else if (dana < 0)         danaTxt = QString("Istekao  %1d").arg(-dana);
    else if (dana == 0)        danaTxt = "⚠  DANAS!";
    else if (dana == 1)        danaTxt = "⚠  SUTRA!";
    else if (dana <= 7)        danaTxt = QString("⚠  %1 dan(a)").arg(dana);
    else                       danaTxt = QString("%1 dana").arg(dana);
    tbl->setItem(row, 6, mk(danaTxt));

    // 7 - Skinut
    tbl->setItem(row, 7, mk(f.filmSkinut ? "✔  Da" : "✘  Ne"));

    // 8 - Injestovan
    tbl->setItem(row, 8, mk(f.filmInjestovan ? "✔  Da" : "✘  Ne"));

    // 9 - Status
    QString stTxt;
    switch (st) {
        case RS_ZAKLJUCAN:   stTxt = "ZAKLJUCAN";   break;
        case RS_NEOGRANICEN: stTxt = "NEOGRANICEN";  break;
        case RS_UREDU:       stTxt = "U REDU"; break;
        case RS_UPOZORENJE:  stTxt = "UPOZORENJE"; break;
        case RS_KRITICNO:    stTxt = "KRITICNO!"; break;
        case RS_ISTEKAO:     stTxt = "ISTEKAO"; break;
        case RS_ARHIVA:      stTxt = "ARHIVA";        break;
    }
    tbl->setItem(row, 9, mk(stTxt));

    // 10 - Napomena
    tbl->setItem(row, 10, mk(f.napomena));
}

void MainWindow::popuniTabelu(const QString &filter) {
    tabela->setSortingEnabled(false);
    QList<Film*> lista;
    for (auto &f : filmovi)
        if (!f.uArhivi && (filter.isEmpty() || f.naziv.contains(filter, Qt::CaseInsensitive)))
            lista.append(&f);
    tabela->setRowCount(lista.size());
    for (int i = 0; i < lista.size(); ++i)
        popuniRed(tabela, i, *lista[i]);
    tabela->setSortingEnabled(true);
}

void MainWindow::popuniArhivu(const QString &filter) {
    tabelaArhiva->setSortingEnabled(false);
    QList<Film*> lista;
    for (auto &f : filmovi)
        if (f.uArhivi && (filter.isEmpty() || f.naziv.contains(filter, Qt::CaseInsensitive)))
            lista.append(&f);
    tabelaArhiva->setRowCount(lista.size());
    for (int i = 0; i < lista.size(); ++i)
        popuniRed(tabelaArhiva, i, *lista[i]);
    tabelaArhiva->setSortingEnabled(true);
}

void MainWindow::azurirajStatusBar() {
    int uk=0, otk=0, upo=0, ist=0, arh=0;
    for (const auto &f : filmovi) {
        if (f.uArhivi) { arh++; continue; }
        uk++;
        if (!f.kljucOtkljucan || f.bezOgranicenja) continue;
        otk++;
        int d = danaDoIsteka(f);
        if (d < 0) ist++; else if (d <= 7) upo++;
    }
    statusBar()->showMessage(
        QString("  Aktivnih: %1   |   Otkljucanih: %2   |   Upozorenja: %3   |   Isteklih: %4   |   Arhiva: %5   |   Valter - KC Bar")
        .arg(uk).arg(otk).arg(upo).arg(ist).arg(arh));
}

void MainWindow::azurirajBrojacTabova() {
    int akt=0, arh=0;
    for (const auto &f : filmovi) { if (f.uArhivi) arh++; else akt++; }
    tabWidget->setTabText(0, QString("  Aktivni filmovi  (%1)  ").arg(akt));
    tabWidget->setTabText(1, QString("  Arhiva  (%1)  ").arg(arh));
}

void MainWindow::provjeriIstekKljuceva() {
    QStringList kriticno, upoz;
    for (const auto &f : filmovi) {
        if (f.uArhivi || !f.kljucOtkljucan || f.bezOgranicenja) continue;
        int d = danaDoIsteka(f);
        if (d < 0)       kriticno << QString("%1 (istekao %2d)").arg(f.naziv).arg(-d);
        else if (d == 0) kriticno << QString("%1 (DANAS!)").arg(f.naziv);
        else if (d == 1) kriticno << QString("%1 (SUTRA!)").arg(f.naziv);
        else if (d <= 7) upoz     << QString("%1 (%2d)").arg(f.naziv).arg(d);
    }
    if (!kriticno.isEmpty()) {
        lblUpozorenje->setObjectName("lblKriticno");
        lblUpozorenje->setStyle(lblUpozorenje->style());
        lblUpozorenje->setText("KRITICNO: " + kriticno.join("   |   "));
        lblUpozorenje->setVisible(true);
    } else if (!upoz.isEmpty()) {
        lblUpozorenje->setObjectName("lblUpozorenje");
        lblUpozorenje->setStyle(lblUpozorenje->style());
        lblUpozorenje->setText("UPOZORENJE: " + upoz.join("   |   "));
        lblUpozorenje->setVisible(true);
    } else {
        lblUpozorenje->setVisible(false);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  CRUD
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::dodajFilm() {
    FilmDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        filmovi.append(dlg.getFilm());
        sacuvajPodatke(); popuniTabelu(); popuniArhivu();
        azurirajStatusBar(); azurirajBrojacTabova(); provjeriIstekKljuceva();
    }
}
void MainWindow::urediFilm() {
    int row = tabela->currentRow();
    if (row < 0) return;
    QString naziv = tabela->item(row, 0)->text().trimmed();
    for (auto &f : filmovi) {
        if (!f.uArhivi && f.naziv == naziv) {
            FilmDialog dlg(this, &f);
            if (dlg.exec() == QDialog::Accepted) {
                bool bio_arhiva = f.uArhivi;
                f = dlg.getFilm(); f.uArhivi = bio_arhiva;
                sacuvajPodatke(); popuniTabelu(edPretraga->text());
                azurirajStatusBar(); azurirajBrojacTabova(); provjeriIstekKljuceva();
            }
            return;
        }
    }
}
void MainWindow::obrisiFilm() {
    int row = tabela->currentRow();
    if (row < 0) return;
    QString naziv = tabela->item(row, 0)->text().trimmed();
    if (QMessageBox::question(this, "Brisanje filma",
            QString("Obrisati film <b>%1</b>?").arg(naziv),
            QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        filmovi.erase(std::remove_if(filmovi.begin(), filmovi.end(),
            [&](const Film &f){ return !f.uArhivi && f.naziv == naziv; }), filmovi.end());
        sacuvajPodatke(); popuniTabelu(edPretraga->text());
        azurirajStatusBar(); azurirajBrojacTabova(); provjeriIstekKljuceva();
    }
}
void MainWindow::posaljiUArhivu() {
    int row = tabela->currentRow();
    if (row < 0) return;
    QString naziv = tabela->item(row, 0)->text().trimmed();
    for (auto &f : filmovi) {
        if (!f.uArhivi && f.naziv == naziv) {
            f.uArhivi = true;
            sacuvajPodatke(); popuniTabelu(edPretraga->text()); popuniArhivu();
            azurirajStatusBar(); azurirajBrojacTabova(); provjeriIstekKljuceva();
            statusBar()->showMessage(QString("Film \"%1\" premjesten u arhivu.").arg(naziv), 4000);
            return;
        }
    }
}
void MainWindow::vratiIzArhive() {
    int row = tabelaArhiva->currentRow();
    if (row < 0) return;
    QString naziv = tabelaArhiva->item(row, 0)->text().trimmed();
    for (auto &f : filmovi) {
        if (f.uArhivi && f.naziv == naziv) {
            f.uArhivi = false;
            sacuvajPodatke(); popuniTabelu(); popuniArhivu();
            azurirajStatusBar(); azurirajBrojacTabova(); provjeriIstekKljuceva();
            statusBar()->showMessage(QString("Film \"%1\" vracen u aktivne.").arg(naziv), 4000);
            return;
        }
    }
}
void MainWindow::obrisiIzArhive() {
    int row = tabelaArhiva->currentRow();
    if (row < 0) return;
    QString naziv = tabelaArhiva->item(row, 0)->text().trimmed();
    if (QMessageBox::question(this, "Trajno brisanje",
            QString("Trajno obrisati <b>%1</b> iz arhive?").arg(naziv),
            QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        filmovi.erase(std::remove_if(filmovi.begin(), filmovi.end(),
            [&](const Film &f){ return f.uArhivi && f.naziv == naziv; }), filmovi.end());
        sacuvajPodatke(); popuniArhivu();
        azurirajStatusBar(); azurirajBrojacTabova();
    }
}
void MainWindow::osveziTabelu() {
    popuniTabelu(edPretraga->text()); popuniArhivu(edPretraga->text());
    azurirajStatusBar(); azurirajBrojacTabova(); provjeriIstekKljuceva();
}
void MainWindow::selekcijaPromijenjena() {
    bool s = tabela->currentRow() >= 0;
    btnUredi->setEnabled(s); btnObrisi->setEnabled(s); btnArhiviraj->setEnabled(s);
}
void MainWindow::selekcijaArhivePromijenjena() {
    bool s = tabelaArhiva->currentRow() >= 0;
    btnVratiIzArhive->setEnabled(s); btnObrisiArhiva->setEnabled(s);
}
void MainWindow::pretragaPromijenjena(const QString &txt) {
    popuniTabelu(txt); popuniArhivu(txt);
}
void MainWindow::tabPromijenjena(int) {
    btnUredi->setEnabled(false); btnObrisi->setEnabled(false); btnArhiviraj->setEnabled(false);
    btnVratiIzArhive->setEnabled(false); btnObrisiArhiva->setEnabled(false);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Export PDF — modern dark theme
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::izvezuPDF() {
    QString path = QFileDialog::getSaveFileName(this, "Izvezi PDF",
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
        + "/Valter_Evidencija_" + QDate::currentDate().toString("dd-MM-yyyy") + ".pdf",
        "PDF fajlovi (*.pdf)");
    if (path.isEmpty()) return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(path);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setPageMargins(QMarginsF(12,12,12,12), QPageLayout::Millimeter);

    QPainter p(&printer);
    if (!p.isActive()) { QMessageBox::warning(this,"Greska","Nije moguce kreirati PDF."); return; }

    // Dimenzije stranice
    QRectF page = printer.pageRect(QPrinter::DevicePixel);
    qreal W = page.width();
    qreal H = page.height();
    qreal x0 = 0, y = 0;

    // ── Pozadina cijele stranice ──────────────────────────────
    p.fillRect(page, QColor(14, 18, 28));

    // ── Header blok ───────────────────────────────────────────
    qreal hdrH = 80;
    // Gradijent header
    QLinearGradient hdrGrad(0, 0, W, hdrH);
    hdrGrad.setColorAt(0.0, QColor(15, 21, 42));
    hdrGrad.setColorAt(0.5, QColor(26, 38, 70));
    hdrGrad.setColorAt(1.0, QColor(15, 21, 42));
    p.fillRect(QRectF(0, 0, W, hdrH), hdrGrad);

    // Plava linija ispod headera
    p.fillRect(QRectF(0, hdrH, W, 3), QColor(37, 99, 235));

    // Zlatni akcenat lijevo
    p.fillRect(QRectF(0, 0, 5, hdrH), QColor(212, 175, 55));

    // Naslov
    p.setFont(QFont("Arial", 22, QFont::Bold));
    p.setPen(QColor(96, 165, 250));
    p.drawText(QRectF(24, 8, W-200, 40), Qt::AlignVCenter | Qt::AlignLeft, "VALTER");

    p.setFont(QFont("Arial", 11));
    p.setPen(QColor(71, 100, 155));
    p.drawText(QRectF(24, 42, W-200, 28), Qt::AlignVCenter | Qt::AlignLeft,
               "Program za evidenciju filmova i kljuceva za bioskop  —  KC Bar");

    // Datum i statistika desno
    int uk=0, upo=0, ist=0;
    for (const auto &f : filmovi) {
        if (f.uArhivi) continue; uk++;
        if (!f.kljucOtkljucan || f.bezOgranicenja) continue;
        int d = danaDoIsteka(f);
        if (d < 0) ist++; else if (d <= 7) upo++;
    }
    p.setFont(QFont("Arial", 9));
    p.setPen(QColor(55, 80, 130));
    QString info = QString("Generirano: %1     Aktivnih: %2     Upozorenja: %3     Isteklih: %4")
        .arg(QDateTime::currentDateTime().toString("dd.MM.yyyy  hh:mm"))
        .arg(uk).arg(upo).arg(ist);
    p.drawText(QRectF(24, 65, W-40, 20), Qt::AlignVCenter | Qt::AlignLeft, info);

    y = hdrH + 3 + 14; // ispod headera + razmak

    // ── Definicija kolona ─────────────────────────────────────
    struct Col { QString name; qreal w; Qt::Alignment al; };
    QVector<Col> cols = {
        {"NAZIV FILMA",  200, Qt::AlignLeft   | Qt::AlignVCenter},
        {"FORMAT",        72, Qt::AlignCenter | Qt::AlignVCenter},
        {"KLJUC",         88, Qt::AlignCenter | Qt::AlignVCenter},
        {"OD",            82, Qt::AlignCenter | Qt::AlignVCenter},
        {"DO",            82, Qt::AlignCenter | Qt::AlignVCenter},
        {"TRAJANJE",      78, Qt::AlignCenter | Qt::AlignVCenter},
        {"PREOSTALO",     88, Qt::AlignCenter | Qt::AlignVCenter},
        {"SKINUT",        62, Qt::AlignCenter | Qt::AlignVCenter},
        {"INJESTOVAN",    75, Qt::AlignCenter | Qt::AlignVCenter},
        {"STATUS",        88, Qt::AlignCenter | Qt::AlignVCenter},
        {"NAPOMENA",       0, Qt::AlignLeft   | Qt::AlignVCenter},
    };
    // Napomena dobija ostatak prostora
    qreal usedW = 0;
    for (int i = 0; i < 10; i++) usedW += cols[i].w;
    cols[10].w = W - usedW - 12;

    // ── Zaglavlje tabele ──────────────────────────────────────
    qreal rowH = 26;

    // Pozadina zaglavlja
    QLinearGradient thGrad(0, y, 0, y + rowH);
    thGrad.setColorAt(0, QColor(22, 32, 58));
    thGrad.setColorAt(1, QColor(16, 24, 44));
    p.fillRect(QRectF(x0, y, W, rowH), thGrad);
    // Donja linija zaglavlja - plava
    p.fillRect(QRectF(x0, y + rowH - 2, W, 2), QColor(37, 99, 235));

    p.setFont(QFont("Arial", 7, QFont::Bold));
    p.setPen(QColor(96, 140, 210));
    qreal cx = x0 + 6;
    for (const auto &col : cols) {
        p.drawText(QRectF(cx, y, col.w - 4, rowH), col.al, col.name);
        cx += col.w;
    }
    y += rowH;

    // ── Redovi tabele ─────────────────────────────────────────
    p.setFont(QFont("Arial", 8));
    bool alt = false;
    for (const auto &f : filmovi) {
        if (f.uArhivi) continue;

        if (y + rowH > H - 20) {
            // Nova stranica
            printer.newPage();
            p.fillRect(QRectF(0, 0, W, H), QColor(14, 18, 28));
            y = 20;
        }

        // Boja reda po statusu
        RowStatus st = rowStatus(f);
        QColor bgRow;
        switch (st) {
            case RS_UREDU:       bgRow = alt ? QColor(16,30,20)  : QColor(14,26,18);  break;
            case RS_UPOZORENJE:  bgRow = alt ? QColor(38,28,8)   : QColor(32,24,6);   break;
            case RS_KRITICNO:    bgRow = alt ? QColor(48,22,6)   : QColor(40,18,4);   break;
            case RS_ISTEKAO:     bgRow = alt ? QColor(44,10,10)  : QColor(36,8,8);    break;
            case RS_NEOGRANICEN: bgRow = alt ? QColor(32,20,52)  : QColor(26,16,44);  break;
            default:             bgRow = alt ? QColor(26,30,42)  : QColor(20,24,36);  break;
        }
        alt = !alt;

        p.fillRect(QRectF(x0, y, W, rowH), bgRow);

        // Lijeva akcentna traka
        QColor acc = accentForStatus(st);
        p.fillRect(QRectF(x0, y, 3, rowH), acc);

        // Separator linija
        p.fillRect(QRectF(x0, y + rowH - 1, W, 1), QColor(28, 35, 52));

        // Status boja teksta
        int dana = danaDoIsteka(f);

        QStringList vals;
        vals << f.naziv
             << f.format
             << (!f.kljucOtkljucan ? "Zakljucan" : f.bezOgranicenja ? "Neogranicen" : "Otkljucan")
             << ((!f.kljucOtkljucan||f.bezOgranicenja) ? "—" : f.datumOd.toString("dd.MM.yyyy"))
             << ((!f.kljucOtkljucan||f.bezOgranicenja) ? "—" : f.isticaKljuca.toString("dd.MM.yyyy"))
             << (!f.kljucOtkljucan ? "—" : f.bezOgranicenja ? "∞" : QString("%1 dana").arg(trajanjeDana(f)))
             << (!f.kljucOtkljucan ? "—" : f.bezOgranicenja ? "∞" :
                 (dana<0 ? QString("Istekao %1d").arg(-dana) : (dana==0?"DANAS":
                  dana==1?"SUTRA":QString("%1 dana").arg(dana))))
             << (f.filmSkinut     ? "Da" : "Ne")
             << (f.filmInjestovan ? "Da" : "Ne")
             << statusKljuca(f)
             << f.napomena;

        cx = x0 + 6;
        for (int i = 0; i < cols.size(); i++) {
            QColor fg;
            if (i == 9) {  // Status kolona — obojena
                fg = accentForStatus(st);
                p.setFont(QFont("Arial", 8, QFont::Bold));
            } else if (i == 7 || i == 8) {  // Skinut/Injestovan
                fg = (vals[i] == "Da") ? QColor(34, 197, 94) : QColor(239, 68, 68);
                p.setFont(QFont("Arial", 8));
            } else {
                fg = QColor(190, 208, 235);
                p.setFont(QFont("Arial", 8));
            }
            p.setPen(fg);
            // Clip tekst u kolonu
            p.save();
            p.setClipRect(QRectF(cx, y, cols[i].w - 4, rowH));
            p.drawText(QRectF(cx, y, cols[i].w - 4, rowH), cols[i].al, vals[i]);
            p.restore();
            cx += cols[i].w;
        }
        y += rowH;
    }

    // ── Footer ────────────────────────────────────────────────
    p.fillRect(QRectF(0, H-22, W, 22), QColor(10, 14, 22));
    p.fillRect(QRectF(0, H-22, W, 1), QColor(30, 42, 70));
    p.setFont(QFont("Arial", 8));
    p.setPen(QColor(40, 60, 100));
    p.drawText(QRectF(10, H-20, W/2, 18), Qt::AlignVCenter | Qt::AlignLeft,
               "Valter — Program za evidenciju filmova i kljuceva za bioskop — KC Bar");
    p.drawText(QRectF(W/2, H-20, W/2-10, 18), Qt::AlignVCenter | Qt::AlignRight,
               QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm"));

    p.end();
    QMessageBox::information(this, "PDF Izvezen",
        QString("PDF uspjesno sacuvan:\n%1").arg(path));
}

// ─────────────────────────────────────────────────────────────────────────────
//  Export CSV/Excel
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::izvezuCSV() {
    QMessageBox fmtBox(this);
    fmtBox.setWindowTitle("Odaberi format");
    fmtBox.setText("Odaberite format za export:");
    QPushButton *btnXls = fmtBox.addButton("  Excel (.xls)", QMessageBox::AcceptRole);
    QPushButton *btnCsv = fmtBox.addButton("  CSV (.csv)",   QMessageBox::AcceptRole);
    fmtBox.addButton("Otkazi", QMessageBox::RejectRole);
    fmtBox.exec();
    bool doExcel = (fmtBox.clickedButton() == btnXls);
    bool doCsv   = (fmtBox.clickedButton() == btnCsv);
    if (!doExcel && !doCsv) return;

    if (doExcel) {
        QString path = QFileDialog::getSaveFileName(this, "Izvezi Excel",
            QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
            + "/Valter_" + QDate::currentDate().toString("dd-MM-yyyy") + ".xls",
            "Excel (*.xls)");
        if (path.isEmpty()) return;
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return;
        QTextStream o(&f);
        o.setCodec("UTF-8");

        // Count active
        int uk=0, upo=0, ist=0;
        for (const auto &fi : filmovi) {
            if (fi.uArhivi) continue; uk++;
            if (!fi.kljucOtkljucan || fi.bezOgranicenja) continue;
            int d=danaDoIsteka(fi); if(d<0) ist++; else if(d<=7) upo++;
        }

        QString nl = "\n";
        o << "<html xmlns:o='urn:schemas-microsoft-com:office:office'"
             " xmlns:x='urn:schemas-microsoft-com:office:excel'"
             " xmlns='http://www.w3.org/TR/REC-html40'>" << nl;
        o << "<head><meta charset='UTF-8'><style>" << nl;
        o << "body{font-family:Calibri,Arial,sans-serif;background:#0e1118}" << nl;
        o << "table{border-collapse:collapse;width:100%}" << nl;
        o << "th{background:#1a2035;color:#7b92c4;font-size:9pt;font-weight:bold;"
             "padding:10px 8px;border:1px solid #2d3a55;border-bottom:3px solid #2563eb;"
             "text-align:center;white-space:nowrap}" << nl;
        o << "td{font-size:10pt;padding:8px 10px;border:1px solid #1e2535;white-space:nowrap}" << nl;
        o << ".ru{background:#0f2016;color:#d0ecd8}" << nl;
        o << ".up{background:#261e06;color:#f0d8a0}" << nl;
        o << ".kr{background:#2a1204;color:#f8c8a0}" << nl;
        o << ".is{background:#280808;color:#f0a0a0}" << nl;
        o << ".ne{background:#1e1030;color:#d4c0f8}" << nl;
        o << ".za{background:#161a28;color:#7890b0}" << nl;
        o << ".gr{color:#22c55e;text-align:center;font-weight:bold}" << nl;
        o << ".rd{color:#ef4444;text-align:center;font-weight:bold}" << nl;
        o << ".am{color:#f59e0b;text-align:center;font-weight:bold}" << nl;
        o << ".pu{color:#a855f7;text-align:center;font-weight:bold}" << nl;
        o << ".ct{text-align:center}" << nl;
        o << ".hdr td{background:#0a0e18;color:#60a5fa;font-size:14pt;"
             "font-weight:bold;padding:14px 12px;border:none;border-bottom:3px solid #2563eb}" << nl;
        o << ".sub td{background:#0a0e18;color:#2d4470;font-size:9pt;"
             "padding:4px 12px 10px;border:none}" << nl;
        o << "</style></head><body><table>" << nl;

        o << "<tr class='hdr'><td colspan='11'>VALTER &mdash; KC Bar &mdash; "
             "Evidencija filmova i kljuceva za bioskop</td></tr>" << nl;
        o << "<tr class='sub'><td colspan='11'>Generirano: "
          << QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm")
          << " | Aktivnih: " << uk << " | Upozorenja: " << upo
          << " | Isteklih: " << ist << "</td></tr>" << nl;

        o << "<tr><th>NAZIV FILMA</th><th>FORMAT</th><th>KLJUC</th>"
             "<th>OD</th><th>DO</th><th>TRAJANJE</th><th>PREOSTALO</th>"
             "<th>SKINUT</th><th>INJESTOVAN</th><th>STATUS</th><th>NAPOMENA</th></tr>" << nl;

        for (const auto &fi : filmovi) {
            if (fi.uArhivi) continue;
            RowStatus st = rowStatus(fi);
            QString rc, sc;
            switch(st){
                case RS_UREDU:       rc="ru"; sc="gr"; break;
                case RS_UPOZORENJE:  rc="up"; sc="am"; break;
                case RS_KRITICNO:    rc="kr"; sc="am"; break;
                case RS_ISTEKAO:     rc="is"; sc="rd"; break;
                case RS_NEOGRANICEN: rc="ne"; sc="pu"; break;
                default:             rc="za"; sc="";   break;
            }
            int dana = danaDoIsteka(fi);
            QString pre;
            if (!fi.kljucOtkljucan)    pre="&mdash;";
            else if (fi.bezOgranicenja) pre="&infin;";
            else if (dana<0)            pre=QString("Istekao %1d").arg(-dana);
            else if (dana==0)           pre="DANAS";
            else if (dana==1)           pre="SUTRA";
            else                        pre=QString("%1 dana").arg(dana);

            QString od = (!fi.kljucOtkljucan||fi.bezOgranicenja) ? "&mdash;" : fi.datumOd.toString("dd.MM.yyyy");
            QString doo= (!fi.kljucOtkljucan||fi.bezOgranicenja) ? "&mdash;" : fi.isticaKljuca.toString("dd.MM.yyyy");
            QString tr = !fi.kljucOtkljucan ? "&mdash;" : fi.bezOgranicenja ? "&infin;" : QString("%1 dana").arg(trajanjeDana(fi));
            QString kl = !fi.kljucOtkljucan ? "Zakljucan" : fi.bezOgranicenja ? "Neogranicen" : "Otkljucan";

            o << "<tr class='" << rc << "'>"
              << "<td>" << fi.naziv.toHtmlEscaped() << "</td>"
              << "<td class='ct'>" << fi.format << "</td>"
              << "<td class='ct'>" << kl << "</td>"
              << "<td class='ct'>" << od  << "</td>"
              << "<td class='ct'>" << doo << "</td>"
              << "<td class='ct'>" << tr  << "</td>"
              << "<td class='" << sc << "'>" << pre << "</td>"
              << "<td class='" << (fi.filmSkinut?"gr":"rd") << "'>" << (fi.filmSkinut?"Da":"Ne") << "</td>"
              << "<td class='" << (fi.filmInjestovan?"gr":"rd") << "'>" << (fi.filmInjestovan?"Da":"Ne") << "</td>"
              << "<td class='" << sc << "'>" << statusKljuca(fi) << "</td>"
              << "<td>" << fi.napomena.toHtmlEscaped() << "</td>"
              << "</tr>" << nl;
        }
        o << "</table></body></html>" << nl;
        f.close();
        QMessageBox::information(this,"Excel Izvezen",
            QString("Excel sacuvan:\n%1\n\nOtvorite u Microsoft Excelu.").arg(path));

    } else {
        QString path = QFileDialog::getSaveFileName(this, "Izvezi CSV",
            QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
            + "/Valter_" + QDate::currentDate().toString("dd-MM-yyyy") + ".csv",
            "CSV (*.csv)");
        if (path.isEmpty()) return;
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return;
        QTextStream o(&f);
        o.setCodec("UTF-8");
        o << "\xEF\xBB\xBF";
        o << "Naziv;Format;Kljuc;Bez ogranicenja;Od;Do;Trajanje;Preostalo;Skinut;Injestovan;Arhiva;Napomena\n";
        for (const auto &fi : filmovi) {
            int d = danaDoIsteka(fi);
            QString pre = fi.bezOgranicenja ? "Neograniceno"
                : (fi.kljucOtkljucan ? QString::number(d) : "-");
            o << "\"" << fi.naziv << "\";" << fi.format << ";"
              << (fi.kljucOtkljucan?"Da":"Ne") << ";"
              << (fi.bezOgranicenja?"Da":"Ne") << ";"
              << (fi.datumOd.isValid()?fi.datumOd.toString("dd.MM.yyyy"):"") << ";"
              << (fi.isticaKljuca.isValid()?fi.isticaKljuca.toString("dd.MM.yyyy"):"") << ";"
              << (fi.bezOgranicenja?"Neograniceno":QString::number(trajanjeDana(fi))) << ";"
              << pre << ";"
              << (fi.filmSkinut?"Da":"Ne") << ";"
              << (fi.filmInjestovan?"Da":"Ne") << ";"
              << (fi.uArhivi?"Da":"Ne") << ";"
              << "\"" << fi.napomena << "\"\n";
        }
        f.close();
        QMessageBox::information(this,"CSV Izvezen",
            QString("CSV sacuvan:\n%1").arg(path));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Uvezi
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::uvezi() {
    QString path = QFileDialog::getOpenFileName(this, "Uvezi podatke",
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
        "JSON fajlovi (*.json);;CSV fajlovi (*.csv)");
    if (path.isEmpty()) return;
    int uvezeno = 0;
    if (path.endsWith(".json", Qt::CaseInsensitive)) {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) { QMessageBox::warning(this,"Greska","Ne mogu otvoriti JSON."); return; }
        QJsonDocument doc = QJsonDocument::fromJson(f.readAll()); f.close();
        if (!doc.isArray()) { QMessageBox::warning(this,"Greska","Neispravan JSON format."); return; }
        for (const QJsonValue &v : doc.array()) {
            QJsonObject o = v.toObject();
            Film film;
            film.naziv          = o["naziv"].toString();
            film.format         = o["format"].toString();
            film.kljucOtkljucan = o["kljucOtkljucan"].toBool();
            film.bezOgranicenja = o["bezOgranicenja"].toBool();
            film.datumOd        = QDate::fromString(o["datumOd"].toString(), Qt::ISODate);
            film.isticaKljuca   = QDate::fromString(o["isticaKljuca"].toString(), Qt::ISODate);
            film.filmSkinut     = o["filmSkinut"].toBool();
            film.filmInjestovan = o["filmInjestovan"].toBool();
            film.uArhivi        = o["uArhivi"].toBool();
            film.napomena       = o["napomena"].toString();
            if (!film.naziv.isEmpty()) { filmovi.append(film); uvezeno++; }
        }
    } else {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly|QIODevice::Text)) { QMessageBox::warning(this,"Greska","Ne mogu otvoriti CSV."); return; }
        QTextStream in(&f);
        in.readLine();
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty()) continue;
            QStringList cols = line.split(";");
            if (cols.size() < 9) continue;
            Film film;
            film.naziv          = cols[0].remove('"');
            film.format         = cols[1];
            film.kljucOtkljucan = (cols[2]=="Da");
            film.bezOgranicenja = (cols[3]=="Da");
            film.datumOd        = QDate::fromString(cols[4],"dd.MM.yyyy");
            film.isticaKljuca   = QDate::fromString(cols[5],"dd.MM.yyyy");
            film.filmSkinut     = (cols[8]=="Da");
            film.filmInjestovan = (cols.size()>9 && cols[9]=="Da");
            film.uArhivi        = (cols.size()>10 && cols[10]=="Da");
            film.napomena       = (cols.size()>11 ? cols[11].remove('"') : "");
            if (!film.naziv.isEmpty()) { filmovi.append(film); uvezeno++; }
        }
        f.close();
    }
    sacuvajPodatke(); popuniTabelu(); popuniArhivu();
    azurirajStatusBar(); azurirajBrojacTabova(); provjeriIstekKljuceva();
    QMessageBox::information(this,"Uvoz Zavrsen", QString("Uvezeno %1 film(ova).").arg(uvezeno));
}

// ─────────────────────────────────────────────────────────────────────────────
//  Perzistencija
// ─────────────────────────────────────────────────────────────────────────────
QString MainWindow::dataFilePath() const {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/evidencija.json";
}
void MainWindow::ucitajPodatke() {
    QFile f(dataFilePath());
    if (!f.open(QIODevice::ReadOnly)) return;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll()); f.close();
    filmovi.clear();
    for (const QJsonValue &v : doc.array()) {
        QJsonObject o = v.toObject();
        Film film;
        film.naziv          = o["naziv"].toString();
        film.format         = o["format"].toString();
        film.kljucOtkljucan = o["kljucOtkljucan"].toBool();
        film.bezOgranicenja = o["bezOgranicenja"].toBool();
        film.datumOd        = QDate::fromString(o["datumOd"].toString(), Qt::ISODate);
        film.isticaKljuca   = QDate::fromString(o["isticaKljuca"].toString(), Qt::ISODate);
        film.filmSkinut     = o["filmSkinut"].toBool();
        film.filmInjestovan = o["filmInjestovan"].toBool();
        film.uArhivi        = o["uArhivi"].toBool();
        film.napomena       = o["napomena"].toString();
        filmovi.append(film);
    }
}
void MainWindow::sacuvajPodatke() {
    QJsonArray arr;
    for (const auto &f : filmovi) {
        QJsonObject o;
        o["naziv"]          = f.naziv;
        o["format"]         = f.format;
        o["kljucOtkljucan"] = f.kljucOtkljucan;
        o["bezOgranicenja"] = f.bezOgranicenja;
        o["datumOd"]        = f.datumOd.toString(Qt::ISODate);
        o["isticaKljuca"]   = f.isticaKljuca.toString(Qt::ISODate);
        o["filmSkinut"]     = f.filmSkinut;
        o["filmInjestovan"] = f.filmInjestovan;
        o["uArhivi"]        = f.uArhivi;
        o["napomena"]       = f.napomena;
        arr.append(o);
    }
    QFile f(dataFilePath());
    if (f.open(QIODevice::WriteOnly))
        f.write(QJsonDocument(arr).toJson());
}
