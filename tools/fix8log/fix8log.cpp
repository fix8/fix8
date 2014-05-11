#include "database.h"
#include "fix8log.h"
#include "fixmimedata.h"
#include "globals.h"
#include "mainwindow.h"
#include "schemaeditordialog.h"
#include "windowdata.h"
#include <QApplication>
#include <QDebug>
#include <QtWidgets>
#include <fix8/f8includes.hpp>
#include <field.hpp>
#include <message.hpp>
#include <f8types.hpp>
#include <Myfix_types.hpp>
#include <Myfix_router.hpp>
#include <Myfix_classes.hpp>
#include <iostream>
#include <string.h>
using namespace GUI;
using namespace FIX8;

Fix8Log::Fix8Log(QObject *parent) :
    QObject(parent),firstTimeToUse(false),database(0),autoSaveOn(false),
    cancelSessionRestore(false),schemaEditorDialog(0),tableSchemaList(0),
    defaultTableSchema(0),worldTableSchema(0),globalSchemaOn(false)
{
    Globals::Instance()->version = 0.1;
    Globals::Instance()->versionStr = "0.1";
    connect(qApp,SIGNAL(lastWindowClosed()),this,SLOT(lastWindowClosedSlot()));
}
void Fix8Log::createNewWindowSlot(MainWindow *mw)
{
    MainWindow *newMW  =new MainWindow(*mw);
    wireSignalAndSlots(newMW);
    newMW->show();
    newMW->showFileDialog();
    mainWindows.append(newMW);
    //const GeneratedTable<const char *, BaseMsgEntry>::Pair *p = TEX::ctx()._bme.at(1);
}
void Fix8Log::copyWindowSlot(MainWindow *mw)
{
    MainWindow *newMW  =new MainWindow(*mw,true);
    wireSignalAndSlots(newMW);
    newMW->show();
    mainWindows.append(newMW);
}
void Fix8Log::wireSignalAndSlots(MainWindow *mw)
{
    if (!mw) {
        qWarning() << "Error - wire signals and slots, window is null" << __FILE__ << __LINE__;
        return;
    }
    connect(mw,SIGNAL(createWindow(MainWindow*)),this,SLOT(createNewWindowSlot(MainWindow*)));
    connect(mw,SIGNAL(copyWindow(MainWindow*)),this,SLOT(copyWindowSlot(MainWindow*)));
    connect(mw,SIGNAL(deleteWindow(MainWindow*)),this,SLOT(deleteMainWindowSlot(MainWindow*)));
    connect(mw,SIGNAL(exitApp()),this,SLOT(exitAppSlot()));
    connect(mw,SIGNAL(autoSaveOn(bool)),this,SLOT(autoSaveOnSlot(bool)));
    connect(mw,SIGNAL(cancelSessionRestore()),this,SLOT(cancelSessionRestoreSlot()));
    connect(mw,SIGNAL(notifyTimeFormatChanged(GUI::Globals::TimeFormat)),
            this,SLOT(setTimeFormatSlot(GUI::Globals::TimeFormat)));
    connect(this,SIGNAL(notifyTimeFormatChanged(GUI::Globals::TimeFormat)),
            mw,SLOT(setTimeFormatSlot(GUI::Globals::TimeFormat)));
     connect(mw,SIGNAL(modelDropped(FixMimeData*)),this,SLOT(modelDroppedSlot(FixMimeData*)));
     connect(mw,SIGNAL(editSchema(MainWindow*,QUuid)),this,SLOT(editSchemaSlot(MainWindow*,QUuid)));
    mw->setAutoSaveOn(autoSaveOn);
}
void Fix8Log::deleteMainWindowSlot(MainWindow *mw)
{
    if (mainWindows.count() == 1)  {
        if (autoSaveOn) {
            saveSession();
        }
    }
    mainWindows.removeOne(mw);
    mw->deleteLater();
    if (mainWindows.count() < 1) {
        writeSettings();
        qApp->exit();
    }
}
void Fix8Log::displayConsoleMessage(GUI::Message msg)
{
    MainWindow *mw;
    QListIterator <MainWindow *> iter(mainWindows);
    while(iter.hasNext()) {
        mw = iter.next();
        mw->displayConsoleMessage(msg);
    }
}
void Fix8Log::displayConsoleMessage(QString str, GUI::Message::MessageType mt)
{
    GUI::Message m(str,mt);
    displayConsoleMessage(m);
}
bool Fix8Log::init()
{
    bool bstatus;
    bool createdTable = false;
    QString dbFileName = "fix8log.sql";
    QString errorStr;
    QStandardItemModel *model = 0;
    QString dbPath = QCoreApplication::applicationDirPath() + QDir::separator()  +  "share";
    QDir dir(dbPath);
    readSettings();
    if (!dir.exists()) {
        bstatus = dir.mkdir(dbPath);
        if (!bstatus) {
            errorStr = "Failed to create directory\n" + dbPath +
                    "\nPlease make sure you have write access to this directory";
            QMessageBox::warning(0,"Fix8Log Error", errorStr);
            qApp->exit();
        }
        else {
            displayConsoleMessage(GUI::Message("Created Database Directory:" + dbPath));
        }
    }
    dbFileName = dbPath + QDir::separator() + dbFileName;
    QFile dbFile(dbFileName);
    if (!dbFile.exists()) {
        firstTimeToUse = true;
        displayConsoleMessage(GUI::Message("Creating Database..."));
    }
    database = new Database(dbFileName,this);
    bstatus = database->open();
    if (!bstatus) {
        errorStr = "Error - open local database: " + dbFileName
                + " failed " + __FILE__;
        QMessageBox::warning(0,"Local Database",errorStr);
        qApp->exit();
    }
    bstatus = database->tableIsValid(Database::Windows);
    if (!bstatus) {
        bstatus = database->createTable(Database::Windows);
        if (!bstatus) {
            errorStr =  "Failed to create windows table.";
            displayConsoleMessage(GUI::Message(errorStr,GUI::Message::ErrorMsg));
        }
    }
    bstatus = database->tableIsValid(Database::WorkSheet);
    if (!bstatus) {
        bstatus = database->createTable(Database::WorkSheet);
        if (!bstatus) {
            errorStr = "Failed to create worksheet table.";
            displayConsoleMessage(GUI::Message(errorStr,GUI::Message::ErrorMsg));
        }
    }
    bstatus = database->tableIsValid(Database::TableSchemas);
    if (!bstatus) {
        bstatus = database->createTable(Database::TableSchemas);
        if (!bstatus) {
            errorStr = "Failed to create table  table.";
            displayConsoleMessage(GUI::Message(errorStr,GUI::Message::ErrorMsg));
        }
        else {
            // create default schema ...
            TableSchema *ts = new TableSchema("Default","Default Table Schema",true);
            if (ts) {
                bstatus = database->addTableSchema(*ts);
                if (!bstatus) {
                     errorStr = "Failed to create default table schema";
                     displayConsoleMessage(GUI::Message(errorStr,GUI::Message::ErrorMsg));
                }
                else {
                    tableSchemaList = database->getTableSchemas();
                    if (!tableSchemaList) {
                        errorStr = "Error - no table schemas found in database";
                        displayConsoleMessage(GUI::Message(errorStr,GUI::Message::ErrorMsg));
                    }
                }
            }
        }
    }
    else {
        tableSchemaList = database->getTableSchemas();
        if (!tableSchemaList) {
            errorStr = "Error - no table schemas found in database, creating default....";
            displayConsoleMessage(GUI::Message(errorStr,GUI::Message::WarningMsg));
            defaultTableSchema = new TableSchema("Default","Default Table Schema",true);

            bstatus = database->addTableSchema(*defaultTableSchema);
            if (!bstatus) {
                 errorStr = "Failed to add default table schema to database";
                 displayConsoleMessage(GUI::Message(errorStr,GUI::Message::ErrorMsg));
            }
            else {
                tableSchemaList = new TableSchemaList();
                tableSchemaList->append(defaultTableSchema);
            }
        }
        else { // lets make sure we have default table schema, else create it
           defaultTableSchema = tableSchemaList->findByName("Default");
           if(!defaultTableSchema) {
               errorStr = "Failed to find default table schema, creating it...";
               displayConsoleMessage(GUI::Message(errorStr,GUI::Message::WarningMsg));
               defaultTableSchema = new TableSchema("Default","Default Table Schema",true);
               tableSchemaList->append(defaultTableSchema);
               bstatus = database->addTableSchema(*defaultTableSchema);
               if (!bstatus) {
                   errorStr = "Failed to add default table schema to database 2";
                   displayConsoleMessage(GUI::Message(errorStr,GUI::Message::ErrorMsg));
               }
           }
        }
    MainWindow::defaultTableSchema = defaultTableSchema;
    }
    if (tableSchemaList)
        qDebug() << "NUM OF TABLE SCHEMAS FOUND IN DB ARE:" << tableSchemaList->count() << __FILE__ << __LINE__;
    QString key;
    QString value;
    quint32 ikey;
    FieldTrait::FieldType ft;
    Globals::messagePairs = new QVector<Globals::MessagePair>(TEX::ctx()._bme.size());
    for(unsigned int ii=0;ii < TEX::ctx()._bme.size();ii++)
    {
        key =
             QString::fromStdString(TEX::ctx()._bme.at(ii)->_key);
        value = QString::fromStdString(TEX::ctx()._bme.at(ii)->_value._name);
        Globals::messagePairs->insert(ii,Globals::MessagePair(key,value));
    }
   int sizeofFieldTable = TEX::ctx()._be.size();
   qDebug() << "SIZE OF FIELD TABLE = " << sizeofFieldTable << __FILE__ << __LINE__;
   for(unsigned int ii=0;ii < TEX::ctx()._be.size();ii++)  {
       ikey = TEX::ctx()._be.at(ii)->_key;
       const BaseEntry be = TEX::ctx()._be.at(ii)->_value;
       const RealmBase *rb = be._rlm;
       FieldTrait::FieldType trait( (FieldTrait::FieldType) ikey);
      // qDebug() << "\tKey of field =" << ikey;
       //qDebug() << "\tBaseEntry:" << be._name;
       QVariant var = trait;
       //qDebug() << "\t Type = " << var.typeName();
       //if (rb)  rb is always null
      // qDebug() << "\tRealm Base ftype:" << rb->_ftype << ", dtype:" << rb->_dtype;
   }
    // initial screeen
    qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,10);
    QList <WindowData> windowDataList = database->getWindows();
    qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,10);
    QListIterator <WindowData> iter(windowDataList);
    QStringList errorStrList;
    MainWindow *newMW;
    QMap <QString, QStandardItemModel *>::iterator currentItemIter;
    bool haveError = false;
    bool isInitial = true;
    if (autoSaveOn){
        while(iter.hasNext()) {
            WindowData wd = iter.next();
            qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,40);
            QList <WorkSheetData> wsdList = database->getWorkSheets(wd.id);
            qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,40);
            if (wsdList.count() > 0) {
                newMW  =new MainWindow(true);
                newMW->setWindowData(wd);
                wireSignalAndSlots(newMW);
                mainWindows.append(newMW);
                newMW->setAutoSaveOn(autoSaveOn);
                newMW->show();
                QListIterator <WorkSheetData> iter2(wsdList);
                while(iter2.hasNext()) {
                    model = 0;
                    WorkSheetData wsd = iter2.next();
                    currentItemIter =  fileNameModelMap.find(wsd.fileName);
                    if (currentItemIter != fileNameModelMap.end()) {
                        model = currentItemIter.value();
                    }
                    else {
                        newMW->setLoadMessage("Loading File " + wsd.fileName);
                        model = readLogFile(wsd.fileName,errorStr);
                        qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,4);
                        if (cancelSessionRestore) {
                            newMW->setLoading(false);
                            goto done;
                        }
                        if (!model)
                            errorStrList.append(errorStr);
                        else
                            fileNameModelMap.insert(wsd.fileName,model);
                    }
                    newMW->setLoading(false);
                    if (model) {
                        newMW->addWorkSheet(model,wsd);
                        newMW->setCurrentTabAndSelectedRow(wd.currentTab,2);
                    }
                }
            }
        }
        qDebug() << "TODO - Display error messages here, and if no work sheets created lets delete main window" << __FILE__ << __LINE__;
        displayConsoleMessage("Session restored from autosave");
    }
