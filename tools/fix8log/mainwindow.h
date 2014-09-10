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
#include <QtScript>
#include "fixtable.h"
#include "globals.h"
#include "pushbuttonmodifykey.h"
#include "searchfunction.h"
#include "windowdata.h"
#include "worksheet.h"
#include "worksheetdata.h"
#include <QList>
#include <QUuid>
class ComboBoxLineEdit;
class Database;
class EditHighLighter;
class Fix8SharedLib;
class FixMimeData;
class FixToolBar;
class LineEdit;
class WorkSheetModel;
class NoDataLabel;
class SearchLineEdit;
class SearchFunctionList;
class TableSchema;
class TableSchemaList;
class QAction;
class QActionGroup;
class QCompleter;
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
    MainWindow(Database *database,bool showLoading=false);
    MainWindow(MainWindow & sibling,Database *database,bool copyAll = false);
    ~MainWindow();
    void addNewSchema(TableSchema *);
    void addWorkSheet(WorkSheetModel *model,WorkSheetData &wsd);
    void addWorkSheet(WorkSheetData &wsd);
    void deletedSchema(int schemaID);
    void displayMessageDialog(QString &message);
    void finishDrop(WorkSheetData &wsd, FixMimeData *);
    void loadFile(QString &fileName);
    void populateFilterList(SearchFunctionList *sfl);
    void populateSearchList(SearchFunctionList *sfl);
    void setFilterFunctions(SearchFunctionList *sfl);

    void setFieldUsePair(QList<QPair<QString ,FieldUse *>> *);
    void setSharedLibrary(Fix8SharedLib *);
    Fix8SharedLib *getSharedLibrary();
    QList<QPair<QString ,FieldUse *>> * getFieldUsePair();
    //void setSearchColumnNames(QStringList columnNames);
    void setSearchFunctions(SearchFunctionList *sfl);
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
    void setWindowData(const WindowData wd);
    void setTableSchema(TableSchema *);
    void tableSchemaModified(TableSchema *);
    void updateFilterFunctions(SearchFunctionList *sfl);

    void updateSearchFunctions(SearchFunctionList *);
    protected slots:
    void autoSaveOnSlot(bool);
    void cancelSessionRestoreSlot();
    void cancelTabNameSlot();
    void closeSlot();
    void currentColorChangedSlot(QColor);
    void hideColumnActionSlot(QAction *);
    void createWindowSlot();
    void configSlot();
    void configFGSlot();
    void copyTabSlot();
    void copyWindowSlot();
    void createTabSlot();
    void displayMessageSlot(GUI::ConsoleMessage);
    void doPopupSlot(const QModelIndex &,const  QPoint &);
    void exportSlot(QAction *);
    void editSchemaSlot();
    void editTabNameSlot(bool isOn);
    void filterToolbarVisibleSlot(bool);
    void fileDirChangedSlot(const QString &);
    void fileFilterSelectedSlot(QString);
    void fileSelectionFinishedSlot(int returnCode);
    void filterFunctionSelectedSlot(int);
    void filterModeChangedSlot(int);
    void filterReturnSlot();
    void filterTextChangedSlot();
    void iconStyleSlot(QAction *);
    void iconSizeSlot(QAction *);
    void linkSearchSlot(bool turnedOn);
    void modelDroppedSlot(FixMimeData *);
    void popupMenuActionSlot(QAction *);
    void popupMenuSlot(const QModelIndex &,const QPoint &);
    void rowSelectedSlot(int);
    void quitSlot();
    void saveFilterStringSlot();
    void saveSearchStringSlot();
    void schemaSelectedSlot(QAction *);
    void searchActionSlot(QAction *);
    void searchReturnSlot();
    void setFontSlot(QAction *fontAction);
    // time format travels up from work sheet
    void setTimeSlotFromWorkSheet(GUI::Globals::TimeFormat);
    void setTimeFormatSlot(GUI::Globals::TimeFormat);
    void setWindowNameSlot();
    void searchFunctionSelectedSlot(int);
    void searchTextChangedSlot();
    void searchToolbarVisibleSlot(bool);
    void setColorSlot(QColor color);
    void showMessageArea(bool);
    void tabCloseRequestSlot(int);
    void tabCurentChangedSlot(int);
    void tabNameModifiedSlot(QString);
    void tabNameReturnKeySlot();
    void terminatedWorkSheetCopySlot(WorkSheet *);
    void toolbarOrientationChangedSlot(Qt::Orientation);
