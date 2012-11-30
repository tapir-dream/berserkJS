#-------------------------------------------------
#
# Project created by QtCreator 2012-01-29T11:27:46
#
#-------------------------------------------------

QT       += core gui
QT       += webkit
QT       += network
QT       += script

CONFIG += qtestlib

TARGET = berserkJS
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    networkaccessmanager.cpp \
    monitordata.cpp \
    monitordatamap.cpp \
    customdownload.cpp \
    selector.cpp \
    scriptbinding.cpp \
    mywebview.cpp \
    pageextension.cpp \
    commandparameters.cpp \
    mywebpage.cpp \
    filesystemwatcher.cpp \
    firstscreen.cpp \
    scriptsignalfactory.cpp \
    appinfo.cpp \
    cookiejar.cpp

HEADERS  += mainwindow.h \
    networkaccessmanager.h \
    monitordata.h \
    monitordatamap.h \
    customdownload.h \
    selector.h \
    scriptbinding.h \
    mywebview.h \
    pageextension.h \
    commandparameters.h \
    mywebpage.h \
    filesystemwatcher.h \
    firstscreen.h \
    scriptsignalfactory.h \
    appinfo.h \
    consts.h \
    cookiejar.h

FORMS    += mainwindow.ui

win32 {
    LIBS += -lpsapi
}





























































