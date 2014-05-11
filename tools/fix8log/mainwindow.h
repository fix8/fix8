#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QQuickItem>
#include "fixtable.h"
#include "globals.h"
#include "windowdata.h"
#include "worksheetdata.h"
#include <QList>
#include <QUuid>
class FixMimeData;
class FixToolBar;
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
class MainWindow : public QMainWindow
{
    Q_OBJECT
    friend class Fix8Log;
public:
    MainWindow(bool showLoading=false);
    MainWindow(const MainWindow & sibling,bool copyAll = false);
    ~MainWindow();
    void addNewSchema(TableSchema *);
    void addWorkSheet(QStandardItemModel *model,WorkSheetData &wsd);
    void finishDrop(WorkSheetData &wsd, FixMimeData *);
    void showFileDialog();
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
    QAction  *schemaApplyTabA;
    QAction  *schemaApplyWindowA;
    QAction  *schemaApplyAllA;
    QAction  *searchBackA;
    QAction  *searchBeginA;
    QAction  *searchEndA;
    QAction  *searchNextA;
    QAction  *searchEditA;
    QAction  *showMessageA;
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
    QLabel   *schemaL;
    QLabel   *schemaV;
    QLabel   *searchL;
    QLabel   *searchLV; // searchLagle to show when vertical
    NoDataLabel   *noDataL;
    QLineEdit *tabNameLineEdit;
    QMenu    *fileMenu;
    QMenu    *hideColumMenu;
    QMenu    *configureIconsMenu;
    QMenu    *iconSizeMenu;
    QMenu    *iconStyleMenu;
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
    void setAutoSaveOn(bool);
    void buildMainWindow();
    void displayConsoleMessage(GUI::Message);
    void showEvent(QShowEvent *);
    void readSettings();
    void writeSettings();
signals:
    void autoSaveOn(bool);
    void cancelSessionRestore();
    void createWindow(MainWindow *);
    void copyWindow(MainWindow *);
    void deleteWindow(MainWindow *);
    void editSchema(MainWindow *, QUuid uuid);
    void exitApp();
    void modelDropped(FixMimeData *);
    void notifyTimeFormatChanged(GUI::Globals::TimeFormat);
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
};

#endif // MAINWINDOW_H