done:
    MainWindow::schemaList = tableSchemaList;

    // if no main windows lets create one
    if (mainWindows.count() < 1) {
        newMW = new MainWindow();
        wireSignalAndSlots(newMW);
        newMW->show();
        newMW->setAutoSaveOn(autoSaveOn);
        mainWindows.append(newMW);
    }
    return bstatus;
}
bool Fix8Log::init(QString fileNameToLoad)
{
    QStandardItemModel *model = 0;
    MainWindow *newMW  =new MainWindow(true);
    WindowData wd;
    WorkSheetData wsd;
    QString errorStr;
    wsd.fileName = fileNameToLoad;
    wd.isVisible = true;
    wd.name = "Fix8LogViewer";
    wd.color = Globals::menubarDefaultColor;
    wireSignalAndSlots(newMW);
    newMW->setWindowData(wd);
    wireSignalAndSlots(newMW);
    mainWindows.append(newMW);
    newMW->setLoadMessage("Loading File " + wsd.fileName);
    model = readLogFile(wsd.fileName,errorStr);
    qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,4);

    if (model) {
        fileNameModelMap.insert(wsd.fileName,model);
        newMW->setLoading(false);
        newMW->addWorkSheet(model,wsd);
        newMW->show();
    }
    return true;
}
void Fix8Log::exitAppSlot()
{
    if (autoSaveOn)
        saveSession();
    //writeSettings();
    qApp->closeAllWindows();
    qApp->quit();
}
void Fix8Log::saveSession()
{
    MainWindow *mw;
    WorkSheetData wsd;
    bool bstatus;
    bstatus = database->deleteAllWindows();
    if (!bstatus)
        qWarning() << "Delete all windows from database failed" << __FILE__ << __LINE__;
    bstatus = database->deleteAllWorkSheets();
    if (!bstatus)
        qWarning() << "Delete all worksheets from database failed" << __FILE__ << __LINE__;
    QListIterator <MainWindow *> iter(mainWindows);
    while(iter.hasNext()) {
        mw = iter.next();
        WindowData wd = mw->getWindowData();
        bstatus = database->addWindow(wd);
        if (!bstatus) {
            displayConsoleMessage("Error failed saving window to database",GUI::Message::ErrorMsg);
        }
        else {
            QList<WorkSheetData> wsdList = mw->getWorksheetData(wd.id);
            QListIterator <WorkSheetData> wsdIter(wsdList);
            int i = 0;
            while(wsdIter.hasNext()) {
                wsd = wsdIter.next();
                bstatus = database->addWorkSheet(wsd);
                i++;
            }
        }
    }
}
void Fix8Log::autoSaveOnSlot(bool on)
{
    MainWindow *mw;
    QListIterator <MainWindow *> iter(mainWindows);
    autoSaveOn = on;
    QString str = "on";
    if (!autoSaveOn)
        str = "off";
    displayConsoleMessage("Autosave turned " + str);
    while(iter.hasNext()) {
        mw = iter.next();
        mw->setAutoSaveOn(autoSaveOn);
    }
    QSettings settings("fix8","logviewer");
    settings.setValue("AutoSave",autoSaveOn);
}
void Fix8Log::readSettings()
{
    QSettings settings("fix8","logviewer");
    autoSaveOn = (bool) settings.value("AutoSave",false).toBool();
    GUI::Globals::timeFormat  = (GUI::Globals::TimeFormat) settings.value("StartTimeFormat",GUI::Globals::HHMMSS).toInt();
    globalSchemaOn = settings.value("GlobalSchemaOn",false).toBool();
    emit notifyTimeFormatChanged(GUI::Globals::timeFormat);

}
void Fix8Log::writeSettings()
{
    QSettings settings("fix8","logviewer");


}
void Fix8Log::lastWindowClosedSlot()
{
    qDebug() << "Last Window closed";
}
void Fix8Log::cancelSessionRestoreSlot()
{
    cancelSessionRestore = true;
    displayConsoleMessage("Session Restore Cancelled");
}
void  Fix8Log::setTimeFormatSlot(GUI::Globals::TimeFormat tf)
{
    GUI::Globals::timeFormat = tf;
    // tell all main windows that format changed
    emit notifyTimeFormatChanged(tf);
    QSettings settings("fix8","logviewer");
    settings.setValue("StartTimeFormat",tf);
}
void  Fix8Log::modelDroppedSlot(FixMimeData* fmd)
{
    bool ok = false;
    WorkSheetData wsd;
    MainWindow *mw;
    MainWindow *mwSender = qobject_cast <MainWindow *> (sender());
    if (!mwSender) {
        qWarning() << "Invalid sender, drop failed " << __FILE__ << __LINE__;
    }
    QListIterator <MainWindow *> iter(mainWindows);
    while(iter.hasNext()) {
        mw = iter.next();
        if (mw->getUuid() == fmd->windowID) {
            wsd = mw->getWorksheetData(fmd->worksheetID, &ok);
            mwSender->finishDrop(wsd,fmd);
            break;
        }
    }
}
void  Fix8Log::editSchemaSlot(MainWindow *mw, QUuid workSheetID)
{
    bool ok;
    QString tabName;
    qDebug() << "EDIT SCHEMA SLOT" << __FILE__ << __LINE__;
    if (!schemaEditorDialog) {
        qDebug() << "\tcreate new scheme edit dialog";
        schemaEditorDialog = new SchemaEditorDialog(database,globalSchemaOn);
        schemaEditorDialog->setTableSchemas(tableSchemaList,defaultTableSchema);
        connect(schemaEditorDialog,SIGNAL(finished(int)),
                this,SLOT(schemaEditorFinishedSlot(int)));
        connect(schemaEditorDialog,SIGNAL(newSchemaCreated(TableSchema*)),
                this,SLOT(newSchemaCreatedSlot(TableSchema *)));
        schemaEditorDialog->restoreSettings();
    }
    qDebug() << "\tshow schema editor dialog...";
    QString windowName = mw->windowTitle();
    if (windowName.length() < 1)
        windowName = qApp->applicationName();
    qDebug() << "WINDOW NAME " << windowName << __FILE__;
    if (!workSheetID.isNull()) {
        WorkSheetData wsd = mw->getWorksheetData(workSheetID,&ok);
        if (ok) {
            qDebug() << "OK" << __FILE__ << __LINE__;
            if (wsd.tabAlias.length() >= 1)
                tabName = wsd.tabAlias;
            else
                tabName = wsd.fileName;
        }
    }
    schemaEditorDialog->setCurrentTarget(windowName,tabName);
    schemaEditorDialog->show();
}
void  Fix8Log::newSchemaCreatedSlot(TableSchema *ts)
{
    MainWindow *mw;
    QListIterator <MainWindow *> iter(mainWindows);
    while(iter.hasNext()) {
        mw = iter.next();
        mw->addNewSchema(ts);
    }

}
void  Fix8Log::schemaEditorFinishedSlot(int returnCode)
{
    if (returnCode == QDialogButtonBox::Close)
      schemaEditorDialog->close();
    schemaEditorDialog->saveSettings();

}
void Fix8Log::setGlobalSchemaOnSlot(bool b)
{
    QSettings settings("fix8","logviewer");
    globalSchemaOn = b;
    settings.setValue("GlobalSchemaOn",b);
}

bool Fix8Log::isGlobalSchemaOn()
{
    return globalSchemaOn;
}
