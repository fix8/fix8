#include "database.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QVariant>

QString Database::tableNames[] = {"sqlinfo","windows","worksheets","tableschemas"};
QString Database::arguments[] = {
    // sqlinfo
    "version integer",
    // windows
    "id INTEGER primary key, red integer, green integer,blue  integer,geometry BLOB,restoreState BLOB, isVisible integer default 1,currentTab integer default 0, name char(32),tableSchemaID integer",
    //worksheets
    "id INTEGER primary key, windowID integer,alias char(32), file char(120),selectedRow integer,splitterState BLOB,headerState BLOB",
    // tableSchemas
    //worksheets
    "id INTEGER primary key, name char(32), description char(120),locked integer default 0",
};

Database::Database(QString fileName,QObject *parent):QObject(parent),name(fileName),handle(0)
{

}
Database::~Database()
{
    if (handle) {
        if (handle->isOpen()) {
            handle->close();
            delete handle;
        }
        else
            qWarning() << "handle not open...." __FILE__;
        QSqlDatabase::removeDatabase(name);
    }
}
bool Database::tableIsValid(TableType tt)
{
    QString str;
    QString str1;
    QSqlQuery query;
    QString ttname = tableNames[tt];
    bool bstatus = false;
    if (!handle) {
        errorMessage = tr("Error Creating Database - handle is not initialized");
        qWarning() << errorMessage;
        goto done;
    }
    if (handle->isOpen() == false) {
        errorMessage = "Database not open, cannot validate table";
        qWarning() << errorMessage;
        goto done;
    }
    query = QSqlQuery(*handle);
    str = "select * from " +  tableNames[tt];
    bstatus = query.exec(str);
    if (bstatus == 0) {
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << "exec failed - " << errorMessage;
        goto done;
    }
done:
    return bstatus;
}
QString Database::getLastError()
{
    QString str;
    QSqlError qsqlError;
    if (handle) {
        qsqlError = handle->lastError();
        str = qsqlError.text();
    }
    return str;
}
QSqlDatabase *Database::getHandle()
{
    return handle;
}
bool Database::isOpen()
{
    bool status = false;
    if (handle) {
        status = handle->isOpen();
    }
    return status;
}
bool Database::open()
{
    bool isopen = false;
    QSqlError qsqlError;
    QString str;
    if ( handle) {
        if (handle->isOpen()) {
            errorMessage = tr("Closing local database, before openning: ") + name;
            qWarning() << errorMessage;
            handle->close();
        }
        delete handle;
    }
    handle = new QSqlDatabase(QSqlDatabase::addDatabase(LDB_DRIVER,name));
    if (!handle) {
        errorMessage = tr("Cannot open local database:" ) + name;
        errorMessage.append(tr(" Handle not created"));
        goto done;
    }
    handle->setDatabaseName(name);

    isopen = handle->open();
    if (!isopen) {
        qsqlError = handle->lastError();
        errorMessage = qsqlError.text();
    }
done:
    if (isopen == false) {
        qWarning() << errorMessage;
        qWarning() << "Openning database " + name;
    }
    return(isopen);
}
bool Database::createTable(TableType tt)
{
    bool bstatus = false;
    QSqlQuery query;
    QString name;
    QString args;
    QString  str;
    QSqlError qsqlError;
    name = tableNames[tt];
    args = arguments[tt];
    if (!handle) {
        errorMessage = tr("Error Creating Database - handle is not initialized");
        qWarning() << errorMessage;
        goto done;
    }
    if (handle->isValid() == false) {
        errorMessage = tr("Error Creating Database - not a valid handle");
        qWarning() << errorMessage;
        goto done;
    }
    query = QSqlQuery(*handle);
    if (name.length() ==  0) {
        errorMessage = tr("Error create tabe: no value given for name");
        qWarning() << errorMessage;
        goto done;
    }
    str = "Create table " + name + " ( " + args + " )";
    bstatus = query.exec(str);

    if (!bstatus) {
        qsqlError  = query.lastError();
        errorMessage = qsqlError.text();
        qWarning() << errorMessage;
    }
done:
    return bstatus;
}
int Database::getVersion()
{
    int version = -1;
    bool bstatus;
    bool ok;
    QString str;
    if (!handle) {
        errorMessage = tr("Error in get database version  - handle is not initialized");
        qWarning() << errorMessage;
        return -1;
    }
    QSqlQuery query(*handle);
    str = "select * from sqlinfo";
    bstatus = query.prepare(str);
    if (bstatus == 0) {
        qWarning("Error in get sql version in prepare statement...");
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return -1;
    }

    bstatus = query.exec();
    if (bstatus == false) {
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return -1;
    }
    while (query.next()) {
        version = query.value(0).toInt(&ok);
        if (!ok) {
            errorMessage = "Invalid database version found";
            return  -1;
        }
        break;
    }
    return version;
}
bool Database::setVersion(int nwVersion)
{
    // to be done
    return true;
}
