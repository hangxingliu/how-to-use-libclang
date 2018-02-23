#-------------------------------------------------
#
# Project created by QtCreator 2018-02-22T16:23:38
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = libclang-gui
TEMPLATE = app

CONFIG += debug c++11

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
		highlighter.cpp \
	clang-utils/clang-utils.cpp \
    color-scheme.cpp

HEADERS += \
        mainwindow.h \
		highlighter.h \
	clang-utils/clang-utils.h \
    color-scheme.h \
    clang-utils/clang-treewidget-item.h

FORMS += \
        mainwindow.ui

# For libclang
# >>>>>>>>>>>>>>>>>>>>>
INCLUDEPATH += $$system('llvm-config --includedir')
LIBS += $$system('llvm-config --ldflags')
LIBS += -lclang

RESOURCES += \
    resources.qrc
