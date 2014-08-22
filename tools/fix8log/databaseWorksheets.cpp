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
        wd.headerExpanded = query.value(7).toBool();
        wd.fieldsExpanded = query.value(8).toBool();
        wd.trailerExpanded = query.value(9).toBool();
        wd.searchFunction.function = query.value(10).toString();
        wd.searchFunction.javascript = query.value(11).toString();
        wd.messageHeaderState = query.value(12).toByteArray();
        wd.fieldsExpansionType = query.value(13).toUInt();
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
    bstatus = query.prepare("INSERT INTO worksheets (id,windowID,alias, file ,selectedRow,splitterState,headerState,headerExpanded,fieldsExpanded,trailerExpanded,searchFunction,searchJavascript, messageAreaHeaderState,fieldsExpansionType)"
                            "VALUES(NULL,:windowID,:alias, :file ,:selectedRow,:splitterState,:headerState,:headerExpanded,:fieldsExpanded,:tailerExpanded,:searchFunction, :searchJavascript,:messageAreaHeaderState, :fieldsExpansionType)");
    if (bstatus == 0) {
        qWarning("Error database - add worksheet failed in prepare statement...");
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return false;
    }
    query.bindValue(":windowID",wsd.windowID);
    query.bindValue(":alias",wsd.tabAlias);
    query.bindValue(":file",wsd.fileName);
    query.bindValue(":selectedRow",wsd.selectedRow);
    query.bindValue(":splitterState",wsd.splitterState);
    query.bindValue(":headerState",wsd.headerState);
    query.bindValue(":headerExpanded",wsd.headerExpanded);
    query.bindValue(":fieldsExpanded",wsd.fieldsExpanded);
    query.bindValue(":trailerExpanded",wsd.trailerExpanded);
    query.bindValue(":searchFunction",wsd.searchFunction.function);
    query.bindValue(":searchJavascript",wsd.searchFunction.javascript);
    query.bindValue(":messageAreaHeaderState",wsd.messageHeaderState);
    query.bindValue(":fieldsExpansionType",wsd.fieldsExpansionType);

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
