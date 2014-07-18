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
#include "fix8log.h"
#include "fixmimedata.h"
#include "globals.h"
#include "mainwindow.h"
#include "messagefield.h"
#include "schemaeditordialog.h"
#include "searchDialog.h"
#include "windowdata.h"
#include <QApplication>
#include <QDebug>
#include <QtWidgets>
#include <fix8/f8includes.hpp>
#include <fix8/field.hpp>
#include <fix8/message.hpp>
#include <fix8/f8types.hpp>
#include <Myfix_types.hpp>
#include <Myfix_router.hpp>
#include <Myfix_classes.hpp>
#include <iostream>
#include <string.h>
using namespace GUI;
using namespace FIX8;
using namespace std;
void Fix8Log::aboutSlot()
{
    QString str = GUI::Globals::appName + ", version  " + GUI::Globals::versionStr + "\n";
    str.append("A opensource FIX log file viewer tool.\n");
    str.append("For questions or comments visit www.fix8.org.\n");
    str.append("This software released under the GPL license.");
    QMessageBox::about(0,GUI::Globals::appName,str);

}
void Fix8Log::createNewWindowSlot(MainWindow *mw)
{
    MainWindow *newMW  =new MainWindow(*mw,database);
    newMW->setSearchFunctions(searchFunctionList);
    wireSignalAndSlots(newMW);
    newMW->show();
    newMW->showFileDialog();
    mainWindows.append(newMW);
    //const GeneratedTable<const char *, BaseMsgEntry>::Pair *p = TEX::ctx()._bme.at(1);
}
void Fix8Log::deleteMainWindowSlot(MainWindow *mw)
{
    qDebug() << "DELETE MAIN WINDOW" << __FILE__ << __LINE__;
    if (mainWindows.count() == 1)  {
        if (autoSaveOn) {
            saveSession();
        }
    }
    if (searchDialog && (searchDialog->getMainWindow() == mw)) {
        searchDialog->deleteLater();
        searchDialog = 0;
    }
    mainWindows.removeOne(mw);
    mw->deleteLater();
    if (mainWindows.count() < 1) {
        writeSettings();
        qApp->exit();
    }
    if (schemaEditorDialog)
        schemaEditorDialog->windowDeleted(mw);
}
void Fix8Log::displayConsoleMessage(GUI::ConsoleMessage msg)
{
    MainWindow *mw;
    QListIterator <MainWindow *> iter(mainWindows);
    while(iter.hasNext()) {
        mw = iter.next();
        mw->displayConsoleMessage(msg);
    }
}
void Fix8Log::exitAppSlot()
{
    if (autoSaveOn)
        saveSession();
    //writeSettings();
    qApp->closeAllWindows();
    qApp->quit();
}
void Fix8Log::toolButtonStyleModfiedSlot(Qt::ToolButtonStyle tbs)
{
    MainWindow *mw;
    QListIterator <MainWindow *> iter(mainWindows);
    while(iter.hasNext()) {
        mw = iter.next();
        mw->setToolButtonStyle(tbs);
    }
    if (schemaEditorDialog)
        schemaEditorDialog->setToolButtonStyle(tbs);
}
void Fix8Log::autoSaveOnSlot(bool on)
{
    MainWindow *mw;
    QListIterator <MainWindow *> iter(mainWindows);
    autoSaveOn = on;
    QString str = "on";
    if (!autoSaveOn)
        str = "off";
    displayConsoleMessage("Autosave turned " + str);
    while(iter.hasNext()) {
        mw = iter.next();
        mw->setAutoSaveOn(autoSaveOn);
    }
    QSettings settings("fix8","logviewer");
    settings.setValue("AutoSave",autoSaveOn);
}
void Fix8Log::lastWindowClosedSlot()
{
    qDebug() << "Last Window closed";
    qApp->exit();
}
void Fix8Log::cancelSessionRestoreSlot()
{
    cancelSessionRestore = true;
    displayConsoleMessage("Session Restore Cancelled");
}
void Fix8Log::schemaModifiedSlot(TableSchema *tableSchema, bool NameOnly)
{
    qDebug() << "Schema Modified Slot " << __FILE__ << __LINE__;
    MainWindow *mw;
    TableSchema * ts;
    bool found = false;
    if (!tableSchemaList) {
        qWarning() << "Schema Modfied Error, tble Schema lis t is null" << __FILE__ << __LINE__;
        return;
    }
    QListIterator <TableSchema *> iter(*tableSchemaList);
    while(iter.hasNext()) {
        ts = iter.next();
        if (ts->id == tableSchema->id) {
            *ts = *tableSchema;
            found = true;
            break;
        }
    }
    if (!found) {
        qWarning() << "Error - update of table schema failed" << __FILE__ << __LINE__;
        return;
    }
    QListIterator <MainWindow *> iter2(mainWindows);
    while(iter2.hasNext()) {
        mw = iter2.next();
        mw->tableSchemaModified(ts);
    }
}
void  Fix8Log::setTimeFormatSlot(GUI::Globals::TimeFormat tf)
{
    GUI::Globals::timeFormat = tf;
    // tell all main windows that format changed
    emit notifyTimeFormatChanged(tf);
    QSettings settings("fix8","logviewer");
    settings.setValue("StartTimeFormat",tf);
}
void  Fix8Log::modelDroppedSlot(FixMimeData* fmd)
{
    bool ok = false;
    WorkSheetData wsd;
    MainWindow *mw;
    MainWindow *mwSender = qobject_cast <MainWindow *> (sender());
    if (!mwSender) {
        qWarning() << "Invalid sender, drop failed " << __FILE__ << __LINE__;
    }
    QListIterator <MainWindow *> iter(mainWindows);
    while(iter.hasNext()) {
        mw = iter.next();
        if (mw->getUuid() == fmd->windowID) {
            wsd = mw->getWorksheetData(fmd->worksheetID, &ok);
            mwSender->finishDrop(wsd,fmd);
            break;
        }
    }
}
void  Fix8Log::editSchemaSlot(MainWindow *mw)
{
    bool ok;
    if (!schemaEditorDialog) {
        schemaEditorDialog = new SchemaEditorDialog(database);
        schemaEditorDialog->populateMessageList(messageFieldList);
        schemaEditorDialog->populateFieldListPair(&fieldUsePairList);

        schemaEditorDialog->setToolButtonStyle(mw->toolButtonStyle());
        schemaEditorDialog->setBaseMaps(baseMap);
        schemaEditorDialog->setFieldUseList(fieldUseList);
        schemaEditorDialog->setDefaultHeaderItems(defaultHeaderItems);
        schemaEditorDialog->setTableSchemas(tableSchemaList,defaultTableSchema);
        connect(schemaEditorDialog,SIGNAL(finished(int)),
                this,SLOT(schemaEditorFinishedSlot(int)));
        connect(schemaEditorDialog,SIGNAL(newSchemaCreated(TableSchema*)),
                this,SLOT(newSchemaCreatedSlot(TableSchema *)));
        connect(schemaEditorDialog,SIGNAL(schemaDeleted(int)),
                this,SLOT(schemaDeletedSlot(int)));
        connect(schemaEditorDialog,SIGNAL(tableSchemaUpdated(TableSchema *,bool)),
                this,SLOT(schemaModifiedSlot(TableSchema *,bool)));
        schemaEditorDialog->restoreSettings();
    }

    QString windowName = mw->windowTitle();
    if (windowName.length() < 1)
        windowName = qApp->applicationName();
    schemaEditorDialog->setCurrentTarget(mw,true);
    if (mw) {
        schemaEditorDialog->setTableSchemaInUse(mw->getTableSchema());
    }
    schemaEditorDialog->show();
    schemaEditorDialog->setVisible(true);
    schemaEditorDialog->showNormal();
    schemaEditorDialog->raise();

}

