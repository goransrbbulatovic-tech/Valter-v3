#include <QApplication>
#include <QFont>
#include <QTimer>
#include "mainwindow.h"
#include "license.h"
#include "licensedialog.h"

#ifdef _WIN32
#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);
    app.setApplicationName("Valter");
    app.setOrganizationName("KCBar");
    app.setApplicationVersion("1.0.0");

    QFont font("Segoe UI", 10);
    app.setFont(font);

    // Provjeri licencu
    if (!License::isActivated()) {
        LicenseDialog dlg(nullptr, true);
        if (dlg.exec() != QDialog::Accepted) {
            return 0; // Izlaz ako korisnik nije aktivirao
        }
    }

    MainWindow window;
    window.show();
    return app.exec();
}