protected:
    enum {ShowNoDataLabel,ShowTab,ShowProgress};
    void validateSearchButtons();
    void validateSearchButtons(quint32 searchStatus, WorkSheet *ws);
    QAction  *aboutA;
    QAction  *aboutQTA;
    QAction  *autoSaveA;
    QAction  *closeA;
    QAction  *copyWindowA;
    QAction  *exportCSVA;
    QAction  *exportXLSXA;
    QAction  *editFilterA;
    QAction  *editSchemaA;
    QAction  *fontIncreaseA;
    QAction  *fontDecreaseA;
    QAction  *fontRegularA;
    QAction  *filterOnA;
    QAction  *filterSenderMenuA;
    QAction  *hideConsoleA;
    QAction  *hideToolBarA;
    QAction  *hideSearchToolBarA;
     QAction  *filterToolBarA;
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
   // QAction  *linkSearchA; ad latter
    QAction  *pasteTabA;
    QAction  *popupCopyTextA;
    QAction  *popupCopyHtmlA;
    QAction  *quitA;
    QAction  *saveA;
    QAction *saveFilterFuncA;
    QAction *saveSearchFuncA;

    QAction  *searchBackA;
    QAction  *searchBeginA;
    QAction  *searchEndA;
    QAction  *searchNextA;
    QAction  *searchEditA;
    QAction  *searchToolBarA;
    QAction  *showMessageA;
    QAction  *whatsThisA;
    QAction  *windowNameA;
    QActionGroup *exportActionGroup;
    QActionGroup *fontActionGroup;
    QActionGroup *popupActionGroup;
    QActionGroup *schemaActionGroup;
    QActionGroup *schemaScopeGroup;
    QActionGroup *hideColActionGroup;
    QActionGroup *iconSizeActionGroup;
    QActionGroup *iconsStyleGroup;
    QButtonGroup *filterButtonGroup;
    QByteArray messageSplitterSettings;
    QString    menuBarStyleSheet;
    LineEdit *filterLineEdit;
    LineEdit *searchLineEdit;
    QDockWidget *consoleDock;
    QFileDialog *fileDialog;
    QLabel   *filterL;
    QLabel   *filterSelectL;
    QLabel   *scopeV;
    QLabel   *schemaL;
    QLabel   *schemaV;
    QLabel   *searchL;
    QLabel   *searchLV; // searchLagle to show when vertical
    NoDataLabel   *noDataL;
    QLineEdit *tabNameLineEdit;
    QMap     <int,QAction *> schemaActionMap;
    QMenu    *exportMenu;
    QMenu    *fileMenu;
    QMenu    *windowMenu;
    QMenu    *hideColumMenu;
    QMenu    *configureIconsMenu;
    QMenu    *iconSizeMenu;
    QMenu    *iconStyleMenu;
    QMenu    *helpMenu;
    QMenu    *optionMenu;
    QMenu    *schemaMenu;
    QMenu    *schemaScopeMenu;
    QMenu    *popupMenu;
    QMenuBar *mainMenuBar;
    PushButtonModifyKey *configPB;
    QPushButton *cancelEditTabNamePB;
    QPushButton *editTabNamePB;

    QQuickItem  *qmlObject;
    QQuickView *progressView;
    QRadioButton *includeFilterB;
    QRadioButton *excludeFilterB;
    QRadioButton *offFilterB;
    QSplitter *workAreaSplitter;
    QStackedWidget *stackW;
    QStandardItemModel *_model;
    QTabWidget *tabW;
    QTextBrowser *consoleArea;
    //QToggleButton editTabNamePB;
    QToolBar *mainToolBar;
    FixToolBar *searchToolBar;
    QToolBar   *filterToolBar;

    QList <FixTable *> fixTableLists;
    QWidget *progressWidget;
    QWidget *filterArea;
    QWidget *searchArea;
    QWidget *tabNameEditArea;
    QWhatsThis *whatsThis;
    void buildMainWindow();
    void closeEvent(QCloseEvent *);
    void displayConsoleMessage(GUI::ConsoleMessage);
    void readSettings();
    void setAutoSaveOn(bool);
    void showEvent(QShowEvent *);
    QSize sizeHint() const;
    void timerEvent(QTimerEvent *);
    void writeSettings();
