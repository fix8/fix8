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
#ifndef DATABASE_H
#define DATABASE_H
#include <QObject>
#include <QByteArray>
#include <QColor>
#include <QList>
#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include "searchfunction.h"
#include "tableschema.h"
#include "windowdata.h"
#include "worksheetdata.h"

#define LDB_DRIVER "QSQLITE"


class Database :public QObject
{
public:
    Database(QString fileName,QObject *parent);
    ~Database();
    typedef  enum {SqlInfo,Windows,WorkSheet,TableSchemas,SchemaFields,SearchFunctions, FilterFunctions,NumOfTables} TableType;
    static QString tableNames[NumOfTables];
    static QString arguments[NumOfTables];
    bool createTable(TableType);
    QSqlDatabase *getHandle();
    QString getLastError();
    int getVersion();
    bool setVersion(int);
    bool isOpen();
    bool open();
    bool tableIsValid(TableType);
    // Window Methods
    QList<WindowData> getWindows();
    bool deleteAllWindows();
    bool deleteWindow(int windowID);
    bool addWindow(WindowData &);
    bool updateWindow(WindowData &);
    // WorkSheets Methods
    QList <WorkSheetData> getWorkSheets(int windowID);
    bool addWorkSheet(WorkSheetData &);
    bool deleteAllWorkSheets();
    bool deleteWorkSheetByWindowID(int windowID);
    bool deleteWorkSheet(int workSheetID);
    // Schema Methods
    TableSchemaList *getTableSchemas();
    TableSchemaList *getTableSchemasByLibName(QString libName);
    bool addTableSchema(TableSchema &);
    bool updateTableSchema(TableSchema &);
    bool deleteTableSchema(qint32 tableSchemaID);
    // SchemaFields
    bool saveTableSchemaFields(TableSchema &ts);
    QStringList getSchemaFields(int schemaID);
    bool addSchemaFields(int schemaID, QStringList fieldNames);
    bool removeSchemaFields(int schemaID);
    // SearchFunctions
    SearchFunctionList *getSearchFunctions();
    bool addSearchFunction(SearchFunction &);
    bool updateSearchFunction(SearchFunction &);
    bool removeSearchFunction(qint32 searchFunctionID);

    // FilterFunctions
    SearchFunctionList *getFilterFunctions();
    bool addFilterFunction(SearchFunction &);
    bool updateFilterFunction(SearchFunction &);
    bool removeFilterFunction(qint32 filterFunctionID);
private:
    QString name;
    QSqlDatabase *handle;
    QString errorMessage;
    QSqlError sqlError;
};

#endif // DATABASE_H
