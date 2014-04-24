#include "database.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QVariant>

QList<WindowData> Database::getWindows()
{
    QList<WindowData> windowDataList;
    bool bstatus;
    bool ok;
    int red,green,blue;
    QString str;
    if (!handle) {
        errorMessage = tr("Error in get windows  - handle is not initialized");
        qWarning() << errorMessage;
        return windowDataList;
    }
    QSqlQuery query(*handle);
    str = "select * from windows";
    bstatus = query.prepare(str);
    if (bstatus == 0) {
        qWarning("Error in get windows in prepare statement...");
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return windowDataList;
    }

    bstatus = query.exec();
    if (bstatus == false) {
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return windowDataList;
    }
    while (query.next()) {
        WindowData wd;
        wd.id = query.value(0).toInt(&ok);
        red   = query.value(1).toInt(&ok);
        green = query.value(2).toInt(&ok);
        blue  = query.value(3).toInt(&ok);
        wd.color    = QColor(red,green,blue);
        wd.geometry = query.value(4).toByteArray();

        wd.state    = query.value(5).toByteArray();
        windowDataList.append(wd);
    }
    return windowDataList;
}
bool Database::addWindow(const WindowData &wd)
{
    bool bstatus;
    QString filter;


    if (!handle) {
        errorMessage = tr("Error database addWindow  - handle is not initialized");
        qWarning() << errorMessage;
        return false;
    }
    QSqlQuery query(*handle);
    bstatus = query.prepare("INSERT INTO windows (id, red, green, blue, geometry, restoreState, isVisible)"
                            "VALUES(NULL, :red, :green, :blue, :geometry, :restoreState, :isVisible)");
    if (bstatus == 0) {
        qWarning("Error database - add window failed in prepare statement...");
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return false;
    }
    query.bindValue(":red",wd.color.red());
    query.bindValue(":green",wd.color.green());
    query.bindValue(":blue",wd.color.blue());
    query.bindValue(":geometry",wd.geometry);
    query.bindValue(":restoreState",wd.state);
    bstatus = query.exec();
    if (bstatus == 0) {
        qWarning("\tDatabase - Add window failed in exec statement...");
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << "*** ** * Last query =" << query.lastQuery();
        qWarning() << errorMessage;
        return false;
    }
    return true;
}
bool Database::deleteWindow(int windowID)
{
    bool   bstatus = false;
    QString filter;
    QString str;
    if (!handle) {
        errorMessage = tr("Error - delete window  - handle is not initialized");
        qWarning() << errorMessage;
        return false;
    }
    QSqlQuery query(*handle);
    filter = "id=\'" + QString::number(windowID) + "\'";
    str  = "delete from windows where " + filter;
    bstatus = query.prepare(str);
    if (bstatus == false) {
        qWarning() << "Delete window failed in prepare " << __FILE__ << __LINE__;
        sqlError = query.lastError();
        goto done;
        bstatus = query.exec();
        if (bstatus == false) {
            sqlError = query.lastError();
            errorMessage = sqlError.databaseText();
            qWarning() << errorMessage;
        }
    }
    // DELETE TABLES
done:
    return bstatus;
}

bool Database::deleteAllWindows()
{
    if (!handle) {
        errorMessage = tr("Error deleteAllWindows  - handle is not initialized");
        qWarning() << errorMessage;
        return false;
    }
    QSqlQuery query(*handle);
    QString str  = "delete from windows";
    bool bstatus = query.prepare(str);
    if (bstatus == 0) {
        qWarning() << "Error Database - delete all windows in prepare statement"
                   << __FILE__ << __LINE__;
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return false;
    }
    bstatus = query.exec();
    if (bstatus == false) {
        qWarning() << "Delete all in windows failed"
                   << __FILE__ << __LINE__;

        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return false;
    }
    return true;
}

