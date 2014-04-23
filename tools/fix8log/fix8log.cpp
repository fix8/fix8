#include "database.h"
#include "fix8log.h"
#include "globals.h"
#include "windowdata.h"
#include <QApplication>
#include <QDebug>
#include <QtWidgets>

using namespace GUI;
Fix8Log::Fix8Log(QObject *parent) :
    QObject(parent),firstTimeToUse(false),database(0)
{
    Globals::Instance()->version = 0.1;
    Globals::Instance()->versionStr = "0.1";
    MainWindow *mw = new MainWindow();
    connect(mw,SIGNAL(createWindow(MainWindow*)),this,SLOT(createNewWindowSlot(MainWindow*)));
    connect(mw,SIGNAL(copyWindow(MainWindow*)),this,SLOT(copyWindowSlot(MainWindow*)));
    connect(mw,SIGNAL(deleteWindow(MainWindow*)),this,SLOT(deleteMainWindowSlot(MainWindow*)));
    connect(mw,SIGNAL(exitApp()),this,SLOT(exitAppSlot()));
    mw->show();
    mainWindows.append(mw);
}
void Fix8Log::createNewWindowSlot(MainWindow *mw)
{
    MainWindow *newMW  =new MainWindow(*mw);
    connect(newMW,SIGNAL(createWindow(MainWindow*)),this,SLOT(createNewWindowSlot(MainWindow*)));
    connect(newMW,SIGNAL(deleteWindow(MainWindow*)),this,SLOT(deleteMainWindowSlot(MainWindow*)));
    connect(newMW,SIGNAL(copyWindow(MainWindow*)),this,SLOT(copyWindowSlot(MainWindow*)));
    connect(newMW,SIGNAL(exitApp()),this,SLOT(exitAppSlot()));
    newMW->show();
    mainWindows.append(newMW);
}
void Fix8Log::copyWindowSlot(MainWindow *mw)
{
    MainWindow *newMW  =new MainWindow(*mw,true);
    connect(newMW,SIGNAL(createWindow(MainWindow*)),this,SLOT(createNewWindowSlot(MainWindow*)));
    connect(newMW,SIGNAL(copyWindow(MainWindow*)),this,SLOT(copyWindowSlot(MainWindow*)));
    connect(newMW,SIGNAL(deleteWindow(MainWindow*)),this,SLOT(deleteMainWindowSlot(MainWindow*)));
    connect(newMW,SIGNAL(exitApp()),this,SLOT(exitAppSlot()));
    newMW->show();
    mainWindows.append(newMW);
}
void Fix8Log::deleteMainWindowSlot(MainWindow *mw)
{
    qDebug() << "Here in delete";
    mainWindows.removeOne(mw);
    mw->deleteLater();
    if (mainWindows.count() < 1) {
        qDebug() << "Exit FROM APP, out of windows";
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
    qDebug() << "DB FILE NAME = " << dbFileName;
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
    return bstatus;
}
void Fix8Log::exitAppSlot()
{
    qDebug() << "Here in Exit App";
    MainWindow *mw;
    bool bstatus;
    bstatus = database->deleteAllWindows();
    QListIterator <MainWindow *> iter(mainWindows);
    while(iter.hasNext()) {
        mw = iter.next();
        WindowData wd = mw->getWindowData();
        bstatus = database->addWindow(wd);
        qDebug() << "Status of add window data to database = " << bstatus << __FILE__;
    }
    qApp->exit();
}