signals:
    void autoSaveOn(bool);
    void cancelSessionRestore();
    void createWindow(MainWindow *);
    void copyWindow(MainWindow *);
    void deleteWindow(MainWindow *);
    void editSchema(MainWindow *);
    void showFilterDialog();
    void showFilterDialogAddMode(QString searchFunction);
    void showSearchDialog();
    void showSearchDialogAddMode(QString searchFunction);
    void tableSchemaChanged(TableSchema *);
    void exitApp();
    void modelDropped(FixMimeData *);
    void notifyTimeFormatChanged(GUI::Globals::TimeFormat);
    void toolButtonStyleModified(Qt::ToolButtonStyle);
private:
    void buildHideColumnMenu();
    void buildSchemaMenu();
    void exportAsCSV(QString fileName,const WorkSheet *ws);
    void exportAsXLSXA(QString fileName,WorkSheet *ws);

    SearchFunction createRoutine(bool &bstatus, bool isSearch=true);
    bool runFilterScript();
    bool runSearchScript();
    void setSearchFunction(const SearchFunction &);
    void validateFilterText();
    void validateSearchText();
    QByteArray fileDirState;
    QString  lastSelectedDir;
    QString fileFilter;
    int windowDataID;
    bool loadingActive;
    QUuid  uuid;
    QString name;
    TableSchema *tableSchema;
    WorkSheetList workSheetList;
    QStringList searchColumnNames;
    EditHighLighter *editHighlighter;
    EditHighLighter *filterEditHighlighter;

    SearchFunction filterFunction;
    SearchFunction searchFunction;
    bool    haveFilterFunction;
    bool    haveSearchFunction;
    QScriptValue filterFunctionVal;
    QScriptValue searchFunctionVal;
    QActionGroup *searchActionGroup;
    QScriptEngine engine;
    QCompleter *filterCompleter;
    QCompleter *searchCompleter;
    QComboBox  *filterSelectCB;
    QComboBox  *searchSelectCB;
    ComboBoxLineEdit *filterSelectLineEdit;
    ComboBoxLineEdit *searchSelectLineEdit;
    QLabel     *searchSelectL;
    QMap <QString, qint32 > filterFunctionMap; //< function, indexofComboBox>
    QMap <QString, qint32 > searchFunctionMap; //< function, indexofComboBox>
    QAbstractItemModel *searchSelectModel;
    SearchFunctionList filterFunctionList;
    SearchFunctionList searchFunctionList;
    Database *database;
    bool colorSelectionFG;
    bool linkSearchOn;
    QColor menuBG;
    QColor menuFG;
    QString menuStyle;
    QLabel  *fix8versionL;
    QLabel  *fix8versionV;
    QList<QPair<QString ,FieldUse *>> *fieldUsePairList;
    QStringList filterArgList;
    QStringList searchArgList;
    Fix8SharedLib *sharedLib;
    QColor fix8RegColor;
    TableSchema *defaultTableSchema;
    TableSchemaList *schemaList;
};

#endif // MAINWINDOW_H
