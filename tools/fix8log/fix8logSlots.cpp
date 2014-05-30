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
#include "windowdata.h"
#include <QApplication>
#include <QDebug>
#include <QtWidgets>
#include <fix8/f8includes.hpp>
#include <field.hpp>
#include <message.hpp>
#include <f8types.hpp>
#include <Myfix_types.hpp>
#include <Myfix_router.hpp>
#include <Myfix_classes.hpp>
#include <iostream>
#include <string.h>
using namespace GUI;
using namespace FIX8;
using namespace std;
void Fix8Log::createNewWindowSlot(MainWindow *mw)
{
    MainWindow *newMW  =new MainWindow(*mw);
    wireSignalAndSlots(newMW);
    newMW->show();
    newMW->showFileDialog();
    mainWindows.append(newMW);
    //const GeneratedTable<const char *, BaseMsgEntry>::Pair *p = TEX::ctx()._bme.at(1);
}
void Fix8Log::deleteMainWindowSlot(MainWindow *mw)
{
    if (mainWindows.count() == 1)  {
        if (autoSaveOn) {
            saveSession();
        }
    }
    mainWindows.removeOne(mw);
    mw->deleteLater();
    if (mainWindows.count() < 1) {
        writeSettings();
        qApp->exit();
    }
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
}
void Fix8Log::cancelSessionRestoreSlot()
{
    cancelSessionRestore = true;
    displayConsoleMessage("Session Restore Cancelled");
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
void  Fix8Log::editSchemaSlot(MainWindow *mw, QUuid workSheetID)
{
    bool ok;
    QString tabName;
    if (!schemaEditorDialog) {
        schemaEditorDialog = new SchemaEditorDialog(database,globalSchemaOn);
        schemaEditorDialog->populateMessageList(messageFieldList);
        QListIterator <MainWindow *> iter(mainWindows);
        if (iter.hasNext()) {
            MainWindow *mw = iter.next();
            schemaEditorDialog->setToolButtonStyle(mw->toolButtonStyle());
        }
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
        schemaEditorDialog->restoreSettings();
    }

    QString windowName = mw->windowTitle();
    if (windowName.length() < 1)
        windowName = qApp->applicationName();
    if (!workSheetID.isNull()) {
        WorkSheetData wsd = mw->getWorksheetData(workSheetID,&ok);
        if (ok) {
            if (wsd.tabAlias.length() >= 1)
                tabName = wsd.tabAlias;
            else
                tabName = wsd.fileName;
        }
    }
    schemaEditorDialog->setCurrentTarget(windowName);
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
    if (returnCode == QDialogButtonBox::Close)
        schemaEditorDialog->close();
    schemaEditorDialog->saveSettings();

}
void Fix8Log::setGlobalSchemaOnSlot(bool b)
{
    QSettings settings("fix8","logviewer");
    globalSchemaOn = b;
    settings.setValue("GlobalSchemaOn",b);
}

