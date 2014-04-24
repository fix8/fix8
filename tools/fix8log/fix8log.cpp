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
bool Fix8Log::init()
{
    bool bstatus;
    bool createdTable = false;
    QString dbFileName = "fix8log.sql";
    QString errorStr;
    QString dbPath = QCoreApplication::applicationDirPath() + QDir::separator()  +  "share";
    QDir dir(dbPath);


    readSettings();
    qDebug() << "AFTER READ SETTINS, autosave = " << autoSaveOn << __FILE__ << __LINE__;
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
            errorStr =  "Failed to Create Windows Table.";
            displayConsoleMessage(GUI::Message(errorStr,GUI::Message::ErrorMsg));
        }
    }
    bstatus = database->tableIsValid(Database::WorkSheet);
    if (!bstatus) {
        bstatus = database->createTable(Database::WorkSheet);
        if (!bstatus) {
            errorStr = "Failed to Create Worksheet Table.";
            displayConsoleMessage(GUI::Message(errorStr,GUI::Message::ErrorMsg));
        }
    }
    QList <WindowData> windowDataList = database->getWindows();
    QListIterator <WindowData> iter(windowDataList);
    MainWindow *newMW;
    if (autoSaveOn){
        while(iter.hasNext()) {
            WindowData wd = iter.next();
            newMW  =new MainWindow();
            wireSignalAndSlots(newMW);
            newMW->setWindowData(wd);
            mainWindows.append(newMW);
            newMW->setAutoSaveOn(autoSaveOn);
            newMW->show();
            qDebug() << "\tWindow FROM Database created";
        }
    }
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
    bool bstatus;
    bstatus = database->deleteAllWindows();
    QListIterator <MainWindow *> iter(mainWindows);
    while(iter.hasNext()) {
        mw = iter.next();
        WindowData wd = mw->getWindowData();
        bstatus = database->addWindow(wd);
    }
    writeSettings();
    qApp->exit();
}
void Fix8Log::autoSaveOnSlot(bool on)
{
    MainWindow *mw;
    QListIterator <MainWindow *> iter(mainWindows);
    autoSaveOn = on;
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
