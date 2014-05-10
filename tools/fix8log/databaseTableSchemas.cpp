#include "database.h"
#include "tableschema.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QVariant>


TableSchemaList *Database::getTableSchemas()
{
    TableSchemaList *tsl=0;
    bool bstatus;
    bool ok;
    QString str;
    if (!handle) {
        errorMessage = tr("Error in get table schemas  - handle is not initialized");
        qWarning() << errorMessage;
        return tsl;
    }
    QSqlQuery query(*handle);
    str = "select * from tableschemas";
    bstatus = query.prepare(str);
    if (bstatus == 0) {
        qWarning("Error in get table schemas in prepare statement...");
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return tsl;
    }

    bstatus = query.exec();
    if (bstatus == false) {
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return tsl;
    }
    tsl = new TableSchemaList();
    while (query.next()) {
        TableSchema *ts = new TableSchema();
        ts->id          = query.value(0).toInt(&ok);
        ts->name        = query.value(1).toString();
        ts->description = query.value(2).toString();
        ts->locked      = query.value(3).toBool();
        tsl->append(ts);
    }
    if(tsl->count() < 1) {
        delete tsl;
        tsl = 0;
    }
    return tsl;
}
bool Database::addTableSchema(TableSchema &ts)
{
    bool bstatus = false;
    QString filter;
    if (!handle) {
        errorMessage = tr("Error database add table schema   - handle is not initialized");
        qWarning() << errorMessage;
        return false;
    }
    QSqlQuery query(*handle);
    bstatus = query.prepare("INSERT INTO tableschemas (id, name,description, locked)"
                            "VALUES(NULL, :name, :description, :locked)");
    if (bstatus == 0) {
        qWarning("Error database - add table schema  failed in prepare statement...");
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return false;
    }
    query.bindValue(":name",ts.name);
    query.bindValue(":description",ts.description);
    query.bindValue(":locked",ts.locked);

    bstatus = query.exec();
    if (bstatus == 0) {
        qWarning("\tDatabase - Add table schema  failed in exec statement...");
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << "*** ** * Last query =" << query.lastQuery();
        qWarning() << errorMessage;
        return false;
    }
    QVariant variant = query.lastInsertId();
    ts.id = variant.toInt();
    return true;
}
bool Database::updateTableSchema(TableSchema &ts)
{
    bool bstatus = false;
    if (!handle) {
        errorMessage = tr("Error - update table schema, handle is not initialized");
        qWarning() << errorMessage;
        return false;
    }
    QSqlQuery query(*handle);
    QString str= "update tableschemas set"
            + QString("  name=:name")
            + QString(", description=:description")
            + QString(", locked=:locked")
            + QString("  WHERE id='")  + QString::number(ts.id)
            + QString("'");

    bstatus = query.prepare(str);
    if (bstatus == 0) {
        qWarning("Error update table schema  failed in prepare statement...");
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return false;
    }
    query.bindValue(":name",ts.name);
    query.bindValue(":description",ts.description);
    query.bindValue(":locked",ts.locked);
    bstatus = query.exec();
    if (bstatus == 0) {
        qWarning("Error - update table schema  failed in exec statement...");
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
    }
    return bstatus;
}
bool Database::deleteTableSchema(qint32 tableSchemaID)
{
    bool bstatus;
    QString filter;
    QString str;
    if (!handle) {
        errorMessage = tr("Error - delete table schema  - handle is not initialized");
        qWarning() << errorMessage;
        return false;
    }
    QSqlQuery query(*handle);
    filter = "id=\'" + QString::number(tableSchemaID) + "\'";
    str  = "delete from tableschemas where " + filter;
    bstatus = query.prepare(str);
    if (bstatus == false) {
        qWarning() << "Error - Delete table schema failed in prepare " << __FILE__ << __LINE__;
        sqlError = query.lastError();
        goto done;
    }
    bstatus = query.exec();
    if (bstatus == false) {
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
    }
done:
    qDebug() << "Add delete columns when we have this table" << __FILE__ << __LINE__;
    //bstatus = deleteWorkSheetByWindowID(windowID);
    return bstatus;
}