void Fix8Log::newSchemaCreatedSlot(TableSchema *ts)
{
    MainWindow *mw;
    QListIterator <MainWindow *> iter(mainWindows);
    while(iter.hasNext()) {
        mw = iter.next();
        mw->addNewSchema(ts);
    }
}
void Fix8Log::schemaDeletedSlot(int schemaID)
{
    MainWindow *mw;
    QListIterator <MainWindow *> iter(mainWindows);
    while(iter.hasNext()) {
        mw = iter.next();
        mw->deletedSchema(schemaID);
    }
    TableSchema *ts = tableSchemaList->findByID(schemaID);
    qDebug() << "REMOVE TS FROM LIST" << __FILE__ << __LINE__;
    if (ts) {
        tableSchemaList->removeOne(ts);
        qDebug() << "\tDelete It";
        delete ts;
        qDebug() << "\tAfter Delete";
    }
}
void  Fix8Log::schemaEditorFinishedSlot(int returnCode)
{
    if (returnCode == QDialogButtonBox::Close) {
        schemaEditorDialog->close();
        schemaEditorDialog->saveSettings();
        schemaEditorDialog->deleteLater();
        schemaEditorDialog = 0;
    }


}

void Fix8Log::tableSchemaSelectedSlot(TableSchema *ts)
{
    if (!ts)  {
        qWarning() << "Error - table schema selected is null" << __FILE__ << __LINE__;
        return;
    }
    qDebug() << "TABLE SCHEMA SELECTED " << ts->name << __FILE__ << __LINE__;
}
void Fix8Log::wakeupSlot(const QString&)
{
    MainWindow *mw;
    QListIterator <MainWindow *> iter(mainWindows);
    while(iter.hasNext()) {
        mw = iter.next();
        mw->showNormal();
    }
}
void Fix8Log::showSearchDialogSlot()
{
    QString str;
    if (!searchDialog) {
        searchDialog = new SearchDialog(database,0);
        QSettings settings("fix8","logviewer");
        QRect rect = settings.value("SearchDialog").toRect();
        QSize sh = searchDialog->sizeHint();
        if (rect.width() < sh.width())
            rect.setWidth(sh.width());
        if (rect.height() < sh.height())
            rect.setHeight(sh.height());

        searchDialog->setGeometry(rect);
        connect(searchDialog,SIGNAL(accepted()),this,SLOT(searchDialogAcceptedSlot()));
        connect(searchDialog,SIGNAL(updatedSearchFunctions(SearchFunctionList *)),
                this,SLOT(updatedSearchFunctionsSlot(SearchFunctionList *)));
    }

    MainWindow *mw = qobject_cast <MainWindow *> (sender());
    if (!mw) {
        qWarning() << "Error show search dialog, main window is null" << __FILE__ << __LINE__;
        return;
    }
    searchDialog->setMainWindow(mw);
    TableSchema *ts = mw->getTableSchema();
    if (!ts) {
        str = "Error - Search Dialog Needs Window to have its table schema set";
        QMessageBox::warning(0,QString("Fix8Log Viewer"),str);
        qWarning() << str << __FILE__ << __LINE__;
        return;
    }
    QRect rect = mw->geometry();
    searchDialog->move(rect.x() + (rect.width()/4),rect.y() + (rect.height()/3));
    searchDialog->setTableSchema(ts);
    searchDialog->showNormal();
    searchDialog->raise();
}
void Fix8Log::showSearchDialogAddModeSlot(QString searchStr)
{
    QString str;
    if (!searchDialog) {
        searchDialog = new SearchDialog(database,0);
        QSettings settings("fix8","logviewer");
        QRect rect = settings.value("SearchDialog").toRect();
        QSize sh = searchDialog->sizeHint();
        if (rect.width() < sh.width())
            rect.setWidth(sh.width());
        if (rect.height() < sh.height())
            rect.setHeight(sh.height());

        searchDialog->setGeometry(rect);
        connect(searchDialog,SIGNAL(accepted()),this,SLOT(searchDialogAcceptedSlot()));
        connect(searchDialog,SIGNAL(updatedSearchFunctions(SearchFunctionList *)),
                this,SLOT(updatedSearchFunctionsSlot(SearchFunctionList *)));
    }

    MainWindow *mw = qobject_cast <MainWindow *> (sender());
    if (!mw) {
        qWarning() << "Error show search dialog, main window is null" << __FILE__ << __LINE__;
        return;
    }
    searchDialog->setMainWindow(mw);
    TableSchema *ts = mw->getTableSchema();
    if (!ts) {
        str = "Error - Search Dialog Needs Window to have its table schema set";
        QMessageBox::warning(0,QString("Fix8Log Viewer"),str);
        qWarning() << str << __FILE__ << __LINE__;
        return;
    }
    QRect rect = mw->geometry();
    searchDialog->move(rect.x() + (rect.width()/4),rect.y() + (rect.height()/3));
    searchDialog->setTableSchema(ts);
    searchDialog->setNewMode(searchStr);
    searchDialog->showNormal();
    searchDialog->raise();
}

void Fix8Log::searchDialogAcceptedSlot()
{
    qDebug() << "Search Accepted" << __FILE__ << __LINE__;
    QSettings settings("fix8","logviewer");
    if (searchDialog)
        settings.setValue("SearchDialog",searchDialog->geometry());
}
void Fix8Log::updatedSearchFunctionsSlot(SearchFunctionList *sfl)
{
    if (!database) {
        qWarning() << "Database not set, update search functions failed" << __FILE__ << __LINE__;
    }

    if (sfl)
        qDebug() << "Update of search Functions, count = " << sfl->count() << __FILE__ << __LINE__;
    else
        qDebug() << "Update search functions, empty" << __FILE__ << __LINE__;

    MainWindow *mw;
    QListIterator <MainWindow *> iter(mainWindows);
    while(iter.hasNext()) {
        mw = iter.next();
        mw->updateSearchFunctions(sfl);
    }
    /*
    if (searchFunctionList) {
        qDeleteAll(searchFunctionList->begin(),searchFunctionList->end());
       // delete searchFunctionList;
    }
    */
    searchFunctionList = sfl;

}
