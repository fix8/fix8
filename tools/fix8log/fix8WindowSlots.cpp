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
#include "searchDialog.h"
#include "worksheetmodel.h"
#include "windowdata.h"
#include <QApplication>
#include <QDebug>
#include <QDialog>
#include <QQuickView>
#include <QQmlContext>
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
void Fix8Log::copyWindowSlot(MainWindow *mw)
{
    MainWindow *newMW  =new MainWindow(*mw,database,true);
    newMW->setFieldUsePair(&fieldUsePairList);
    newMW->setSearchFunctions(searchFunctionList);
    wireSignalAndSlots(newMW);
    newMW->show();
    mainWindows.append(newMW);
}
void Fix8Log::createNewWindowSlot(MainWindow *mw)
{

    Fix8SharedLib *f8lib;
    QString str;
    newWindowWizard = new NewWindowWizard(fix8ShareLibList);
    int status = newWindowWizard->exec();
    newWindowWizard->saveSettings();
    if (status == QDialog::Accepted	) {
        QString fileName = newWindowWizard->getSelectedFile();
        QString libName = newWindowWizard->getSelectedLib();
        //qDebug() << "Selected File = " << fileName << __FILE__ << __LINE__;
        // qDebug() << "Selected Lib = " << libName << __FILE__ << __LINE__;
        f8lib =  fix8ShareLibList.findByFileName(libName);
        if (!f8lib) {
            f8lib = Fix8SharedLib::create(libName);
            if (!f8lib) {
                str = "Error - failed to load FIX8 sharelib: " + libName;
                QMessageBox::warning(0,Globals::appName,str);
                newWindowWizard->deleteLater();
                return;
            }
            if (!f8lib->isOK) {
                str = "Error - " + f8lib->errorMessage;
                QMessageBox::warning(0,Globals::appName,str);
                delete f8lib;
                newWindowWizard->deleteLater();
                return;
            }
        }
        MainWindow *newMW  =new MainWindow(*mw,database);
        newMW->setSearchFunctions(searchFunctionList);
        wireSignalAndSlots(newMW);
        newMW->setSharedLibrary(f8lib);
        newMW->setFieldUsePair(&(f8lib->fieldUsePairList));
        newMW->setAutoSaveOn(autoSaveOn);
        newMW->setWindowData(mw->getWindowData());

        newMW->show();
        newMW->loadFile(fileName);
        mainWindows.append(newMW);
    }
    newWindowWizard->saveSettings();
    newWindowWizard->deleteLater();
    /*

    newWindowWizard = new QQuickView(QUrl("qrc:qml/newMainWindow.qml"));
    QQmlContext *context = newWindowWizard->rootContext();

    newWinWizObject = newWindowWizard->rootObject();
    QObject *schemaPage = newWinWizObject->findChild<QObject*>("schemaPage");
    QObject *filePage = newWinWizObject->findChild<QObject*>("filePage");
    newWindowWizard->setResizeMode(QQuickView::SizeRootObjectToView);
    newWindowDialog = new QDialog(0);
    schemaPage->setProperty("color", newWindowDialog->palette().color(QPalette::Window));
    //filePage->setProperty("color", newWindowDialog->palette().color(QPalette::Window));

    QFont fnt = newWindowDialog->font();

    schemaPage->setProperty("fontPointSize",fnt.pointSize()+2);
    //filePage->setProperty("fontPointSize",fnt.pointSize()+2);
    newWindowWidget = QWidget::createWindowContainer(newWindowWizard,newWindowDialog);
   // newWindowWizard->setProperty("backgroundColor", newWindowDialog->palette().color(QPalette::Window));

    QVBoxLayout *vbox = new QVBoxLayout(newWindowDialog);
    newWindowDialog->setLayout(vbox);
    vbox->addWidget(newWindowWidget);
    newWindowDialog->resize(640,480);
    newWindowDialog->exec();
    */



    /*
    MainWindow *newMW  =new MainWindow(*mw,database);
    newMW->setSearchFunctions(searchFunctionList);
    wireSignalAndSlots(newMW);
    newMW->show();
    newMW->showFileDialog();
    mainWindows.append(newMW);
    */
    //const GeneratedTable<const char *, BaseMsgEntry>::Pair *p = TEX::ctx()._bme.at(1);

}
void Fix8Log::deleteMainWindowSlot(MainWindow *mw)
{
    qDebug() << "DELETE MAIN WINDOW SLOT, window count = "<< mainWindows.count() << __FILE__ << __LINE__;
    if (mainWindows.count() == 1)  {

        if (autoSaveOn) {
            qDebug() << "\tSavie Session....";
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
       if (schemaEditorDialog)
            schemaEditorDialog->windowDeleted(mw);
        writeSettings();
        qApp->exit();
    }

}
void Fix8Log::lastWindowClosedSlot()
{
    qDebug() << "Last Window closed";
    qApp->exit();
}

void Fix8Log::wireSignalAndSlots(MainWindow *mw)
{
    if (!mw) {
        qWarning() << "Error - wire signals and slots, window is null" << __FILE__ << __LINE__;
        return;
    }
    connect(mw,SIGNAL(toolButtonStyleModified(Qt::ToolButtonStyle)),
            this,SLOT(toolButtonStyleModfiedSlot(Qt::ToolButtonStyle)));
    connect(mw,SIGNAL(createWindow(MainWindow*)),this,SLOT(createNewWindowSlot(MainWindow*)));
    connect(mw,SIGNAL(copyWindow(MainWindow*)),this,SLOT(copyWindowSlot(MainWindow*)));
    connect(mw,SIGNAL(deleteWindow(MainWindow*)),this,SLOT(deleteMainWindowSlot(MainWindow*)));
    connect(mw,SIGNAL(exitApp()),this,SLOT(exitAppSlot()));
    connect(mw,SIGNAL(autoSaveOn(bool)),this,SLOT(autoSaveOnSlot(bool)));
    connect(mw,SIGNAL(cancelSessionRestore()),this,SLOT(cancelSessionRestoreSlot()));
    connect(mw,SIGNAL(notifyTimeFormatChanged(GUI::Globals::TimeFormat)),
            this,SLOT(setTimeFormatSlot(GUI::Globals::TimeFormat)));
    connect(this,SIGNAL(notifyTimeFormatChanged(GUI::Globals::TimeFormat)),
            mw,SLOT(setTimeFormatSlot(GUI::Globals::TimeFormat)));
    connect(mw,SIGNAL(modelDropped(FixMimeData*)),this,SLOT(modelDroppedSlot(FixMimeData*)));
    connect(mw,SIGNAL(editSchema(MainWindow*)),this,SLOT(editSchemaSlot(MainWindow  *)));
    connect(mw,SIGNAL(tableSchemaChanged(TableSchema*)),this,SLOT(tableSchemaSelectedSlot(TableSchema *)));
    connect(mw,SIGNAL(showSearchDialog()),this,SLOT(showSearchDialogSlot()));
    connect(mw,SIGNAL(showSearchDialogAddMode(QString)),this,SLOT(showSearchDialogAddModeSlot(QString)));
    connect(mw,SIGNAL(showFilterDialog()),this,SLOT(showFilterDialogSlot()));
    connect(mw,SIGNAL(showFilterDialogAddMode(QString)),this,SLOT(showFilterDialogAddModeSlot(QString)));
    connect(mw->aboutA,SIGNAL(triggered()),this,SLOT(aboutSlot()));
    mw->setAutoSaveOn(autoSaveOn);
}
void Fix8Log::newMainWindowWizardSlot()
{
    qDebug() << "REMOVE THIS SLOT" << __FILE__ << __LINE__;
    /* newWindowWizard = new QQuickView(QUrl("qrc:qml/newMainWindow.qml"));
    newWinWizObject = newWindowWizard->rootObject();
    if (!newWinWizObject) {
        qWarning() << "qml root object not found for New Window  Wizard" << __FILE__ << __LINE__ ;
        return;
    }
    //else
    //    connect(newWinWizObject,SIGNAL(cancel()),this,SLOT(cancelSessionRestoreSlot()));
    */
}
