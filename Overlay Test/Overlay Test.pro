#-------------------------------------------------
#
# Project created by QtCreator 2015-06-10T16:57:45
#
#-------------------------------------------------

QT       += core gui opengl
CONFIG   += c++11
CONFIG += console

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Overlay Test
TEMPLATE = app


SOURCES += main.cpp\
        overlaywidget.cpp \
    openvroverlaycontroller.cpp

HEADERS  += overlaywidget.h \
    openvroverlaycontroller.h

FORMS    += overlaywidget.ui

LIBS += -lopengl32 -lglu32

#INCLUDEPATH += ../../headers

#LIBS += -L../../lib/win32 -lopenvr_api

#DESTDIR = ../bin/win32
