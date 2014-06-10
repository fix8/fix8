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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QQuickItem>
#include "fixtable.h"
#include "globals.h"
#include "windowdata.h"
#include "worksheet.h"

#include "worksheetdata.h"
#include <QList>
#include <QUuid>
class FixMimeData;
class FixToolBar;
class WorkSheetModel;
class NoDataLabel;
class SearchLineEdit;
class TableSchema;
class TableSchemaList;
class QAction;
class QActionGroup;
class QDockWidget;
class QFileDialog;
class QLabel;
class QLineEdit;
class QMenu;
class QMenuBar;
class QPushButton;
class QQuickView;
class QSplitter;
class QStackedWidget;
class QStandardItemModel;
class QTabWidget;
class QTextBrowser;
class QTogleButton;
class QToolBar;
class QWhatsThis;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    friend class Fix8Log;
public:
    MainWindow(bool showLoading=false);
    MainWindow(MainWindow & sibling,bool copyAll = false);
    ~MainWindow();
    void addNewSchema(TableSchema *);
    void addWorkSheet(WorkSheetModel *model,WorkSheetData &wsd);
    void deletedSchema(int schemaID);
    void displayMessageDialog(QString &message);
    void finishDrop(WorkSheetData &wsd, FixMimeData *);
    static void setTableSchemaList(TableSchemaList *);
    void showFileDialog();
    QString getName();
    TableSchema* getTableSchema();
    const QUuid &getUuid();
    WindowData getWindowData();
    QList <WorkSheetData> getWorksheetData(int windowID);
    WorkSheetData getWorksheetData(QUuid &workSheetID,bool *ok);
    void setCurrentTabAndSelectedRow(int currentTab, int selectedRow);
    void setLoading(bool);
    void setLoadMessage(QString);
    void setWindowData(const WindowData &wd);
    void setTableSchema(TableSchema *);
    void tableSchemaModified(TableSchema *);
    protected slots:
    void autoSaveOnSlot(bool);
    void cancelSessionRestoreSlot();
    void cancelTabNameSlot();
    void closeSlot();
    void currentColorChangedSlot(QColor);
    void hideColumnActionSlot(QAction *);
    void createWindowSlot();
    void configSlot();
    void copyTabSlot();
    void copyWindowSlot();
    void createTabSlot();
    void editSchemaSlot();
    void editTabNameSlot(bool isOn);
    void fileDirChangedSlot(const QString &);
    void fileFilterSelectedSlot(QString);
    void fileSelectionFinishedSlot(int returnCode);
    void iconStyleSlot(QAction *);
    void iconSizeSlot(QAction *);
    void modelDroppedSlot(FixMimeData *);
    void popupMenuSlot(const QModelIndex &,const QPoint &);
    void quitSlot();
    void schemaSelectedSlot(QAction *);
    // time format travels up from work sheet
    void setTimeSlotFromWorkSheet(GUI::Globals::TimeFormat);
    void setTimeFormatSlot(GUI::Globals::TimeFormat);
    void setWindowNameSlot();
    QSize sizeHint() const;
    void setColorSlot(QColor color);
    void showMessageArea(bool);
    void tabCloseRequestSlot(int);
    void tabCurentChangedSlot(int);
    void tabNameModifiedSlot(QString);
    void tabNameReturnKeySlot();
    void toolbarOrientationChangedSlot(Qt::Orientation);
protected:
    enum {ShowNoDataLabel,ShowTab,ShowProgress};
    QAction  *aboutA;
    QAction  *aboutQTA;
    QAction  *autoSaveA;
    QAction  *closeA;
    QAction  *copyWindowA;
    QAction  *editSchemaA;
    QAction  *filterOnA;
    QAction  *hideConsoleA;
    QAction  *hideToolBarA;
    QAction  *hideSearchToolBarA;
    QAction  *iconSizeSmallA;
    QAction  *iconSizeRegA;
    QAction  *iconSizeLargeA;
    QAction  *iconsOnlyA;
    QAction  *iconsWithTextA;
    QAction  *iconsTextOnlyA;
    QAction  *newWindowA;
    QAction  *newTabA;
    QAction  *copyTabA;
    QAction  *cutTabA;
    QAction  *pasteTabA;
    QAction  *quitA;
    QAction  *saveA;
    QAction  *searchBackA;
    QAction  *searchBeginA;
    QAction  *searchEndA;
    QAction  *searchNextA;
    QAction  *searchEditA;
    QAction  *showMessageA;
    QAction  *whatsThisA;
    QAction  *windowNameA;
    QActionGroup *schemaActionGroup;
    QActionGroup *schemaScopeGroup;
    QActionGroup *hideColActionGroup;
    QActionGroup *iconSizeActionGroup;
    QActionGroup *iconsStyleGroup;
    QByteArray messageSplitterSettings;
    QColor menubarColor;
    SearchLineEdit *searchLineEdit;
    QDockWidget *consoleDock;
    QFileDialog *fileDialog;
    QLabel   *scopeV;
    QLabel   *schemaL;
    QLabel   *schemaV;
    QLabel   *searchL;
    QLabel   *searchLV; // searchLagle to show when vertical
    NoDataLabel   *noDataL;
    QLineEdit *tabNameLineEdit;
    QMap     <int,QAction *> schemaActionMap;
    QMenu    *fileMenu;
    QMenu    *hideColumMenu;
    QMenu    *configureIconsMenu;
    QMenu    *iconSizeMenu;
    QMenu    *iconStyleMenu;
    QMenu    *helpMenu;
    QMenu    *optionMenu;
    QMenu    *schemaMenu;
    QMenu    *schemaScopeMenu;
    QMenu    *poupMenu;
    QMenuBar *mainMenuBar;
    QPushButton *cancelEditTabNamePB;
    QPushButton *configPB;
    QPushButton *editTabNamePB;
    QQuickItem  *qmlObject;
    QQuickView *progressView;
    QSplitter *workAreaSplitter;
    QStackedWidget *stackW;
    QStandardItemModel *_model;
    QTabWidget *tabW;
    QTextBrowser *consoleArea;
    //QToggleButton editTabNamePB;
    QToolBar *mainToolBar;
    FixToolBar *searchToolBar;
    static TableSchema *defaultTableSchema;
    static TableSchemaList *schemaList;
    QList <FixTable *> fixTableLists;
    QWidget *progressWidget;
    QWidget *searchArea;
    QWidget *tabNameEditArea;
    QWhatsThis *whatsThis;
    void buildMainWindow();
    void displayConsoleMessage(GUI::ConsoleMessage);
    void readSettings();
    void setAutoSaveOn(bool);
    void showEvent(QShowEvent *);
    void timerEvent(QTimerEvent *);
    void writeSettings();
signals:
    void autoSaveOn(bool);
    void cancelSessionRestore();
    void createWindow(MainWindow *);
    void copyWindow(MainWindow *);
    void deleteWindow(MainWindow *);
    void editSchema(MainWindow *);
    void tableSchemaChanged(TableSchema *);
    void exitApp();
    void modelDropped(FixMimeData *);
    void notifyTimeFormatChanged(GUI::Globals::TimeFormat);
    void toolButtonStyleModified(Qt::ToolButtonStyle);
private:
    void buildHideColumnMenu();
    void buildSchemaMenu();
    QByteArray fileDirState;
    QString  lastSelectedDir;
    QString fileFilter;
    int windowDataID;
    bool loadingActive;
    QUuid  uuid;
    QString name;
    TableSchema *tableSchema;
    WorkSheetList workSheetList;

};

#endif // MAINWINDOW_H
