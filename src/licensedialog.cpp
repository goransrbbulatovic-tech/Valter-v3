#include <QTimer>
#include "licensedialog.h"
#include "license.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QApplication>

LicenseDialog::LicenseDialog(QWidget *parent, bool firstRun)
    : QDialog(parent), m_firstRun(firstRun)
{
    setWindowTitle("Valter — Aktivacija");
    setFixedSize(520, 360);
    setModal(true);
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    setStyleSheet(R"(
        QDialog { background: #0e1118; }
        QLabel { color: #c8d4f0; font-size: 13px; }
        QLineEdit {
            background: #141a28; border: 2px solid #2d3a55;
            border-radius: 8px; padding: 10px 16px;
            color: #e0eaff; font-size: 15px; font-family: 'Courier New', monospace;
            letter-spacing: 2px;
        }
        QLineEdit:focus { border-color: #3b82f6; }
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #2563eb, stop:1 #1d4ed8);
            color: white; border: 1px solid #3b82f6;
            border-radius: 7px; padding: 10px 24px;
            font-size: 13px; font-weight: 700; min-width: 120px;
        }
        QPushButton:hover { background: #3b82f6; }
        QPushButton:disabled { background: #1e2535; color: #3a4460; border-color: #242b3d; }
        QPushButton#btnExit {
            background: #1e2535; color: #6b7fa8;
            border-color: #2d3a55;
        }
        QPushButton#btnExit:hover { background: #242b3d; color: #93c5fd; }
    )");

    QVBoxLayout *main = new QVBoxLayout(this);
    main->setSpacing(0);
    main->setContentsMargins(0, 0, 0, 0);

    // Header
    QWidget *hdr = new QWidget();
    hdr->setStyleSheet("background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
                       "stop:0 #0f1520, stop:0.5 #1a2540, stop:1 #0f1520);"
                       "border-bottom: 2px solid #2563eb;");
    QVBoxLayout *hdrL = new QVBoxLayout(hdr);
    hdrL->setContentsMargins(30, 24, 30, 20);

    QLabel *lblIcon = new QLabel("🎬  VALTER");
    lblIcon->setStyleSheet("font-size: 22px; font-weight: 800; color: #60a5fa; letter-spacing: 1px;");
    QLabel *lblSub = new QLabel("Program za evidenciju filmova i kljuceva za bioskop");
    lblSub->setStyleSheet("font-size: 12px; color: #3a5080; font-weight: 500;");
    hdrL->addWidget(lblIcon);
    hdrL->addWidget(lblSub);
    main->addWidget(hdr);

    // Body
    QWidget *body = new QWidget();
    QVBoxLayout *bodyL = new QVBoxLayout(body);
    bodyL->setContentsMargins(30, 24, 30, 24);
    bodyL->setSpacing(16);

    QLabel *lblTitle = new QLabel(firstRun
        ? "Dobrodosli! Unesite vas licencni kljuc da biste aktivirali program."
        : "Unesite vas licencni kljuc za aktivaciju programa.");
    lblTitle->setWordWrap(true);
    lblTitle->setStyleSheet("color: #8fa4cc; font-size: 13px;");
    bodyL->addWidget(lblTitle);

    // Prikaz Machine ID (korisno za korisnicku podrsku)
    QLabel *lblMid = new QLabel(QString("ID ovog racunara: <b style=\"color:#3b82f6\">%1</b>").arg(License::machineId()));
    lblMid->setStyleSheet("color: #4a5e85; font-size: 11px;");
    lblMid->setWordWrap(true);
    bodyL->addWidget(lblMid);

    QLabel *lblKeyLabel = new QLabel("Licencni kljuc:");
    lblKeyLabel->setStyleSheet("color: #6b82aa; font-size: 12px; font-weight: 600;");
    bodyL->addWidget(lblKeyLabel);

    edKey = new QLineEdit();
    edKey->setPlaceholderText("VALTR-XXXXX-XXXXX-XXXXX-XXXXX");
    edKey->setMaxLength(29);
    bodyL->addWidget(edKey);

    lblStatus = new QLabel("");
    lblStatus->setStyleSheet("font-size: 12px; min-height: 20px;");
    lblStatus->setWordWrap(true);
    bodyL->addWidget(lblStatus);

    bodyL->addStretch();

    // Buttons
    QHBoxLayout *btnL = new QHBoxLayout();
    btnL->setSpacing(10);
    btnExit = new QPushButton("Izlaz");
    btnExit->setObjectName("btnExit");
    btnActivate = new QPushButton("Aktiviraj");
    btnActivate->setEnabled(false);
    btnL->addWidget(btnExit);
    btnL->addStretch();
    btnL->addWidget(btnActivate);
    bodyL->addLayout(btnL);

    main->addWidget(body, 1);

    // Ako vec ima kljuc, prikazi ga
    QString saved = License::savedKey();
    if (!saved.isEmpty()) edKey->setText(saved);

    connect(edKey,       &QLineEdit::textChanged, this, &LicenseDialog::onKeyChanged);
    connect(btnActivate, &QPushButton::clicked,   this, &LicenseDialog::onActivate);
    connect(btnExit,     &QPushButton::clicked,   this, firstRun ? &QDialog::reject : &QDialog::reject);

    // Auto-format dok kuca
    connect(edKey, &QLineEdit::textEdited, this, [this](const QString &txt) {
        QString clean = txt.toUpper();
        // Ukloni sve sto nije slovo/broj/crtica
        QString filtered;
        for (QChar c : clean)
            if (c.isLetterOrNumber() || c == '-') filtered += c;
        // Auto-dodaj crtice
        QString bare;
        for (QChar c : filtered) if (c != '-') bare += c;
        QString formatted;
        for (int i = 0; i < bare.size(); i++) {
            if (i == 5 || i == 10 || i == 15 || i == 20) formatted += '-';
            formatted += bare[i];
        }
        if (formatted != txt) {
            edKey->blockSignals(true);
            edKey->setText(formatted);
            edKey->blockSignals(false);
        }
        onKeyChanged(formatted);
    });
}

void LicenseDialog::onKeyChanged(const QString &text) {
    bool valid = License::isValidKey(text);
    btnActivate->setEnabled(valid);
    if (text.isEmpty()) {
        lblStatus->setText("");
    } else if (text.size() < 29) {
        lblStatus->setText("");
        lblStatus->setStyleSheet("color: #4a5e85; font-size: 12px;");
    } else if (valid) {
        lblStatus->setText("✔  Kljuc je validan — kliknite Aktiviraj");
        lblStatus->setStyleSheet("color: #22c55e; font-size: 12px; font-weight: 600;");
    } else {
        lblStatus->setText("✕  Kljuc nije validan. Provjerite unos.");
        lblStatus->setStyleSheet("color: #ef4444; font-size: 12px;");
    }
}

void LicenseDialog::onActivate() {
    int result = License::activate(edKey->text());
    if (result == 0) {
        lblStatus->setText("✔  Aktivacija uspjesna! Program je licenciran za ovaj racunar.");
        lblStatus->setStyleSheet("color: #22c55e; font-size: 13px; font-weight: 700;");
        btnActivate->setEnabled(false);
        QTimer::singleShot(1400, this, &QDialog::accept);
    } else if (result == 2) {
        lblStatus->setText("✕  Ovaj kljuc je vec aktiviran na drugom racunaru. Kontaktirajte prodavca.");
        lblStatus->setStyleSheet("color: #f97316; font-size: 12px; font-weight: 600;");
    } else {
        lblStatus->setText("✕  Kljuc nije validan. Provjerite unos i pokusajte ponovo.");
        lblStatus->setStyleSheet("color: #ef4444; font-size: 12px;");
    }
}
