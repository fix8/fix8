#-------------------------------------------------
#
# Project created by QtCreator 2014-03-15T21:24:01
#
#-------------------------------------------------

QT       += core gui sql qml quick widgets

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
    messagearea.cpp \
    database.cpp \
    windowdata.cpp \
    databaseWindows.cpp \
    worksheetdata.cpp \
    databaseWorksheets.cpp \
    fix8logDataFile.cpp \
    messageitem.cpp \
    fixHeaderView.cpp \
    dateTimeDelegate.cpp \
    futurereaddata.cpp \
    searchlineedit.cpp \
    fixtoolbar.cpp \
    fixmimedata.cpp \
    nodatalabel.cpp \
    schemaeditordialog.cpp \
    schemaitem.cpp \
    databaseTableSchemas.cpp \
    tableschema.cpp \
    schemaEditorDialogSlots.cpp \
    databaseSchemaFields.cpp \
    fix8logSlots.cpp \
    schemadelegate.cpp \
    selectedfieldstreeview.cpp

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
    messagearea.h \
    database.h \
    windowdata.h \
    worksheetdata.h \
    messageitem.h \
    fixHeaderView.h \
    dateTimeDelegate.h \
    futurereaddata.h \
    searchlineedit.h \
    fixtoolbar.h \
    fixmimedata.h \
    nodatalabel.h \
    schemaeditordialog.h \
    schemaitem.h \
    tableschema.h \
    schemadelegate.h \
    selectedfieldstreeview.h

RESOURCES += \
    resources.qrc

unix {
QMAKE_CXXFLAGS += -Wno-missing-field-initializers -Wno-ignored-qualifiers -Wno-missing-field-initializers -Wno-uninitialized -Wno-unused-variable -Wno-unused-parameter -std=c++11
LIBS += -lz -L/usr/local/lib  -lrt -lfix8 -ltbb  -lPocoFoundation -lPocoNet -lPocoUtil
INCLUDEPATH += /usr/local/include /usr/local/include/fix8 ./f8
DEPENDPATH += /usr/local/include
}
win32 {
    message("Windows Compile")
    MOC_DIR = moc
    UI_DIR = ui
    OBJECTS_DIR = obj
    INCLUDEPATH += $$(HOME)/include
    LIBS +=
}
OTHER_FILES += \
    qml/loadProgress.qml \    
    fix8log.pro.user \
    fix8log.pro.user.7112d3e \


