#-------------------------------------------------
#
# Project created by QtCreator 2016-06-06T09:48:11
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = trackDB
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    tracksmodel.cpp \
    filesmodel.cpp \
    database.cpp \
    propertymodel.cpp \
    tagsmodel.cpp \
    filtermodel.cpp

HEADERS  += mainwindow.h \
    tracksmodel.h \
    filesmodel.h \
    database.h \
    propertymodel.h \
    tagsmodel.h \
    filtermodel.h

FORMS    += mainwindow.ui

DISTFILES += \
    schema.sql

RESOURCES += \
    pics.qrc
