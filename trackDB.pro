#-------------------------------------------------
#
# Project created by QtCreator 2016-06-06T09:48:11
#
#-------------------------------------------------

QT       += core gui sql printsupport

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
    filtermodel.cpp \
    checklistcombobox.cpp \
    checklistmodel.cpp \
    combocheckboxdelegate.cpp \
    printthread.cpp \
    printselectdialog.cpp \
    settingsdialog.cpp

HEADERS  += mainwindow.h \
    tracksmodel.h \
    filesmodel.h \
    database.h \
    propertymodel.h \
    tagsmodel.h \
    filtermodel.h \
    checklistcombobox.h \
    checklistmodel.h \
    combocheckboxdelegate.h \
    printthread.h \
    printselectdialog.h \
    settingsdialog.h

FORMS    += mainwindow.ui \
    printselectdialog.ui \
    settingsdialog.ui

DISTFILES += \
    schema.sql \
    icon.rc

RESOURCES += \
    pics.qrc

RC_FILE = icon.rc
