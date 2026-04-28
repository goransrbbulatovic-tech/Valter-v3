#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QStyledItemDelegate>
#include <QPushButton>
#include <QLabel>
#include <QDate>
#include <QTimer>
#include <QTabWidget>
#include <QLineEdit>

// Status kodovi za bojanje redova
enum RowStatus {
    RS_ZAKLJUCAN   = 0,
    RS_NEOGRANICEN = 1,
    RS_UREDU       = 2,
    RS_UPOZORENJE  = 3,
    RS_KRITICNO    = 4,
    RS_ISTEKAO     = 5,
    RS_ARHIVA      = 6
};

// Delegat koji osigurava da selekcija uvijek boji CIJELI red uniformno
class RowDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit RowDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};

struct Film {
    QString naziv;
    QString format;
    bool    kljucOtkljucan;
    bool    bezOgranicenja;
    QDate   datumOd;
    QDate   isticaKljuca;
    bool    filmSkinut;
    bool    filmInjestovan;
    bool    uArhivi;
    QString napomena;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void dodajFilm();
    void urediFilm();
    void obrisiFilm();
    void posaljiUArhivu();
    void vratiIzArhive();
    void obrisiIzArhive();
    void osveziTabelu();
    void provjeriIstekKljuceva();
    void selekcijaPromijenjena();
    void selekcijaArhivePromijenjena();
    void pretragaPromijenjena(const QString &tekst);
    void izvezuPDF();
    void izvezuCSV();
    void uvezi();
    void tabPromijenjena(int index);

private:
    void setupUI();
    void setupStyle();
    void setupTabela(QTableWidget *t);
    void ucitajPodatke();
    void sacuvajPodatke();
    void popuniTabelu(const QString &filter = "");
    void popuniArhivu(const QString &filter = "");
    void popuniRed(QTableWidget *tbl, int row, const Film &f);
    void azurirajStatusBar();
    void azurirajBrojacTabova();
    QString statusKljuca(const Film &f) const;
    int     danaDoIsteka(const Film &f) const;
    int     trajanjeDana(const Film &f) const;
    QString dataFilePath() const;
    RowStatus rowStatus(const Film &f) const;

    QTabWidget   *tabWidget;
    QTableWidget *tabela;
    QTableWidget *tabelaArhiva;
    QPushButton  *btnDodaj, *btnUredi, *btnObrisi, *btnArhiviraj;
    QPushButton  *btnVratiIzArhive, *btnObrisiArhiva;
    QPushButton  *btnOsvezi, *btnPDF, *btnCSV, *btnUvezi;
    QLineEdit    *edPretraga;
    QLabel       *lblUpozorenje;
    QTimer       *timerProvjera;
    QList<Film>   filmovi;
};

#endif
