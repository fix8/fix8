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

QString Database::tableNames[] = {"sqlinfo","windows","worksheets","tableschemas","schemafields","searchfunctions","filterfunctions"};
QString Database::arguments[] = {
    // sqlinfo
    "version integer",
    // windows
    "id INTEGER primary key,menubarStyleSheet char[256],geometry BLOB,restoreState BLOB, isVisible integer default 1,currentTab integer default 0, name char(32),tableSchemaID integer, searchAll integer default 0,searchFunction char[60], searchJavascript char[60], fix8sharedlib char[120], fontPtSize INTEGER",
    //worksheets
    "id INTEGER primary key, windowID integer,alias char(32), file char(120),selectedRow integer,splitterState BLOB,headerState BLOB,headerExpanded integer default 0, fieldsExpanded integer default 0,trailerExpanded integer default 0, searchFunction char[60], searchJavascript char[60], messageAreaHeaderState BLOB, fieldsExpansionType integer default 0",
    // tableschemas
    "id INTEGER primary key, name char(32), description char(120),locked integer default 0, xmlSchema char(36), fixSharedLibFile char(120)",
    // schemafields
    "id INTEGER primary key,name char(60),schemaID integer",
    // searchfunctions
    "id INTEGER primary key,alias char(32),function char(60), javascript char(60)",
    // filterfunctions
    "id INTEGER primary key,alias char(32),function char(60), javascript char(60)"
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
