#-------------------------------------------------
#
# Project created by QtCreator 2014-03-15T21:24:01
#
#-------------------------------------------------
CONFIG += x86_64  debug_and_release
QT       += core gui sql qml quick widgets  script
include(./qtsingleapplication/src/qtsingleapplication.pri)
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
    selectedfieldstreeview.cpp \
    worksheetmodel.cpp \
    ../../test/Myfix_classes.cpp \
    ../../test/Myfix_traits.cpp \
    ../../test/Myfix_types.cpp \
    messageitemdelegate.cpp \
    proxyFilter.cpp \
    mainWindowSearch.cpp \
    lineedit.cpp \
    editHighLighter.cpp \
    searchDelegate.cpp \
    fixtableverticaheaderview.cpp

HEADERS  += mainwindow.h \
    globals.h \
    fix8log.h \
    fixtable.h \
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
    selectedfieldstreeview.h \
    worksheetmodel.h \
    ../../test/Myfix_classes.hpp \
    ../../test/Myfix_router.hpp \
    ../../test/Myfix_types.hpp \
    messageitemdelegate.h \
    proxyFilter.h \
    lineedit.h \
    editHighLighter.h \
    searchDelegate.h \
    fixtableverticaheaderview.h

RESOURCES += \
    resources.qrc

unix {
QMAKE_CXXFLAGS += -Wno-missing-field-initializers -Wno-ignored-qualifiers -Wno-missing-field-initializers -Wno-uninitialized -Wno-unused-variable -Wno-unused-parameter -std=c++11
LIBS += -lz -L/usr/local/lib  -lrt -lfix8 -ltbb  -lPocoFoundation -lPocoNet -lPocoUtil
INCLUDEPATH += /usr/local/include /usr/local/include/fix8 ../../test
DEPENDPATH += /usr/local/include
}
win32 {
    message("Windows Compile")
QMAKE_CXXFLAGS += /bigobj
    MOC_DIR = moc
    UI_DIR = ui
    OBJECTS_DIR = obj
    INCLUDEPATH += . ./f8 ../../test \
                ../../msvc/packages/fix8.dev.1.1.20140603.1/build/native/include \
                ../../msvc/packages/fix8.dependencies.getopt.1.0.20140509.1/build/native/include \
                ../../msvc/packages/fix8.dependencies.openssl.1.0.20140509.1/build/native/include/x64/v120/Release/Desktop \
                ../../msvc/packages/fix8.dependencies.poco.1.5.20140509.1/build/native/include \
                ../../msvc/packages/fix8.dependencies.tbb.4.2.20140416.1/build/native/include
    LIBS +=
}
OTHER_FILES += \
    qml/loadProgress.qml \    
    Screenshot 2014-05-12 at 07.29.46.png \
    schemaeditordialog.cpp.oow \
    schemaeditordialog.h.oow \
    schemaEditorDialogSlots.cpp.0529 \
    schemaEditorDialogSlots.cpp.oow


