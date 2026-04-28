#ifndef LICENSEDIALOG_H
#define LICENSEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

class LicenseDialog : public QDialog {
    Q_OBJECT
public:
    explicit LicenseDialog(QWidget *parent = nullptr, bool firstRun = true);

private slots:
    void onActivate();
    void onKeyChanged(const QString &text);

private:
    QLineEdit   *edKey;
    QLabel      *lblStatus;
    QPushButton *btnActivate;
    QPushButton *btnExit;
    bool         m_firstRun;
};

#endif
