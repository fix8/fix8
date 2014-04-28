#ifndef FIX8LOG_H
#define FIX8LOG_H

#include <QObject>
#include <QMap>
#include <QString>
#include <globals.h>
#include "mainwindow.h"
class QStandardItemModel;
class Database;

class Fix8Log : public QObject
{
    Q_OBJECT
public:
    explicit Fix8Log(QObject *parent = 0);
    bool init();
    void readSettings();
    void writeSettings();
public slots:
    void autoSaveOnSlot(bool);
    void cancelSessionRestoreSlot();
    void createNewWindowSlot(MainWindow *mw=0);
    void copyWindowSlot(MainWindow *mw);
    void deleteMainWindowSlot(MainWindow *mw);
    void displayConsoleMessage(GUI::Message);
    void displayConsoleMessage(QString, GUI::Message::MessageType = GUI::Message::InfoMsg);
    void exitAppSlot();
    void lastWindowClosedSlot();
    void  setTimeFormatSlot(GUI::Globals::TimeFormat);
protected:
    QStandardItemModel *readLogFile(const QString &fileName,QString &errorStr);
    void saveSession();
    void wireSignalAndSlots(MainWindow *mw);
    QList <MainWindow *> mainWindows;
    bool firstTimeToUse;
    Database *database;
    bool autoSaveOn;
    QMap <QString, QStandardItemModel *> fileNameModelMap;
    bool cancelSessionRestore;
signals:
    void notifyTimeFormatChanged(GUI::Globals::TimeFormat);
};

#endif // FIX8LOG_H
