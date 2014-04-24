#ifndef DATABASE_H
#define DATABASE_H
#include <QObject>
#include <QByteArray>
#include <QColor>
#include <QList>
#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include "windowdata.h"
#include "worksheetdata.h"

#define LDB_DRIVER "QSQLITE"


class Database :public QObject
{
public:
    Database(QString fileName,QObject *parent);
    ~Database();
    typedef  enum {Windows,WorkSheet,NumOfTables} TableType;
    static QString tableNames[NumOfTables];
    static QString arguments[NumOfTables];
    bool createTable(TableType);
    QSqlDatabase *getHandle();
    QString getLastError();
    bool isOpen();
    bool open();
    bool tableIsValid(TableType);
    // Window Methods
    QList<WindowData> getWindows();
    bool deleteAllWindows();
    bool deleteWindow(int windowID);
    bool addWindow(const WindowData &);    
    // WorkSheets Methods
    QList <WorkSheetData> getWorkSheets(int windowID);
    bool addWorkSheet(WorkSheetData &);
    bool deleteWorkSheetByWindowID(int windowID);
    bool deleteWorkSheet(int workSheetID);
private:
    QString name;
    QSqlDatabase *handle;
    QString errorMessage;
    QSqlError sqlError;
};

#endif // DATABASE_H
