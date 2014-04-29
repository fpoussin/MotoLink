#-------------------------------------------------
#
# Project created by QtCreator 2014-04-08T14:53:12
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MotoLink
TEMPLATE = app

VERSION = 0.1
message(Version $$VERSION)

VERSTR = '\\"$${VERSION}\\"'  # place quotes around the version string
DEFINES += __MTL_VER__=\"$${VERSTR}\" # create a VER macro containing the version string

INCLUDEPATH += inc

include(QtUsb/QtUsb.pri)

SOURCES += src/main.cpp \
    src/mainwindow.cpp \
    src/intelhexclass.cpp \
    src/bootloader.cpp \
    src/transferthread.cpp \
    src/hrc.cpp \
    src/updatewizard.cpp \
    src/updatewizardpage1.cpp \
    src/updatewizardpage2.cpp

HEADERS  += \
    inc/compat.h \
    inc/mainwindow.h \
    inc/intelhexclass.h \
    inc/bootloader.h \
    inc/transferthread.h \
    inc/datastructures.h \
    inc/hrc.h \
    inc/updatewizard.h \
    inc/updatewizardpage1.h \
    inc/updatewizardpage2.h

FORMS    += ui/main.ui \
    ui/updatewizard.ui \
    ui/updatewizardpage1.ui \
    ui/updatewizardpage2.ui

RESOURCES += \
    res/oxygen.qrc \
    res/binaries.qrc \
    res/images.qrc
