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

include(QtUsb/QtUsb.pri)

SOURCES += src/main.cpp \
    src/mainwindow.cpp \
    src/intelhexclass.cpp \
    src/bootloader.cpp \
    src/transferthread.cpp

HEADERS  += \
    inc/compat.h \
    inc/mainwindow.h \
    inc/intelhexclass.h \
    inc/bootloader.h \
    inc/transferthread.h

FORMS    += ui/main.ui

RESOURCES += \
    res/oxygen.qrc \
    res/binaries.qrc \
    res/images.qrc
