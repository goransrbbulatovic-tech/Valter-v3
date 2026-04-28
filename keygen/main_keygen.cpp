#include <QApplication>
#include <QFont>
#include "keygen.h"
int main(int argc, char *argv[]) {
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication app(argc, argv);
    app.setApplicationName("Valter Keygen");
    app.setOrganizationName("KCBar");
    QFont f("Segoe UI", 10); app.setFont(f);
    KeygenWindow w; w.show();
    return app.exec();
}
