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
#include "fix8sharedlib.h"
#include "fixmimedata.h"
#include "globals.h"
#include "mainwindow.h"
#include "messagefield.h"
#include "newwindowwizard.h"
#include "schemaeditordialog.h"
#include "tableschema.h"
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
Fix8Log::Fix8Log(QtSingleApplication *qsa) :
    QObject(),firstTimeToUse(false),database(0),autoSaveOn(false),
    cancelSessionRestore(false),schemaEditorDialog(0),tableSchemaList(0),
    defaultTableSchema(0),worldTableSchema(0),applicationInstance(qsa),searchDialog(0),
    filterDialog(0),searchFunctionList(0),filterFunctionList(0), newWindowWizard(0)
{
    GUI::Globals::Instance();
    connect(qApp,SIGNAL(lastWindowClosed()),this,SLOT(lastWindowClosedSlot()));
    defaultHeaderStrs << "MsgSeqNum" << "MsgType" << "SendingTime" << "SenderCompID" << "TargetCompID";
}

void Fix8Log::displayConsoleMessage(QString str, GUI::ConsoleMessage::ConsoleMessageType mt)
{
    GUI::ConsoleMessage m(str,mt);
    displayConsoleMessage(m);
}

void Fix8Log::saveSession()
{
    MainWindow *mw;
    WorkSheetData wsd;
    bool bstatus;
    bstatus = database->deleteAllWindows();
    if (!bstatus)
        qWarning() << "Delete all windows from database failed" << __FILE__ << __LINE__;
    bstatus = database->deleteAllWorkSheets();
    if (!bstatus)
        qWarning() << "Delete all worksheets from database failed" << __FILE__ << __LINE__;
    QListIterator <MainWindow *> iter(mainWindows);
    while(iter.hasNext()) {
        mw = iter.next();
        WindowData wd = mw->getWindowData();
        bstatus = database->addWindow(wd);
        if (!bstatus) {
            displayConsoleMessage("Error failed saving window to database",GUI::ConsoleMessage::ErrorMsg);
        }
        else {
            QList<WorkSheetData> wsdList = mw->getWorksheetData(wd.id);
            QListIterator <WorkSheetData> wsdIter(wsdList);
            int i = 0;
            while(wsdIter.hasNext()) {
                wsd = wsdIter.next();
                bstatus = database->addWorkSheet(wsd);
                i++;
            }
        }
    }
}
void Fix8Log::readSettings()
{
    QSettings settings("fix8","logviewer");
    autoSaveOn = (bool) settings.value("AutoSave",false).toBool();
    GUI::Globals::timeFormat  = (GUI::Globals::TimeFormat) settings.value("StartTimeFormat",GUI::Globals::HHMMSS).toInt();
    emit notifyTimeFormatChanged(GUI::Globals::timeFormat);
}
void Fix8Log::writeSettings()
{
    QSettings settings("fix8","logviewer");
}
