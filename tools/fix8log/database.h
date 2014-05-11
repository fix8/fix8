#ifndef DATABASE_H
#define DATABASE_H
#include <QObject>
#include <QByteArray>
#include <QColor>
#include <QList>
#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include "tableschema.h"
#include "windowdata.h"
#include "worksheetdata.h"

#define LDB_DRIVER "QSQLITE"


class Database :public QObject
{
public:
    Database(QString fileName,QObject *parent);
    ~Database();
    typedef  enum {SqlInfo,Windows,WorkSheet,TableSchemas,NumOfTables} TableType;
    static QString tableNames[NumOfTables];
    static QString arguments[NumOfTables];
    bool createTable(TableType);
    QSqlDatabase *getHandle();
    QString getLastError();
    int getVersion();
    bool setVersion(int);
    bool isOpen();
    bool open();
    bool tableIsValid(TableType);
    // Window Methods
    QList<WindowData> getWindows();
    bool deleteAllWindows();
    bool deleteWindow(int windowID);
    bool addWindow(WindowData &);
    // WorkSheets Methods
    QList <WorkSheetData> getWorkSheets(int windowID);
    bool addWorkSheet(WorkSheetData &);
    bool deleteAllWorkSheets();
    bool deleteWorkSheetByWindowID(int windowID);
    bool deleteWorkSheet(int workSheetID);
    // Table Schema Methods
    TableSchemaList *getTableSchemas();
    bool addTableSchema(TableSchema &);
    bool updateTableSchema(TableSchema &);
    bool deleteTableSchema(qint32 tableSchemaID);
private:
    QString name;
    QSqlDatabase *handle;
    QString errorMessage;
    QSqlError sqlError;
};

#endif // DATABASE_H
