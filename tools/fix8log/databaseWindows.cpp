//-------------------------------------------------------------------------------------------------
/*
Fix8logviewer is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8logviewer Open Source FIX Log Viewer.
Copyright (C) 2010-14 David N Boosalis dboosalis@fix8.org, David L. Dight <fix@fix8.org>

Fix8logviewer is free software: you can  redistribute it and / or modify  it under the  terms of the
GNU Lesser General  Public License as  published  by the Free  Software Foundation,  either
version 3 of the License, or (at your option) any later version.

Fix8logviewer is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should  have received a copy of the GNU Lesser General Public  License along with Fix8.
If not, see <http://www.gnu.org/licenses/>.

BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*/
//-------------------------------------------------------------------------------------------------

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
        wd.tableSchema = 0;
        wd.id = query.value(0).toInt();
        wd.menubarStyleSheet = query.value(1).toString();
        wd.geometry = query.value(2).toByteArray();
        wd.state    = query.value(3).toByteArray();
        wd.isVisible    = query.value(4).toBool();
        wd.currentTab   = query.value(5).toInt(&ok);
        if (!ok)
            wd.currentTab = 0;
        wd.name = query.value(6).toString();
        wd.tableSchemaID = query.value(7).toInt(&ok);
        if (!ok)
            wd.tableSchemaID = -1;
        wd.searchAll = query.value(8).toBool();
        wd.searchFunction.function = query.value(9).toString();
        wd.searchFunction.javascript =  query.value(10).toString();
        wd.fix8sharedlib =  query.value(11).toString();
        wd.fontPtSize =  query.value(12).toInt();

        windowDataList.append(wd);
    }
    return windowDataList;
}
bool Database::addWindow(WindowData &wd)
{
    bool bstatus;
    QString filter;
    if (!handle) {
        errorMessage = tr("Error database addWindow  - handle is not initialized");
        qWarning() << errorMessage;
        return false;
    }
    QSqlQuery query(*handle);
    bstatus = query.prepare("INSERT INTO windows (id, menubarStyleSheet, geometry, restoreState, isVisible, currentTab, name,tableSchemaID, searchAll, searchFunction, searchJavascript,fix8sharedlib,fontPtSize)"
                            "VALUES(NULL, :menubarStyleSheet, :geometry, :restoreState, :isVisible, :currentTab, :name, :tableSchemaID, :searchAll, :searchFunction, :searchJavascript, :fix8sharedlib, :fontPtSize)");
    if (bstatus == 0) {
        qWarning("Error database - add window failed in prepare statement...");
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return false;
    }
    query.bindValue(":menubarStyleSheet",wd.menubarStyleSheet);
    query.bindValue(":geometry",wd.geometry);
    query.bindValue(":restoreState",wd.state);
    query.bindValue("isVisible",wd.isVisible);
    query.bindValue(":currentTab",wd.currentTab);
    query.bindValue(":name",wd.name);
    query.bindValue(":tableSchemaID",wd.tableSchemaID);
    query.bindValue(":searchAll",wd.searchAll);
    query.bindValue(":searchFunction",wd.searchFunction.function);
    query.bindValue(":searchJavascript",wd.searchFunction.javascript);
    query.bindValue(":fix8sharedlib",wd.fix8sharedlib);
    query.bindValue(":fontPtSize",wd.fontPtSize);


    bstatus = query.exec();
    if (bstatus == 0) {
        qWarning("\tDatabase - Add window failed in exec statement...");
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << "*** ** * Last query =" << query.lastQuery();
        qWarning() << errorMessage;
        return false;
    }
    QVariant variant = query.lastInsertId();
    wd.id = variant.toInt();
    return true;
}
bool Database::updateWindow(WindowData &wd)
{
    bool bstatus;
    if (!handle) {
        errorMessage = tr("Error in update window  - handle is not initialized");
        qWarning() << errorMessage;
        return false;
    }

    QSqlQuery query(*handle);
    QString str= "update windows set"
            + QString("  id=:id")
            + QString(", menubarStyleSheet=:menubarStyleSheet")
            + QString(", geometry=:geometry")
            + QString(", restoreState=:restoreState")
            + QString(", currentTab=:currentTab")
            + QString(", name=:name")
            + QString(", tableSchemaID=:tableSchemaID")
            + QString(", searchAll=:searchAll")
            + QString(", searchFunction=:searchFunction")
            + QString(", fix8sharedlib=:fix8sharedlib")
            + QString(", fontPtSize=:fontPtSize")
            + QString("  WHERE id='")  + QString::number(wd.id)
            + QString("'");

    bstatus = query.prepare(str);
    if (bstatus == 0) {
        qWarning("Error update window failed in prepare statement...");
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return false;
    }
    query.bindValue(":name",wd.name);
    query.bindValue(":menubarStyleSheet",wd.menubarStyleSheet);
    query.bindValue(":geometry",wd.geometry);
    query.bindValue(":restoreState",wd.state);
    query.bindValue(":tableSchemaID",wd.tableSchemaID);
    query.bindValue(":currentTab",wd.currentTab);
    query.bindValue(":searchAll",wd.searchAll);
    query.bindValue(":searchFunction",wd.searchFunction.function);
     query.bindValue(":searchJavascript",wd.searchFunction.javascript);
     query.bindValue(":fix8sharedlib",wd.fix8sharedlib);
     query.bindValue(":fontPtSize",wd.fontPtSize);
    bstatus = query.exec();
    if (bstatus == 0) {
        qWarning("Error - update window failed in exec statement...");
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
    }
    return bstatus;
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
    }
    bstatus = query.exec();
    if (bstatus == false) {
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
    }
done:
    bstatus = deleteWorkSheetByWindowID(windowID);
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

