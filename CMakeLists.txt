cmake_minimum_required(VERSION 3.16)
project(EvidencijaKCBar VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/filmdialog.cpp
)

set(HEADERS
    src/mainwindow.h
    src/filmdialog.h
)

if(WIN32)
    add_executable(EvidencijaKCBar WIN32 ${SOURCES} ${HEADERS})
else()
    add_executable(EvidencijaKCBar ${SOURCES} ${HEADERS})
endif()

target_link_libraries(EvidencijaKCBar Qt5::Core Qt5::Widgets)

install(TARGETS EvidencijaKCBar DESTINATION bin)
