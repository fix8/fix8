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
#include "newwindowwizard.h"
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

#include <iostream>
#include <string.h>
using namespace GUI;
using namespace FIX8;
using namespace std;

bool Fix8Log::init()
{
    bool bstatus;
    bool createdTable = false;
    QString errorStr;
    QString fieldName;
    QString key;
    QString value;
    quint32 ikey;
    Fix8SharedLib *f8lib = 0;
    QString fileName;
    QString libName;
    int status;
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
    readSettings();
    initDatabase();

    // QListIterator <QPair<QString ,FieldUse *>> pairListIter(fix8shareLib->fieldUsePairList);

    // initial screeen
    qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,10);
    windowDataList = database->getWindows();
    // cleanWindowDataList(windowDataList);
    qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,10);
    QListIterator <WindowData> iter(windowDataList);
    Fix8SharedLib  *fixlib;
    if (autoSaveOn){
        while(iter.hasNext()) {
            wd = iter.next();
            if (wd.fix8sharedlib.length() > 0) {
                fixlib = fix8ShareLibList.findByFileName(wd.fix8sharedlib);
                if (!fixlib) {
                    bstatus = createSharedLib(wd.fix8sharedlib,&fixlib,defaultTableSchema);
                    if (!bstatus)
                        goto done;
                    fix8ShareLibList.append(fixlib);
                }
                wd.tableSchema  = fixlib->getTableSchema(wd.tableSchemaID);
                if (!wd.tableSchema) {
                    wd.tableSchema = fixlib->getDefaultTableSchema();
                    if (wd.tableSchema)
                        wd.tableSchemaID = wd.tableSchema->id;
                }
                else
                    fixlib->generateSchema(wd.tableSchema);
                newMW  = new MainWindow(database,true);
                newMW->setSharedLibrary(fixlib);
                //newMW->setTableSchema(wd.tableSchema);
                newMW->setFieldUsePair(&(fixlib->fieldUsePairList));
                newMW->setSearchFunctions(searchFunctionList);
                newMW->setFilterFunctions(filterFunctionList);

                wireSignalAndSlots(newMW);
                mainWindows.append(newMW);
                newMW->setAutoSaveOn(autoSaveOn);
                // have to set style sheet after show to see it take effect
                newMW->mainMenuBar->setStyleSheet(wd.menubarStyleSheet);
                qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,40);
                QList <WorkSheetData> wsdList = database->getWorkSheets(wd.id);
                qApp->processEvents(QEventLoop::ExcludeSocketNotifiers,40);
                if (!wd.tableSchema) {
                    wd.tableSchema = fixlib->getDefaultTableSchema();
                }
                newMW->setWindowData(wd);
                newMW->show();
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

        done:
            ;
        }
    }
    displayConsoleMessage("Session restored from autosave");
    if (mainWindows.count() < 1) {
        newWindowWizard = new NewWindowWizard(fix8ShareLibList,firstTimeToUse);
        if (!firstTimeToUse)
            newWindowWizard->readSettings();
        else
            newWindowWizard->resize(newWindowWizard->sizeHint());

        QDesktopWidget *desktop = QApplication::desktop();
        status = newWindowWizard->exec();
        newWindowWizard->saveSettings();
        if (status != QDialog::Accepted) {
            if (dbFile) {
                dbFile->remove();
                delete dbFile;
            }
            qApp->exit(0);
        }
        fileName = newWindowWizard->getSelectedFile();
        libName = newWindowWizard->getSelectedLib();
        bstatus = createSharedLib(libName,&f8lib,defaultTableSchema);
        if (!bstatus) {
            errorStr = "Error - failed to load FIX8 sharelib: " + libName;
            QMessageBox::warning(0,Globals::appName,errorStr);
            newWindowWizard->deleteLater();
            qApp->exit(0);
        }

        if (!f8lib) {
            qWarning() << ">>>>>>>>>>>>>SHARE LIB SET TO" << f8lib->fileName << __FILE__ << __LINE__;
            errorStr = "Error - failed to create FIX8 sharelib: " + libName;
            QMessageBox::warning(0,Globals::appName,errorStr);
            newWindowWizard->deleteLater();
            qApp->exit(0);
        }
        if (!(f8lib->isOK)) {
            qWarning() << ">>>>>>>>>>>>>SHARE LIB SET TO" << f8lib->fileName << __FILE__ << __LINE__;
            errorStr = "Error - FIX8 sharelib: " + libName + " has error.";
            QMessageBox::warning(0,Globals::appName,errorStr);
            newWindowWizard->deleteLater();
            qApp->exit(0);
        }
        newMW = new MainWindow(database);
        newMW->setAutoSaveOn(autoSaveOn);
        // have to set style sheet after show to see it take effect
        newMW->mainMenuBar->setStyleSheet(wd.menubarStyleSheet);
        fix8ShareLibList.append(f8lib);

        newMW->setSearchFunctions(searchFunctionList);
        newMW->setFilterFunctions(filterFunctionList);

        wireSignalAndSlots(newMW);
        newMW->setSharedLibrary(f8lib);
        newMW->setFieldUsePair(&(f8lib->fieldUsePairList));
        newMW->setAutoSaveOn(autoSaveOn);

        //windowData.tableSchema = f8lib->getDefaultTableSchema();
        wd =  newMW->getWindowData();
        wd.tableSchema = f8lib->defaultTableSchema;
        wd.tableSchemaID = f8lib->defaultTableSchema->id;
        wd.fix8sharedlib = f8lib->fileName;
        newMW->setWindowData(wd);
        newMW->show();
        newMW->loadFile(fileName);
        newMW->setAutoSaveOn(autoSaveOn);
        mainWindows.append(newMW);
    }
    return true;
}
bool Fix8Log::init(QString fileNameToLoad)
{
    /*
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
    */
    return true;
}
bool Fix8Log::createSharedLib(QString &fix8sharedlib,Fix8SharedLib **fixlib,
                              TableSchema *defaultTableSchema)
{
    bool bstatus;
    TableSchemaList *tsl;
    *fixlib = Fix8SharedLib::create(fix8sharedlib);
    if (!(*fixlib)->isOK) {
        qWarning() << "FAILED TO CREATED SHARED LIB FOR " << fix8sharedlib;
        return false;
    }

    tsl  = database->getTableSchemasByLibName((*fixlib)->fileName);
    if (!tsl) {
        qWarning() << "NO table schemas found for shared lib:"
                   << (*fixlib)->fileName << __FILE__ << __LINE__;
        tsl = new TableSchemaList();
    }
    QListIterator <TableSchema *> tsiter(*tsl);
    while(tsiter.hasNext()) {
        TableSchema *ts = tsiter.next();
        ts->fieldNames = database->getSchemaFields(ts->id);
         (*fixlib)->generateSchema(ts);
    }
    (*fixlib)->setTableSchemas(tsl);
    defaultTableSchema  = tsl->findDefault();
    if (!defaultTableSchema) {
        qWarning() << "HEY NO DEFAULT TABLE SCHEMA FOUND............" << __FILE__ << __LINE__;
        defaultTableSchema = (*fixlib)->createDefaultTableSchema();
        if (defaultTableSchema) {
            tsl->append(defaultTableSchema);
            bstatus = database->addTableSchema(*defaultTableSchema);
            if (bstatus ) {
                qWarning() << "Save default table schema to database";
                database->saveTableSchemaFields(*defaultTableSchema);
            }
        }
        else {
            qWarning() << "Failed to create default table schema for share lib: " << (*fixlib)->fileName << __FILE__ << __LINE__;
        }


        //qDebug() << "FIELD NAMES: " << defaultTableSchema->fieldNames << __FILE__ << __LINE__;
        (*fixlib)->generateSchema(defaultTableSchema);
    }

    (*fixlib)->setDefaultTableSchema(defaultTableSchema);
    (*fixlib)->setTableSchemas(tsl);
    //qDebug() << "STATUS OF SHARED LIB AT END = " <<  (*fixlib)->isOK << __FILE__ << __LINE__;
    return true;
}
