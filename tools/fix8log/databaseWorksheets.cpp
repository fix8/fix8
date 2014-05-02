#include "database.h"
#include "worksheetdata.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QVariant>

QList <WorkSheetData> Database::getWorkSheets(int windowID)
{
    QList <WorkSheetData> wsdList;
    bool bstatus;
    bool ok;
    int red,green,blue;
    QString str;
    QString filter;
    if (!handle) {
        errorMessage = tr("Error in get worksheets  - handle is not initialized");
        qWarning() << errorMessage;
        return wsdList;
    }
    QSqlQuery query(*handle);
    filter = "windowID = " + QString::number(windowID);
    str =  "select * from worksheets where " + filter;
    bstatus = query.prepare(str);
    if (bstatus == 0) {
        qWarning("Error in get worksheets in prepare statement...");
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return wsdList;
    }
    bstatus = query.exec();
    if (bstatus == false) {
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return wsdList;
    }
    while (query.next()) {
        WorkSheetData wd;
        wd.id = query.value(0).toInt(&ok);
        wd.windowID  = query.value(1).toInt(&ok);
        wd.tabAlias = query.value(2).toString();
        wd.fileName = query.value(3).toString();
        wd.selectedRow = query.value(4).toInt();
        wd.splitterState    = query.value(5).toByteArray();
        wd.headerState = query.value(6).toByteArray();
        wsdList.append(wd);
    }
    return wsdList;
}

bool Database::addWorkSheet(WorkSheetData &wsd)
{
    bool bstatus = false;
    QString filter;
    if (!handle) {
        errorMessage = tr("Error database add worksheet  - handle is not initialized");
        qWarning() << errorMessage;
        return false;
    }
    QSqlQuery query(*handle);
    bstatus = query.prepare("INSERT INTO worksheets (id,windowID,alias, file ,selectedRow,splitterState,headerState)"
                            "VALUES(NULL,:windowID,:alias, :file ,:selectedRow,:splitterState,:headerState)");
    if (bstatus == 0) {
        qWarning("Error database - add worksheet failed in prepare statement...");
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return false;
    }
    qDebug() << "Database save work sheet with window id = " << wsd.windowID << __FILE__ << __LINE__;
    query.bindValue(":windowID",wsd.windowID);
    query.bindValue(":alias",wsd.tabAlias);
    query.bindValue(":file",wsd.fileName);
    query.bindValue(":selectedRow",wsd.selectedRow);
    query.bindValue(":splitterState",wsd.splitterState);
    query.bindValue(":headerState",wsd.headerState);
    bstatus = query.exec();
    if (bstatus == 0) {
        qWarning("\tDatabase - Add worksheet failed in exec statement...");
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << "*** ** * Last query =" << query.lastQuery();
        qWarning() << errorMessage;
        return false;
    }
    return true;
}
bool Database::deleteWorkSheetByWindowID(int windowID)
{
    bool bstatus = false;
    QString filter;
    QString str;
    if (!handle) {
        errorMessage = tr("Error - delete worksheet  - handle is not initialized");
        qWarning() << errorMessage;
        return false;
    }
    QSqlQuery query(*handle);
    filter = "windowID=\'" + QString::number(windowID) + "\'";
    str  = "delete from worksheets where " + filter;
    bstatus = query.prepare(str);
    if (bstatus == false) {
        qWarning() << "Delete worksheet failed in prepare " << __FILE__ << __LINE__;
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
    return bstatus;
}
bool Database::deleteWorkSheet(int workSheetID)
{
    bool bstatus = false;
    QString filter;
    QString str;
    if (!handle) {
        errorMessage = tr("Error - delete worksheet  - handle is not initialized");
        qWarning() << errorMessage;
        return false;
    }
    QSqlQuery query(*handle);
    filter = "id=\'" + QString::number(workSheetID) + "\'";
    str  = "delete from worksheets where " + filter;
    bstatus = query.prepare(str);
    if (bstatus == false) {
        qWarning() << "Delete worksheet failed in prepare " << __FILE__ << __LINE__;
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
    return bstatus;
}
bool Database::deleteAllWorkSheets()
{
    if (!handle) {
        errorMessage = tr("Error deleteAllWorkSheets  - handle is not initialized");
        qWarning() << errorMessage;
        return false;
    }
    QSqlQuery query(*handle);
    QString str  = "delete from worksheets";
    bool bstatus = query.prepare(str);
    if (bstatus == 0) {
        qWarning() << "Error Database - delete all worksheets in prepare statement"
                   << __FILE__ << __LINE__;
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return false;
    }
    bstatus = query.exec();
    if (bstatus == false) {
        qWarning() << "Delete all in worksheets failed"
                   << __FILE__ << __LINE__;

        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return false;
    }
    return true;
}
