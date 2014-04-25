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
    void createNewWindowSlot(MainWindow *mw=0);
    void copyWindowSlot(MainWindow *mw);
    void deleteMainWindowSlot(MainWindow *mw);
    void displayConsoleMessage(GUI::Message);
    void displayConsoleMessage(QString, GUI::Message::MessageType = GUI::Message::InfoMsg);
    void exitAppSlot();
protected:
    void wireSignalAndSlots(MainWindow *mw);
    QStandardItemModel *readLogFile(const QString &fileName,QString &errorStr);

    QList <MainWindow *> mainWindows;
    bool firstTimeToUse;
    Database *database;
    bool autoSaveOn;
    QMap <QString, QStandardItemModel *> fileNameModelMap;
};

#endif // FIX8LOG_H
