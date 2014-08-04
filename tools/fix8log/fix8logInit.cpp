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
#include <Myfix_types.hpp>
#include <Myfix_router.hpp>
#include <Myfix_classes.hpp>
#include <iostream>
#include <string.h>
using namespace GUI;
using namespace FIX8;
using namespace std;

bool Fix8Log::init()
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
    QString key;
    QString value;
    quint32 ikey;
    FieldTrait::FieldType ft;
    const BaseMsgEntry *tbme;
    //Globals::messagePairs = new QVector<Globals::MessagePair>(TEX::ctx()._bme.size());
    //Globals::messagePairs = new QVector<Globals::MessagePair>();
    int messageCount = TEX::ctx()._bme.size();
    // qDebug() << "SIZE OF MESSAGE TABLE = " << messageCount ;
    const BaseEntry *tbe;
    messageFieldList = new MessageFieldList();
    MessageField *messageField;
    fieldTraitV.resize( TEX::ctx()._be.size());
    ///dnb
    QString fieldName;
    FIX8::TEX::NewOrderSingle nos;
    //qDebug() << "VERSION OF CODE = " << TEX::ctx().version() << __FILE__ << __LINE__;
    MessageBase *_header = nos.Header();
    for (Fields::const_iterator hiter = _header->fields_begin();
         hiter != _header->fields_end();
         hiter++) {
    }
    for(int ii=0;ii < messageCount; ii++)
    {
        const char *kk = TEX::ctx()._bme.at(ii)->_key;
        const TraitHelper tr = TEX::ctx()._bme.at(ii)->_value._create._get_traits();
        QBaseEntryList *qbaseEntryList = new QBaseEntryList();
        value = QString::fromStdString(TEX::ctx()._bme.at(ii)->_value._name);
        key =
                QString::fromStdString(TEX::ctx()._bme.at(ii)->_key);
        messageField = new MessageField(key,value);
        int level = 0;
        generate_traits(tr,baseMap,fieldUseList,messageField,qbaseEntryList,&level);
        messageField->qbel = qbaseEntryList;
        messageFieldList->append(messageField);
    }
    QListIterator <FieldUse *> fieldIter(fieldUseList);
    while(fieldIter.hasNext()) {
        FieldUse *mf = fieldIter.next();
        fieldUsePairList.append(qMakePair(mf->name,mf));
    }
    int pp = 0;
    //qDebug() << ">>>>>>>>>>>>>> Pair list:" << fieldUsePairList.count() << __FILE__ << __LINE__;
    qSort(fieldUsePairList.begin(), fieldUsePairList.end());
    QListIterator <QPair<QString ,FieldUse *>> pairListIter(fieldUsePairList);
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
    QFile dbFile(dbFileName);
    if (!dbFile.exists()) {
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

    bstatus = database->tableIsValid(Database::TableSchemas);
    if (!bstatus) {
        bstatus = database->createTable(Database::TableSchemas);
        if (!bstatus) {
            errorStr = "Failed to create table  table.";
            displayConsoleMessage(GUI::ConsoleMessage(errorStr,GUI::ConsoleMessage::ErrorMsg));
        }
        else {
            // create default schema ...
            TableSchema *ts = new TableSchema("Default","Default Table Schema",true);
            ts->setFields(defaultHeaderItems.clone());
            ts->fieldNames = defaultHeaderItems.getFieldNames();
            qDebug() << "************* 1 SET DEF FIELD NAMES TO: " << ts->fieldNames << __FILE__ << __LINE__;
            bstatus = database->addTableSchema(*ts);
            if (!bstatus) {
                errorStr = "Failed to create default table schema";
                displayConsoleMessage(GUI::ConsoleMessage(errorStr,GUI::ConsoleMessage::ErrorMsg));
            }
            else {
                database->saveTableSchemaFields(*ts);
                tableSchemaList = database->getTableSchemas();
                defaultTableSchema = tableSchemaList->findByName("Default");
                defaultTableSchema->fieldNames = database->getSchemaFields(defaultTableSchema->id);
                qDebug() << "GOT DEFAULT FROM DATABASE: " << defaultTableSchema->fieldNames << __FILE__ << __LINE__;
                if (!tableSchemaList) {
                    errorStr = "Error - no table schemas found in database";
                    displayConsoleMessage(GUI::ConsoleMessage(errorStr,GUI::ConsoleMessage::ErrorMsg));
                }
            }
        }
    }
    else {
        tableSchemaList = database->getTableSchemas();
        if (!tableSchemaList) {
            errorStr = "Error - no table schemas found in database, creating default....";
            displayConsoleMessage(GUI::ConsoleMessage(errorStr,GUI::ConsoleMessage::WarningMsg));
            defaultTableSchema = new TableSchema("Default","Default Table Schema",true);
            defaultTableSchema->setFields(defaultHeaderItems.clone());
            defaultTableSchema->fieldNames = defaultHeaderItems.getFieldNames();
            bstatus = database->addTableSchema(*defaultTableSchema);
            if (!bstatus) {
                errorStr = "Failed to add default table schema to database";
                displayConsoleMessage(GUI::ConsoleMessage(errorStr,GUI::ConsoleMessage::ErrorMsg));
            }
            else {
                tableSchemaList = new TableSchemaList();
                tableSchemaList->append(defaultTableSchema);
            }
        }
        else { // lets make sure we have default table schema, else create it
            defaultTableSchema = tableSchemaList->findByName("Default");
            if(!defaultTableSchema) {
                errorStr = "Failed to find default table schema, creating it...";
                qDebug() << errorStr << __FILE__ << __LINE__;
                displayConsoleMessage(GUI::ConsoleMessage(errorStr,GUI::ConsoleMessage::WarningMsg));
                defaultTableSchema = new TableSchema("Default","Default Table Schema",true);
                tableSchemaList->append(defaultTableSchema);
                bstatus = database->addTableSchema(*defaultTableSchema);
                if (!bstatus) {
                    errorStr = "Failed to add default table schema to database 2";
                    displayConsoleMessage(GUI::ConsoleMessage(errorStr,GUI::ConsoleMessage::ErrorMsg));
                }
            }
            MainWindow::defaultTableSchema = defaultTableSchema;
        }
    }
    if (tableSchemaList) {
        QListIterator <TableSchema *> tsIter(*tableSchemaList);
        while(tsIter.hasNext()) {
            TableSchema *ts = tsIter.next();
            ts->fieldNames = database->getSchemaFields(ts->id);
        }
    }

    // generate fieldList from names;
    if (tableSchemaList) {
        QListIterator <TableSchema *> tsIter(*tableSchemaList);
        while(tsIter.hasNext()) {
            TableSchema *ts = tsIter.next();
            ts->fieldList  = new QBaseEntryList();
            QStringListIterator fieldNameIter(ts->fieldNames);
            while(fieldNameIter.hasNext()) {
                fieldName = fieldNameIter.next();
                QBaseEntry *qbe = baseMap.value(fieldName);
                if (qbe) {
                    ts->fieldList->append(qbe);
                }
            }
        }
    }
    MainWindow::setTableSchemaList(tableSchemaList);
    int sizeofFieldTable = TEX::ctx()._be.size();
    // initial screeen
    qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,10);
    // QList <WindowData> windowDataList = database->getWindows();
    windowDataList = database->getWindows();
    cleanWindowDataList(windowDataList);
    qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,10);
    QListIterator <WindowData> iter(windowDataList);
    QStringList errorStrList;
    MainWindow *newMW;
    QMap <QString, QStandardItemModel *>::iterator currentItemIter;
    bool haveError = false;
    bool isInitial = true;
    if (autoSaveOn){
        while(iter.hasNext()) {
            wd = iter.next();
            newMW  = new MainWindow(database,true);
            newMW->setFieldUsePair(&fieldUsePairList);
            newMW->setSearchFunctions(searchFunctionList);
            wireSignalAndSlots(newMW);
            mainWindows.append(newMW);
            newMW->setAutoSaveOn(autoSaveOn);
            newMW->setWindowData(wd);
            newMW->show();
            // have to set style sheet after show to see it take effect
            newMW->mainMenuBar->setStyleSheet(wd.menubarStyleSheet);
            qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,40);
            QList <WorkSheetData> wsdList = database->getWorkSheets(wd.id);
            qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,40);
            if (wsdList.count() > 0) {
                QListIterator <WorkSheetData> iter2(wsdList);
                while(iter2.hasNext()) {
                    WorkSheetData wsd = iter2.next();
                    currentItemIter =  fileNameModelMap.find(wsd.fileName);
                    newMW->addWorkSheet(wsd); // do not create model, have code reuse and redo  busy screen for each tab
                    qDebug() << "FIX THIS HARD CODE ROW SETTING of 2 ?" << __FILE__ << __LINE__;
                    newMW->setCurrentTabAndSelectedRow(wd.currentTab,2);
                }
            }
            newMW->setLoading(false);
        }
        displayConsoleMessage("Session restored from autosave");
    }
    else
        qDebug() << "TODO - Display error messages here, and if no work sheets created lets delete main window" << __FILE__ << __LINE__;
    // if no main windows lets create one
    if (mainWindows.count() < 1) {
        newMW = new MainWindow(database);
        newMW->setFieldUsePair(&fieldUsePairList);
        newMW->setSearchFunctions(searchFunctionList);
        if (windowDataList.count() > 0) {
            wd = windowDataList.at(0);
            if (!wd.tableSchema) {
                qWarning() << "No Table schema found for window, setting it to default" << __FILE__ << __LINE__;
                wd.tableSchema = defaultTableSchema;
            }
            newMW->setWindowData(wd);
        }
        newMW->setTableSchema(defaultTableSchema);
        wireSignalAndSlots(newMW);
        newMW->show();
        newMW->setAutoSaveOn(autoSaveOn);
        mainWindows.append(newMW);
    }
    return bstatus;
}
bool Fix8Log::init(QString fileNameToLoad)
{
    WorkSheetModel *model = 0;
    MainWindow *newMW  =new MainWindow(database,true);
    newMW->setFieldUsePair(&fieldUsePairList);
    newMW->setSearchFunctions(searchFunctionList);
    WindowData wd;
    WorkSheetData wsd;
    QString errorStr;
    wsd.fileName = fileNameToLoad;
    wd.isVisible = true;
    wd.name = "Fix8LogViewer";
    wireSignalAndSlots(newMW);
    newMW->setWindowData(wd);
    wireSignalAndSlots(newMW);
    mainWindows.append(newMW);
    newMW->setLoadMessage("Loading File " + wsd.fileName);
    model = readLogFile(wsd.fileName,errorStr);
    qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,4);
    if (model) {
        fileNameModelMap.insert(wsd.fileName,model);
        newMW->addWorkSheet(model,wsd);
        newMW->show();
    }
    newMW->setLoading(false);
    return true;
}
