#ifndef KEYGEN_H
#define KEYGEN_H
#include <QMainWindow>
#include <QTextEdit>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
class KeygenWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit KeygenWindow(QWidget *parent = nullptr);
private slots:
    void generiraj();
    void kopirajSelektovani();
    void kopirajSve();
    void obrisi();
    void posaljiEmail();
private:
    QSpinBox    *spnKolicina;
    QListWidget *lstKljucevi;
    QPushButton *btnGeneriraj;
    QPushButton *btnKopiraj;
    QPushButton *btnKopirajSve;
    QPushButton *btnObrisi;
    QPushButton *btnEmail;
    QLabel      *lblBroj;
};
#endif
