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
/*
#include <Myfix_types.hpp>
#include <Myfix_router.hpp>
#include <Myfix_classes.hpp>
*/
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
    QString fieldName;
    QString key;
    QString value;
    quint32 ikey;
    FieldTrait::FieldType ft;
    const BaseMsgEntry *tbme;

    WorkSheetModel *model = 0;
    QList <WindowData> windowDataList;
    WindowData wd;
    const BaseEntry *tbe;
    messageFieldList = new MessageFieldList();
    MessageField *messageField;
    QStringList errorStrList;
    MainWindow *newMW;
    QMap <QString, QStandardItemModel *>::iterator currentItemIter;
    bool haveError = false;
    bool isInitial = true;
    readSettings();
    initDatabase();
    Fix8SharedLib *fix8shareLib = Fix8SharedLib::create(QString("/home/david/f8logview/fixschemas/libTEX.so"));

    qDebug() << "Status of create fix8 share lib = " << fix8shareLib->isOK << __FILE__ << __LINE__;
    if (!fix8shareLib->isOK) {
        qDebug() << "\tError Str = " << fix8shareLib->errorMessage;
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
                QBaseEntry *qbe = fix8shareLib->baseMap.value(fieldName);
                if (qbe) {
                    ts->fieldList->append(qbe);
                }
            }
        }
    }
    QListIterator <QPair<QString ,FieldUse *>> pairListIter(fix8shareLib->fieldUsePairList);
    MainWindow::setTableSchemaList(tableSchemaList);
    //int sizeofFieldTable = TEX::ctx()._be.size();
    int sizeofFieldTable = fix8shareLib->ctxFunc()._be.size();
    // initial screeen
    qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,10);
    windowDataList = database->getWindows();
    cleanWindowDataList(windowDataList);
    qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,10);
    QListIterator <WindowData> iter(windowDataList);
    Fix8SharedLib  *fixlib;
    if (autoSaveOn){
        while(iter.hasNext()) {
            wd = iter.next();
            if (wd.fix8sharedlib.length() < 1) {
                qWarning() << " FIX Lib is not defined" << __FILE__ << __LINE__;
            }
            else {
                fixlib = fix8ShareLibList.findByFileName(wd.fix8sharedlib);
                if (!fixlib) {
                    qDebug() << "Fix lib: " << wd.fix8sharedlib << " not found creating it...." << __FILE__ << __LINE__;
                    fixlib = Fix8SharedLib::create(wd.fix8sharedlib);
                    if (fixlib->isOK)
                        fix8ShareLibList.append(fixlib);
                }
                newMW  = new MainWindow(database,true);
                newMW->setSharedLibrary(fixlib);
                newMW->setFieldUsePair(&(fixlib->fieldUsePairList));
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
                        newMW->setCurrentTabAndSelectedRow(wd.currentTab,2);
                    }
                }
                newMW->setLoading(false);
            }
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
