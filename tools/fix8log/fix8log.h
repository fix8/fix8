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

#ifndef FIX8LOG_H
#define FIX8LOG_H
#include <QFutureWatcher>
#include <QObject>
#include <QtConcurrent/QtConcurrent>
#include <QMap>
#include <QString>
#include "globals.h"
#include "mainwindow.h"
#include "messagefield.h"
#include "searchfunction.h"
#include "tableschema.h"
#include <qtsingleapplication.h>

#include <fix8/f8includes.hpp>
#include <fix8/field.hpp>
#include <fix8/message.hpp>
#include <fix8/f8types.hpp>
#include "fix8/traits.hpp"

class QStandardItemModel;
class QQuickView;
class Database;
class SchemaEditorDialog;
class FutureReadData;
class FixMimeData;
class NewWindowWizard;
class SearchDialog;
class WorkSheetModel;
FutureReadData * readLogFileInThread(const QString &fileName,QString &errorStr);

class Fix8Log : public QObject
{
    Q_OBJECT
public:
    explicit Fix8Log(QtSingleApplication *);
    bool init();
    bool init(QString fileNameToLoad);
    void readFileInAnotherThread(const QString &fileName,QString &errorStr);
    void readSettings();
    void writeSettings();
public slots:
    void aboutSlot();
    void autoSaveOnSlot(bool);
    void cancelSessionRestoreSlot();
    void createNewWindowSlot(MainWindow *mw=0);
    void copyWindowSlot(MainWindow *mw);
    void deleteMainWindowSlot(MainWindow *mw);
    void displayConsoleMessage(GUI::ConsoleMessage);
    void displayConsoleMessage(QString, GUI::ConsoleMessage::ConsoleMessageType = GUI::ConsoleMessage::InfoMsg);
    void editSchemaSlot(MainWindow *);
    void exitAppSlot();
     void finishedReadingDataFileSlot();
    void lastWindowClosedSlot();
    void modelDroppedSlot(FixMimeData *);
    void newSchemaCreatedSlot(TableSchema *);
    void newMainWindowWizardSlot();
    void setTimeFormatSlot(GUI::Globals::TimeFormat);
    void schemaDeletedSlot(int schemaID);
    void schemaEditorFinishedSlot(int);
    void schemaModifiedSlot(TableSchema *,bool nameAndOrDescriptionOnly);
    void searchDialogAcceptedSlot();
    void showSearchDialogSlot();
    void showSearchDialogAddModeSlot(QString);
    void tableSchemaSelectedSlot(TableSchema *);
    void toolButtonStyleModfiedSlot(Qt::ToolButtonStyle);
    void updatedSearchFunctionsSlot(SearchFunctionList *sfl);
    void wakeupSlot(const QString&);
protected:
    WorkSheetModel *readLogFile(const QString &fileName,QString &errorStr);
    void saveSession();
    void wireSignalAndSlots(MainWindow *mw);
    QList <MainWindow *> mainWindows;
    bool firstTimeToUse;
    Database *database;
    bool autoSaveOn;
    QMap <QString, QStandardItemModel *> fileNameModelMap;
    bool cancelSessionRestore;
    SchemaEditorDialog *schemaEditorDialog;
    TableSchemaList *tableSchemaList;
    TableSchema *defaultTableSchema;
    TableSchema *worldTableSchema;
    MessageFieldList *messageFieldList;
    FieldTraitVector fieldTraitV;
    QList<QPair<QString ,FieldUse *>> fieldUsePairList;
    QMap<QString, QBaseEntry *> baseMap;
    QMultiMap <QString,FieldTrait *> fieldsInUseMap;
    FieldUseList fieldUseList;
    QStringList defaultHeaderStrs;
    QBaseEntryList defaultHeaderItems;
    QtSingleApplication *applicationInstance;
signals:
    void notifyTimeFormatChanged(GUI::Globals::TimeFormat);
private:
    void cleanWindowDataList(QList <WindowData> &windowDataList);

    void generate_traits(const TraitHelper &tr, QMap <QString, QBaseEntry *> &baseMap,FieldUseList &ful,
                      MessageField *mf,QList <QBaseEntry *> *qbaseEntryList, int *level);
    void generate_traits(const TraitHelper &tr,QMap <QString, QBaseEntry *> &baseMap,FieldUseList &ful,
                      MessageField *mf,QBaseEntryList *qbaseEntryList,int *level);
    void initDatabase();
    std::function<const F8MetaCntx&()> loadFix8so(QString libName, bool &bstatus);
    SearchDialog *searchDialog;
    SearchFunctionList *searchFunctionList;
    NewWindowWizard *newWindowWizard;

};

#endif // FIX8LOG_H
