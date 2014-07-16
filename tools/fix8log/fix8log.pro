#-------------------------------------------------
#
# Project created by QtCreator 2014-03-15T21:24:01
#
#-------------------------------------------------
CONFIG += debug_release
QT       += core gui sql qml quick widgets quickwidgets script
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
    fixtableverticaheaderview.cpp \
    worksheetSearches.cpp \
    searchDialog.cpp \
    searchfunction.cpp \
    databaseSearchFunctions.cpp \
    comboboxlineedit.cpp \
    qtlocalpeer.cpp \
    qtlockedfile.cpp \
    qtsingleapplication.cpp \
    qtsinglecoreapplication.cpp \
    pushbuttonmodifykey.cpp \
    schemaeditordialogMessageView.cpp \
    fieldsview.cpp

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
    fixtableverticaheaderview.h \
    searchDialog.h \
    searchfunction.h \
    comboboxlineedit.h \
    qtlocalpeer.h \
    QtLockedFile \
    qtlockedfile.h \
    qtsingleapplication.h \
    qtsinglecoreapplication.h \
    pushbuttonmodifykey.h \
    fieldsview.h

RESOURCES += \
    resources.qrc

    UI_DIR = ui
    OBJECTS_DIR = obj
unix {
message("Unix Compile.")
QMAKE_CXXFLAGS += -Wno-missing-field-initializers -Wno-ignored-qualifiers -Wno-missing-field-initializers -Wno-uninitialized -Wno-unused-variable -Wno-unused-parameter -std=c++11
LIBS += -lz -L/usr/local/lib  -lrt -lfix8 -ltbb  -lPocoFoundation -lPocoNet -lPocoUtil
INCLUDEPATH += /usr/local/include /usr/local/include/fix8 ../../test
DEPENDPATH += /usr/local/include
SOURCES += qtlockedfile_unix.cpp
}
win32 {
     message("Windows Compile")
    CONFIG -= console
    SOURCES += qtlockedfile_win.cpp
    QMAKE_CXXFLAGS += /bigobj -D WIN32_LEAN_AND_MEAN
    INCLUDEPATH += . ./f8 ../../test \
                ../../msvc/packages/fix8.dev.1.2.20140629.1/build/native/include \
                ../../msvc/packages/fix8.dependencies.getopt.1.0.20140509.1/build/native/include \
                ../../msvc/packages/fix8.dependencies.openssl.1.0.20140509.1/build/native/include/x64/v120/Release/Desktop \
                ../../msvc/packages/fix8.dependencies.poco.1.5.20140509.1/build/native/include \
                ../../msvc/packages/fix8.dependencies.tbb.4.2.20140416.1/build/native/include

release {
 message("Release")
   LIBS +=  rpcrt4.lib \
        ../../msvc/packages/fix8.dev.1.2.20140629.1/build/native/lib/x64/v120/Release/Desktop/fix8.lib \
        ../../msvc/packages/fix8.dependencies.tbb.4.2.20140416.1/build/native/lib/x64/v120/Release/Desktop/tbb.lib \
        ../../msvc/packages/fix8.dependencies.tbb.4.2.20140416.1/build/native/lib/x64/v120/Release/Desktop/tbbmalloc_proxy.lib \
        ../../msvc/packages/fix8.dependencies.tbb.4.2.20140416.1/build/native/lib/x64/v120/Release/Desktop/tbbmalloc.lib \
        ../../msvc/packages/fix8.dependencies.poco.1.5.20140509.1/build/native/lib/x64/v120/Release/Desktop/PocoFoundation.lib \
        ../../msvc/packages/fix8.dependencies.poco.1.5.20140509.1/build/native/lib/x64/v120/Release/Desktop/PocoNet.lib \
        ../../msvc/packages/fix8.dependencies.poco.1.5.20140509.1/build/native/lib/x64/v120/Release/Desktop/PocoNetSSL.lib \
        ../../msvc/packages/fix8.dependencies.poco.1.5.20140509.1/build/native/lib/x64/v120/Release/Desktop/PocoCrypto.lib \
        ../../msvc/packages/fix8.dependencies.poco.1.5.20140509.1/build/native/lib/x64/v120/Release/Desktop/PocoUtil.lib
}
debug {
    message("Debug")
   LIBS +=     ../../msvc/packages/fix8.dev.1.2.20140629.1/build/native/lib/x64/v120/Debug/Desktop/fix8d.lib \
                ../../msvc/packages/fix8.dependencies.poco.1.5.20140509.1/build/native/lib/x64/v120/Debug/Desktop/PocoFoundationd.lib \
                ../../msvc/packages/fix8.dependencies.tbb.4.2.20140416.1/build/native/lib/x64/v120/Debug/Desktop/tbb_debug.lib \
                ../../msvc/packages/fix8.dependencies.tbb.4.2.20140416.1/build/native/lib/x64/v120/Debug/Desktop/tbbmalloc_proxy_debug.lib \
                ../../msvc/packages/fix8.dependencies.poco.1.5.20140509.1/build/native/lib/x64/v120/Debug/Desktop/PocoNetd.lib \
                ../../msvc/packages/fix8.dependencies.poco.1.5.20140509.1/build/native/lib/x64/v120/Debug/Desktop/PocoNetSSLd.lib \
                ../../msvc/packages/fix8.dependencies.poco.1.5.20140509.1/build/native/lib/x64/v120/Debug/Desktop/PocoCryptod.lib \
                ../../msvc/packages/fix8.dependencies.poco.1.5.20140509.1/build/native/lib/x64/v120/Debug/Desktop/PocoUtild.lib
 }
}
OTHER_FILES += \
    qml/loadProgress.qml \
    qml/fieldView.qml \
    images/svg/messageView.svg

