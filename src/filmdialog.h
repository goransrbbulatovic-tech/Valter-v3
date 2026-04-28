#ifndef FILMDIALOG_H
#define FILMDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QDateEdit>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include "mainwindow.h"

class FilmDialog : public QDialog {
    Q_OBJECT

public:
    explicit FilmDialog(QWidget *parent = nullptr, const Film *film = nullptr);
    Film getFilm() const;

private slots:
    void provjeriFormu();
    void bezOgranicenjaToggled(bool checked);
    void datumOdPromijenjen(const QDate &d);
    void datumDoPromijenjen(const QDate &d);

private:
    void setupUI(const Film *film);
    void azurirajTrajanje();

    QLineEdit   *edNaziv;
    QComboBox   *cbFormat;
    QCheckBox   *chkOtkljucan;
    QCheckBox   *chkBezOgranicenja;
    QDateEdit   *deOd;
    QDateEdit   *deDo;
    QLabel      *lblTrajanje;
    QCheckBox   *chkSkinut;
    QCheckBox   *chkInjestovan;
    QTextEdit   *edNapomena;
    QPushButton *btnOk;
    QPushButton *btnOtkazi;
};

#endif // FILMDIALOG_H
