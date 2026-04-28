QT += core widgets printsupport network

CONFIG += c++17
TARGET = Valter
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/filmdialog.cpp \
    src/license.cpp \
    src/licensedialog.cpp

HEADERS += \
    src/mainwindow.h \
    src/filmdialog.h \
    src/license.h \
    src/licensedialog.h

win32 {
    RC_ICONS = $$PWD/icon.ico
    QMAKE_TARGET_COMPANY     = "KC Bar"
    QMAKE_TARGET_PRODUCT     = "Valter - Program za evidenciju filmova i kljuceva za bioskop"
    QMAKE_TARGET_DESCRIPTION = "Valter - KC Bar Bioskop"
    QMAKE_TARGET_COPYRIGHT   = "KC Bar 2025"
}

CONFIG(release, debug|release): DEFINES += QT_NO_DEBUG_OUTPUT
