#-------------------------------------------------
#
# Project created by QtCreator 2015-06-10T16:57:45
#
#-------------------------------------------------

QT     += core;gui;widgets;opengl;openglwidgets
CONFIG += c++11
CONFIG += console

TARGET = Overlay Test
TEMPLATE = app


SOURCES += main.cpp\
        overlaywidget.cpp \
    openvroverlaycontroller.cpp

HEADERS  += overlaywidget.h \
    openvroverlaycontroller.h

FORMS    += overlaywidget.ui

LIBS += -lopengl64 -lglu64

#INCLUDEPATH += ../../headers

#LIBS += -L../../lib/win32 -lopenvr_api

#DESTDIR = ../bin/win32
