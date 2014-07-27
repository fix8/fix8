
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
#include "searchfunction.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QVariant>
SearchFunctionList *Database::getSearchFunctions()
{
    SearchFunctionList *sfl = 0;
    bool bstatus;
    bool ok;
    QString str;
    if (!handle) {
        errorMessage = tr("Error in get schema fields  - handle is not initialized");
        qWarning() << errorMessage;
        return sfl;
    }
    QSqlQuery query(*handle);
    str =  "select * from searchfunctions";
    bstatus = query.prepare(str);
    if (bstatus == 0) {
        qWarning("Error in get search functions in prepare statement...");
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return sfl;
    }

    bstatus = query.exec();
    if (bstatus == false) {
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return sfl;
    }
    sfl = new SearchFunctionList();
    while (query.next()) {
        SearchFunction *sf = new SearchFunction();
        sf->id = query.value(0).toInt();
        sf->alias = query.value(1).toString();
        sf->function = query.value(2).toString();
        sf->javascript = query.value(3).toString();
        sfl->append(sf);
    }
    return sfl;
}
bool Database::addSearchFunction(SearchFunction &sf)
{
    bool bstatus = false;
    QString filter;
    if (!handle) {
        errorMessage = tr("Error database add search function  - handle is not initialized");
        qWarning() << errorMessage;
        return false;
    }
    QSqlQuery query(*handle);
    bstatus = query.prepare("INSERT INTO searchfunctions (id, alias,function, javascript)"
                            "VALUES(NULL, :alias, :function, :javascript)");
    if (bstatus == 0) {
        qWarning("Error database - add search function  failed in prepare statement...");
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return false;
    }
    query.bindValue(":alias",sf.alias);
    query.bindValue(":function",sf.function);
     query.bindValue(":javascript",sf.javascript);

    bstatus = query.exec();
    if (bstatus == 0) {
        qWarning("\tDatabase - Add search function  failed in exec statement...");
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << "*** ** * Last query =" << query.lastQuery();
        qWarning() << errorMessage;
        return false;
    }
    QVariant variant = query.lastInsertId();
    sf.id = variant.toInt();
    return true;
}
bool Database::updateSearchFunction(SearchFunction &sf)
{
    bool bstatus = false;
    if (!handle) {
        errorMessage = tr("Error - update search function, handle is not initialized");
        qWarning() << errorMessage;
        return false;
    }
    QSqlQuery query(*handle);
    QString str= "update searchfunctions set"
            + QString("  alias=:alias")
            + QString(", function=:function")
            + QString(",javascript=:javascript")
            + QString("  WHERE id='")  + QString::number(sf.id)
            + QString("'");

    bstatus = query.prepare(str);
    if (bstatus == 0) {
        qWarning("Error update table search function  failed in prepare statement...");
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return false;
    }
    query.bindValue(":alias",sf.alias);
    query.bindValue(":function",sf.function);
     query.bindValue(":javascript",sf.javascript);
    bstatus = query.exec();
    if (bstatus == 0) {
        qWarning("Error - update table search function failed in exec statement...");
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
    }
    return bstatus;
}
bool Database::removeSearchFunction(qint32 searchFunctionID)
{
    bool bstatus;
    QString filter;
    QString str;
    if (!handle) {
        errorMessage = tr("Error - delete search function   - handle is not initialized");
        qWarning() << errorMessage;
        return false;
    }
    QSqlQuery query(*handle);
    filter = "id=\'" + QString::number(searchFunctionID) + "\'";
    str  = "delete from searchfunctions where " + filter;
    bstatus = query.prepare(str);
    if (bstatus == false) {
        qWarning() << "Error - Delete searchfunction failed in prepare " << __FILE__ << __LINE__;
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
