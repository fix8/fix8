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
#include "tableschema.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QVariant>

QStringList Database::getSchemaFields(int schemaID)
{
    QStringList fieldList;
    bool bstatus;
    bool ok;
    QString str;
    QString filter;
    if (!handle) {
        errorMessage = tr("Error in get schema fields  - handle is not initialized");
        qWarning() << errorMessage;
        return fieldList;
    }
    QSqlQuery query(*handle);
    filter = "schemaID = " + QString::number(schemaID);
    str =  "select * from schemafields where " + filter;
    bstatus = query.prepare(str);
    if (bstatus == 0) {
        qWarning("Error in get schema fields in prepare statement...");
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return fieldList;
    }

    bstatus = query.exec();
    if (bstatus == false) {
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return fieldList;
    }
    while (query.next()) {
        str = query.value(1).toString();
        fieldList.append(str);
    }
    return fieldList;
}

bool Database::addSchemaFields(int schemaID, QStringList FieldNames)
{
    bool bstatus = false;
    QString filter;
    filter = "schemaID=\'" + QString::number(schemaID) + "\'";
    if (!handle) {
        errorMessage = tr("Error addSchemaFields  - handle is not initialized");
        qWarning() << errorMessage;
        return false;
    }
    QSqlQuery query(*handle);

    // delete old ones
    QString str  = "delete from schemafields where " + filter;
    bstatus = query.prepare(str);
    if (bstatus == 0) {
        qWarning("addSchemaFields failed in prepare statement of delete fields");
        sqlError = query.lastError();
        errorMessage = sqlError.databaseText();
        qWarning() << errorMessage;
        return false;
    }
    QVariantList nullIDs;
    QVariantList schemaIDs;
    QVariantList fieldNames;
    QStringListIterator iter(FieldNames);
    query.prepare("insert into schemafields values (?, ?, ?)");
    while(iter.hasNext()) {
        nullIDs << QVariant(QVariant::Int);
        schemaIDs << schemaID;
        fieldNames << iter.next();
    }
    query.addBindValue(nullIDs);
    query.addBindValue(fieldNames);
    query.addBindValue(schemaIDs);

    if (!query.execBatch()) {
        qWarning() << query.lastError();
        return false;
    }
    return bstatus;
}
bool Database::removeSchemaFields(int schemaID)
{
    bool bstatus = false;
    QString filter;
    QString str;
    if (!handle) {
        errorMessage = tr("Error - remove schema fields  - handle is not initialized");
        qWarning() << errorMessage;
        return false;
    }
    QSqlQuery query(*handle);
    filter = "schemaID=\'" + QString::number(schemaID) + "\'";
    str  = "delete from schemafields where " + filter;
    bstatus = query.prepare(str);
    qDebug() << "DETE STR = :" << str;
    if (bstatus == false) {
        qWarning() << "Delete schemafields failed in prepare " << __FILE__ << __LINE__;
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
bool Database::saveTableSchemaFields(TableSchema &ts)
{
   bool bstatus;
   QString fieldName;
   QStringList fieldNames;
   QBaseEntry *qbe;
   bstatus = removeSchemaFields(ts.id);
   qDebug() << "Status of remove shema fields " << bstatus << __FILE__;
   if (! ts.fieldList || ts.fieldList->count() < 1) {
       qDebug() << "No fields to add for schema " << __FILE__ << __LINE__;
       return true;
   }
   if (bstatus) {
       QListIterator <QBaseEntry *> iter(*ts.fieldList);
       while(iter.hasNext()) {
          qbe = iter.next();
          fieldNames << qbe->name;
       }
   }
   qDebug() << "updates table schema to have this many fields:" << fieldNames.count();
   bstatus =   addSchemaFields(ts.id,fieldNames);
   return bstatus;
}
