#-------------------------------------------------
#
# Project created by QtCreator 2014-01-01T18:24:09
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BootloaderGui
TEMPLATE = app

INCLUDEPATH += inc

SOURCES += src/main.cpp\
        src/mainwindow.cpp \
    src/transferthread.cpp \
    src/bootloader.cpp

HEADERS  += inc/mainwindow.h \
    inc/transferthread.h \
    inc/libusbwrapper.h \
    inc/compat.h \
    inc/bootloader.h

win32 {
    message(Building with WinUSB support.)
    DEFINES += QWINUSB
    SOURCES  += src/qwinusb.cpp
    HEADERS  += inc/qwinusb.h
    RC_FILE = res/rc.rc
    CONFIG += console
}
else {
    message(Building with LibUsb support.)
    SOURCES  += src/LibUsb.cpp
    HEADERS  += inc/LibUsb.h
    LIBS += -lusb-1.0
}

FORMS    += mainwindow.ui

RESOURCES += res/ressources.qrc
