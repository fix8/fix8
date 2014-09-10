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

#include "tableschema.h"
#include <QDebug>
TableSchema::TableSchema():id(-1),locked(false),fieldList(0)
{
}
TableSchema::TableSchema(QString Name, QString Description,bool Locked):id(-1),
    name(Name),description(Description),locked(Locked),fieldList(0)
{

}
TableSchema::TableSchema(QString Name, QString Description,bool Locked, QString SharedLib):id(-1),
    name(Name),description(Description),locked(Locked),sharedLib(SharedLib),fieldList(0)
{
}
TableSchema::TableSchema(const TableSchema &ts)
{
    name = ts.name;
    description = ts.description;
    locked = ts.locked;
    fieldNames = ts.fieldNames;
    fieldList = 0;
    id = ts.id;
    sharedLib = ts.sharedLib;
}
TableSchema::~TableSchema()
{
    qDebug() << "DELETE TABLE SCHEMA" << __FILE__ << __LINE__;
}

TableSchema & TableSchema::operator=( const TableSchema &rhs)
{
    if (this == &rhs)
        return *this;
    id          = rhs.id;
    name        = rhs.name;
    description = rhs.description;
    locked      = rhs.locked;
    fieldNames  = rhs.fieldNames;
    sharedLib   = rhs.sharedLib;
    fieldList   = rhs.fieldList;
    return *this;
}
bool   TableSchema::operator!=( const TableSchema &ts) const
{
    bool ok = false;
    if ((ts.name != name) && (ts.description != description)) {
        return true;
    }


    if (ts.fieldNames != fieldNames) {
        return true;
    }
    if (ts.fieldList && fieldList) {
        if (ts.fieldList && !fieldList) {
            if (ts.fieldList->count() != 0)
                return true;

        }
        if (!ts.fieldList && fieldList) {
            if (fieldList->count() != 0) {
                return true;
            }
        }

        if (ts.fieldList->count() != fieldList->count())
            return true;

            QListIterator <QBaseEntry *> iter(*ts.fieldList);
            QBaseEntry *be;
        while (iter.hasNext()) {
            be = iter.next();
            if (!fieldList->findByName(be->name))
            return true;
        }
    }
    if (sharedLib != ts.sharedLib)
        return true;
    return false;
}


bool   TableSchema::operator==( const TableSchema &ts) const
{
    bool ok = true;
    if ((ts.name != name) && (ts.description != description)) {
        return false;
    }
    if (ts.fieldNames != fieldNames) {
        return false;
    }
    if (ts.fieldList && !fieldList) {
        if (ts.fieldList->count() == 0)
            return true;
        else
            return false;
    }
    if (!ts.fieldList && !fieldList)
        return true;

    if (!ts.fieldList && fieldList) {
        if (fieldList->count() == 0)
            return true;
        else
            return false;
    }
    if (!ts.fieldList && !fieldList)
        return true;

    if (ts.fieldList->count() != fieldList->count()) {
        return false;
    }
    QListIterator <QBaseEntry *> iter(*ts.fieldList);
    QBaseEntry *be;
    while (iter.hasNext()) {
        be = iter.next();
        if (!fieldList->findByName(be->name)) {
            return false;
        }
    }
    if (sharedLib != ts.sharedLib)
        return false;
    return true;
}
TableSchema *TableSchema::clone()
{
    TableSchema *ts = new TableSchema(*this);
    if (fieldList) {
        ts->fieldList = fieldList->clone();
    }
    else
        ts->fieldList = 0;
    ts->sharedLib = sharedLib;
    return ts;
}

void TableSchema::setFields(QBaseEntryList * qel)
{
    fieldList = qel;
}
QStringList TableSchema::getColumnNames()
{
  return fieldNames;
}

QBaseEntryList *TableSchema::getFields()
{
    return fieldList;
}
void TableSchema::removeFieldByName(QString &name)
{
    if(fieldList)
        fieldList->removeByName(name);
}
void TableSchema::removeAllFields()
{
    if (fieldList) {
        fieldList->clear();
        qDeleteAll(fieldList->begin(),fieldList->end());
        delete fieldList;
        fieldList = 0;
    }
}
void TableSchema::addField(QBaseEntry *qbe)
{
    if (!qbe)
        return;
    if (!fieldList)
        fieldList = new QBaseEntryList();
    fieldList->append(qbe);
}

TableSchemaList::TableSchemaList():QList <TableSchema *>()
{

}

TableSchemaList::~TableSchemaList()
{
    qDebug() << "Delete Table Schema List" << __FILE__ << __LINE__;
}

TableSchema *TableSchemaList::findByID(qint32 id)
{
    TableSchema *ts = 0;
    QListIterator <TableSchema *> iter(*this);
    while(iter.hasNext()) {
        ts = iter.next();
        if (ts->id == id) {
            return ts;

        }
    }
    return 0;
}

TableSchema *TableSchemaList::findByName(const QString &name)
{
    TableSchema *ts = 0;
    QListIterator <TableSchema *> iter(*this);
    while(iter.hasNext()) {
        ts = iter.next();
        if (ts->name == name)
            return ts;
    }
    return 0;
}
TableSchema *TableSchemaList::findDefault()
{
    TableSchema *ts = 0;
    QListIterator <TableSchema *> iter(*this);
    while(iter.hasNext()) {
        ts = iter.next();
        if (ts->locked)
            return ts;
    }
    return 0;
}
