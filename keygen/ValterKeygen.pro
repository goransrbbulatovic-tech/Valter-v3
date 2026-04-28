QT += core widgets network
CONFIG += c++17
TARGET = ValterKeygen
TEMPLATE = app
SOURCES += main_keygen.cpp keygen.cpp ../src/license.cpp
HEADERS += keygen.h ../src/license.h
win32 {
    RC_ICONS = ../icon.ico
    QMAKE_TARGET_COMPANY = "KC Bar"
    QMAKE_TARGET_PRODUCT = "Valter Keygen"
}
CONFIG(release, debug|release): DEFINES += QT_NO_DEBUG_OUTPUT
