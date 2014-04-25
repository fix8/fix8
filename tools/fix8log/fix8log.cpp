#include "database.h"
#include "fix8log.h"
#include "globals.h"
#include "windowdata.h"
#include <QApplication>
#include <QDebug>
#include <QtWidgets>

using namespace GUI;
Fix8Log::Fix8Log(QObject *parent) :
    QObject(parent),firstTimeToUse(false),database(0),autoSaveOn(false)
{
    Globals::Instance()->version = 0.1;
    Globals::Instance()->versionStr = "0.1";
}
void Fix8Log::createNewWindowSlot(MainWindow *mw)
{
    MainWindow *newMW  =new MainWindow(*mw);
    wireSignalAndSlots(newMW);
    newMW->show();
    mainWindows.append(newMW);
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
    mw->setAutoSaveOn(autoSaveOn);
}
void Fix8Log::deleteMainWindowSlot(MainWindow *mw)
{
    mainWindows.removeOne(mw);
    mw->deleteLater();
    if (mainWindows.count() < 1) {
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
    QStandardItemModel *model;
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
    QList <WindowData> windowDataList = database->getWindows();
    QListIterator <WindowData> iter(windowDataList);
    QStringList errorStrList;
    MainWindow *newMW;
    QMap <QString, QStandardItemModel *>::iterator currentItemIter;
    bool haveError = false;
    if (autoSaveOn){
        while(iter.hasNext()) {
            WindowData wd = iter.next();
            QList <WorkSheetData> wsdList = database->getWorkSheets(wd.id);
            if (wsdList.count() > 0) {
                newMW  =new MainWindow();
                newMW->setWindowData(wd);
                QListIterator <WorkSheetData> iter2(wsdList);
                while(iter2.hasNext()) {
                    model = 0;
                    WorkSheetData wsd = iter2.next();
                    currentItemIter =  fileNameModelMap.find(wsd.fileName);
                    if (currentItemIter != fileNameModelMap.end())
                        model = currentItemIter.value();
                    else {
                        model = readLogFile(wsd.fileName,errorStr);
                        if (!model)
                            errorStrList.append(errorStr);
                        else
                            fileNameModelMap.insert(wsd.fileName,model);
                    }
                    if (model) {
                        newMW->addWorkSheet(model,wsd);
                        wireSignalAndSlots(newMW);
                        mainWindows.append(newMW);
                        newMW->setAutoSaveOn(autoSaveOn);
                        newMW->show();
                    }
                }
            }
        }
        qDebug() << "TODO - Display error messages here, and if no work sheets created lets delete main window" << __FILE__ << __LINE__;
        displayConsoleMessage("Session restored from autosave");
    }
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
void Fix8Log::exitAppSlot()
{
    MainWindow *mw;
    WorkSheetData wsd;
    bool bstatus;
    if (autoSaveOn) {
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
            if (!bstatus)
                displayConsoleMessage("Error failed saving window to database",GUI::Message::ErrorMsg);
            else {
                QList<WorkSheetData> wsdList = mw->getWorksheetData(wd.id);
                qDebug() << "Save worksheet data one at a time, later do batch for speed" << __FILE__ << __LINE__;
                QListIterator <WorkSheetData> wsdIter(wsdList);
                while(wsdIter.hasNext()) {
                    wsd = wsdIter.next();
                    bstatus = database->addWorkSheet(wsd);
                    qDebug() << "Status of add work sheet to database = " << bstatus;
                }
            }
        }
    }
    writeSettings();
    qApp->exit();
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
}
void Fix8Log::writeSettings()
{
    QSettings settings("fix8","logviewer");
    settings.setValue("AutoSave",autoSaveOn);
}

