#
#  QCustomPlot Demo Project
#

QT       += core gui
TARGET = plot-examples
TEMPLATE = app

SOURCES += main.cpp\
           mainwindow.cpp \
        ../qcustomplot.cpp

HEADERS  += mainwindow.h \
         ../qcustomplot.h

FORMS    += mainwindow.ui

