/*
Fix8logviewer is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8logviewer Open Source FIX Log Viewer.
Copyright (C) 2010-14 David N Boosalis dboosalis@fix8.org>

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
#include "database.h"
#include "fix8log.h"
#include "fixmimedata.h"
#include "globals.h"
#include "mainwindow.h"
#include "messagefield.h"
#include "schemaeditordialog.h"
#include "worksheetmodel.h"
#include "windowdata.h"
#include <QApplication>
#include <QDebug>
#include <QtWidgets>
#include "worksheetmodel.h"
#include <fix8/f8includes.hpp>
#include "fix8/field.hpp"
#include "fix8/message.hpp"
#include <fix8/f8types.hpp>
void Fix8Log::initDatabase()
{
    bool bstatus;
    bool createdTable = false;
    QString dbFileName = "fix8log.sql";
    QString errorStr;
    WorkSheetModel *model = 0;
    QList <WindowData> windowDataList;
    WindowData wd;
    QString dbPath = QDir::homePath() + QDir::separator()  +  "f8logview";
    QDir dir(dbPath);
    readSettings();
    if (!dir.exists()) {
        bstatus = dir.mkdir(dbPath);
        if (!bstatus) {
            errorStr = "Failed to create directory\n" + dbPath +
                    "\nPlease make sure you have write access to this directory";
            QMessageBox::warning(0,"Fix8Log Error", errorStr);
            qApp->exit();
        }
        else {
            displayConsoleMessage(GUI::ConsoleMessage("Created Database Directory:" + dbPath));
        }
    }
    dbFileName = dbPath + QDir::separator() + dbFileName;
    dbFile  = new QFile(dbFileName);
    if (!dbFile->exists()) {
        firstTimeToUse = true;
        GUI::Globals::isFirstTime = true;
        displayConsoleMessage(GUI::ConsoleMessage("Creating Database..."));
    }
    database = new Database(dbFileName,this);
    bstatus = database->open();
    if (!bstatus) {
        errorStr = "Error - open local database: " + dbFileName
                + " failed " + __FILE__;
        QMessageBox::warning(0,"Local Database",errorStr);
        qApp->exit();
    }
    bstatus = database->tableIsValid(Database::Windows);
    if (!bstatus) {
        bstatus = database->createTable(Database::Windows);
        if (!bstatus) {
            errorStr =  "Failed to create windows table.";
            displayConsoleMessage(GUI::ConsoleMessage(errorStr,GUI::ConsoleMessage::ErrorMsg));
        }
    }
    bstatus = database->tableIsValid(Database::WorkSheet);
    if (!bstatus) {
        bstatus = database->createTable(Database::WorkSheet);
        if (!bstatus) {
            errorStr = "Failed to create worksheet table.";
            displayConsoleMessage(GUI::ConsoleMessage(errorStr,GUI::ConsoleMessage::ErrorMsg));
        }
    }
    bstatus = database->tableIsValid(Database::SchemaFields);
    if (!bstatus) {
        bstatus = database->createTable(Database::SchemaFields);
        if (!bstatus) {
            errorStr = "Failed to create table  fields table.";
            displayConsoleMessage(GUI::ConsoleMessage(errorStr,GUI::ConsoleMessage::ErrorMsg));
        }
    }
    bstatus = database->tableIsValid(Database::SearchFunctions);
    if (!bstatus) {
        bstatus = database->createTable(Database::SearchFunctions);
        if (!bstatus) {
            errorStr = "Failed to create table  search table.";
            displayConsoleMessage(GUI::ConsoleMessage(errorStr,GUI::ConsoleMessage::ErrorMsg));
        }
    }
    else
        searchFunctionList = database->getSearchFunctions();
    bstatus = database->tableIsValid(Database::FilterFunctions);
    if (!bstatus) {
        bstatus = database->createTable(Database::FilterFunctions);
        if (!bstatus) {
            errorStr = "Failed to create table  filter table.";
            displayConsoleMessage(GUI::ConsoleMessage(errorStr,GUI::ConsoleMessage::ErrorMsg));
        }
    }
    else
        filterFunctionList = database->getFilterFunctions();


    bstatus = database->tableIsValid(Database::TableSchemas);
    if (!bstatus) {
        bstatus = database->createTable(Database::TableSchemas);
        if (!bstatus) {
            errorStr = "Failed to create table  table.";
            displayConsoleMessage(GUI::ConsoleMessage(errorStr,GUI::ConsoleMessage::ErrorMsg));
        }



    }
    else {
        tableSchemaList = database->getTableSchemas();
        if (!tableSchemaList) {
            errorStr = "Error - no table schemas found in database, creating default....";
            displayConsoleMessage(GUI::ConsoleMessage(errorStr,GUI::ConsoleMessage::WarningMsg));

        }
    }
    if (tableSchemaList) {
        QListIterator <TableSchema *> tsIter(*tableSchemaList);
        while(tsIter.hasNext()) {
            TableSchema *ts = tsIter.next();
            ts->fieldNames = database->getSchemaFields(ts->id);
        }
    }


}
