#include "fix8log.h"
#include "globals.h"
#include <QApplication>
#include <QDebug>
using namespace GUI;
Fix8Log::Fix8Log(QObject *parent) :
    QObject(parent)
{
    Globals::Instance()->version = 0.1;
    Globals::Instance()->versionStr = "0.1";
    MainWindow *mw = new MainWindow();
    connect(mw,SIGNAL(createWindow(MainWindow*)),this,SLOT(createNewWindowSlot(MainWindow*)));
    connect(mw,SIGNAL(copyWindow(MainWindow*)),this,SLOT(copyWindowSlot(MainWindow*)));
    connect(mw,SIGNAL(deleteWindow(MainWindow*)),this,SLOT(deleteMainWindowSlot(MainWindow*)));
    mw->show();
    mainWindows.append(mw);
}
void Fix8Log::createNewWindowSlot(MainWindow *mw)
{
    qDebug() << "HERE IN CREATE NEW WINDOW";
    MainWindow *newMW  =new MainWindow(*mw);
    connect(newMW,SIGNAL(createWindow(MainWindow*)),this,SLOT(createNewWindowSlot(MainWindow*)));
    connect(newMW,SIGNAL(deleteWindow(MainWindow*)),this,SLOT(deleteMainWindowSlot(MainWindow*)));
    connect(newMW,SIGNAL(copyWindow(MainWindow*)),this,SLOT(copyWindowSlot(MainWindow*)));

    newMW->show();
    mainWindows.append(newMW);
}
void Fix8Log::copyWindowSlot(MainWindow *mw)
{
    qDebug() << "HERE IN CREATE NEW WINDOW";
    MainWindow *newMW  =new MainWindow(*mw,true);
    connect(newMW,SIGNAL(createWindow(MainWindow*)),this,SLOT(createNewWindowSlot(MainWindow*)));
    connect(newMW,SIGNAL(copyWindow(MainWindow*)),this,SLOT(copyWindowSlot(MainWindow*)));
    connect(newMW,SIGNAL(deleteWindow(MainWindow*)),this,SLOT(deleteMainWindowSlot(MainWindow*)));

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
