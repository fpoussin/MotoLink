#-------------------------------------------------
#
# Project created by QtCreator 2014-04-08T14:53:12
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MotoLink
TEMPLATE = app

INCLUDEPATH += inc

SOURCES += src/main.cpp \
    src/mainwindow.cpp \
    src/intelhexclass.cpp

HEADERS  += \
    inc/compat.h \
    inc/mainwindow.h \
    inc/intelhexclass.h

FORMS    += ui/main.ui

win32 {
    message(Building with WinUSB support.)
    DEFINES += QWINUSB
    SOURCES  += src/qwinusb.cpp
    HEADERS  += inc/qwinusb.h
    RC_FILE = res/motolink.rc
    CONFIG += console
}
else {
    message(Building with LibUsb support.)
    SOURCES  += src/qlibusb.cpp
    HEADERS  += inc/qlibusb.h
    LIBS += -lusb-1.0
}

RESOURCES += res/resources.qrc \
    res/oxygen.qrc
