#-------------------------------------------------
#
# Project created by QtCreator 2014-03-15T21:24:01
#
#-------------------------------------------------

QT       += core gui
QMAKE_CXXFLAGS += -Wno-uninitialized -Wno-unused-variable -Wno-unused-parameter
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = fix8log
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    settings.cpp \
    globals.cpp \
    fix8log.cpp \
    fixtable.cpp \
    mainFileSlots.cpp \
    mainWindowSlots.cpp \
    f8/Myfix_classes.cpp \
    f8/Myfix_traits.cpp \
    f8/Myfix_types.cpp \
    intItem.cpp \
    worksheet.cpp \
    messagefield.cpp \
    messagearea.cpp

HEADERS  += mainwindow.h \
    globals.h \
    fix8log.h \
    fixtable.h \
    f8/Myfix_classes.hpp \
    f8/Myfix_router.hpp \
    f8/Myfix_types.hpp \
    intItem.h \
    worksheet.h \
    messagefield.h \
    messagearea.h

RESOURCES += \
    resources.qrc


LIBS += -L/usr/local/lib  -lrt -lfix8 -ltbb  -lPocoFoundation -lPocoNet -lPocoUtil

INCLUDEPATH += /usr/local/include /usr/local/include/fix8 ./f8
DEPENDPATH += /usr/local/include


